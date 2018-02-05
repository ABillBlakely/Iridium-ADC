#include "mbed.h"

Serial pc(USBTX, USBRX); // tx, rx
PwmOut led(LED1);

float brightness = 0.0;

int main() {
    pc.printf("Press U to turn LED1 brightness up, D to turn it down\n");

    while(1) {
        char c = pc.getc();
        if((c == 'u') && (brightness < 0.5)) {
            brightness += 0.01;
            led = brightness;
            // stdout, stderr and stdin default to pc serial
            printf("Brightness: %f \n", brightness );
        }
        if((c == 'd') && (brightness > 0.0)) {
            brightness -= 0.01;
            led = brightness;
            printf("Brightness: %f \n", brightness );
        }
    }
}
