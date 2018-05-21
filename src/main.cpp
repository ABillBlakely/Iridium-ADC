#include "main.h"

int main()
{
    usb_serial.baud(2000000);

    // attach the serial interrupt. These signals are handled in communications.cpp by the control signals function.
    usb_serial.attach(&control_signals);
    while(1)
    {

        if(data_ready == 1)
        {
            // detach the serial interrupt. This is reattached when the master sends the start sampling command.
            // usb_serial.attach(0);
            // Send the data array.
            data_tx(sample_array);
            // Reset the flag
            data_ready = 0;
        }
    }
    printf("ERROR: main loop ended somehow.\n");
}
