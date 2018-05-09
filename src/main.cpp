#include "main.h"

int main()
{
    int not_done = 1;

    while(1)
    {

        printf("OpenDrainPullUp\n");
        setup_ADC();
        wait_ms(2000);
        // do
        // {
        // not_done = collect_samples();
        // }
        // while(1 == not_done);

    }
}
