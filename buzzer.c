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
void BuzzerActivate()
{
    MAP_GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_3, 0x00);
}

void BuzzerDeactivate()
{
    MAP_GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_3, 0x08);
}
