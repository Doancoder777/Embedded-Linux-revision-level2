/*****************************************************************************
*  @brief      Userspace application to test gpio - sysfs
*
*  @author    Sigma
*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "gpio.h"

#define LED     (31)

int main()
{
    printf("*********************************\n");
    printf("******* Linux From Scratch *******\n\n");

    GPIOExport(LED);            // 1. export the pin
    GPIODirection(LED, OUT);    // 2. set direction = output
    GPIOWrite(LED, HIGH);       // 3. LED on
    sleep(2);                   //    keep it on for 2 seconds
    GPIOWrite(LED, LOW);        //    LED off
    sleep(2);
    GPIOUnexport(LED);          // 4. release the pin (added)

    return 0;
}
