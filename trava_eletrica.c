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
#include"driverlib/pin_map.h"
#include "driverlib/rom_map.h"
#include "driverlib/gpio.h"
#include "trava_eletrica.h"

bool trava_acionada = 0;

void TravaInit()
{
    MAP_GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_4, 0x00);
}

void AcionarTrava()
{
    MAP_GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_4, 0x10);
    trava_acionada = 1;
}

void DesacionarTrava()
{
    MAP_GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_4, 0x00);
    trava_acionada = 0;
}

extern bool TravaAcionada()
{
    return trava_acionada;
}


