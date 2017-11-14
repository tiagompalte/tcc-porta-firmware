/*
 * buzzer.c
 *
 *  Created on: 3 de set de 2017
 *      Author: thfax
 */
#include <stdbool.h>
#include <stdint.h>
#include "driverlib/buzzer.h"
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_sysctl.h"
#include "inc/hw_types.h"

#include "driverlib/sysctl.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
//#include "delay.h"
void BuzzerInit()
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOG);

    GPIOPinTypeGPIOOutput(GPIO_PORTG_BASE, GPIO_PIN_0);
}

void BuzzerEnable()
{
    GPIOPinWrite(GPIO_PORTG_BASE, GPIO_PIN_0, 0x01);
}

void BuzzerDisable()
{
    GPIOPinWrite(GPIO_PORTG_BASE, GPIO_PIN_0, 0x00);
}

void BuzzerEntradaPermitida()
{
    BuzzerEnable();
    //DelayMS(200);
    BuzzerDisable();
}

void BuzzerNaoPermitida()
{
    BuzzerEnable();
    //DelayMS(2000);
    BuzzerDisable();
}
