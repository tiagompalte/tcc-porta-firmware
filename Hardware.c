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
#include "driverlib/buzzer.h"
#include "driverlib/MyString.h"
#include "driverlib/trava_eletrica.h"

unsigned char RFIDCardPassWord[5];
unsigned char RFIDCardPassWord1[] = {3, 67, 147, 229, 54};
unsigned char RFIDCardPassWord2[] = {117, 224, 6, 136, 27};
unsigned char RFIDCardPassWord3[] = {201, 66, 106, 123, 54};
CardStatus cardStatus = CardNotDetected;
UserStatus userStatus = EntryNotAllowed;
static volatile bool g_bIntFlag = true;
static volatile bool TestFlag = false;
void HardwareInit()
{

    ui32SysClock = MAP_SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480), 120000000);
    ulDelayms = (ui32SysClock / 3000);
    uint32_t initialData = 0;

    InitConsole();
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI2);
    while(!MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_SSI2));
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    while(!MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOB));
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
    while(!MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOC));
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    while(!MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOD));
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOG);
    while(!MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOG));
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOK);
    while(!MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOK));
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOM);
    while(!MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOM));
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);
    while(!MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_GPION));

    MAP_GPIOPinConfigure(GPIO_PD3_SSI2CLK);
    MAP_GPIOPinConfigure(GPIO_PD0_SSI2XDAT1);
    MAP_GPIOPinConfigure(GPIO_PD1_SSI2XDAT0); //MOSI

    MAP_GPIOPinTypeSSI(GPIO_PORTD_BASE, GPIO_PIN_1|GPIO_PIN_0|GPIO_PIN_3);

    MAP_GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE,GPIO_PIN_3|GPIO_PIN_2);
    MAP_GPIOPinTypeGPIOOutput(GPIO_PORTC_BASE,GPIO_PIN_4);
    MAP_GPIOPinTypeGPIOOutput(GPIO_PORTG_BASE,GPIO_PIN_0);
    MAP_GPIOPinTypeGPIOOutput(GPIO_PORTD_BASE,GPIO_PIN_7|GPIO_PIN_2);
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

    MAP_GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_2, 0x00);
    MAP_GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_2, 0x04);
    MAP_GPIOPinWrite(GPIO_PORTG_BASE, GPIO_PIN_0, 1);
    MAP_GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1|GPIO_PIN_0, 2);
    MAP_SysCtlDelay(50*ulDelayms);

    LCDInit();
    MFRC522Init();
    InitTimer();
    DesacionarTrava();
    BuzzerDeactivate();
}

void InitTimer()
{
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    while(!MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER0));

    MAP_TimerConfigure(TIMER0_BASE, TIMER_CFG_A_ONE_SHOT);
    //MAP_TimerLoadSet(TIMER3_BASE, TIMER_A, 3 * ui32SysClock);
    MAP_IntMasterEnable();
    TimerIntRegister(TIMER0_BASE, TIMER_A, &BuzzerIntHandler);

    MAP_TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    MAP_IntEnable(INT_TIMER0A);
    MAP_TimerEnable(TIMER0_BASE, TIMER_A);

    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER3);
    while(!MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER3));

    //
    // The Timer3 peripheral must be enabled for use.
    //
    MAP_TimerConfigure(TIMER3_BASE, TIMER_CFG_A_ONE_SHOT);
    MAP_TimerLoadSet(TIMER3_BASE, TIMER_A, 3 * ui32SysClock);

    //
    // Enable processor interrupts.
    //
    //MAP_IntMasterEnable();
    TimerIntRegister(TIMER3_BASE, TIMER_A, &LCDIntHandler);
    //
    // Configure the Timer3B interrupt for timer timeout.
    //
    MAP_TimerIntEnable(TIMER3_BASE, TIMER_TIMA_TIMEOUT);

    //
    // Enable the Timer3B interrupt on the processor (NVIC).
    //
    MAP_IntEnable(INT_TIMER3A);

    //
    // Enable Timer0B.
    //
    MAP_TimerEnable(TIMER3_BASE, TIMER_A);
}


void LCDIntHandler(void)
{
  MAP_TimerIntClear(TIMER3_BASE, TIMER_TIMA_TIMEOUT);

  //MAP_GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, 0);
  //
  // Set a flag to indicate that the interrupt occurred.
  //
  //MAP_SysCtlDelay(1000*ulDelayms);
  //MAP_GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, 1);
  LCDClear();
  LCDInicio();
  DesacionarTrava();
  MAP_GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1|GPIO_PIN_0, 2);
  MAP_GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_2, 0x00);
  //cardStatus = CardNotDetected;
  //userStatus = EntryNotAllowed;
  g_bIntFlag = true;
  //MAP_TimerEnable(TIMER3_BASE, TIMER_A);
}

void BuzzerIntHandler(void)
{
  MAP_TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
  BuzzerDeactivate();

}

void HardwareLoop()
{
    HardwareInit();
    while(1)
    {
        HardwareRFID();
        HardwareControl();
    }

}
void HardwareRFID()
{
    unsigned char status;
    unsigned char str[MAX_LEN];


    if (!MFRC522IsCard())
    {
        cardStatus = CardNotDetected;
        return;
    }
    cardStatus = CardDetected;
    status = MFRC522Anticoll(str);
    memcpy(RFIDCardPassWord, str, 5);
    if (status == MI_OK)
    {
        buscaCadastro();
    }

    MFRC522Halt();
}

void HardwareControl()
{
    if(userStatus == EntryAllowed && cardStatus == CardDetected && g_bIntFlag == true)
    {
        g_bIntFlag = false;
        LCDAllowed();
        AcionarTrava();
        BuzzerActivate();
        MAP_GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_2, 0x04);
        MAP_TimerLoadSet(TIMER0_BASE, TIMER_A, 0.5 * ui32SysClock);
        MAP_TimerEnable(TIMER0_BASE, TIMER_A);
        MAP_TimerEnable(TIMER3_BASE, TIMER_A);
    }
    else if(userStatus == EntryNotAllowed && cardStatus == CardDetected && g_bIntFlag == true)
    {
        g_bIntFlag = false;
        LCDNotAllowed();
        BuzzerActivate();
        MAP_TimerLoadSet(TIMER0_BASE, TIMER_A, 1 * ui32SysClock);
        MAP_TimerEnable(TIMER0_BASE, TIMER_A);
        MAP_TimerEnable(TIMER3_BASE, TIMER_A);
    }
    else if(userStatus == UserNotRegistered && cardStatus == CardDetected && g_bIntFlag == true)
    {
        g_bIntFlag = false;
        LCDNotRegister();
        BuzzerActivate();
        MAP_TimerLoadSet(TIMER0_BASE, TIMER_A, 1 * ui32SysClock);
        MAP_TimerEnable(TIMER0_BASE, TIMER_A);
        MAP_TimerEnable(TIMER3_BASE, TIMER_A);
    }
    else if(cardStatus == CardNotDetected && g_bIntFlag == true)
    {
        //g_bIntFlag = false;
    }

    cardStatus = CardNotDetected;
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
