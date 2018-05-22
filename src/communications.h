
#ifndef COMMUNICATIONS_H
#define COMMUNICATIONS_H

#include <stdint.h>
#include <string.h>
#include "mbed.h"

#include "adc.h"
#include "pins.h"

extern Serial usb_serial;
extern volatile int abort_transfer;

void control_signals();

void data_tx();

#endif
