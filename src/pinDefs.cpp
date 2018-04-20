
#include "pinDefs.h"

BusInOut dataBus(DATAPINS);

DigitalOut notSync(PC_10);
DigitalOut notRead(PA_13);
DigitalOut notChipSelect(PA_14);
DigitalOut notReset(PC_11);

DigitalIn notDataReady(PC_12);

BusIn controlBus(PC_10, PC_11, PC_12, PA_13, PA_14);

