#include "communications.h"

Serial usb_serial(USBTX, USBRX);
// usb_serial.set_flow_control(CTS, CTS);
volatile int abort_transfer = 0;

void control_signals()
{
    switch (usb_serial.getc())
    {
        case 'P':
        case 'p':
        {
            // prepare the adc.
            adc.setup();
            break;
        }
        case 'R':
        case 'r':
        {
            // Check status register
            // printf("%d samples, %dx decimation\n", NUMBER_OF_SAMPLES, 1<<DECIMATION_RATE);
            adc.read_status_register(true);
            break;
        }
        case 'S':
        case 's':
        {
            // Receive samples
            abort_transfer = 0;
            adc.start_sampling();
            break;
        }
        case 'T':
        case 't':
        {
            // Stop sampling
            abort_transfer = 1;
            adc.stop_sampling();
            break;
        }
        case 'D':
        case 'd':
        {
            // Change decimation rate
            switch (usb_serial.getc())
            {
                case '0': {adc.change_decimation_rate(0); break;}
                case '1': {adc.change_decimation_rate(1); break;}
                case '2': {adc.change_decimation_rate(2); break;}
                case '3': {adc.change_decimation_rate(3); break;}
                case '4': {adc.change_decimation_rate(4); break;}
                case '5': //fall through to default
                default : {adc.change_decimation_rate(5);}
            }
        }
        default:
        {}
    }
}

void data_tx(volatile uint32_t data_packet[])
{
    uint32_t last_sent = 0xDEADBEEF;

    printf("start\n");

    for(int tx_index = 0; tx_index < SAMPLES_PER_PAGE; tx_index++)
    {
        switch (usb_serial.getc())
        {
            case 'N':
            case 'n':
            {
                last_sent = data_packet[tx_index];
                break;
            }
            case 'B':
            case 'b':
            {
                // resend  the last sent from before, decrement the counter send
                tx_index--;
                break;
            }
            default:
            // Invalid control signal received
            {
                tx_index--;
                last_sent = 0xDEADBEEF;
            }
        }
        printf("%08lx\n", last_sent);
    }

    printf("stop\n");

}
