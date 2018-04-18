/* Test files

Results: should alternate between 0x00 and 0x1F in serial terminal.
This is performed by alternating the pull up or pull down mode and taking a reading.
*/

#include "unit_tests.h"
#include "mbed.h"

BusIn dataBus(PC_8, PC_6, PC_5, PA_12, PA_11, PA_9, PA_8, PA_10, PC_1, PA_4, PA_1, PA_0, PC_15, PC_4, PC_13, PC_9);
BusIn controlBus(PC_10, PC_11, PC_12, PA_13, PA_14);

void allTests()
{
    while(1)
    {
        controlBusTest();
        wait_ms(250);
        dataBusTest();
        wait_ms(250);
    }
}


void controlBusTest()
{
    // Test of the controlBus by reading the pins under the
    // effect of pull up or pull down resistors
    printf("\ncontrolBus Test begin.\n");
    controlBus.mode(PullUp);
    if ((controlBus.read() & 0x1F) == 0x1F)
    {
        printf("controlBus pull up passed\n");
    }
    else
    {
        printf("controlBus pull up failed on bit %x\n", controlBus.read() ^ 0x1F);
    }
    controlBus.mode(PullDown);
    if ((controlBus.read() & 0x1F) == 0x00)
    {
        printf("controlBus pull down passed\n");
    }
    else
    {
        printf("controlBus pull down failed on bit 0x%x\n", controlBus.read());
    }
    printf("controlBus Test end.\n\n");
}

void dataBusTest()
{
    // Test of the dataBus by reading the pins under the
    // effect of pull up or pull down resistors
    printf("\ndataBus Test begin.\n");
    dataBus.mode(PullUp);
    if ((dataBus.read() & 0xFFFF) == 0xFFFF)
    {
        printf("dataBus pull up passed\n");
    }
    else
    {
        printf("dataBus pull up failed on bit %x\n", dataBus.read() ^ 0xFF);
    }
    dataBus.mode(PullDown);
    if ((dataBus.read() & 0xFFFF) == 0x0000)
    {
        printf("dataBus pull down passed\n");
    }
    else
    {
        printf("dataBus pull down failed on bit 0x%x\n", dataBus.read());
    }
    printf("dataBus testend.\n\n");
}
