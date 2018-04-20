#include "adc.h"

void setup_ADC()
{
    uint16_t status_reg;

    // configure pins for write operations.
    dataBus.output();
    dataBus.mode(PullDown);
    dataBus = LOW;
    notSync = HIGH;
    notRead = HIGH;
    notChipSelect = HIGH;
    notReset = HIGH;
    notDataReady.mode(PullDown);

    // reset to start in a defined state.
    notReset = LOW;
    wait_MCLK_cycles(2);
    notReset = HIGH;
    wait_MCLK_cycles(2);

    // Set control register 2:
    dataBus = 0x0002;
    notRead = HIGH;
    notChipSelect = LOW;
    // wait_MCLK_cycles(8);
    notChipSelect = HIGH;
    // wait_MCLK_cycles(8);

    // BIT | NAME  | DESCRIPTION
    // ----|------ |-------------
    //  5  | ~CDIV | Clock Divider Bit. This sets the divide ratio of the MCLK signal to produce the internal ICLK. Setting CDIV = 0 divides the MCLK by 2. If CDIV = 1, the ICLK frequency is equal to the MCLK.
    //  3  | PD    | Power Down. Setting this bit powers down the AD7760, reducing the power consumption to 6.35 mW.
    //  2  | LPWR  | Low Power. If this bit is set, the AD7760 is operating in a low power mode. The power consumption is reduced for a 6 dB reduction in noise performance.
    //  1  | 1     | Write 1 to this bit.
    //  0  | D1PD  | Differential Amplifier Power Down. Setting this bit powers down the on-chip differential amplifier.
    dataBus = (0x0000 | 0b000110);
    notChipSelect = LOW;
    // wait_MCLK_cycles(8);
    notChipSelect = HIGH;

    // Control Register 2:
    dataBus = 0x0001;
    notRead = HIGH;
    notChipSelect = LOW;
    // wait_MCLK_cycles(8);
    notChipSelect = HIGH;
    // wait_MCLK_cycles(8);

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
    dataBus = (0x0000 | 1<<11 | 1<<4 | 1<<3);
    notChipSelect = LOW;
    // wait_MCLK_cycles(8);
    notChipSelect = HIGH;
    // wait_MCLK_cycles(2);

    dataBus.input();
    notRead = LOW;
    notChipSelect = LOW;
    status_reg = dataBus.read();
    notChipSelect = HIGH;
    notRead = HIGH;

    // Print status register to confirm correct operation.
    // Note that Byp Filt bits are inverted before printing.
    // In the data sheet these are not bypass bits.
    printf("Status Register\n");
    printf("| Part No.  | Die No. | Low Pwr | Overrange | Download O.K. | User Filt | User Filt O.K. | Byp Filt 3 | Byp Filt 1 | Decimation |\n");
    printf("|    %02d     |   %03d   |    %d    |     %d     |       %d       |     %d     |        %d       |     %d      |     %d      |      %d     |\n\n",
            (status_reg & 0xC000)>>14,
            (status_reg & 0x3800)>>11,
            (status_reg & 0x0200)>>9,
            (status_reg & 0x0100)>>8,
            (status_reg & 0x0080)>>7,
            (status_reg & 0x0020)>>5,
            (status_reg & 0x0040)>>6,
            !((status_reg & 0x0010)>>4),
            !((status_reg & 0x0008)>>3),
            (status_reg & 0x0007));
}

void wait_MCLK_cycles(uint8_t cycles)
{
    uint32_t time_us;
    time_us = cycles * 1e6 / MCLK_FREQ;
    if (time_us < 1)
    {
        time_us = 1;
    }
    wait_us(time_us);
}
