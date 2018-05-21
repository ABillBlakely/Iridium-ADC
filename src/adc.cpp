#include "adc.h"

uint16_t ADC_Class::control_reg_1_state = 0x0000;
uint16_t ADC_Class::control_reg_2_state = 0x0000;

ADC_Class adc;

volatile uint32_t busy_wait_variable;
volatile uint32_t sample_array[SAMPLES_PER_PAGE];
volatile int data_ready = 0;

ADC_Class::ADC_Class()
{
    dataBus.input();
    notSync = HIGH;
    notReset = HIGH;
    notChipSelect = HIGH;
    notRead = HIGH;

    // reset to start in a defined state.
    notReset = LOW;
    wait_4_MCLK_cycles();
    notReset = HIGH;
    wait_4_MCLK_cycles();
    power_down();
}

void ADC_Class::setup()
{

    // configure pins for write operations.
    // dataBus is bi directional.
    dataBus.output();

    // Control Register 2
    // // BIT | NAME  | DESCRIPTION
    // // ----|------ |-------------
    // //  5  | ~CDIV | Clock Divider Bit. This sets the divide ratio of the MCLK signal to produce the internal ICLK. Setting CDIV = 0 divides the MCLK by 2. If CDIV = 1, the ICLK frequency is equal to the MCLK.
    // //  3  | PD    | Power Down. Setting this bit powers down the AD7760, reducing the power consumption to 6.35 mW.
    // //  2  | LPWR  | Low Power. If this bit is set, the AD7760 is operating in a low power mode. The power consumption is reduced for a 6 dB reduction in noise performance.
    // //  1  | 1     | Write 1 to this bit.
    // //  0  | D1PD  | Differential Amplifier Power Down. Setting this bit powers down the on-chip differential amplifier.

    // // Set lowpower mode for improved noise performance.
    control_reg_2_state = (0x0000 | 1 << 2 | 1 << 1);
    write_control_register(0x0002, control_reg_2_state);

   // Control Register 1:
    // | BIT    | NAME      | DESCRIPTION
    // | 15     | DL_FILT   | Download Filter. Before downloading a user-defined filter, this bit must be set. The filter length bits must also be set at this time. The write operations that follow are interpreted as the user coefficients for the FIR filter until all the coefficients and the checksum have been written.
    // | 14     | RD_OVR    | 2 Read Overrange. If this bit has been set, the next read operation outputs the contents of the overrange threshold register instead of a conversion result.
    // | 13     | RD_GAIN   | 2 Read Gain. If this bit has been set, the next read operation outputs the contents of the digital gain register.
    // | 12     | RD_OFF    | 2 Read Offset. If this bit has been set, the next read operation outputs the contents of the digital offset register.
    // | 11     | RD_STAT   | 2 Read Status. If this bit has been set, the next read operation outputs the contents of the status register.
    // | 10     | 0         | 0 must be written to this bit.
    // | 9      | SYNC      | Synchronize. Setting this bit initiates an internal synchronization routine. Setting this bit simultaneously on multiple devices synchronizes all filters.
    // | 8 to 5 | FLEN[3:0] | Filter Length Bits. These bits must be set when the DL_FILT bit is set before a user-defined filter is downloaded.
    // | 4      | ~BYP_F3   | Bypass Filter 3. If this bit is 0, Filter 3 (programmable FIR) is bypassed.
    // | 3      | ~BYP_F1   | Bypass Filter 1. If this bit is 0, Filter 1 is bypassed. This should only occur when the user requires unfiltered modulator data to be output.
    // | 2 to 0 | DEC[2:0]  | Decimation Rate. These bits set the decimation rate of Filter 2. All 0s implies that the filter is bypassed. A value of 1 corresponds to 2× decimation, a value of 2 corresponds to 4× decimation, and so on, up to the maximum value of 5, corresponding to 32× decimation.
    //
    // Set for 2.5MHZ output data rate (enable Filt 1 & 3, No Decimation).
    // In testing a lower rate may be required, and 0b11 == 3 should be included
    // to enable 8x decimation, lowering data rate to 312.5 kHz.
    control_reg_1_state = (0x0000 | 1<<4 | 1<<3 | DEFAULT_DECIMATION_RATE);
    write_control_register(0x0001, control_reg_1_state);

    // These might be redundant, but this will
    // guarantee a defined state for all pins.
    dataBus.input();
    notRead = HIGH;
    notChipSelect = HIGH;
    notSync = HIGH;
    notReset = HIGH;

    power_down();
}

uint16_t ADC_Class::read_status_register(bool print_to_console)
{
    uint16_t status_reg = 0;

    // TODO: define register offset macros.

    power_up();
    status_reg = read_adc_reg(11);
    power_down();
    if (print_to_console)
    {
        // Pretty print the received status register to stdout.
        printf("Status Register\n");
        printf("| Part No.  | Die No. | Low Pwr | Overrange | Download O.K. | User Filt O.K. | User Filt EN | Byp Filt 3 | Byp Filt 1 | Dec Rate |\n");
        printf("|     %d     |    %d    |    %d    |     %d     |       %d       |       %d        |      %d       |      %d     |     %d      |    %2d    |\n",
                (status_reg & 0xC000) >> 14, //Part No.
                (status_reg & 0x3800) >> 11, // Die No.
                (status_reg & 0x0200) >> 9,  // Low Pwr
                (status_reg & 0x0100) >> 8,  // Overrange
                (status_reg & 0x0080) >> 7,  // Download OK
                (status_reg & 0x0040) >> 6,  // User Filt OK
                (status_reg & 0x0020) >> 5,  // User Filt EN
               !(status_reg & 0x0010) >> 4,  // Byp Filt 3
               !(status_reg & 0x0008) >> 3,  // Byp Filt 1
                1 << (status_reg & 0x0007)); // Dec 2:0, Use left shift to do exponentiation.
    }

    return status_reg;
}

void ADC_Class::start_sampling()
{
    power_up();
    wait_ms(1);
    collect_samples();
    // notDataReady.rise(&collect_samples);
    // notDataReady.enable_irq();
}

void ADC_Class::stop_sampling()
{
    // notDataReady.disable_irq();
    // Save power, and power down when not being used.
    power_down();
}

void ADC_Class::power_down()
{
    write_control_register(0x0002, (control_reg_2_state | 1 << 3));
}

void ADC_Class::power_up()
{
    write_control_register(0x0002, (control_reg_2_state & ~(1 << 3)));
    // worst case filter latency is about 350 us.
    wait_ms(10);
}

void ADC_Class::change_decimation_rate(int multiplier)
{
    power_up();
    control_reg_1_state = ((control_reg_1_state & 0xFFF8) | multiplier);
    write_control_register(0x0001, control_reg_1_state);
    power_down();
}

uint16_t ADC_Class::read_adc_reg(uint8_t offset)
{
    uint16_t adc_reg;

    // Set signals up for writing to adc.
    notRead = HIGH;
    notChipSelect = HIGH;
    dataBus.output();

    // write to Control Register 1.

    // | BIT    | NAME      | DESCRIPTION
    // | 15     | DL_FILT   | Download Filter. Before downloading a user-defined filter, this bit must be set. The filter length bits must also be set at this time. The write operations that follow are interpreted as the user coefficients for the FIR filter until all the coefficients and the checksum have been written.
    // | 14     | RD_OVR    | 2 Read Overrange. If this bit has been set, the next read operation outputs the contents of the overrange threshold register instead of a conversion result.
    // | 13     | RD_GAIN   | 2 Read Gain. If this bit has been set, the next read operation outputs the contents of the digital gain register.
    // | 12     | RD_OFF    | 2 Read Offset. If this bit has been set, the next read operation outputs the contents of the digital offset register.
    // | 11     | RD_STAT   | 2 Read Status. If this bit has been set, the next read operation outputs the contents of the status register.
    // | 10     | 0         | 0 must be written to this bit.
    // | 9      | SYNC      | Synchronize. Setting this bit initiates an internal synchronization routine. Setting this bit simultaneously on multiple devices synchronizes all filters.
    // | 8 to 5 | FLEN[3:0] | Filter Length Bits. These bits must be set when the DL_FILT bit is set before a user-defined filter is downloaded.
    // | 4      | ~BYP_F3   | Bypass Filter 3. If this bit is 0, Filter 3 (programmable FIR) is bypassed.
    // | 3      | ~BYP_F1   | Bypass Filter 1. If this bit is 0, Filter 1 is bypassed. This should only occur when the user requires unfiltered modulator data to be output.
    // | 2 to 0 | DEC[2:0]  | Decimation Rate. These bits set the decimation rate of Filter 2. All 0s implies that the filter is bypassed. A value of 1 corresponds to 2× decimation, a value of 2 corresponds to 4× decimation, and so on, up to the maximum value of 5, corresponding to 32× decimation.
    //
    // Set for 1MHZ output data rate and to read the status register.

    // Set the particular read status bit. Should be 11, 12, 13, or 14.
    write_control_register(0x0001, control_reg_1_state | 1 << offset);

    notChipSelect = LOW;
    wait_4_MCLK_cycles();
    notChipSelect = HIGH;

    // Reset dataBus to read data, this takes long enough
    // that a explicit pause isn't necessary.
    dataBus.input();
    // The dataBus should now have the status on it.
    notRead = LOW;
    notChipSelect = LOW;
    // Grab the raw input
    adc_reg = dataBus.read();
    notChipSelect = HIGH;
    notRead = HIGH;

    // put the bit from the raw input into the correct order.
    adc_reg = dataBus.detangle(adc_reg);

    return adc_reg;
}

void ADC_Class::write_control_register(uint16_t control_register, uint16_t value)
{
    dataBus.output();

    dataBus.write(control_register);
    notChipSelect = LOW;
    wait_4_MCLK_cycles();
    notChipSelect = HIGH;
    wait_4_MCLK_cycles();

    dataBus.write(value);
    notChipSelect = LOW;
    wait_4_MCLK_cycles();
    notChipSelect = HIGH;
    wait_4_MCLK_cycles();

    // Pull the lines down before switching to input
    dataBus.write (0x0000);

    dataBus.input();
}

void ADC_Class::wait_4_MCLK_cycles()
{
    // Calling this function results in a delay of about 100 ns
    // or around 4 cycles of the ADC MCLK at 40 MHz.
    // Without this a high-low-high cycle take about 30 ns ~= 1 MCLK cycle.
    busy_wait_variable = 0;
    busy_wait_variable ++;
}

uint32_t ADC_Class::read_data_word()
{
    static uint16_t MSB_16;
    static uint16_t LSB_16;

    notRead = LOW;
    notChipSelect = LOW;
    MSB_16 = dataBus.read();
    notChipSelect = HIGH;
    notRead = HIGH;

    wait_4_MCLK_cycles();

    notRead = LOW;
    notChipSelect = LOW;
    LSB_16 = dataBus.read();
    notChipSelect = HIGH;
    notRead = HIGH;

    // concatenate and return the 32 bit data word.
    return ((MSB_16 << 16) | LSB_16);
}

void ADC_Class::collect_samples()
{
    // called on interrupt from notDataReady pin
    // static int32_t sample_index = 0;

    for (uint32_t sample_index = 0; sample_index < SAMPLES_PER_PAGE; sample_index++)
    {
        // this should all happen in under 250 ns.
        while(notDataReady.read() == 1)
        {
            // wait for notDataReady to be pulled low
        }
        sample_array[sample_index] = ADC_Class::read_data_word();
        // sample_index++;
    }
    {
        stop_sampling();
        // sample_index = 0;
        data_ready = 1;
    }
}
