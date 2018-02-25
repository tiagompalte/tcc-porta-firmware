/* ************************************************************** *
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


#include "inc/hw_types.h"
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_nvic.h"
#include "inc/hw_gpio.h"
#include "driverlib/flash.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/emac.h"
#include "drivers/pinout.h"
#include "utils/cmdline.h"
#include "utils/locator.h"
#include "utils/flash_pb.h"
#include "utils/lwiplib.h"
#include "utils/uartstdio.h"
#include "utils/ustdlib.h"
#include "eth_client.h"
#include "json.h"
#include "communication.h"
#include "driverlib/Hardware.h"

#include <stdint.h>
#include <stdbool.h>
#include <math.h>

#include "inc/hw_adc.h"
#include "inc/hw_gpio.h"
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_nvic.h"
#include "inc/hw_sysctl.h"
#include "inc/hw_timer.h"
#include "inc/hw_types.h"

#include "driverlib/adc.h"
#include "driverlib/algorithm.h"
#include "driverlib/pin_map.h"
#include "driverlib/gpio.h"
#include "driverlib/Hardware.h"
#include "driverlib/interrupt.h"
#include "driverlib/lcd.h"
#include "driverlib/rfid.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/timer.h"
#include "driverlib/ssi.h"
#include "utils/wavfile.h"

#define GET         0
#define POST        1
#define POSTKEY     2
#define POSTACCESS  3
#define GETteste    4
#define errorConnection    -1
#define OK    200

#define KEYBOARD       "1234"
#define IDBOARD         "11"

int led3s, ledConv;
int i, tempIndex;
int conversionEnd;
int status;

/*
 * ISRs
 */
void ISR_Timer1A() {
    MAP_TimerIntClear(TIMER1_BASE, TIMER_TIMA_TIMEOUT);

    if (led3s == 0) {
        MAP_GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, GPIO_PIN_0);
        MAP_GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_2, GPIO_PIN_2);
        led3s = 1;
    } else {
        led3s = 0;
        MAP_GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, 0);
        MAP_GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_2, 0);
    }

    return;
}

void ISR_ADC0() {
    MAP_ADCIntClear(ADC0_BASE, 3);
    MAP_ADCSequenceDataGet(ADC0_BASE, 3, bufferCapture);

    if (ledConv == 0) {
        MAP_GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, GPIO_PIN_1);
        MAP_GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_3, GPIO_PIN_3);
        ledConv = 1;
    } else {
        ledConv = 0;
        MAP_GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, 0);
        MAP_GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_3, 0);
    }

    if (indiceAmostra <= NUM_AMOSTRAS + DELAY_MAX - 1) {
      bufferConversao[indiceAmostra] = (uint8_t) bufferCapture[0];
      bufferDatabase[indiceAmostra] = (uint8_t) bufferCapture[0];
      indiceAmostra++;
    } else {
      MAP_TimerDisable(TIMER0_BASE, TIMER_A);
      MAP_ADCSequenceDisable(ADC0_BASE, 3);
      MAP_ADCIntDisable(ADC0_BASE, 3);
      MAP_IntDisable(INT_ADC0SS3);
      indiceAmostra = DELAY_MAX;
      conversionEnd = 1;
      MAP_GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, 0);
      MAP_GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_3, 0);
    }

    return;
}

/*
 * Funï¿½ï¿½es Auxiliares
 */
void systemInit() {
	g_ui32SysClock = MAP_SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480), 120000000);

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
    MAP_SysCtlPeripheralPowerOn(SYSCTL_PERIPH_ADC0);
    MAP_SysCtlPeripheralPowerOn(SYSCTL_PERIPH_SSI2);
}

void GPIOInit() {
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI2);
    while(!MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_SSI2));
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    while(!MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOB));
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
    while(!MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOC));
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    while(!MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOD));
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    while(!MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF));
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOG);
    while(!MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOG));
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ);
    while(!MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOJ));
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOK);
    while(!MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOK));
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOL);
    while(!MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOL));
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOM);
    while(!MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOM));
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);

    while (!MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_GPION)) {

    }
    MAP_GPIOPinConfigure(GPIO_PD3_SSI2CLK);
    MAP_GPIOPinConfigure(GPIO_PD0_SSI2XDAT1);
    MAP_GPIOPinConfigure(GPIO_PD1_SSI2XDAT0); //MOSI

    MAP_GPIOPinTypeSSI(GPIO_PORTD_BASE, GPIO_PIN_1|GPIO_PIN_0|GPIO_PIN_3);
    
    MAP_GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE,GPIO_PIN_3|GPIO_PIN_2);
    MAP_GPIOPinTypeGPIOOutput(GPIO_PORTC_BASE,GPIO_PIN_4);
    MAP_GPIOPinTypeGPIOOutput(GPIO_PORTG_BASE,GPIO_PIN_0);
    MAP_GPIOPinTypeGPIOOutput(GPIO_PORTD_BASE,GPIO_PIN_7|GPIO_PIN_2);
    MAP_GPIOPinTypeGPIOOutput(GPIO_PORTM_BASE, GPIO_PIN_2|GPIO_PIN_1|GPIO_PIN_0);
    MAP_GPIOPinTypeGPIOOutput(GPIO_PORTK_BASE, GPIO_PIN_7|GPIO_PIN_6|GPIO_PIN_5|GPIO_PIN_4|GPIO_PIN_3|GPIO_PIN_2|GPIO_PIN_1|GPIO_PIN_0);
    MAP_GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);
    
    MAP_GPIOPinTypeGPIOInput(GPIO_PORTE_BASE, GPIO_PIN_3|GPIO_PIN_2|GPIO_PIN_1|GPIO_PIN_0);
    MAP_GPIOPinTypeGPIOInput(GPIO_PORTL_BASE, GPIO_PIN_3|GPIO_PIN_2|GPIO_PIN_1|GPIO_PIN_0);
    MAP_GPIOPinTypeGPIOInput(GPIO_PORTJ_BASE, GPIO_PIN_1|GPIO_PIN_0);
    
    MAP_GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_5);
}

void SSIInit()
{
    uint32_t initialData = 0;
    MAP_SSIIntClear(SSI2_BASE,SSI_TXEOT);
    MAP_SSIClockSourceSet(SSI2_BASE, SSI_CLOCK_SYSTEM);
    MAP_SSIAdvModeSet(SSI2_BASE, SSI_ADV_MODE_LEGACY);
    MAP_SSIConfigSetExpClk(SSI2_BASE, g_ui32SysClock, SSI_FRF_MOTO_MODE_0,
                               SSI_MODE_MASTER, 1000000, 16);
    MAP_SSIEnable(SSI2_BASE);

    //clear out any initial data that might be present in the RX FIFO
    while(MAP_SSIDataGetNonBlocking(SSI2_BASE, &initialData))
    {

    }
}


void interruptInit() {
    MAP_ADCIntDisable(ADC0_BASE, 3);
    MAP_IntDisable(INT_ADC0SS3);
    MAP_ADCIntClear(ADC0_BASE, 3);
    MAP_IntEnable(INT_ADC0SS3);
    MAP_ADCIntEnable(ADC0_BASE, 3);
    MAP_ADCIntEnableEx(ADC0_BASE, ADC_INT_SS3);

    TimerIntRegister(TIMER1_BASE, TIMER_A, ISR_Timer1A);
    MAP_TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);

    TimerIntRegister(TIMER4_BASE, TIMER_A, &BuzzerIntHandler);
    MAP_TimerIntEnable(TIMER4_BASE, TIMER_TIMA_TIMEOUT);
    MAP_IntEnable(INT_TIMER4A);
    
    TimerIntRegister(TIMER3_BASE, TIMER_A, &LCDIntHandler);
    MAP_TimerIntEnable(TIMER3_BASE, TIMER_TIMA_TIMEOUT);
    MAP_IntEnable(INT_TIMER3A);
    
    TimerIntRegister(TIMER5_BASE, TIMER_A, &PasswordIntHandler);
    MAP_TimerIntEnable(TIMER5_BASE, TIMER_TIMA_TIMEOUT);
    MAP_IntEnable(INT_TIMER5A);

    MAP_IntMasterEnable();
}

void timerInit() {
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);

    while (!MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER0)) {

    }

    MAP_TimerConfigure(TIMER0_BASE, TIMER_CFG_A_PERIODIC_UP);
    TimerUpdateMode(TIMER0_BASE, TIMER_A, TIMER_UP_LOAD_IMMEDIATE);
    MAP_TimerClockSourceSet(TIMER0_BASE, TIMER_CLOCK_PIOSC);
    MAP_TimerLoadSet(TIMER0_BASE, TIMER_A, 1011);
    MAP_TimerADCEventSet(TIMER0_BASE, TIMER_ADC_TIMEOUT_A);
    MAP_TimerControlTrigger(TIMER0_BASE, TIMER_A, true);

    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);

    while (!MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER1)) {

    }

    MAP_TimerConfigure(TIMER1_BASE, TIMER_CFG_PERIODIC_UP);
    TimerUpdateMode(TIMER1_BASE, TIMER_BOTH, TIMER_UP_LOAD_IMMEDIATE);
    MAP_TimerClockSourceSet(TIMER1_BASE, TIMER_CLOCK_PIOSC);
    MAP_TimerLoadSet(TIMER1_BASE, TIMER_A, 48484848);

/*    while (!MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER2)) {

    }

    MAP_TimerConfigure(TIMER2_BASE, TIMER_CFG_PERIODIC_UP);
    TimerUpdateMode(TIMER2_BASE, TIMER_BOTH, TIMER_UP_LOAD_IMMEDIATE);
    MAP_TimerClockSourceSet(TIMER2_BASE, TIMER_CLOCK_PIOSC);
    MAP_TimerLoadSet(TIMER2_BASE, TIMER_A, 145454544);*/
    
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER4);
    while(!MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER4));
    MAP_TimerConfigure(TIMER4_BASE, TIMER_CFG_A_ONE_SHOT);
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER3);
    while(!MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER3));
    MAP_TimerConfigure(TIMER3_BASE, TIMER_CFG_A_ONE_SHOT);
    MAP_TimerLoadSet(TIMER3_BASE, TIMER_A, 3 * g_ui32SysClock);

    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER5);
    while(!MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER5));

    MAP_TimerConfigure(TIMER5_BASE, TIMER_CFG_A_ONE_SHOT);
    MAP_TimerLoadSet(TIMER5_BASE, TIMER_A, 10 * ui32SysClock);

}

void ADCInit() {
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    MAP_SysCtlPeripheralReset(SYSCTL_PERIPH_ADC0);

    while (!MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_ADC0)) {

    }

    ADCClockConfigSet(ADC0_BASE, (ADC_CLOCK_SRC_PIOSC | ADC_CLOCK_RATE_FULL), 1);
    MAP_ADCPhaseDelaySet(ADC0_BASE, ADC_PHASE_0);
    MAP_ADCReferenceSet(ADC0_BASE, ADC_REF_INT);
    MAP_ADCSequenceDisable(ADC0_BASE, 3);
    MAP_ADCSequenceConfigure(ADC0_BASE, 3, ADC_TRIGGER_TIMER, 0);
    MAP_ADCSequenceStepConfigure(ADC0_BASE, 3, 0, (ADC_CTL_IE | ADC_CTL_SHOLD_8 | ADC_CTL_CH8 | ADC_CTL_END));
    MAP_ADCSequenceEnable(ADC0_BASE, 3);
}

#ifdef DEBUG
void
__error__(char *pcFilename, uint32_t ui32Line)
{
}
#endif

void SysTickIntHandler(void) {
    EthClientTick(SYSTEM_TICK_MS);
    if(g_ui32Delay != 0) {
        g_ui32Delay--;
    }
}



int
main(void)
{

    led3s = 0;
    ledConv = 0;
    indiceAmostra = DELAY_MAX;
    conversionEnd = 0;


    // Inicializar sistema
    systemInit();
    GPIOInit();
    SSIInit();
    // Inicializar perifÃˆricos
    ADCInit();
    timerInit();
    interruptInit();

    MAP_TimerEnable(TIMER0_BASE, TIMER_A); // Timer ADC

    /*
    //// TESTES
     tWavFile wavTeste;

    WavOpen("Default.wav", &wavTeste);
    WavRead(&wavTeste, bufferDatabase, 144000/6);
    WavClose(&wavTeste); */


    //HardwareInit();

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
	g_psUserInfo.sReport.userKey = "1234";
	g_psUserInfo.sReport.idBoard = IDBOARD;
	g_psUserInfo.sReport.keyBoard = KEYBOARD;
	//g_psUserInfo.sReport.rfid = "12345678";
	strcpy(g_psUserInfo.sReport.rfid, "12345678");
	//HardwareLoop();

	if (CommunicationConnecting(VOICE) == errorConnection) {
		status = -1;
	} else {
		status = 1;
        CommunicationLog();
	}


    while (1) {
            // AGUARDA VERIFICACAO DO RFID
            //MAP_TimerEnable(TIMER2_BASE, TIMER_A); // Timer 10s
            // tem 10 segundos pra enviar o cÃ›digo RFID e comeÃ�ar a receber os dados
            // Se o RFID deu ok, liberar timer pra comecar a receber a voz, mostrar msgs etc...
            // verificar se o usuario quer usar o teclado ao inves da voz...


          if (conversionEnd == 1) { // Quando terminar de fazer a conversâ€žo
                validate();
        	    if (validate()) { // Verificar a senha
                    // ABRIR A PORTA
                    //MAP_TimerEnable(TIMER1_BASE, TIMER_A); // Timer 5s
                    // 5s pra abrir a porta, se passar o tempo tranca de novo...
                } else {
                    // MENSAGEM DE ERRO
                }
                conversionEnd = 0;
          }
    }
}

