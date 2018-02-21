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
 * Controlador do Teclado
 * v1.0
 * 2017
 * ************************************************************** */

#include "driverlib/keyboard.h"

#include "inc/hw_gpio.h"
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include"driverlib/pin_map.h"
#include "driverlib/rom_map.h"
#include "inc/hw_sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
//#include "delay.h"
bool IsPressed = 0;
uint8_t column = 0xFF;
uint8_t row = 0xFF;
uint8_t key = 0xFF;
uint8_t lastKey = 0xFF;
uint32_t DebouncerDelay = 500;
uint32_t DebouncerDelayCount = 0;

uint8_t KeyboardGetKey()
{

    column = MAP_GPIOPinRead(GPIO_PORTE_BASE, GPIO_PIN_3|GPIO_PIN_2|GPIO_PIN_1|GPIO_PIN_0);
    row = MAP_GPIOPinRead(GPIO_PORTL_BASE, GPIO_PIN_3|GPIO_PIN_2|GPIO_PIN_1|GPIO_PIN_0)<<4;
    key = row|column;

    if(KeyboardIsPressed()){

    switch(key)
    {
        case 0x77:
            return 0x01;
        case 0x7B:
            return 0x02;
        case 0x7D:
            return 0x03;
        case 0x7E:
            return 0x0A;
        case 0xB7:
            return 0x04;
        case 0xBB:
            return 0x05;
        case 0xBD:
            return 0x06;
        case 0xBE:
            return 0x0B;
        case 0xD7:
            return 0x07;
        case 0xDB:
            return 0x08;
        case 0xDD:
            return 0x09;
        case 0xDE:
            return 0x0C;
        case 0xE7:
            return 0x0E;
        case 0xEB:
            return 0x00;
        case 0xED:
            return 0x0F;
        case 0xEE:
            return 0x0D;
        default:
            return 0xFF;
    }
    }
    else{
        return 0xFF;
    }
}

bool KeyboardIsPressed()
{
        if(DebouncerDelayCount == 0)
        {
            lastKey = key;
        }
        DebouncerDelayCount++;

        if(key != lastKey & DebouncerDelayCount > DebouncerDelay){
            DebouncerDelayCount = 0;
            return 1;
        }
        else
        {
            return 0;
        }
}
