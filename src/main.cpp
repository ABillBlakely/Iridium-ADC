#include "main.h"

int main()
{
    while(1)
    {
        setup_ADC();
        wait_ms(1000);
        clear_terminal();
    }
}
