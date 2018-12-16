from tkinter import *
from usbcom import USBCom

class App:
    def __init__(self, root):
        # GUI
        frame = Frame(root)
        frame.pack(fill=BOTH, expand=True)
        self.txt_out = Text(frame, state=DISABLED)
        self.txt_out.tag_config("in", foreground="red")
        self.txt_out.pack(fill=BOTH, expand=True)
        self.txt_in = Entry(frame)
        self.txt_in.bind("<Key>", self.key)
        self.txt_in.pack(fill=X)
        root.update()
        root.minsize(width=root.winfo_width(), height=root.winfo_height())

        # USBCom
        self.usbcom = USBCom()
        if not self.usbcom.connect(self.on_read):
            print(self.usbcom.connection_error_message)

    def key(self, event):
        if event.char == '\r':
            str_in = self.txt_in.get() + "\n"
            self.txt_out.config(state=NORMAL)
            self.txt_out.insert(END, str_in, "in")
            self.txt_out.config(state=DISABLED)
            self.txt_in.delete(0, END)
            self.usbcom.write(str_in)

    def on_read(self, data):
        self.txt_out.config(state=NORMAL)
        data_str = ""
        for b in data:
            data_str += chr(b)
        self.txt_out.insert(END, data_str)
        self.txt_out.config(state=DISABLED)
        self.txt_out.see(END)

if __name__ == '__main__':
    root = Tk()
    app = App(root)
    root.mainloop()
    app.usbcom.disconnect()