/*
 * teclado.c
 *
 *  Created on: 1 de set de 2017
 *      Author: thfax
 */
#include "driverlib/keyboard.h"

#include "inc/hw_gpio.h"
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
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
void KeyboardInit()
{
    // Habilita periférios GPIOE e GPIOL
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOL);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);

    // Configura GPIO como input
    GPIOPinTypeGPIOInput(GPIO_PORTE_BASE, GPIO_PIN_3|GPIO_PIN_2|GPIO_PIN_1|GPIO_PIN_0);
    GPIOPinTypeGPIOInput(GPIO_PORTL_BASE, GPIO_PIN_3|GPIO_PIN_2|GPIO_PIN_1|GPIO_PIN_0);
}

uint8_t KeyboardGetKey()
{

    column = GPIOPinRead(GPIO_PORTE_BASE, GPIO_PIN_3|GPIO_PIN_2|GPIO_PIN_1|GPIO_PIN_0);
    row = GPIOPinRead(GPIO_PORTL_BASE, GPIO_PIN_3|GPIO_PIN_2|GPIO_PIN_1|GPIO_PIN_0)<<4;
    key = row|column;

    if(KeyboardIsPressed()){
    switch(key)
    {
        case 0x77:
            return '1';
        case 0x7B:
            return '2';
        case 0x7D:
            return '3';
        case 0x7E:
            return 'A';
        case 0xB7:
            return '4';
        case 0xBB:
            return '5';
        case 0xBD:
            return '6';
        case 0xBE:
            return 'B';
        case 0xD7:
            return '7';
        case 0xDB:
            return '8';
        case 0xDD:
            return '9';
        case 0xDE:
            return 'C';
        case 0xE7:
            return '*';
        case 0xEB:
            return '0';
        case 0xED:
            return '#';
        case 0xEE:
            return 'D';
        default:
            return 0x00;
    }
    }
    else{
        return 0x00;
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

void testeTeclado()
{
    uint8_t tecla = 0xFF;
       uint8_t i = 0;
       //testeSPI_LIBRARY();
       //teste();
       KeyboardInit();
       SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);
       while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPION))
          {
          }
       GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_0);
       while(1)
       {
           tecla = KeyboardGetKey();
           if(tecla != 0x00)
           {
               if(i == 0){
                   GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, GPIO_PIN_0);
                   i = 1;
               }
               else
               {
                   GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, 0x0);
                   i = 0;
               }


           }
       }
}
