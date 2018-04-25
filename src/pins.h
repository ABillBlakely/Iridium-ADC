
#ifndef PINDEFS_H
#define PINDEFS_H

#include <stdint.h>
#include "mbed.h"

//                  LSB0    1     2     3      4      5     6     7      8     9    10    11    12     13    14    MSB
// #define DATAPINS PC_8, PC_6, PC_5, PA_12, PA_11, PA_9, PA_8, PA_10, PC_1, PA_4, PA_1, PA_0, PA_6, PA_7, PC_7, PA_15
#define PORT_A_MASK (                 1<<12 |1<<11 |1<<9 |1<<8 |1<<10       |1<<4 |1<<1 |1<<0 |1<<6 |1<<7       |1<<15 )
#define PORT_C_MASK (1<<8|1<<6 |1<<5                                  |1<<1                               |1<<7        )

#define HIGH 1
#define LOW 0

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
