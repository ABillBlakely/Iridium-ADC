
#ifndef COMMUNICATIONS_H
#define COMMUNICATIONS_H

#include <stdint.h>
#include "mbed.h"
#include "pins.h"
#include "adc.h"

extern Serial usb_serial;
extern volatile int abort_transfer;

void control_signals();

void data_tx(volatile uint32_t data_packet[]);

#endif
