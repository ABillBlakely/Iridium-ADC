
#ifndef ADC_H
#define ADC_H

#include <stdint.h>
#include "mbed.h"
#include "pins.h"
#include "communications.h"

#define MCLK_FREQ 40000000

#define HIGH 1
#define LOW 0


#define DEFAULT_DECIMATION_RATE 0x5
// Decimation rate table:
// | Binary | Rate |   BW   | Output Data Rate |
// |--------|------|--------|------------------|
// | 0b000  |  1x  |   1M   |       2.5 MSPS   |
// | 0b001  |  2x  |  500k  |      1.25 MSPS   |
// | 0b010  |  4x  |  250k  |       625 kSPS   |
// | 0b011  |  8x  |  125k  |     312.5 kSPS   |
// | 0b100  | 16x  | 62.5k  |    156.25 kSPS   |
// | 0b101  | 32x  | 31.25k |    78.125 kSPS   |

// 256 [kB] * 1024 [B/kB] * 8 [b/B] / 32 [b/sample] = 65536 samples.
#define NUMBER_OF_SAMPLES 16384

class ADC_Class
{
public:
    ADC_Class();
    void static setup();
    void static power_up();
    void static power_down();
    void static start_sampling();
    void static stop_sampling();
    void static change_decimation_rate(int multiplier);
    uint16_t static read_status_register(bool print_to_console);
    uint16_t static read_offset_register(bool print_to_console);
    uint16_t static read_gain_register(bool print_to_console);
    uint16_t static read_overrange_register(bool print_to_console);
    void static collect_samples();

// private:
    uint16_t static control_reg_1_state;
    uint16_t static control_reg_2_state;

    uint16_t static read_adc_reg(uint8_t offset);
    uint32_t static read_data_word();
    void static write_control_register(uint16_t control_register, uint16_t value);
    void static wait_4_MCLK_cycles();

};

extern ADC_Class adc;
extern volatile int data_ready;
extern volatile uint32_t sample_array[];
#endif
