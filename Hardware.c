/*
 * Hardware.c
 *
 *  Created on: 31 de ago de 2017
 *      Author: thfax
 */
#include "driverlib/Hardware.h"
#include "string.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include"driverlib/pin_map.h"
#include "driverlib/rom_map.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"
#include "inc/hw_ssi.h"
#include "driverlib/ssi.h"
#include "driverlib/spi.h"
#include "driverlib/rfid.h"
#include "driverlib/lcd.h"
#include "driverlib/MyString.h"

unsigned char RFIDCardPassWord[5];
unsigned char RFIDCardPassWord1[] = {3, 67, 147, 229, 54};
unsigned char RFIDCardPassWord2[] = {117, 224, 6, 136, 27};
unsigned char RFIDCardPassWord3[] = {201, 66, 106, 123, 54};
CardStatus cardStatus = CardNotDetected;
UserStatus userStatus = EntryNotAllowed;

void HardwareInit()
{

    ui32SysClock = MAP_SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480), 120000000);
    ulDelayms = (ui32SysClock / 3000);
    uint32_t initialData = 0;

    InitConsole();
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI2);
    while(!MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_SSI2));
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    while(!MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOD));
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOG);
    while(!MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOG));
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOM);
    while(!MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOM));
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOK);
    while(!MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOK));

    MAP_GPIOPinConfigure(GPIO_PD3_SSI2CLK);
    MAP_GPIOPinConfigure(GPIO_PD0_SSI2XDAT1);
    MAP_GPIOPinConfigure(GPIO_PD1_SSI2XDAT0); //MOSI

    MAP_GPIOPinTypeSSI(GPIO_PORTD_BASE, GPIO_PIN_1|GPIO_PIN_0|GPIO_PIN_3);

    MAP_GPIOPinTypeGPIOOutput(GPIO_PORTG_BASE,GPIO_PIN_0);
    MAP_GPIOPinTypeGPIOOutput(GPIO_PORTD_BASE,GPIO_PIN_2);
    MAP_GPIOPinTypeGPIOOutput(LCD_CMD, LCD_RS|LCD_RW|LCD_ENABLE);
    MAP_GPIOPinTypeGPIOOutput(LCD_DATA, DATA7|DATA6|DATA5|DATA4|DATA3|DATA2|DATA1|DATA0);


    MAP_SSIIntClear(SSI2_BASE,SSI_TXEOT);
    MAP_SSIClockSourceSet(SSI2_BASE, SSI_CLOCK_SYSTEM);
    MAP_SSIAdvModeSet(SSI2_BASE, SSI_ADV_MODE_LEGACY);
    MAP_SSIConfigSetExpClk(SSI2_BASE, ui32SysClock, SSI_FRF_MOTO_MODE_0,
                               SSI_MODE_MASTER, 1000000, 16);
    MAP_SSIEnable(SSI2_BASE);

    //clear out any initial data that might be present in the RX FIFO
    while(MAP_SSIDataGetNonBlocking(SSI2_BASE, &initialData));

    MAP_GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_2, 0x04);
    MAP_GPIOPinWrite(GPIO_PORTG_BASE, GPIO_PIN_0, 1);

    MAP_SysCtlDelay(50*ulDelayms);

    LCDInit();
    MFRC522Init();

    while(1)
    {
        TesteRFID();
//        LCDControl();
    }

}

void TesteRFID()
{
    unsigned char status;
    unsigned char str[MAX_LEN];


    if (!MFRC522IsCard())
    {
        cardStatus = CardNotDetected;
        UARTprintf("Card not Detected\n");
        return;
    }
    cardStatus = CardDetected;
    status = MFRC522Anticoll(str);
    memcpy(RFIDCardPassWord, str, 5);
    if (status == MI_OK)
    {
        UARTprintf("Card Detected PORRA\n");
        UARTprintf("\n The card's number is  : ");
        UARTprintf("%d", RFIDCardPassWord[0]);
        UARTprintf(" , ");
        UARTprintf("%d", RFIDCardPassWord[1]);
        UARTprintf(" , ");
        UARTprintf("%d", RFIDCardPassWord[2]);
        UARTprintf(" , ");
        UARTprintf("%d", RFIDCardPassWord[3]);
        UARTprintf(" , ");
        UARTprintf("%d", RFIDCardPassWord[4]);
        UARTprintf(" ");
        MAP_SysCtlDelay(1000*ulDelayms);
        buscaCadastro();
    }

    MFRC522Halt();
}

void LCDControl()
{

    if(userStatus == EntryAllowed && cardStatus == CardDetected)
    {
        LCDAllowed();
        MAP_SysCtlDelay(1000*ulDelayms);
        LCDClear();

    }
    else if(userStatus == EntryNotAllowed && cardStatus == CardDetected)
    {
        LCDNotAllowed();
        MAP_SysCtlDelay(1000*ulDelayms);
        LCDClear();

    }
    else if(userStatus == UserNotRegistered && cardStatus == CardDetected)
    {
        LCDNotRegister();
        MAP_SysCtlDelay(1000*ulDelayms);
        LCDClear();

    }
    else
    {
        LCDInicio();
        //MAP_SysCtlDelay(3000*ulDelayms);

    }
    LCDInicio();
    cardStatus = CardNotDetected;
    //MAP_SysCtlDelay(1000*ulDelayms);
}

void buscaCadastro()
{
    if(StrCompare(RFIDCardPassWord,RFIDCardPassWord1, 5))
    {
        userStatus = EntryAllowed;
    }
    else if(StrCompare(RFIDCardPassWord,RFIDCardPassWord2, 5))
    {
        userStatus = EntryNotAllowed;
    }
    else if(StrCompare(RFIDCardPassWord,RFIDCardPassWord3, 5))
    {
        userStatus = EntryAllowed;
    }
    else
    {
        userStatus = UserNotRegistered;
    }
    //cardStatus = CardNotDetected;
}
void InitConsole()
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

    UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);

    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    UARTStdioConfig(0, 115200, 16000000);
}
