#include "mbed.h"

#define SPI_WORD 8
#define SPI_FREQ 1000000
#define CPOL 0
#define CPHA 0

#define TEST_BYTE 0xAA

SPI spi1(PA_7, PA_6, PA_5, PA_4);
PwmOut led(LED1);

uint8_t rec_data = 0;
uint32_t correct = 0;

int main()
{
    spi1.frequency(SPI_FREQ);
    spi1.format(SPI_WORD, CPOL|CPHA);

    led.write(0.1);
    printf("\nInitiating Test\n");

    while(1)
    {
        printf("In Loop\n");
        // Slave test.
        // if (spi1.receive())
        // {
        //     data = spi1.read();
        //     printf("0x%04x",data);
        // // }
        rec_data = spi1.write(TEST_BYTE);

        printf("Data Transmitted");

        if (rec_data == TEST_BYTE)
        {
            printf("if true");
            correct++;
            printf("%u correct TX/RX\n", correct);
        }
        else
        {
            printf("if false");
            printf("Error found after %u correct transmissions\n", correct);
            led.write(0.5);
            correct = 0;
        }

    }
}
