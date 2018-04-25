#include "main.h"

int main()
{
    // allTests();
    setup_ADC();
    while(1)
    {
        collect_samples();
    }
}
