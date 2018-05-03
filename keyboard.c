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
#include "driverlib/hardware.h"
bool IsPressed = 0;
uint8_t key = 0xFF;
uint8_t lastKey = 0xFF;
uint32_t DebouncerDelay = 500;
uint32_t DebouncerDelayCount = 0;

uint8_t KeyboardGetKey()
{
    key = KeyboardScan();
   //if(KeyboardIsPressed()){

        switch(key)
        {
            case 0xEE:
                return '1';
            case 0xED:
                return '2';
            case 0xEB:
                return '3';
            case 0xE7:
                return 'A';
            case 0xDE:
                return '4';
            case 0xDD:
                return '5';
            case 0xDB:
                return '6';
            case 0xD7:
                return 'B';
            case 0xBE:
                return '7';
            case 0xBD:
                return '8';
            case 0xBB:
                return '9';
            case 0xB7:
                return 'C';
            case 0x7E:
                return '*';
            case 0x7D:
                return '0';
            case 0x7B:
                return '#';
            case 0x77:
                return 'D';
            default:
                return 0xFF;
       // }
   }
   // else{
   //     return 0xFF;
    //}
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

uint8_t KeyboardColumnsScan(int r)
{

    uint8_t column = 0x00;
    uint8_t keypadValue = 0x00;
    column = MAP_GPIOPinRead(GPIO_PORTE_BASE, GPIO_PIN_3|GPIO_PIN_2|GPIO_PIN_1|GPIO_PIN_0);
    //printf("row: %x", r);
    //printf("column: %x", column);
    keypadValue = ((column == 0x07) ||
                   (column == 0x0B) ||
                   (column == 0x0D) ||
                   (column == 0x0E))?(((r & 0x0F) << 4)| (column & 0x0F)) : 0x00;
    return keypadValue;
}

uint8_t KeyboardScan()
{
    uint8_t readStatus = 0x00;
    uint8_t i = 0;
    uint8_t rows[4] = {0x07, 0x0B, 0x0D, 0x0E};
    //while(readStatus == 0x00)
    while(1)
    {
        MAP_GPIOPinWrite(GPIO_PORTL_BASE, GPIO_PIN_3|GPIO_PIN_2|GPIO_PIN_1|GPIO_PIN_0, rows[i]);
        readStatus = KeyboardColumnsScan(rows[i]);
        i = ((i + 1) % 4);
        if(readStatus != 0x00 || KeyBoardIntFlag == true)
        {
            break;
        }
        SysCtlDelay(15*40000);

    };
    return readStatus;
}
