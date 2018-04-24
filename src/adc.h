
#ifndef ADC_H
#define ADC_H

#include <stdint.h>
#include "mbed.h"
#include "pins.h"

#define MCLK_FREQ 40000000

#define HIGH 1
#define LOW 0

// setup_ADC() configures the ADC by writing to control register 2,
// and output data rate by writing to register 1. It also sets
// up the pin modes correctly for the other functions.
// setup_ADC() must always be called before any other functions.
void setup_ADC();

void get_LSB();

void get_MSB();

void wait_MCLK_cycles(uint8_t cycles);

uint32_t read_data_word();

void collect_samples();

void receive_data();

#endif
