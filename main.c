﻿/* ************************************************************** *
 * UTFPR - Universidade Tecnologica Federal do ParanÃ¡
 * Engenharia Eletronica
 * Trabalho de Conclusao de Curso
 * ************************************************************** *
 * Sistema de Seguranca baseado em Reconhecimento de Senha Falada
 * ************************************************************** *
 * Equipe:
 * Luiz Felipe Kim Evaristo
 * Tauan Marinho
 * Tiago Henrique Faxina
 * Tiago Mariani Palte
 * ************************************************************** *
 * main.c
 * ************************************************************** */

#include <enet.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>

#include "eth_client.h"
#include "json.h"
#include "communication.h"

#include "inc/hw_adc.h"
#include "inc/hw_gpio.h"
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_nvic.h"
#include "inc/hw_sysctl.h"
#include "inc/hw_timer.h"
#include "inc/hw_types.h"

#include "driverlib/adc.h"
#include "driverlib/audioSample.h"
#include "driverlib/emac.h"
#include "driverlib/fir.h"
#include "driverlib/flash.h"
#include "driverlib/gpio.h"
#include "driverlib/Hardware.h"
#include "driverlib/interrupt.h"
#include "driverlib/lcd.h"
#include "driverlib/pin_map.h"
#include "driverlib/rfid.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/timer.h"
#include "driverlib/ssi.h"

#include "drivers/pinout.h"

#include "utils/cmdline.h"
#include "utils/locator.h"
#include "utils/flash_pb.h"
#include "utils/lwiplib.h"
#include "utils/uartstdio.h"
#include "utils/ustdlib.h"

#define GET                     0
#define POST                    1
#define POSTKEY                 2
#define POSTACCESS              3
#define GETteste                4
#define errorConnection        -1
#define ERROR                  -1
#define OK                      1
#define OK_connection         200

#define KEYBOARD       "1234"
#define IDBOARD         "11"

int led3s, ledConv;
int i, tempIndex;
int status;

char *str_noConnection = "Sem Conexao";
char *str_noServer = "Servidor Fora do Ar";
char *str_UserNotRegistered = "RFID NAO Cadastrado";
UserStatus uStatus = EntryNotAllowed;
/*
 * ISRs
 */
void ISR_Timer1A()
{
    MAP_TimerIntClear(TIMER1_BASE, TIMER_TIMA_TIMEOUT);

    if (led3s == 0)
    {
        MAP_GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, GPIO_PIN_0);
        MAP_GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_2, GPIO_PIN_2);
        led3s = 1;
    }
    else
    {
        led3s = 0;
        MAP_GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, 0);
        MAP_GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_2, 0);
    }

    return;
}

void ISR_ADC0()
{
    MAP_ADCIntClear(ADC0_BASE, 3);
    MAP_ADCSequenceDataGet(ADC0_BASE, 3, bufferCapture);

    if (indiceAmostra <= NUM_AMOSTRAS - 1)
    {
        bufferConversao[indiceAmostra] = bufferCapture[0];
        indiceAmostra++;
    }
    else
    {
        MAP_TimerDisable(TIMER0_BASE, TIMER_A);
        MAP_ADCSequenceDisable(ADC0_BASE, 3);
        MAP_ADCIntDisable(ADC0_BASE, 3);
        MAP_IntDisable(INT_ADC0SS3);
        indiceAmostra = 0;
        conversionEnd = 1;
    }

    return;
}

/*
 * Funï¿½ï¿½es Auxiliares
 */
void systemInit()
{
    g_ui32SysClock = MAP_SysCtlClockFreqSet(
            (SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL
                    | SYSCTL_CFG_VCO_480),
            120000000);

    MAP_SysCtlPeripheralPowerOn(SYSCTL_PERIPH_GPIOB);
    MAP_SysCtlPeripheralPowerOn(SYSCTL_PERIPH_GPIOD);
    MAP_SysCtlPeripheralPowerOn(SYSCTL_PERIPH_GPIOE);
    MAP_SysCtlPeripheralPowerOn(SYSCTL_PERIPH_GPIOG);
    MAP_SysCtlPeripheralPowerOn(SYSCTL_PERIPH_GPIOK);
    MAP_SysCtlPeripheralPowerOn(SYSCTL_PERIPH_GPIOL);
    MAP_SysCtlPeripheralPowerOn(SYSCTL_PERIPH_GPIOM);
    MAP_SysCtlPeripheralPowerOn(SYSCTL_PERIPH_GPION);

    MAP_SysCtlPeripheralPowerOn(SYSCTL_PERIPH_TIMER0);
    MAP_SysCtlPeripheralPowerOn(SYSCTL_PERIPH_TIMER1);
    MAP_SysCtlPeripheralPowerOn(SYSCTL_PERIPH_TIMER2);
    MAP_SysCtlPeripheralPowerOn(SYSCTL_PERIPH_TIMER3);
    MAP_SysCtlPeripheralPowerOn(SYSCTL_PERIPH_TIMER4);
    MAP_SysCtlPeripheralPowerOn(SYSCTL_PERIPH_ADC0);
    MAP_SysCtlPeripheralPowerOn(SYSCTL_PERIPH_SSI2);
}

void GPIOInit()
{
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI2);
    while (!MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_SSI2))
        ;
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    while (!MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOB))
        ;
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
    while (!MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOC))
        ;
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    while (!MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOD))
        ;
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    while (!MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOE))
        ;
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    while (!MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF))
        ;
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOG);
    while (!MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOG))
        ;
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ);
    while (!MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOJ))
        ;
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOK);
    while (!MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOK))
        ;
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOL);
    while (!MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOL))
        ;
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOM);
    while (!MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOM))
        ;
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);

    while (!MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_GPION))
    {

    }
    MAP_GPIOPinConfigure(GPIO_PD3_SSI2CLK);
    MAP_GPIOPinConfigure(GPIO_PD0_SSI2XDAT1);
    MAP_GPIOPinConfigure(GPIO_PD1_SSI2XDAT0); //MOSI

    //GPIOPinConfigure(GPIO_PA2_T1CCP0);

    MAP_GPIOPinTypeSSI(GPIO_PORTD_BASE, GPIO_PIN_1 | GPIO_PIN_0 | GPIO_PIN_3);

    MAP_GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, GPIO_PIN_3 | GPIO_PIN_2);
    MAP_GPIOPinTypeGPIOOutput(GPIO_PORTC_BASE, GPIO_PIN_4);
    MAP_GPIOPinTypeGPIOOutput(GPIO_PORTG_BASE, GPIO_PIN_0);
    MAP_GPIOPinTypeGPIOOutput(GPIO_PORTD_BASE, GPIO_PIN_7 | GPIO_PIN_2);
    MAP_GPIOPinTypeGPIOOutput(GPIO_PORTM_BASE,
                              GPIO_PIN_2 | GPIO_PIN_1 | GPIO_PIN_0);
    MAP_GPIOPinTypeGPIOOutput(
            GPIO_PORTK_BASE,
            GPIO_PIN_7 | GPIO_PIN_6 | GPIO_PIN_5 | GPIO_PIN_4 | GPIO_PIN_3
                    | GPIO_PIN_2 | GPIO_PIN_1 | GPIO_PIN_0);
    MAP_GPIOPinTypeGPIOOutput(
            GPIO_PORTN_BASE, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);

    MAP_GPIOPinTypeGPIOInput(GPIO_PORTE_BASE,
                             GPIO_PIN_3 | GPIO_PIN_2 | GPIO_PIN_1 | GPIO_PIN_0);
    MAP_GPIOPadConfigSet(GPIO_PORTE_BASE,
                         GPIO_PIN_3 | GPIO_PIN_2 | GPIO_PIN_1 | GPIO_PIN_0,
                         GPIO_STRENGTH_4MA, GPIO_PIN_TYPE_STD_WPU);

    MAP_GPIOPinTypeGPIOOutput(GPIO_PORTL_BASE,
                             GPIO_PIN_3 | GPIO_PIN_2 | GPIO_PIN_1 | GPIO_PIN_0);
    MAP_GPIOPinTypeGPIOInput(GPIO_PORTJ_BASE, GPIO_PIN_1 | GPIO_PIN_0);

    MAP_GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_5);
}

void SSIInit()
{
    uint32_t initialData = 0;
    MAP_SSIIntClear(SSI2_BASE, SSI_TXEOT);
    MAP_SSIClockSourceSet(SSI2_BASE, SSI_CLOCK_SYSTEM);
    MAP_SSIAdvModeSet(SSI2_BASE, SSI_ADV_MODE_LEGACY);
    MAP_SSIConfigSetExpClk(SSI2_BASE, g_ui32SysClock, SSI_FRF_MOTO_MODE_0,
    SSI_MODE_MASTER,
                           1000000, 16);
    MAP_SSIEnable(SSI2_BASE);

    //clear out any initial data that might be present in the RX FIFO
    while (MAP_SSIDataGetNonBlocking(SSI2_BASE, &initialData))
    {

    }
}

void interruptInit()
{
    MAP_ADCIntDisable(ADC0_BASE, 3);
    MAP_IntDisable(INT_ADC0SS3);
    MAP_ADCIntClear(ADC0_BASE, 3);
    MAP_IntEnable(INT_ADC0SS3);
    MAP_ADCIntEnable(ADC0_BASE, 3);
    MAP_ADCIntEnableEx(ADC0_BASE, ADC_INT_SS3);

    TimerIntRegister(TIMER1_BASE, TIMER_A, &LCDIntHandler);
    MAP_TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
    MAP_IntEnable(INT_TIMER1A);


    TimerIntRegister(TIMER2_BASE, TIMER_A, &PasswordIntHandler);
    MAP_TimerIntEnable(TIMER2_BASE, TIMER_TIMA_TIMEOUT);
    MAP_IntEnable(INT_TIMER2A);

    TimerIntRegister(TIMER3_BASE, TIMER_A, &BuzzerIntHandler);
    MAP_TimerIntEnable(TIMER3_BASE, TIMER_TIMA_TIMEOUT);
    MAP_IntEnable(INT_TIMER3A);

    TimerIntRegister(TIMER4_BASE, TIMER_A, &intHandlerTimer4);
    MAP_TimerIntEnable(TIMER4_BASE, TIMER_TIMA_TIMEOUT);
    MAP_IntEnable(INT_TIMER4A);

    MAP_IntMasterEnable();
}

void timerInit()
{
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);

    while (!MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER0))
    {

    }

    MAP_TimerConfigure(TIMER0_BASE, TIMER_CFG_A_PERIODIC_UP);
    TimerUpdateMode(TIMER0_BASE, TIMER_A, TIMER_UP_LOAD_IMMEDIATE);
    MAP_TimerClockSourceSet(TIMER0_BASE, TIMER_CLOCK_PIOSC);
    MAP_TimerLoadSet(TIMER0_BASE, TIMER_A, 1011);
    MAP_TimerADCEventSet(TIMER0_BASE, TIMER_ADC_TIMEOUT_A);
    MAP_TimerControlTrigger(TIMER0_BASE, TIMER_A, true);

    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);

    while (!MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER1))
    {

    }

    MAP_TimerConfigure(TIMER1_BASE, TIMER_CFG_A_ONE_SHOT);
    MAP_TimerClockSourceSet(TIMER1_BASE, TIMER_CLOCK_SYSTEM);

    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER2);
    while (!MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER2))
    {

    }
    MAP_TimerConfigure(TIMER2_BASE, TIMER_CFG_A_ONE_SHOT);
    MAP_TimerClockSourceSet(TIMER2_BASE, TIMER_CLOCK_SYSTEM);

    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER3);
    while (!MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER3))
    {

    }
    MAP_TimerConfigure(TIMER3_BASE, TIMER_CFG_A_ONE_SHOT);
    MAP_TimerClockSourceSet(TIMER3_BASE, TIMER_CLOCK_SYSTEM);

    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER4);
    while (!MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER4))
    {

    }
    MAP_TimerConfigure(TIMER4_BASE, TIMER_CFG_A_PERIODIC);
    MAP_TimerLoadSet(TIMER4_BASE, TIMER_A, 1 * g_ui32SysClock);
    MAP_TimerClockSourceSet(TIMER4_BASE, TIMER_CLOCK_SYSTEM);

}

void ADCInit()
{
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    MAP_SysCtlPeripheralReset(SYSCTL_PERIPH_ADC0);

    while (!MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_ADC0))
    {

    }

    ADCClockConfigSet(ADC0_BASE, (ADC_CLOCK_SRC_PIOSC | ADC_CLOCK_RATE_FULL),
                      1);
    MAP_ADCPhaseDelaySet(ADC0_BASE, ADC_PHASE_0);
    MAP_ADCReferenceSet(ADC0_BASE, ADC_REF_INT);
    MAP_ADCSequenceDisable(ADC0_BASE, 3);
    MAP_ADCSequenceConfigure(ADC0_BASE, 3, ADC_TRIGGER_TIMER, 0);
    MAP_ADCSequenceStepConfigure(
            ADC0_BASE, 3, 0,
            (ADC_CTL_IE | ADC_CTL_SHOLD_8 | ADC_CTL_CH8 | ADC_CTL_END));
    MAP_ADCSequenceEnable(ADC0_BASE, 3);
}

#ifdef DEBUG
void
__error__(char *pcFilename, uint32_t ui32Line)
{
}
#endif

void SysTickIntHandler(void)
{
    EthClientTick(SYSTEM_TICK_MS);
    if (g_ui32Delay != 0)
    {
        g_ui32Delay--;
    }
}

/*
 * SEQUENCIA DO LOOP PRINCIPAL
 * 1. IDLE
 * 2. RFID DETECTADO
 * 3. INICIA COMUNICACAO P VERIFICAR SE USUARIO EXISTE
 * 4. SE USUARIO NAO ENCONTRADO -> MSG ERRO
 *    SE USUARIO ENCONTRADO -> INICIAR COLETA DE AUDIO
 * 5. PROCESSAR AUDIO
 * 6. SE AUDIO CONFIRMADO -> ACIONAR TRANCA
 *    SE AUDIO NAO CONFIRMADO -> MSG ERRO
 * 7. TIMEOUT DE 10SEGS P REACIONAR A TRANCA
 * 8. IDLE
 *
 */

void principalLoop()
{
    HardwareLoop(); //adquire o rfid

    if (CommunicationConnecting() == ERROR)
    {
        if (g_psUserInfo.sReport.status == 400)
        {
            strcpy(g_psUserInfo.sReport.log, str_noConnection);
            LCDErroLog();
            MAP_SysCtlDelay(3000*ulDelayms);
            //LCDNotAllowed();
            return;
        }
        else if (g_psUserInfo.sReport.status == 404)
        {
            strcpy(g_psUserInfo.sReport.log, str_noServer);
            LCDErroLog();
            MAP_SysCtlDelay(3000*ulDelayms);
            //LCDNotAllowed();
            return;
        }
        else
        {
            strcpy(g_psUserInfo.sReport.log, str_noConnection);
            LCDErroLog();
            MAP_SysCtlDelay(3000*ulDelayms);
            //LCDNotAllowed();
            return;
        }
    }
    else
    {
        strcpy(g_psUserInfo.sReport.log, "Codigo: 200");
    }

    //Display: Digite 0 para voz e 1 para senha
    //Leitura do teclado

    int status = NONE;
    status = hardwareVoiceKey();
    //printf("\n %c", status);
    //status = VOICE;
    if (status == ERROR)
    {
        return;    //Opção não digitada
    }
    else
    {
        HardwarePassWordControl(status);
    }

    if (CommunicationSending(status) == ERROR)
    {
        //strcpy(g_psUserInfo.sReport.log, str_UserNotRegistered);
        uStatus = EntryNotAllowed;
        HardwareControl(uStatus);
        return;
        //imprimir erro de log
    }
    else
    {
        uStatus = EntryAllowed;
        HardwareControl(uStatus);

        //Tudo certo

    }
}

int main(void)
{
    led3s = 0;
    ledConv = 0;
    indiceAmostra = 0;
    conversionEnd = 0;

    // Inicializar sistema
    systemInit();
    GPIOInit();
    SSIInit();
    // Inicializar perifericos
    ADCInit();
    timerInit();
    interruptInit();
    HardwareInit(IDBOARD);
    PinoutSet(true, false);

    SysTickPeriodSet((g_ui32SysClock / 1000) * SYSTEM_TICK_MS);
    SysTickEnable();
    SysTickIntEnable();
    FlashPBInit(FLASH_PB_START, FLASH_PB_END, 256);

    ResetUser();

    UpdateIPAddress(g_pcIPAddr, 0);

    IntPriorityGroupingSet(4);
    IntPrioritySet(INT_EMAC0, ETHERNET_INT_PRIORITY);
    IntPrioritySet(FAULT_SYSTICK, SYSTICK_INT_PRIORITY);

    EthClientProxySet(0);
    EthClientInit(EnetEvents);

    g_ui32IPaddr = EthClientAddrGet();

    strcpy(g_psUserInfo.sReport.idBoard, IDBOARD);
    strcpy(g_psUserInfo.sReport.keyBoard, KEYBOARD);
    //strcpy(g_psUserInfo.sReport.rfid, "C9 42 6A 7B");

    //strcpy(g_psUserInfo.sReport.userKey, "1234");
    //printf("\n Lendo teclado %d",g_ui32SysClock);
    //Ligando o equipamento, inicia o display
    LCDInicio();
    while (1)
    {
        principalLoop();
        LCDPassRFID();

    }

}

