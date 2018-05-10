/* Test files

Results: should alternate between 0x00 and 0x1F in serial terminal.
This is performed by alternating the pull up or pull down mode and taking a reading.
*/

#include "unit_tests.h"


void allTests()
{

}

void UnitTests::dataBus_pin_write()
{
    dataBus.output();
    dataBus.write(0x0000);

    // Signal Start by turning led off

    for(int kk = 0; kk < 16; kk++)
    {
        dataBus.write(1 << kk);
        // if (!(dataBus.read() & (1 << kk)))
        // {
        //     printf("DataBus Test FAILE on pin DB%d\n", kk);
        //     break;
        // }
    }
}

// void controlBusTest()
// {
//     // Test of the controlBus by reading the pins under the
//     // effect of pull up or pull down resistors
//     printf("\ncontrolBus Test begin.\n");
//     controlBus.mode(PullUp);
//     if ((controlBus.read() & 0x1F) == 0x1F)
//     {
//         printf("controlBus pull up passed\n");
//     }
//     else
//     {
//         printf("controlBus pull up FAILED on bit %02x\n", controlBus.read() ^ 0x1F);
//     }
//     controlBus.mode(PullDown);
//     if ((controlBus.read() & 0x1F) == 0x00)
//     {
//         printf("controlBus pull down passed\n");
//     }
//     else
//     {
//         printf("controlBus pull down FAILED on bit 0x%02x\n", controlBus.read());
//     }
//     printf("controlBus Test end.\n\n");
// }

// void dataBusTest()
// {
//     // Test of the dataBus by reading the pins under the
//     // effect of pull up or pull down resistors
//     printf("\ndataBus Test begin.\n");
//     dataBus.mode(PullUp);
//     if ((dataBus.read() & 0xFFFF) == 0xFFFF)
//     {
//         printf("dataBus pull up passed\n");
//     }
//     else
//     {
//         printf("dataBus pull up FAILED on bit %04x\n", dataBus.read() ^ 0xFF);
//     }
//     dataBus.mode(PullDown);
//     if ((dataBus.read() & 0xFFFF) == 0x0000)
//     {
//         printf("dataBus pull down passed\n");
//     }
//     else
//     {
//         printf("dataBus pull down FAILED on bit 0x%04x\n", dataBus.read());
//     }
//     printf("dataBus test end.\n\n");
// }

void UnitTests::twos_complement()
{
    int8_t signed_int = 0b11111101;
    printf("\ntwosComplementTest begin\n");
    if (signed_int == -3)
    {
        printf("twosComplementTest passed.\n");
    }
    else
    {
        printf("twosComplementTest FAILED\n");
    }
    printf("twosComplementTest end\n\n");


}
