#include "main.h"

int main()
{
    while(1)
    {
        setup_ADC();
        wait_ms(2000);
        clear_terminal();
    }
}
