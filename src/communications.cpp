#include "communications.h"

Serial usb_serial(USBTX, USBRX);

volatile int number_of_samples_to_TX = 0;

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
            adc.start_sampling();
            break;
        }
        case 'T':
        case 't':
        {
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
            break;
        }
        case 'L':
        case 'l':
        {

            switch (usb_serial.getc())
            {
                case '3': {number_of_samples_to_TX = 16384; break;}
                case '2': {number_of_samples_to_TX = 8192; break;}
                case '1': {number_of_samples_to_TX = 4096; break;}
                case '0': // fall through to default
                default: {number_of_samples_to_TX = 1024; break;}
            }
            break;
        }

        case 'F':
        case 'f':
        {
            adc.power_up();
            adc.write_control_register(0x0003, 0x23F0);
            adc.power_down();
            adc.read_offset_register(true);
            break;
        }
        case 'G':
        case 'g':
        {
            // adc.power_up();
            // adc.write_control_register(0x0004, 0x5FFF);
            // adc.power_down();
            adc.read_gain_register(true);
            break;
        }
        case 'V':
        case 'v':
        {
            // adc.power_up();
            // adc.write_control_register(0x0005, 0x0000);
            // adc.power_down();
            adc.read_overrange_register(true);
            break;
        }
        default:
        {}
    }
}

void data_tx()
{

    printf("start\n");

    for(int tx_index = 0; tx_index < number_of_samples_to_TX; tx_index++)
    {
        printf("%08lx\n", sample_array[tx_index]);
    }
    printf("stop\n");

}
