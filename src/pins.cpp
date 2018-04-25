
#include "pins.h"

// BusInOut dataBus(DATAPINS);
DataBusClass dataBus;

InterruptIn notDataReady(PC_12);
DigitalOut notSync(PC_10);
DigitalOut notRead(PA_13);
DigitalOut notChipSelect(PA_14);
DigitalOut notReset(PC_11);

PortInOut dataBusA(PortA, PORT_A_MASK);
PortInOut dataBusC(PortC, PORT_C_MASK);

DataBusClass::DataBusClass()
{
    port_a = 0;
    port_c = 0;
}

void DataBusClass::input()
{
    dataBusA.input();
    dataBusC.input();
}

void DataBusClass::output()
{
    dataBusA.output();
    dataBusC.output();
}

void DataBusClass::mode(PinMode mode)
{
    dataBusA.mode(mode);
    dataBusC.mode(mode);
}

uint16_t DataBusClass::read()
{
    uint16_t received = 0x0000;

    port_a = dataBusA.read();
    port_c = dataBusC.read();


    // port,   bit select, shift by (pin num - bit pos)
    received |= (port_c & (1 <<  8)) >> (8);       // pc08 to bit 0
    received |= (port_c & (1 <<  6)) >> (5);       // pc06 to bit 1
    received |= (port_c & (1 <<  5)) >> (3);       // pc05 to bit 2

    received |= (port_a & (1 << 12)) >> (9);      // pa12 to bit 3
    received |= (port_a & (1 << 11)) >> (7);      // pa11 to bit 4
    received |= (port_a & (1 <<  9)) >> (4);       // pa09 to bit 5
    received |= (port_a & (1 << 10)) >> (3);      // pa10 to bit 7
    received |= (port_a & (1 <<  8)) >> (2);       // pa08 to bit 6

    received |= (port_c & ((1 <<  1)|(1 << 7))) << (7);    // pc01 to bit 8, pc07 to bit 14

    received |= (port_a & (1 <<  4)) << (5);    // pa04 to bit 9
    received |= (port_a & (1 <<  1)) << (9);   // pa01 to bit 10
    received |= (port_a & (1 <<  0)) << (11);   // pa00 to bit 11
    received |= (port_a & ((1 <<  6)|(1 << 7))) << (6);   // pa06 to bit 12 // pa07 to bit 13
    received |= (port_a & (1 << 15));  // pa15 to bit 15

    return received;
}

void DataBusClass::write(uint16_t word)
{
    port_a = 0x0000;
    port_c = 0x0000;

    // port,   bit select, shift by (pin num - bit pos)
    port_c |= (word & (1 << 0)) << (8 - 0);       // pc08
    port_c |= (word & (1 << 1)) << (6 - 1);       // pc06
    port_c |= (word & (1 << 2)) << (5 - 2);       // pc05
    port_a |= (word & (1 << 3)) << (12 - 3);      // pa12
    port_a |= (word & (1 << 4)) << (11 - 4);      // pa11
    port_a |= (word & (1 << 5)) << (9 - 5);       // pa09
    port_a |= (word & (1 << 6)) << (8 - 6);       // pa08
    port_a |= (word & (1 << 7)) << (10 - 7);      // pa10
    port_c |= (word & (1 << 8)) >> (-(1 - 8));    // pc01
    port_a |= (word & (1 << 9)) >> (-(4 - 9));    // pa04
    port_a |= (word & (1 << 10)) >> (-(1 - 10));   // pa01
    port_a |= (word & (1 << 11)) >> (-(0 - 11));   // pa00
    port_a |= (word & (1 << 12)) >> (-(6 - 12));   // pa06
    port_a |= (word & (1 << 13)) >> (-(7 - 13));   // pa07
    port_c |= (word & (1 << 14)) >> (-(7 - 14));   // pc07
    port_a |= (word & (1 << 15)) >> (-(15 - 15));  // pa15

    dataBusA.write(port_a);
    dataBusC.write(port_c);
}
