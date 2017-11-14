/*
 * trava_eletrica.c
 *
 *  Created on: 1 de set de 2017
 *      Author: thfax
 */
#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "driverlib/debug.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/trava_eletrica.h"

bool trava_acionada = 0;

void TravaInit()
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOG);

    // Configura GPIO como input
    GPIOPinTypeGPIOOutput(GPIO_PORTG_BASE, GPIO_PIN_1);
}

void AcionarTrava()
{
    GPIOPinWrite(GPIO_PORTG_BASE, GPIO_PIN_1, 0x02);
    trava_acionada = 1;
}

void DesacionarTrava()
{
    GPIOPinWrite(GPIO_PORTG_BASE, GPIO_PIN_1, 0x00);
    trava_acionada = 0;
}

extern bool TravaAcionada()
{
    return trava_acionada;
}


