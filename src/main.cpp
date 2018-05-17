#include "main.h"

int main()
{
    usb_serial.baud(115200);
    usb_serial.attach(&control_signals);

    while(1)
    {
        if(data_ready == 1)
        {
            data_tx(sample_array);
            data_ready = 0;
            if (!abort_transfer)
            {
                // adc.start_sampling();
            }
        }
    }
    printf("ERROR: main loop ended somehow.\n");


}
