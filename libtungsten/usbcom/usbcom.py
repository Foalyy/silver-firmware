import usb.core, usb.util
import threading

class USBCom:
    OUT = 0
    IN = 1

    REQ_BOOTLOADER = 0x00
    REQ_CONNECT = 0x01
    REQ_DISCONNECT = 0x02

    def __init__(self):
        self.connected = False
        self.connection_error = False
        self.connection_error_message = ""

        self._dev = None
        self._ep_in = None
        self._ep_out = None

        self._read_handler = None
        self._thread_usb = None

    def connect(self, read_handler):
        # Reset state
        if self.connected and not self.connection_error:
            self.disconnect()
        self.connected = False
        self.connection_error = False
        self.connection_error_message = ""

        # Look for an USB device with matching ids
        self._dev = usb.core.find(idVendor=0x03eb, idProduct=0xcabd);
        if self._dev is None:
            self.connection_error = True
            self.connection_error_message = "Device not found"
            for listener in self.listeners:
                listener._on_connection_status(False, self.connection_error_message)
            return False

        # Configure the device
        self._dev.set_configuration()
        cfg = self._dev.get_active_configuration()
        intf = cfg[(0, 0)]

        # Find the IN and OUT endpoints
        self._ep_in = usb.util.find_descriptor(intf, custom_match=lambda e: usb.util.endpoint_direction(e.bEndpointAddress) == usb.util.ENDPOINT_IN)
        self._ep_in = usb.util.find_descriptor(intf, custom_match=lambda e: usb.util.endpoint_direction(e.bEndpointAddress) == usb.util.ENDPOINT_IN)
        self._ep_out = usb.util.find_descriptor(intf, custom_match=lambda e: usb.util.endpoint_direction(e.bEndpointAddress) == usb.util.ENDPOINT_OUT)
        assert self._ep_in is not None
        assert self._ep_out is not None

        # Connected
        self.connected = True
        self._send_request_internal(USBCom.REQ_CONNECT)

        # Start the read thread
        self._read_handler = read_handler
        self._thread_usb = threading.Thread(target=self._usb_handler)
        self._thread_usb.daemon = True
        self._thread_usb.start()

        return True

    def disconnect(self):
        if self.connected and not self.connection_error:
            self._send_request_internal(USBCom.REQ_DISCONNECT)
            self.connected = False
            self._thread_usb.join()

    def _send_request_internal(self, request, value=0, index=0, direction=OUT, payload=None):
        if self.connected and not self.connection_error:
            bmRequestType = direction << 7 | 2 << 5
            self._dev.ctrl_transfer(bmRequestType, request, value, index, payload)

    def send_request(self, request, value=0, index=0, direction=OUT, payload=None):
        return self._send_request_internal(request | 0x80, value, index, direction, payload)

    def write(self, data):
        if self.connected and not self.connection_error:
            self._ep_out.write(data)

    def _usb_handler(self):
        while self.connected and not self.connection_error:
            try:
                buff = self._ep_in.read(64, 100)
            except usb.core.USBError as e:
                if e.errno != 110:
                    self.connected = False
                    self.connection_error = True
                    self.connection_error_message = str(e)
                    print("")
                    print("USB error " + str(e.errno) + " : " + e.strerror)
                    return
            else:
                if len(buff) > 0:
                    self._read_handler(buff)