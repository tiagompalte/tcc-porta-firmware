/* ************************************************************** *
 * UTFPR - Universidade Tecnologica Federal do Paraná
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


#define GET         0
#define POST        1
#define POSTKEY     2
#define POSTACCESS  3
#define GETteste    4

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
 * Fun��es Auxiliares
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
    MAP_TimerLoadSet(TIMER0_BASE, TIMER_A, 337);
    MAP_TimerADCEventSet(TIMER0_BASE, TIMER_ADC_TIMEOUT_A);
    MAP_TimerControlTrigger(TIMER0_BASE, TIMER_A, true);

    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);

    while (!MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER1)) {

    }

    MAP_TimerConfigure(TIMER1_BASE, TIMER_CFG_PERIODIC_UP);
    TimerUpdateMode(TIMER1_BASE, TIMER_BOTH, TIMER_UP_LOAD_IMMEDIATE);
    MAP_TimerClockSourceSet(TIMER1_BASE, TIMER_CLOCK_PIOSC);
    MAP_TimerLoadSet(TIMER1_BASE, TIMER_A, 72727272);

    while (!MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER2)) {

    }

    MAP_TimerConfigure(TIMER2_BASE, TIMER_CFG_PERIODIC_UP);
    TimerUpdateMode(TIMER2_BASE, TIMER_BOTH, TIMER_UP_LOAD_IMMEDIATE);
    MAP_TimerClockSourceSet(TIMER2_BASE, TIMER_CLOCK_PIOSC);
    MAP_TimerLoadSet(TIMER2_BASE, TIMER_A, 145454544);
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

//*****************************************************************************
//
// Connection states for weather application.
//
//*****************************************************************************
volatile enum
{
    STATE_NOT_CONNECTED,
        STATE_NEW_CONNECTION,
        STATE_CONNECTED_IDLE,
        STATE_WAIT_DATA,
        STATE_UPDATE_USER,
        STATE_WAIT_NICE,
}
g_iState = STATE_NOT_CONNECTED;

//*****************************************************************************
//
// Global for IP address.
//
//*****************************************************************************
uint32_t g_ui32IPaddr;

//*****************************************************************************
//
// Global to track whether we are processing commands or not.
//
//*****************************************************************************
uint32_t g_ui32ProcessingCmds;

//*****************************************************************************
//
// The delay count to reduce traffic to the weather server.
//
//*****************************************************************************
volatile uint32_t g_ui32Delay;

//*****************************************************************************
//
// Global to track number of times the app has cycled through the list of
// cities.
//
//*****************************************************************************
volatile uint32_t g_ui32Cycles;



//*****************************************************************************
//
// System Clock rate in Hertz.
//
//*****************************************************************************
uint32_t g_ui32SysClock;

//*****************************************************************************
//




//*****************************************************************************
//
// The state of each city panel.
//
//*****************************************************************************
struct
{
    //
    // The last update time for this city.
    //
    uint32_t ui32LastUpdate;

    //
    // The current weather report data for this city.
    //
    tWeatherReport sReport;

    //
    // The name of the current city.
    //
    const char *pcName;
}
g_psUserInfo;



//*****************************************************************************
//
// Constant strings for status messages.
//
//*****************************************************************************
const char g_pcNotFound[] = "City Not Found";
const char g_pcServerBusy[] = "Server Busy";
const char g_pcWaitData[] = "Waiting for Data";

//*****************************************************************************
//
// Interrupt priority definitions.  The top 3 bits of these values are
// significant with lower values indicating higher priority interrupts.
//
//*****************************************************************************
#define SYSTICK_INT_PRIORITY    0x80
#define ETHERNET_INT_PRIORITY   0xC0

//*****************************************************************************
//
// The error routine that is called if the driver library encounters an error.
//
//*****************************************************************************
#ifdef DEBUG
void
__error__(char *pcFilename, uint32_t ui32Line)
{
}
#endif



//*****************************************************************************
//
// MAC address.
//
//*****************************************************************************
char g_pcMACAddr[40];

//*****************************************************************************
//
// IP address.
//
//*****************************************************************************
char g_pcIPAddr[20];



//*****************************************************************************
//
// Reset a user information so that it can be properly updated.
//
//*****************************************************************************
void
ResetUser(uint32_t ui32Idx)
{
    g_psUserInfo.sReport.pcDescription = 0;
    g_psUserInfo.sReport.audio = 0;
    g_psUserInfo.sReport.novo = 0;
    g_psUserInfo.sReport.senha = 0;
    g_psUserInfo.ui32LastUpdate = 0;
}

//*****************************************************************************
//
// Update the IP address string.
//
//*****************************************************************************
void
UpdateIPAddress(char *pcAddr, uint32_t ipAddr)
{
    uint8_t *pui8Temp = (uint8_t *)&ipAddr;

    if(ipAddr == 0)
    {
        ustrncpy(pcAddr, "IP: ---.---.---.---", sizeof(g_pcIPAddr));
    }
    else
    {
        usprintf(pcAddr,"IP: %d.%d.%d.%d", pui8Temp[0], pui8Temp[1],
                pui8Temp[2], pui8Temp[3]);
    }
}

//*****************************************************************************
//
// The weather event handler.
//
//*****************************************************************************
void
WeatherEvent(uint32_t ui32Event, void* pvData, uint32_t ui32Param)
{

    if(ui32Event == ETH_EVENT_RECEIVE)
    {
        //
        // Let the main loop update the city.
        //
        g_iState = STATE_UPDATE_USER;

        g_psUserInfo.ui32LastUpdate =
            g_psUserInfo.sReport.ui32Time;
    }
    else if(ui32Event == ETH_EVENT_INVALID_REQ)
    {
        g_psUserInfo.sReport.pcDescription = g_pcNotFound;
        g_iState = STATE_UPDATE_USER;
    }
    else if(ui32Event == ETH_EVENT_CLOSE)
    {
        if(g_iState == STATE_WAIT_DATA)
        {
            g_psUserInfo.sReport.pcDescription =
                g_pcServerBusy;

            g_iState = STATE_UPDATE_USER;
        }
    }

    //
    // If the update indicated no time, then just set the time to a value
    // to indicate that at least the update occurred.
    //
    if(g_psUserInfo.ui32LastUpdate == 0)
    {
        //
        // Failed to update for some reason.
        //
        g_psUserInfo.ui32LastUpdate = 1;
    }
}

//*****************************************************************************
//
// The interrupt handler for the SysTick interrupt.
//
//*****************************************************************************
void
SysTickIntHandler(void)
{
    //
    // Call the lwIP timer handler.
    //
    EthClientTick(SYSTEM_TICK_MS);

    //
    // Handle the delay between accesses to the weather server.
    //
    if(g_ui32Delay != 0)
    {
        g_ui32Delay--;
    }
}

//*****************************************************************************
//
// Network events handler.
//
//*****************************************************************************
void
EnetEvents(uint32_t ui32Event, void *pvData, uint32_t ui32Param)
{
    if(ui32Event == ETH_EVENT_CONNECT)
    {
        g_iState = STATE_NEW_CONNECTION;

        //
        // Update the string version of the address.
        //
        UpdateIPAddress(g_pcIPAddr, EthClientAddrGet());
    }
    else if(ui32Event == ETH_EVENT_DISCONNECT)
    {
        //
        // If a city was waiting to be updated then reset its data.
        //
        if(g_iState != STATE_CONNECTED_IDLE)
        {
            ResetUser(0);
        }

        g_iState = STATE_NOT_CONNECTED;

        //
        // Update the address to 0.0.0.0.
        //
        UpdateIPAddress(g_pcIPAddr, 0);
    }
}


int Communication (int request, char* size, char* code, char* key, char *rfid){
    enum
    {
        eRequestIdle,
        eRequestUpdate,
        eRequestGET,
        eRequestPOST,
        eRequestPOSTKEY,
        eRequestPOSTACCESS,
        eRequestGETteste,
        eRequestComplete,
    }
    iRequest;
    iRequest = eRequestIdle;

    while(1)
    {
        if(g_iState == STATE_NEW_CONNECTION)
        {
            iRequest = eRequestIdle;

            g_iState = STATE_CONNECTED_IDLE;
        }
        else if(g_iState == STATE_CONNECTED_IDLE)
        {
            if(iRequest == eRequestIdle)
            {
                if (request == GET){
                    iRequest = eRequestGET; //--- para GET
                } else if (request == POST){
                    iRequest = eRequestPOST;// Para POST
                } else if (request == POSTKEY){
                    iRequest = eRequestPOSTKEY;// Para POSTKEY
                } else if (request == POSTACCESS){
                    iRequest = eRequestPOSTACCESS;// Para POSTACCESS
                } else if (request == GETteste){
                    iRequest = eRequestGETteste;// Para GETteste
                }
            }
            else if(iRequest == eRequestGETteste)
            {


                            g_iState = STATE_WAIT_DATA;

                            //
                            // Timeout at 10 seconds.
                            //
                            g_ui32Delay = 1000;



                           requestGETteste(iWSrcOpenWeatherMap,
                                    g_psUserInfo.pcName,
                                    &g_psUserInfo.sReport,
                                    WeatherEvent);

                            iRequest = eRequestUpdate;
            }
            else if(iRequest == eRequestGET)
            {


                g_iState = STATE_WAIT_DATA;

                //
                // Timeout at 10 seconds.
                //
                g_ui32Delay = 1000;



               requestGET(iWSrcOpenWeatherMap,
                        g_psUserInfo.pcName,
                        &g_psUserInfo.sReport,
                        WeatherEvent, rfid);

                iRequest = eRequestUpdate;
            }
            else if(iRequest == eRequestPOST)
            {
                g_iState = STATE_WAIT_DATA;

                //
                // Timeout at 10 seconds.
                //
                g_ui32Delay = 1000;

                requestPOST(iWSrcOpenWeatherMap,
                        g_psUserInfo.pcName,
                        &g_psUserInfo.sReport,
                        WeatherEvent, size, code, key);

                iRequest = eRequestUpdate;
            }
            else if(iRequest == eRequestPOSTKEY)
                        {
                            g_iState = STATE_WAIT_DATA;

                            //
                            // Timeout at 10 seconds.
                            //
                            g_ui32Delay = 1000;

                            requestPOSTKEY(iWSrcOpenWeatherMap,
                                    g_psUserInfo.pcName,
                                    &g_psUserInfo.sReport,
                                    WeatherEvent, size, code, key);

                            iRequest = eRequestUpdate;
                        }
            else if(iRequest == eRequestPOSTACCESS)
                        {
                            g_iState = STATE_WAIT_DATA;

                            //
                            // Timeout at 10 seconds.
                            //
                            g_ui32Delay = 1000;

                            requestPOSTACCESS(iWSrcOpenWeatherMap,
                                    g_psUserInfo.pcName,
                                    &g_psUserInfo.sReport,
                                    WeatherEvent, size, code, key, rfid);

                            iRequest = eRequestUpdate;
                        }
            else if(iRequest == eRequestUpdate)
            {
                //
                // Return to the idle request state.
                //
                iRequest = eRequestIdle;


            }
        }
        else if(g_iState == STATE_UPDATE_USER)
        {

            //
            // Go to the wait nice state.
            //
            g_iState = STATE_WAIT_NICE;

            //
            // 10ms * 10 is a 1 second delay between updates.
            //
            g_ui32Delay = SYSTEM_TICK_MS * 10;
        }
        else if(g_iState == STATE_WAIT_NICE)
        {
            //
            // Wait for the "nice" delay to not hit the server too often.
            //
            if(g_ui32Delay == 0)
            {
                g_iState = STATE_CONNECTED_IDLE;
            }
        }
        else if(g_iState == STATE_WAIT_DATA)
        {
            //
            // If no data has been received by the timeout then close the
            // connection.
            //
            if(g_ui32Delay == 0)
            {
                //
                // Close out the current connection.
                //
                EthClientTCPDisconnect();
                return 0;
            }
        }

    }
}

int loginServer(){
    //Communication(GETteste);
    //g_iState == STATE_NEW_CONNECTION;
    //g_ui32Delay = 500;
    //Communication(GET);
    return 1;
}


int
main(void)
{


    //
    // Make sure the main oscillator is enabled because this is required by
    // the PHY.  The system must have a 25MHz crystal attached to the OSC
    // pins.  The SYSCTL_MOSC_HIGHFREQ parameter is used when the crystal
    // frequency is 10MHz or higher.
    //
    SysCtlMOSCConfigSet(SYSCTL_MOSC_HIGHFREQ);

    //
    // Run from the PLL at 120 MHz.
    //
    g_ui32SysClock = MAP_SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |
                SYSCTL_OSC_MAIN |
                SYSCTL_USE_PLL |
                SYSCTL_CFG_VCO_480), 120000000);

    //
    // Configure the device pins.
    //
    PinoutSet(true, false);


    //
    // Configure SysTick for a periodic interrupt at 10ms.
    //
    SysTickPeriodSet((g_ui32SysClock / 1000) * SYSTEM_TICK_MS);
    SysTickEnable();
    SysTickIntEnable();

    //
    // Initialized the flash program block and read the current settings.
    //
    FlashPBInit(FLASH_PB_START, FLASH_PB_END, 256);



        ResetUser(0);


    //
    // Set the IP address to 0.0.0.0.
    //
    UpdateIPAddress(g_pcIPAddr, 0);

    //
    // Enable processor interrupts.
    //
    IntMasterEnable();

    //
    // Set the interrupt priorities.  We set the SysTick interrupt to a higher
    // priority than the Ethernet interrupt to ensure that the file system
    // tick is processed if SysTick occurs while the Ethernet handler is being
    // processed.  This is very likely since all the TCP/IP and HTTP work is
    // done in the context of the Ethernet interrupt.
    //
    IntPriorityGroupingSet(4);
    IntPrioritySet(INT_EMAC0, ETHERNET_INT_PRIORITY);
    IntPrioritySet(FAULT_SYSTICK, SYSTICK_INT_PRIORITY);

        EthClientProxySet(0);
        EthClientInit(EnetEvents);

    //
    // Get the IP address.
    //
    g_ui32IPaddr = EthClientAddrGet();

    HardwareInit();
   // Communication(GETteste, "38", "2","TZgJS", "1111");
    //loginServer();

}

