#include "main.h"

int main()
{
    ADC_Class adc;
    UnitTests tests;

    adc.clear_terminal();
    printf("20 samples\n");


    adc.setup_ADC();

    adc.receive_data();

    while(1)
    {

        // do
        // {
        // not_done = collect_samples();
        // }
        // while(1 == not_done);

    }
    printf("while loop ended somehow.\n");

}
