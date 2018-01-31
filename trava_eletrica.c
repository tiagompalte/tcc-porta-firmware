/* ************************************************************** *
 * UTFPR - Universidade Tecnologica Federal do Paraná
 * Engenharia Eletrônica
 * Trabalho de Conclusão de Curso
 * ************************************************************** *
 * Sistema de Segurança baseado em Reconhecimento de Senha Falada
 * ************************************************************** *
 * Equipe:
 * Luiz Felipe Kim Evaristo
 * Tauan Marinho
 * Tiago Henrique Faxina
 * Tiago Mariani Palte
 * ************************************************************** *
 * Controlador da Trava Elétrica
 * v1.0
 * 2017
 * ************************************************************** */

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


