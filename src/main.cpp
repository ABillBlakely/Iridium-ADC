#include "main.h"

Serial usb_serial(USBTX, USBRX);
char serial_rx_char;

int main()
{
    ADC_Class adc;
    UnitTests tests;

    printf("Dec %d\n", 1<<DECIMATION_RATE);
    adc.setup();

    while(1)
    {
        switch (usb_serial.putc(usb_serial.getc()))
        {
            case 'R':
            case 'r':
            {
                // Receive samples
                printf("\n");
                adc.start_sampling();
                break;
            }
            case 'S':
            case 's':
            {
                // Setup
                adc.clear_terminal();
                adc.setup();
                break;
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
