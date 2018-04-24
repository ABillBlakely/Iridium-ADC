
#ifndef PINDEFS_H
#define PINDEFS_H

#include <stdint.h>
#include "mbed.h"
//                  LSB0    1     2     3      4      5     6     7      8     9    10    11    12     13    14    MSB
// #define DATAPINS PC_8, PC_6, PC_5, PA_12, PA_11, PA_9, PA_8, PA_10, PC_1, PA_4, PA_1, PA_0, PC_15, PC_4, PC_14, PC_9
#define PORT_A_MASK 0x3E13
#define PORT_C_MASK 0xC371

#define HIGH 1
#define LOW 0

// 256 [kB] * 1024 [B/kB] * 8 [b/B] / 32 [b/sample] = 65536 samples.
#define NUMBER_OF_SAMPLES 100

class DataBusClass
{
private:
    uint16_t port_a;
    uint16_t port_c;
public:
    DataBusClass();
    void input();
    void output();
    void mode(PinMode mode);
    uint16_t read();
    void write(uint16_t word);
};

extern DataBusClass dataBus;

extern InterruptIn notDataReady;
extern DigitalOut notSync;
extern DigitalOut notRead;
extern DigitalOut notChipSelect;
extern DigitalOut notReset;

#endif
