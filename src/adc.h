
#ifndef ADC_H
#define ADC_H

#include <stdint.h>
#include "mbed.h"
#include "pins.h"

#define MCLK_FREQ 40000000

#define HIGH 1
#define LOW 0


#define DECIMATION_RATE 0b011

// 256 [kB] * 1024 [B/kB] * 8 [b/B] / 32 [b/sample] = 65536 samples.
#define NUMBER_OF_SAMPLES 20
#define NUMBER_OF_PAGES 1
#define SAMPLES_PER_PAGE NUMBER_OF_SAMPLES / NUMBER_OF_PAGES

volatile extern int inter_flag;

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

void power_down();

void power_up();

void write_control_register(uint16_t control_register, uint16_t value);

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

void clear_terminal();

void interrupt_test();
#endif
