/*
Working example of SPI 2 function.
The clock seems to operate at half of the specified frequency
Seems to work up to about 25 MHz clock speed,
increase in SPI_FREQ beyond that results in no increase in clock speed

For Some reason SPI1 refuses to cooperate, attempting to use it results in a clock speed of 50 Hz.

At SPI_FREQ = 1 MHz clock speed, we are able to transfer 19 8 bit numbers in about 280 us
for a bit rate of 540 kbps.

At SPI_FREQ = 20 MHz, 14 8 bit numbers are transferred in 64.8 us
for a bit rate of 1.73 Mbps.

*/



#include "mbed.h"

#define SPI_WORD 8
#define SPI_FREQ 20000000
#define CPOL 0
#define CPHA 0



// spi1(MOSI, MISO, SCLK, SS);
SPI spi2(PB_15, PB_14, PB_13);
DigitalOut cs(PB_12);
PwmOut led(LED1);

uint8_t rec_data = 0;
uint32_t correct = 0;

int main()
{
    cs = 1;
    spi2.frequency(SPI_FREQ);
    spi2.format(SPI_WORD, CPOL|CPHA);

    led.write(0.1);
    // printf("\nInitiating Test\n");

    while(1)
    {
        // printf("In Loop\n");
        for (int kk = 0; kk <=255; kk++)
        {
            cs = 0;
            spi2.write(kk);
            cs = 1;
            // wait_us(10);
        }
    }
}
