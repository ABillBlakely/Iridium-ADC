
#ifndef ADC_H
#define ADC_H

#include <stdint.h>
#include "mbed.h"
#include "pins.h"

#define MCLK_FREQ 40000000

#define HIGH 1
#define LOW 0

// 256 [kB] * 1024 [B/kB] * 8 [b/B] / 32 [b/sample] = 65536 samples.
#define NUMBER_OF_SAMPLES 16384

//
// public functions:
//
// setup_ADC() configures the ADC by writing to control register 2,
// and output data rate by writing to register 1. It also sets
// up the pin modes correctly for the other functions.
// setup_ADC() must always be called before any other functions.
void setup_ADC();

uint16_t read_status_reg(bool print_to_console);

void receive_data();

//
// private:
//
// offset should be either
//     11: status register
//     12: Offset register
//     13: Gain register
//     14: Overrange register
uint16_t read_adc_reg(uint8_t offset);

void wait_4_MCLK_cycles();

void get_LSB();

void get_MSB();

uint32_t read_data_word();

void collect_samples();

#endif
