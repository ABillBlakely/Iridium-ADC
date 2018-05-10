/* Test the Control signals to make sure every pin can be read correctly.

Results: should alternate between 0x00 and 0x1F in serial terminal.
This is performed by alternating the pull up or pull down mode and taking a reading.
*/
#ifndef UNIT_TESTS_H
#define UNIT_TESTS_H
#include "mbed.h"
#include "pins.h"

void allTests();

class UnitTests
{
private:

public:
    void dataBus_pin_write();
    void twos_complement();
};




#endif
