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
    const int BUFFER_SIZE = 64;
    uint8_t buffer[BUFFER_SIZE];
    int transferredLength = 0;
    while (!_exit) {
        // Read
        transferredLength = usbcom.read(buffer, BUFFER_SIZE);

        // Print
        for (int i = 0; i < transferredLength; i++) {
            std::cout << (char)buffer[i];
        }
        std::cout << std::flush;
    }

    // Close the USB connection
    usbcom.close();
}
