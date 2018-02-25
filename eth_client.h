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
 * eth_client.h - Prototypes for the driver for the eth_client.
 * ************************************************************** */
//*****************************************************************************
#ifndef ETH_CLIENT_H_
#define ETH_CLIENT_H_

#include "driverlib/algorithm.h"

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

//*****************************************************************************
//
// Events passed back to the application.
//
//*****************************************************************************
#define ETH_EVENT_RECEIVE       0x00000001
#define ETH_EVENT_CONNECT       0x00000002
#define ETH_EVENT_DISCONNECT    0x00000003
#define ETH_EVENT_CLOSE         0x00000004
#define ETH_EVENT_INVALID_REQ   0x00000005


//*****************************************************************************
//
// Generic weather reporting structure.
//
//*****************************************************************************
typedef struct
{
    //
    // The brief user description, this is caller provided.
    //
    const char *pcDescription;

    //
    // User voice.
    //
    char audio[NUM_AMOSTRAS + 2*DELAY_MAX];

    //
    // The current user status.
    //

    int status;

    char name[32];

    char *data;

    char *keyBoard;

    char *userKey;//TODO ver quantos caracteres é o máximo

    char *idBoard;

    char token[512];

    //
    // The current rfid.
    //
    char rfid[32];

    char log[32];

    char logConnection[32];

    char authorization[32];

    //
    // The last time these values were update (GMT Unix time).
    //
    uint32_t ui32Time;

}
tUserReport;

//*****************************************************************************
//
// The type definition for event functions.
//
//*****************************************************************************
typedef void (* tEventFunction)(uint32_t ui32Event, void* pvData,
                                uint32_t ui32Param);

//*****************************************************************************
//
// Exported Ethernet function prototypes.
//
//*****************************************************************************
extern void EthClientInit(tEventFunction pfnEvent);
extern void EthClientProxySet(const char *pcProxyName);
extern void EthClientTick(uint32_t ui32TickMS);
extern uint32_t EthClientAddrGet(void);
extern void EthClientMACAddrGet(uint8_t *pui8Addr);
extern uint32_t EthClientServerAddrGet(void);
extern void EthClientReset(void);
void EthClientTCPDisconnect(void);

//*****************************************************************************
//
// Exported user related prototypes.
//
//*****************************************************************************
extern void userSourceSet();

extern int32_t requestGET(     const char *pcQuery,
                               tUserReport *psUser,
                               tEventFunction pfnEvent);
extern int32_t requestGETteste(const char *pcQuery,
                               tUserReport *psUser,
                               tEventFunction pfnEvent);
extern int32_t requestPOST(    const char *pcQuery,
                               tUserReport *psUser,
                               tEventFunction pfnEvent, char* size);

extern int32_t requestPOSTKEY( const char *pcQuery,
                               tUserReport *psUser,
                               tEventFunction pfnEvent, char* size);
extern int32_t requestPOSTACCESS(const char *pcQuery,
                               tUserReport *psUser,
                               tEventFunction pfnEvent, char* size);

#ifdef __cplusplus
}
#endif

#endif
