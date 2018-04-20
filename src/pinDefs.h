
#ifndef PINDEFS_H
#define PINDEFS_H

#include "mbed.h"

#define DATAPINS PC_8, PC_6, PC_5, PA_12, PA_11, PA_9, PA_8, PA_10, PC_1, PA_4, PA_1, PA_0, PC_15, PC_4, PC_14, PC_9

extern BusInOut dataBus;
extern DigitalOut notSync;
extern DigitalOut notRead;
extern DigitalOut notChipSelect;
extern DigitalOut notReset;
extern DigitalIn notDataReady;

extern BusIn controlBus;

#endif
