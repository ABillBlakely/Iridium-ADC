#include "main.h"

BusIn dataBus(PC_8, PC_6, PC_5, PA_12, PA_11, PA_9, PA_8, PA_10, PC_1, PA_4, PA_1, PA_0, PC_15, PC_4, PC_13, PC_9);
BusIn controlBus(PC_10, PC_11, PC_12, PA_13, PA_14);

int main()
{
    allTests();
}
