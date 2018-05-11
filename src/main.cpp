#include "main.h"

Serial usb_serial(USBTX, USBRX);
char serial_rx_char;

int main()
{
    usb_serial.baud(115200);

    ADC_Class adc;
    // UnitTests tests;
    adc.clear_terminal();
    printf("%d samples, %dx decimation\n", NUMBER_OF_SAMPLES, 1<<DECIMATION_RATE);
    adc.setup();


    while(1)
    {
        switch (usb_serial.getc())
        {
            case 'R':
            case 'r':
            {
                // Check status register
                adc.clear_terminal();
                printf("%d samples, %dx decimation\n", NUMBER_OF_SAMPLES, 1<<DECIMATION_RATE);
                adc.read_status_register(true);
                break;
            }
            case 'S':
            case 's':
            {
                // Receive samples
                adc.clear_terminal();
                adc.start_sampling();
                break;
            }
            case 'T':
            case 't':
            {
                // Stop sampling
            }
            case 'C':
            case 'c':
            {
                // Clear
                adc.clear_terminal();
                break;
            }
            default:
            {}
        }
    }
    printf("while loop ended somehow.\n");

}
