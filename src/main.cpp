#include "main.h"

int main()
{
    // usb_serial.baud(115200);
    usb_serial.baud(1500000);

    // attach the serial interrupt. These signals are handled in
    // communications.cpp by the control signals function.
    usb_serial.attach(&control_signals);
    while(1)
    {

        if(data_ready == 1)
        {
            // Send the data array.
            data_tx();
            // Reset the flag
            data_ready = 0;
        }
    }
    printf("ERROR: main loop ended somehow.\n");
}
