#include <iostream>
#include <stdlib.h>
#include <signal.h>
#include "USBCom.h"


// Simple program to receive data using the USBCom class and print it
// on the standard output.


// Handler called to stop listening when Ctrl+C is pressed (SIGINT is sent)
volatile bool _exit = false;
void sigIntHandler(int signum) {
    _exit = true;
}

int main(int argc, char** argv) {
    USBCom usbcom;
    usbcom.init();

    // Register the SIGINT handler
    struct sigaction sigIntAction;
    sigIntAction.sa_handler = sigIntHandler;
    sigemptyset(&sigIntAction.sa_mask);
    sigIntAction.sa_flags = 0;
    sigaction(SIGINT, &sigIntAction, NULL);

    // Read and print data until _exit is set
    while (!_exit) {
        // Read a string on stdin
        std::string str;
        std::cin >> str;
        str += "\n";

        // Send to USBCom
        usbcom.write((const uint8_t*)str.c_str(), str.size());
    }

    // Close the USB connection
    usbcom.close();
}
