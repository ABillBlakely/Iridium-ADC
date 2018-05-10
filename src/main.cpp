#include "main.h"

int main()
{
    clear_terminal();
    printf("20 samples\n");

    UnitTests tests;
    setup_ADC();

    receive_data();

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
