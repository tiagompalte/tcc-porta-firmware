/* ************************************************************** *
 * UTFPR - Universidade Tecnologica Federal do Paraná
 * Engenharia Eletrônica
 * Trabalho de Conclusão de Curso
 * ************************************************************** *
 * Sistema de Seguranca baseado em Reconhecimento de Senha Falada
 * ************************************************************** *
 * Equipe:
 * Luiz Felipe Kim Evaristo
 * Tauan Marinho
 * Tiago Henrique Faxina
 * Tiago Mariani Palte
 * ************************************************************** *
 * communication.c - Prototypes for the driver for the communication.
 * ************************************************************** */
#include "communication.h"
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


//*****************************************************************************
//
// Constant strings for status messages.
//
//*****************************************************************************
const char g_pcNotFound[] = "User Not Found";
const char g_pcServerBusy[] = "Server Busy";
const char g_pcWaitData[] = "Waiting for Data";


//*****************************************************************************
//
// Connection states.
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
} g_iState = STATE_NOT_CONNECTED;



//*****************************************************************************
//
// MAC address.
//
//*****************************************************************************
char g_pcMACAddr[40];

//*****************************************************************************
//
// Reset a user information so that it can be properly updated.
//
//*****************************************************************************
int ResetUser()
{
    g_psUserInfo.sReport.pcDescription = 0;
    //g_psUserInfo.sReport.audio[0] = ' ';
    g_psUserInfo.sReport.status = 0;
    g_psUserInfo.sReport.rfid[0] = ' ';
    g_psUserInfo.ui32LastUpdate = 0;

    return OK;
}

int ResetStatus()
{
    g_psUserInfo.sReport.pcDescription = 0;
    g_psUserInfo.sReport.status = 0;
    g_psUserInfo.ui32LastUpdate = 0;

    return 0;
}

//*****************************************************************************
//
// Update the IP address string.
//
//*****************************************************************************
void UpdateIPAddress(char *pcAddr, uint32_t ipAddr)
{
    uint8_t *pui8Temp = (uint8_t *) &ipAddr;

    if (ipAddr == 0)
    {
        ustrncpy(pcAddr, "IP: ---.---.---.---", sizeof(g_pcIPAddr));
    }
    else
    {
        usprintf(pcAddr, "IP: %d.%d.%d.%d", pui8Temp[0], pui8Temp[1],
                 pui8Temp[2], pui8Temp[3]);
    }
}

//*****************************************************************************
//
// The communication event handler.
//
//*****************************************************************************
void CommunicationEvent(uint32_t ui32Event, void* pvData, uint32_t ui32Param)
{

    if (ui32Event == ETH_EVENT_RECEIVE)
    {
        g_iState = STATE_UPDATE_USER;

        g_psUserInfo.ui32LastUpdate = g_psUserInfo.sReport.ui32Time;
    }
    else if (ui32Event == ETH_EVENT_INVALID_REQ)
    {
        g_psUserInfo.sReport.pcDescription = g_pcNotFound;
        g_iState = STATE_UPDATE_USER;
    }
    else if (ui32Event == ETH_EVENT_CLOSE)
    {
        if (g_iState == STATE_WAIT_DATA)
        {
            g_psUserInfo.sReport.pcDescription = g_pcServerBusy;

            g_iState = STATE_UPDATE_USER;
        }
    }

    //
    // If the update indicated no time, then just set the time to a value
    // to indicate that at least the update occurred.
    //
    if (g_psUserInfo.ui32LastUpdate == 0)
    {
        //
        // Failed to update for some reason.
        //
        g_psUserInfo.ui32LastUpdate = 1;
    }
}


//*****************************************************************************
//
// Network events handler.
//
//*****************************************************************************
void EnetEvents(uint32_t ui32Event, void *pvData, uint32_t ui32Param)
{
    if (ui32Event == ETH_EVENT_CONNECT)
    {
        g_iState = STATE_NEW_CONNECTION;

        //
        // Update the string version of the address.
        //
        UpdateIPAddress(g_pcIPAddr, EthClientAddrGet());
    }
    else if (ui32Event == ETH_EVENT_DISCONNECT)
    {
        //
        // If a USER was waiting to be updated then reset its data.
        //
        if (g_iState != STATE_CONNECTED_IDLE)
        {
            ResetUser();
        }

        g_iState = STATE_NOT_CONNECTED;

        //
        // Update the address to 0.0.0.0.
        //
        UpdateIPAddress(g_pcIPAddr, 0);
    }
}

//*****************************************************************************
//
// Communication send the respective request to the server
//
// \param request is the int used set what kind of request is gonna be
// Put GET on the parameter for a GET request
// Put POST on the parameter for a POST request
// Put POSTACCESS on the parameter for a POSTACCESS request
// Put POSTKEY on the parameter for a POSTKEY request
// The function try three times each request if it was some problems with
// the communication
// \return 0
//
//*****************************************************************************

int Communication(int request, char* size)
{
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
    } iRequest;
    iRequest = eRequestIdle;

    while (1)
    {
        if (g_iState == STATE_NEW_CONNECTION)
        {
            iRequest = eRequestIdle;

            g_iState = STATE_CONNECTED_IDLE;
        }
        else if (g_iState == STATE_CONNECTED_IDLE)
        {
            if (iRequest == eRequestIdle)
            {
                if (request == GET)
                {
                    iRequest = eRequestGET;
                }
                else if (request == POST)
                {
                    iRequest = eRequestPOST;
                }
                else if (request == POSTKEY)
                {
                    iRequest = eRequestPOSTKEY;
                }
                else if (request == POSTACCESS)
                {
                    iRequest = eRequestPOSTACCESS;
                }
                else if (request == GETteste)
                {
                    iRequest = eRequestGETteste;
                }
            }
            else if (iRequest == eRequestGETteste)
            {
                g_iState = STATE_WAIT_DATA;

                //
                // Timeout at 10 seconds.
                //
                g_ui32Delay = 100;

                requestGETteste(g_psUserInfo.pcName, &g_psUserInfo.sReport,
                                CommunicationEvent);

                iRequest = eRequestUpdate;
            }
            else if (iRequest == eRequestGET)
            {
                g_iState = STATE_WAIT_DATA;

                //
                // Timeout at 10 seconds.
                //
                g_ui32Delay = 100;

                requestGET(g_psUserInfo.pcName, &g_psUserInfo.sReport,
                           CommunicationEvent);

                iRequest = eRequestUpdate;
            }
            else if (iRequest == eRequestPOST)
            {
                g_iState = STATE_WAIT_DATA;

                //
                // Timeout at 10 seconds.
                //
                g_ui32Delay = 100;

                requestPOST(g_psUserInfo.pcName, &g_psUserInfo.sReport,
                            CommunicationEvent, size);

                iRequest = eRequestUpdate;
            }
            else if (iRequest == eRequestPOSTKEY)
            {
                g_iState = STATE_WAIT_DATA;

                //
                // Timeout at 10 seconds.
                //
                g_ui32Delay = 100;

                requestPOSTKEY(g_psUserInfo.pcName, &g_psUserInfo.sReport,
                               CommunicationEvent, size);

                iRequest = eRequestUpdate;
            }
            else if (iRequest == eRequestPOSTACCESS)
            {
                g_iState = STATE_WAIT_DATA;

                //
                // Timeout at 10 seconds.
                //
                g_ui32Delay = 100;

                requestPOSTACCESS(g_psUserInfo.pcName, &g_psUserInfo.sReport,
                                  CommunicationEvent, size);

                iRequest = eRequestUpdate;
            }
            else if (iRequest == eRequestUpdate)
            {
                //
                // Return to the idle request state.
                //
                iRequest = eRequestIdle;

            }
        }
        else if (g_iState == STATE_UPDATE_USER)
        {

            //
            // Go to the wait nice state.
            //
            g_iState = STATE_WAIT_NICE;

            //
            // 10ms * 10 is a 1 second delay between updates.
            //
            g_ui32Delay = SYSTEM_TICK_MS * 1;
        }
        else if (g_iState == STATE_WAIT_NICE)
        {
            //
            // Wait for the "nice" delay to not hit the server too often.
            //
            if (g_ui32Delay == 0)
            {
                g_iState = STATE_CONNECTED_IDLE;
            }
        }
        else if (g_iState == STATE_WAIT_DATA)
        {
            //
            // If no data has been received by the timeout then close the
            // connection.
            //
            if (g_ui32Delay == 0)
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

//*****************************************************************************
//
// CommunicationConnecting is the final function of this layer. Communicate with
// the server
// \param type is the int used set whether is voice or key.
// Put VOICE on the parameter if the user is using access by voice
// Put KEY on the parameter if the user is using access by key
// The function try three times each request if it was some problems with
// the communication
// \return OK or errorConnection.
//
//*****************************************************************************

int CommunicationConnecting(int type)
{
    int try = ResetStatus();
    while ((g_psUserInfo.sReport.status != OK) && (try < 3))
    {
        Communication(POST, "38");
        g_psUserInfo.t_error[0][try] = g_psUserInfo.sReport.status;
        try++;
        g_ui32Delay = 500;
    }

    if (try == 3)
    {
        ustrncpy(g_psUserInfo.sReport.logConnection, "Error 404", 9);
        return errorConnection;
    }
    else
    {
        ustrncpy(g_psUserInfo.sReport.logConnection, "Code 200", 8);
    }

    try = ResetStatus();

    while ((g_psUserInfo.sReport.status != OK) && (try < 3))
    {
        if (type == VOICE)
        {
            Communication(GET, "38");
        }
        else if (type == KEY)
        {
            Communication(POSTKEY, "42");
        }
        g_psUserInfo.t_error[1][try] = g_psUserInfo.sReport.status;
        try++;
        g_ui32Delay = 500;
    }

    if (try == 3)
    {
        ustrncpy(g_psUserInfo.sReport.logConnection, "Error 404", 9);
        return errorConnection;
    }
    else
    {
        ustrncpy(g_psUserInfo.sReport.logConnection, "Code 200", 8);
        return OK;
    }

}

//*****************************************************************************
//
// CommunicationLog is the final function of this layer. Communicate with the server
// sending a log for each successful access
// \param none
// The function try three times the post access request if it was some problems with
// the communication
// \return OK or errorConnection.
//
//*****************************************************************************

int CommunicationLog()
{

    int try = ResetStatus();

    while ((g_psUserInfo.sReport.status != OK) && (try < 3))
    {
        Communication(POSTACCESS, "38");
        g_psUserInfo.t_error[2][try] = g_psUserInfo.sReport.status;
        try++;
        g_ui32Delay = 1000;
    }
    if (try == 3)
    {
        ustrncpy(g_psUserInfo.sReport.logConnection, "Error 404", 9);
        return errorConnection;
    }
    else
    {
        ustrncpy(g_psUserInfo.sReport.logConnection, "Code 200", 8);
        return OK;
    }
}

