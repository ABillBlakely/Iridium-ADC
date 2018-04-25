#include "adc.h"

Timer sample_timer;
volatile int busy_wait_variable = 0;

void setup_ADC()
{
    // configure pins for write operations.
    // dataBus is bi directional.
    dataBus.output();
    dataBus.write(LOW);

    // notDataReady is an input:
    notDataReady.mode(PullDown);

    // Remaining pins are always outputs.
    notSync = HIGH;
    notReset = HIGH;
    notChipSelect = HIGH;
    notRead = HIGH;

    // reset to start in a defined state.
    notReset = LOW;
    wait_4_MCLK_cycles();
    notReset = HIGH;
    wait_4_MCLK_cycles();

    // Select control register 2:
    dataBus.write(0x0002);
    notChipSelect = LOW;
    wait_4_MCLK_cycles();
    notChipSelect = HIGH;
    wait_4_MCLK_cycles();

    // BIT | NAME  | DESCRIPTION
    // ----|------ |-------------
    //  5  | ~CDIV | Clock Divider Bit. This sets the divide ratio of the MCLK signal to produce the internal ICLK. Setting CDIV = 0 divides the MCLK by 2. If CDIV = 1, the ICLK frequency is equal to the MCLK.
    //  3  | PD    | Power Down. Setting this bit powers down the AD7760, reducing the power consumption to 6.35 mW.
    //  2  | LPWR  | Low Power. If this bit is set, the AD7760 is operating in a low power mode. The power consumption is reduced for a 6 dB reduction in noise performance.
    //  1  | 1     | Write 1 to this bit.
    //  0  | D1PD  | Differential Amplifier Power Down. Setting this bit powers down the on-chip differential amplifier.

    // Set lowpower mode for improved Distortion characteristics.
    dataBus.write((0x0000 | 1 << 2 | 1 << 1));
    notChipSelect = LOW;
    wait_4_MCLK_cycles();
    notChipSelect = HIGH;

    // Control Register 2:
    dataBus.write(0x0002);
    notChipSelect = LOW;
    wait_4_MCLK_cycles();
    notChipSelect = HIGH;
    wait_4_MCLK_cycles();

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
    // In testing a lower rate may be required, and 0b010 should be included
    // to enable 4x decimation, lowering data rate to 625 kHz.
    dataBus.write((0x0000 | 1<<4 | 1<<3));
    notChipSelect = LOW;
    wait_4_MCLK_cycles();
    notChipSelect = HIGH;
    wait_4_MCLK_cycles();

    // read and print the status register
    read_status_reg(true);

    // These might be redundant, but this will
    // guarantee a defined state for all pins.
    dataBus.input();
    dataBus.mode(PullDown);
    notRead = HIGH;
    notChipSelect = HIGH;
    notSync = HIGH;
    notReset = HIGH;

}

uint16_t read_status_reg(bool print_to_console)
{
    uint16_t status_reg = 0;

    status_reg = read_adc_reg(11);

    if (print_to_console)
    {
        // Pretty print the received status register to stdout.
        printf("Status Register\n");
        printf("| Part No.  | Die No. | Low Pwr | Overrange | Download O.K. | User Filt O.K. | Byp Filt 3 | Byp Filt 1 | Dec 2 | Dec 1 | Dec 0 |\n");
        printf("|    %02d     |   %03d   |    %d    |     %d     |       %d       |     %d          |      %d     |     %d      |   %d   |   %d   |   %d   |\n\n",
                (status_reg & 0xC000) >> 14,
                (status_reg & 0x3800) >> 11,
                (status_reg & 0x0200) >> 9,
                (status_reg & 0x0100) >> 8,
                (status_reg & 0x0080) >> 7,
                (status_reg & 0x0040) >> 6,
                (status_reg & 0x0020) >> 5,
               !(status_reg & 0x0010) >> 4,
               !(status_reg & 0x0008) >> 3,
                (status_reg & 0x0004) >> 2,
                (status_reg & 0x0003));
    }

    return status_reg;
}

uint16_t read_adc_reg(uint8_t offset)
{
    uint16_t adc_reg;

    // Set signals up for writing to adc.
    dataBus.output();
    notRead = HIGH;
    notChipSelect = HIGH;

    // write to address 2.
    dataBus.write(0x0002);
    notChipSelect = LOW;
    wait_4_MCLK_cycles();
    notChipSelect = HIGH;
    wait_4_MCLK_cycles();

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

    // Set read status bit.
    dataBus.write(0x0000 | 1 << offset);
    notChipSelect = LOW;
    wait_4_MCLK_cycles();
    notChipSelect = HIGH;

    // Reset dataBus to read data, this takes long enough
    // that a explicit pause isn't necessary.
    dataBus.input();
    dataBus.mode(PullDown);

    // The dataBus should now have the status on it.
    notRead = LOW;
    notChipSelect = LOW;
    adc_reg = dataBus.read();
    notChipSelect = HIGH;
    notRead = HIGH;

    return adc_reg;
}

void wait_4_MCLK_cycles()
{
    // Calling this function results in a delay of about 100 ns
    // or around 4 cycles of the ADC MCLK at 40 MHz.
    // Without this a high-low-high cycle take about 30 ns ~= 1 MCLK cycle.
    busy_wait_variable = 0;
    busy_wait_variable ++;
}

uint32_t read_data_word()
{
    uint16_t MSB_16;
    uint16_t LSB_16;

    notRead = LOW;
    notChipSelect = LOW;
    MSB_16 = dataBus.read();
    notChipSelect = HIGH;
    // notRead = HIGH;

    // notRead = LOW;
    notChipSelect = LOW;
    LSB_16 = dataBus.read();
    notChipSelect = HIGH;
    notRead = HIGH;

    // NOTE: I think these need to be unsigned for this concat to work,
    // but the 32 bit word result is actually a 24 bit signed integer
    // followed by 8 status bits.
    return ((MSB_16 << 16) | LSB_16);
    // return LSB_16;
}

void collect_samples()
{
    static uint32_t sample_array[NUMBER_OF_SAMPLES];
    static uint32_t sample_index = 0;
    uint32_t tx_index;

    if (0 == sample_index)
    {
        sample_timer.reset();
        sample_timer.start();
    }
    if (sample_index < NUMBER_OF_SAMPLES)
    {
        // this should all happen in under 250 ns.
        sample_array[sample_index] = read_data_word();
        sample_index++;
    }
    else
    {
        sample_timer.stop();
        // Need to trigger the data transfer
        // and reset the sample_index variable.
        printf("Begin Data Transfer\n");
        for(tx_index = 0; tx_index < NUMBER_OF_SAMPLES; tx_index++)
        {
            // printf("%04x \n", sample_array[tx_index]);
        }
        printf("Sampling took: \n\t%f seconds \n\t%d samples \n\t%f SPS.\n", sample_timer.read(), NUMBER_OF_SAMPLES, NUMBER_OF_SAMPLES/sample_timer.read());
        printf("END Data Transfer\n\n");
        wait_ms(500);
        printf("\014");
        sample_index = 0;
    }
}

void receive_data()
{
    notDataReady.fall(&collect_samples);
}
