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
 * Arquivo Principal
 * ************************************************************** */

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
#include "driverlib/interrupt.h"
#include "driverlib/lcd.h"
#include "driverlib/rfid.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/timer.h"

int led3s, ledConv;
int i, tempIndex;
bool conversionEnd;

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
		if (tempIndex <= N_MEDIAS - 1) {
			bufferTemp[tempIndex] = bufferCapture[0];
			tempIndex++;
		}

		if (tempIndex > N_MEDIAS - 1) {
			bufferConversao[indiceAmostra] = (float)(bufferTemp[0] + bufferTemp[1] + bufferTemp[2] + bufferTemp[3] + bufferTemp[4] + bufferTemp[5])/N_MEDIAS;
			bufferDatabase[indiceAmostra] = (float)(bufferTemp[0] + bufferTemp[1] + bufferTemp[2] + bufferTemp[3] + bufferTemp[4] + bufferTemp[5])/N_MEDIAS;
			indiceAmostra++;
			tempIndex = 0;
		}
	} else {
		MAP_TimerDisable(TIMER0_BASE, TIMER_A);
		MAP_ADCSequenceDisable(ADC0_BASE, 3);
		MAP_ADCIntDisable(ADC0_BASE, 3);
		MAP_IntDisable(INT_ADC0SS3);
		indiceAmostra = DELAY_MAX;
		conversionEnd = true;
		MAP_GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, 0);
		MAP_GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_3, 0);
	}

	return;
}

/*
 * Funções Auxiliares
 */
void systemInit() {
	MAP_SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480), 120000000);

	//SysCtlPeripheralPowerOn(SYSCTL_PERIPH_GPIOB);
	//SysCtlPeripheralPowerOn(SYSCTL_PERIPH_GPIOD);
	MAP_SysCtlPeripheralPowerOn(SYSCTL_PERIPH_GPIOE);
	//SysCtlPeripheralPowerOn(SYSCTL_PERIPH_GPIOG);
	//SysCtlPeripheralPowerOn(SYSCTL_PERIPH_GPIOK);
	//SysCtlPeripheralPowerOn(SYSCTL_PERIPH_GPIOL);
	//SysCtlPeripheralPowerOn(SYSCTL_PERIPH_GPIOM);
	MAP_SysCtlPeripheralPowerOn(SYSCTL_PERIPH_GPION);

	MAP_SysCtlPeripheralPowerOn(SYSCTL_PERIPH_TIMER0);
	MAP_SysCtlPeripheralPowerOn(SYSCTL_PERIPH_TIMER1);
	MAP_SysCtlPeripheralPowerOn(SYSCTL_PERIPH_ADC0);
	//SysCtlPeripheralPowerOn(SYSCTL_PERIPH_SSI2);
}

void GPIOInit() {
	//SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
	//SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
	MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
	//SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOG);
	//SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOK);
	//SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOL);
	//SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOM);
	MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);

	while (!MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_GPION)) {

	}

	MAP_GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);
	MAP_GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_5);
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

	MAP_IntMasterEnable();
}

void timerInit() {
	MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);

	while (!MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER0)) {

	}

	MAP_TimerConfigure(TIMER0_BASE, TIMER_CFG_A_PERIODIC_UP);
	TimerUpdateMode(TIMER0_BASE, TIMER_A, TIMER_UP_LOAD_IMMEDIATE);
	MAP_TimerClockSourceSet(TIMER0_BASE, TIMER_CLOCK_PIOSC);
	MAP_TimerLoadSet(TIMER0_BASE, TIMER_A, 332);
	MAP_TimerADCEventSet(TIMER0_BASE, TIMER_ADC_TIMEOUT_A);
	MAP_TimerControlTrigger(TIMER0_BASE, TIMER_A, true);

	MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);

	while (!MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER1)) {

	}

	MAP_TimerConfigure(TIMER1_BASE, TIMER_CFG_PERIODIC_UP);
	TimerUpdateMode(TIMER1_BASE, TIMER_BOTH, TIMER_UP_LOAD_IMMEDIATE);
	MAP_TimerClockSourceSet(TIMER1_BASE, TIMER_CLOCK_PIOSC);
	MAP_TimerLoadSet(TIMER1_BASE, TIMER_A, 48484848);
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
/*	MAP_ADCSequenceStepConfigure(ADC0_BASE, 2, 0, (ADC_CTL_IE | ADC_CTL_SHOLD_8 | ADC_CTL_CH8));
	MAP_ADCSequenceStepConfigure(ADC0_BASE, 2, 1, (ADC_CTL_IE | ADC_CTL_SHOLD_8 | ADC_CTL_CH8));
	MAP_ADCSequenceStepConfigure(ADC0_BASE, 2, 2, (ADC_CTL_IE | ADC_CTL_SHOLD_8 | ADC_CTL_CH8)); */
	MAP_ADCSequenceStepConfigure(ADC0_BASE, 3, 0, (ADC_CTL_IE | ADC_CTL_SHOLD_8 | ADC_CTL_CH8 | ADC_CTL_END));
	MAP_ADCSequenceEnable(ADC0_BASE, 3);
}
/*
void LCDInit() {
}

void RFIDInit() {
	//SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI2);
}
*/
/*
 * Loop Principal
 */

int main(void) {
	tempIndex = 0;
	led3s = 0;
	ledConv = 0;
	indiceAmostra = DELAY_MAX;
	conversionEnd = false;

	systemInit();
	GPIOInit();

	ADCInit();
	timerInit();
	interruptInit();

	MAP_TimerEnable(TIMER0_BASE, TIMER_A);
	MAP_TimerEnable(TIMER1_BASE, TIMER_A);

	while (1) {
		if (conversionEnd) {
			validate();
			conversionEnd = false;
		}
	}
	return 0;
}
