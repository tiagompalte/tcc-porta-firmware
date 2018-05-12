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
 * communication.h - Prototypes for the driver to the communication.
 * ************************************************************** */
#ifndef COMMUNICATION_H_
#define COMMUNICATION_H_

//*****************************************************************************
//
// If building with a C++ compiler, make all of the definitions in this header
// have a C binding.
//
//*****************************************************************************
#ifdef __cplusplus
extern "C"
{
#endif
#include <stdint.h>
#include "eth_client.h"

//*****************************************************************************
//
// Defines for setting up the system tick clock.
//
//*****************************************************************************
#define SYSTEM_TICK_MS          20
#define SYSTEM_TICK_S           100
//#define SYSTEM_TICK_MMS          15


#define GET                 0
#define POST                1
#define POSTKEY             2
#define POSTACCESS          3
#define PostSendAudio       4
#define errorConnection    -1
#define errorKey            400
#define errorConnect        404
#define OK                  200

#define VOICE                0
#define KEY                  1
#define NONE                 2

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
// The state of each user panel.
//
//*****************************************************************************
struct
{
    //
    // The last update time for this user.
    //
    uint32_t ui32LastUpdate;

    //
    // The current report data for this user.
    //
    tUserReport sReport;

    int t_error[3][3];

    //
    // The name of the current.
    //
    const char *pcName;
} g_psUserInfo;


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
// IP address.
//
//*****************************************************************************
char g_pcIPAddr[20];

extern int ResetUser();
extern int ResetStatus();
extern void UpdateIPAddress(char *pcAddr, uint32_t ipAddr);
extern void CommunicationEvent(uint32_t ui32Event, void* pvData, uint32_t ui32Param);
extern void SysTickIntHandler(void);
extern void EnetEvents(uint32_t ui32Event, void *pvData, uint32_t ui32Param);
extern int Communication(int request, char* size);
extern int CommunicationConnecting();
extern int CommunicationSending(int type);
extern int CommunicationLog();


#ifdef __cplusplus
}
#endif

#endif /* COMMUNICATION_H_ */
