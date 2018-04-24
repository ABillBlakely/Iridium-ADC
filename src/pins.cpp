
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

    received |= (port_c & 0x0080) >> (8 - 0);       // pc08 to bit 0
    received |= (port_c & 0x0020) >> (6 - 1);       // pc06 to bit 1
    received |= (port_c & 0x0010) >> (5 - 2);       // pc51 to bit 2
    received |= (port_a & 0x0800) >> (12 - 3);      // pa12 to bit 3
    received |= (port_a & 0x0400) >> (11 - 4);      // pa11 to bit 4
    received |= (port_a & 0x0100) >> (9 - 5);       // pa09 to bit 5
    received |= (port_a & 0x0080) >> (8 - 6);       // pa08 to bit 6
    received |= (port_a & 0x0200) >> (10 - 7);      // pa10 to bit 7
    received |= (port_c & 0x0002) << (-(1 - 8));    // pc01 to bit 8
    received |= (port_a & 0x0010) << (-(4 - 9));    // pa04 to bit 9
    received |= (port_a & 0x0002) << (-(1 - 10));   // pa01 to bit 10
    received |= (port_a & 0x0001) << (-(0 - 11));   // pa00 to bit 11
    received |= (port_c & 0x8000) >> (15 - 12);     // pc15 to bit 12
    received |= (port_c & 0x0008) << (-(4 - 13));   // pc04 to bit 13
    received |= (port_c & 0x4000) >> (14 - 14);     // pc14 to bit 14
    received |= (port_c & 0x0200) << (-(9 - 15));   // pc09 to bit 15

    return received;
}

void DataBusClass::write(uint16_t word)
{
    port_a = 0x0000;
    port_c = 0x0000;

    // port,   bit select, shift by (pin num - bit pos)
    port_c |= (word & 0x0001) << (8 - 0); // pc8
    port_c |= (word & 0x0002) << (6 - 1); // pc6
    port_c |= (word & 0x0004) << (5 - 2); // pc51
    port_a |= (word & 0x0008) << (12 - 3); // pa12
    port_a |= (word & 0x0010) << (11 - 4); // pa11
    port_a |= (word & 0x0020) << (9 - 5); // pa9
    port_a |= (word & 0x0040) << (8 - 6); // pa8
    port_a |= (word & 0x0080) << (10 - 7); // pa10
    port_c |= (word & 0x0100) >> (-(1 - 8)); // pc1
    port_a |= (word & 0x0200) >> (-(4 - 9)); // pa4
    port_a |= (word & 0x0400) >> (-(1 - 10)); // pa1
    port_a |= (word & 0x0800) >> (-(0 - 11)); // pa0
    port_c |= (word & 0x1000) << (15 - 12); // pc15
    port_c |= (word & 0x2000) >> (-(4 - 13)); // pc4
    port_c |= (word & 0x4000) << (14 - 14); // pc14
    port_c |= (word & 0x8000) >> (-(9 - 15)); // pc9

    dataBusA.write(port_a);
    dataBusC.write(port_c);
}
