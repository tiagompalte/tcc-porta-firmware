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
// The weather access methods.
//
//*****************************************************************************
typedef enum
{
    //
    // openweathermap.org.
    //
    iWSrcOpenWeatherMap
}
tWeatherSource;

//*****************************************************************************
//
// Generic weather reporting structure.
//
//*****************************************************************************
typedef struct
{
    //
    // The brief weather description, this is caller provided.
    //
    const char *pcDescription;

    char *audio;

    char *novo;

    char *codigoNome;

    char token[512];


    uint32_t senha;
    //
    // The last time these values were update (GMT Unix time).
    //
    uint32_t ui32Time;



}
tWeatherReport;

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
// Exported weather related prototypes.
//
//*****************************************************************************
extern void WeatherSourceSet(tWeatherSource eWeatherSource);

extern int32_t requestGET(tWeatherSource eWeatherSource,
                               const char *pcQuery,
                               tWeatherReport *psWeather,
                               tEventFunction pfnEvent,char* rfid);
extern int32_t requestGETteste(tWeatherSource eWeatherSource,
                               const char *pcQuery,
                               tWeatherReport *psWeather,
                               tEventFunction pfnEvent);
extern int32_t requestPOST(tWeatherSource eWeatherSource,
                               const char *pcQuery,
                               tWeatherReport *psWeather,
                               tEventFunction pfnEvent, char* size, char* code, char* key);

extern int32_t requestPOSTKEY(tWeatherSource eWeatherSource,
                               const char *pcQuery,
                               tWeatherReport *psWeather,
                               tEventFunction pfnEvent, char* size, char* code, char* key);
extern int32_t requestPOSTACCESS(tWeatherSource eWeatherSource,
                               const char *pcQuery,
                               tWeatherReport *psWeather,
                               tEventFunction pfnEvent, char* size, char* code, char* key,char* rfid);

#ifdef __cplusplus
}
#endif

#endif
