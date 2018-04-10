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
 * eth_client.c - Prototypes for the driver for the eth_client.
 * ************************************************************** */
#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/flash.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/sysctl.h"
#include "utils/lwiplib.h"
#include "lwip/dns.h"
#include "driverlib/systick.h"
#include "lwip/dns.h"
#include "eth_client.h"
#include "json.h"

//*****************************************************************************
//
// Port number and Host name
//
//*****************************************************************************
#define n_Port                  80
//#define n_Port                  8090
#define hostName                "portaeletronica-api.herokuapp.com"
//#define hostName                "10.3.2.158"
//#define n_Port                  3000
//#define hostName                "192.168.100.2"


//*****************************************************************************
//
// Flag indexes for g_sEnet.ui32Flags
//
//*****************************************************************************
#define FLAG_TIMER_DHCP_EN      0
#define FLAG_TIMER_DNS_EN       1
#define FLAG_TIMER_TCP_EN       2
#define FLAG_DHCP_STARTED       3
#define FLAG_DNS_ADDRFOUND      4

//*****************************************************************************
//
// g_sEnet.ulRequest Values
//
//*****************************************************************************
#define NONE                    0
#define POST                    1
#define GET                     2
#define POSTKEY                 3
#define POSTACCESS              4
#define PostSendAudio           5

//*****************************************************************************
//
// The current state of the Ethernet connection.
//
//*****************************************************************************
struct
{
    volatile uint32_t ui32Flags;

    //
    // Array to hold the MAC addresses.
    //
    uint8_t pui8MACAddr[8];

    //
    // Global define of the TCP structure used.
    //
    struct tcp_pcb *psTCP;

    //
    // Global IP structure to hold a copy of the IP address.
    //
    struct ip_addr sLocalIP;

    //
    // Global IP structure to hold a copy of the DNS resolved address.
    //
    struct ip_addr sServerIP;

    //
    // The saved proxy name as a text string.
    //
    const char *pcProxyName;

    volatile enum
    {
        iEthNoConnection,
        iEthDHCPWait,
        iEthDHCPComplete,
        iEthDNSWait,
        iEthTCPConnectWait,
        iEthTCPConnectComplete,
        iEthQueryWait,
        iEthTCPOpen,
        iEthIdle
    } eState;

    unsigned long ulRequest;

    tEventFunction pfnEvent;
} g_sEnet;

//*****************************************************************************
//
// Maximum size of an weather request.
//
//*****************************************************************************
#define MAX_REQUEST             28000

extern uint32_t g_ui32SysClock;

//*****************************************************************************
//
// Various strings used to access user information on the API  - GET and POST.
//
//*****************************************************************************


static const char getRequest[] = "GET /api/usuarios/rfid/"; //rfid/codigo do usuario

static const char postRequestSendAudio[] = "POST /api/usuarios/audio";

static const char postRequest[] = "POST /token";
//static const char postRequest[] = "POST porta-api/token";

static const char postRequestAcess[] = "POST /api/usuarios/confirmacaoAcesso/";

static const char postRequestKey[] = "POST /api/usuarios/autenticacaoSenha";

static char g_cHTTP11[] = " HTTP/1.1\r\n";

static const char g_ControlCache[] = "Cache-Control: no-cache\r\n";

static const char g_ControlCacheTeste[] = "cache-control: no-cache\r\n";

static const char g_Zone[] = "zone: UTC-3\r\n";

static const char g_Authorization[] = "Authorization: Bearer ";

static const char g_ContentType[] = "Content-Type: application/json\r\n";

static const char g_UserAgent[] = "User-Agent: Placa1 Version/1.0\r\n";

static const char g_Accept[] = "Accept: */*\r\n";

static const char g_host[] = "Host: portaeletronica-api.herokuapp.com\r\n";
//static const char g_host[] = "Host: 10.3.2.158:8090\r\n";

static const char g_AcceptEncoding[] = "accept-encoding: gzip, deflate\r\n";

static const char g_AcceptEncodingTeste[] = "accept-encoding: chunked\r\n";

static const char g_ContentLength[] = "Content-Length:";

static const char g_AfterLength[] = "\r\n";

static const char g_Conncection[] = "Connection: keep-alive\r\n\r\n";

static const char g_Open[] = "{\r\n";

static const char g_Code1[] = "\"codigo\": \"";

static const char g_rfid[] = "\"rfid\": \"";

static const char g_Code2[] = "\",\r\n";

static const char g_Comma[] = ",";

static const char g_OpenVector[] = "[";

static const char g_CloseVector[] = "]";

static const char g_Key1[] = "\"senha\": \"";

static const char g_audio[] = "\"audio\":\"";

static const char g_Key2[] = "\"\r\n";

static const char g_Key3[] = "\r\n";

static const char g_Close[] = "}\r\n";

//*****************************************************************************
//
// This structure holds the state and control values for the weather requests.
//
//*****************************************************************************
struct
{
    //
    // The format expected
    //
    enum
    {
        iFormatJSON
    } eFormat;

    //
    // The application provided callback function.
    //
    tEventFunction pfnEvent;

    //
    // The application provided user information structure.
    //
    tUserReport *psUserReport;

    //
    // The local buffer used to store the current user request.
    //
    char pcRequest[MAX_REQUEST];

    //
    // The number of valid bytes in the request.
    //
    uint32_t ui32RequestSize;
} g_sUser;

//*****************************************************************************
//
// Close an active connection.
//
//*****************************************************************************
void EthClientReset(void)
{
    //
    // No longer have a link.
    //
    g_sEnet.eState = iEthNoConnection;

    //
    // Reset the flags to just enable the lwIP timer.
    //
    g_sEnet.ui32Flags = (1 << FLAG_TIMER_DHCP_EN);

    //
    // Reset the addresses.
    //
    g_sEnet.sLocalIP.addr = 0;
    g_sEnet.sServerIP.addr = 0;

    //
    // Deallocate the TCP structure if it was already allocated.
    //
    if (g_sEnet.psTCP)
    {
        //
        // Clear out all of the TCP callbacks.
        //
        tcp_sent(g_sEnet.psTCP, NULL);
        tcp_recv(g_sEnet.psTCP, NULL);
        tcp_err(g_sEnet.psTCP, NULL);

        //
        // Close the TCP connection.
        //
        tcp_close(g_sEnet.psTCP);
        g_sEnet.psTCP = 0;
    }
}

//*****************************************************************************
//
// Handles lwIP TCP/IP errors.
//
// \param vPArg is the state data for this connection.
// \param iErr is the error that was detected.
//
// This function is called when the lwIP TCP/IP stack has detected an error.
// The connection is no longer valid.
//
// \return None.
//
//*****************************************************************************
static void TCPError(void *vPArg, err_t iErr)
{
}

//*****************************************************************************
//
// Finalizes the TCP connection in client mode.
//
// \param pvArg is the state data for this connection.
// \param psPcb is the pointer to the TCP control structure.
// \param psBuf is the buffer structure that holds the data for this receive
// event.
// \param iErr is not used in this implementation.
//
// This function is called when the lwIP TCP/IP stack has completed a TCP
// connection.
//
// \return This function will return an lwIP defined error code.
//
//*****************************************************************************
static err_t TCPReceive(void *pvArg, struct tcp_pcb *psPcb, struct pbuf *psBuf,
                        err_t iErr)
{
    struct pbuf *psBufCur;
    int32_t i32Items;

    if (psBuf == 0)
    {
        //
        // Tell the application that the connection was closed.
        //
        if (g_sUser.pfnEvent)
        {
            g_sUser.pfnEvent(ETH_EVENT_CLOSE, 0, 0);
            g_sUser.pfnEvent = 0;
        }

        //
        // Close out the port.
        //
        tcp_close(psPcb);

        if (psPcb == g_sEnet.psTCP)
        {
            g_sEnet.psTCP = 0;
        }

        g_sEnet.eState = iEthIdle;

        return (ERR_OK);
    }

    if (g_sEnet.eState == iEthQueryWait)
    {
        if (g_sEnet.ulRequest == POST)
        {
            //
            // Read items from the buffer.
            //
            i32Items = JSONParsePOST(0, g_sUser.psUserReport, psBuf);

            //
            // Make sure some items were found.
            //
            if (i32Items > 0)
            {
                if (g_sUser.pfnEvent)
                {
                    g_sUser.pfnEvent(ETH_EVENT_RECEIVE,
                                     (void *) g_sUser.psUserReport, 0);

                    //
                    // Clear the event function and return to the idle state.
                    //
                    g_sEnet.eState = iEthIdle;
                }
            }
            else if (i32Items < 0)
            {
                if (g_sUser.pfnEvent)
                {
                    //
                    // This was not a valid request.
                    //
                    g_sUser.pfnEvent(ETH_EVENT_INVALID_REQ, 0, 0);

                    //
                    // Clear the event function and return to the idle state.
                    //
                    g_sEnet.eState = iEthIdle;
                }
            }
        }
        else if (g_sEnet.ulRequest == POSTKEY)
        {
            //
            // Read items from the buffer.
            //
            i32Items = JSONParsePOSTKEY(0, g_sUser.psUserReport, psBuf);

            if (i32Items > 0)
            {
                if (g_sUser.pfnEvent)
                {
                    g_sUser.pfnEvent(ETH_EVENT_RECEIVE,
                                     (void *) g_sUser.psUserReport, 0);

                    //
                    // Clear the event function and return to the idle state.
                    //
                    g_sEnet.eState = iEthIdle;
                }
            }
            else if (i32Items < 0)
            {
                if (g_sUser.pfnEvent)
                {
                    //
                    // This was not a valid request.
                    //
                    g_sUser.pfnEvent(ETH_EVENT_INVALID_REQ, 0, 0);

                    //
                    // Clear the event function and return to the idle state.
                    //
                    g_sEnet.eState = iEthIdle;
                }
            }
        }
        else if (g_sEnet.ulRequest == POSTACCESS)
        {
            //
            // Read items from the buffer.
            //
            i32Items = JSONParsePOSTACCESS(0, g_sUser.psUserReport, psBuf);

            if (i32Items > 0)
            {
                if (g_sUser.pfnEvent)
                {
                    g_sUser.pfnEvent(ETH_EVENT_RECEIVE,
                                     (void *) g_sUser.psUserReport, 0);

                    //
                    // Clear the event function and return to the idle state.
                    //
                    g_sEnet.eState = iEthIdle;
                }
            }
            else if (i32Items < 0)
            {
                if (g_sUser.pfnEvent)
                {
                    //
                    // This was not a valid request.
                    //
                    g_sUser.pfnEvent(ETH_EVENT_INVALID_REQ, 0, 0);

                    //
                    // Clear the event function and return to the idle state.
                    //
                    g_sEnet.eState = iEthIdle;
                }
            }
        }
        else if (g_sEnet.ulRequest == GET)
        {
            //
            // Read items from the buffer.
            //
            i32Items = JSONParseGET(0, g_sUser.psUserReport, psBuf);

            if (i32Items > 0)
            {
                if (g_sUser.pfnEvent)
                {
                    g_sUser.pfnEvent(ETH_EVENT_RECEIVE,
                                     (void *) g_sUser.psUserReport, 0);

                    //
                    // Clear the event function and return to the idle state.
                    //
                    g_sEnet.eState = iEthIdle;
                }
            }
            else if (i32Items < 0)
            {
                if (g_sUser.pfnEvent)
                {
                    //
                    // This was not a valid request.
                    //
                    g_sUser.pfnEvent(ETH_EVENT_INVALID_REQ, 0, 0);

                    //
                    // Clear the event function and return to the idle state.
                    //
                    g_sEnet.eState = iEthIdle;
                }
            }
        }
        else if (g_sEnet.ulRequest == PostSendAudio)
        {
            //
            // Read items from the buffer.
            //
            i32Items = JSONParsePostSendAudio(0, g_sUser.psUserReport, psBuf);

            if (i32Items > 0)
            {
                if (g_sUser.pfnEvent)
                {
                    g_sUser.pfnEvent(ETH_EVENT_RECEIVE,
                                     (void *) g_sUser.psUserReport, 0);

                    //
                    // Clear the event function and return to the idle state.
                    //
                    g_sEnet.eState = iEthIdle;
                }
            }
            else if (i32Items < 0)
            {
                if (g_sUser.pfnEvent)
                {
                    //
                    // This was not a valid request.
                    //
                    g_sUser.pfnEvent(ETH_EVENT_INVALID_REQ, 0, 0);

                    //
                    // Clear the event function and return to the idle state.
                    //
                    g_sEnet.eState = iEthIdle;
                }
            }
        }
    }
    else
    {
        //
        // Go to idle state.
        //
        g_sEnet.eState = iEthIdle;
    }

    //
    // Initialize the linked list pointer to parse.
    //
    psBufCur = psBuf;

    //
    // Free the buffers used since they have been processed.
    //
    while (psBufCur->len != 0)
    {
        //
        // Indicate that you have received and processed this set of TCP data.
        //
        tcp_recved(psPcb, psBufCur->len);

        //
        // Go to the next buffer.
        //
        psBufCur = psBufCur->next;

        //
        // Terminate if there are no more buffers.
        //
        if (psBufCur == 0)
        {
            break;
        }
    }

    //
    // Free the memory space allocated for this receive.
    //
    pbuf_free(psBuf);

    //
    // Return.
    //
    return (ERR_OK);
}

//*****************************************************************************
//
// Handles acknowledgment of data transmitted via Ethernet.
//
// \param pvArg is the state data for this connection.
// \param psPcb is the pointer to the TCP control structure.
// \param ui16Len is the length of the data transmitted.
//
// This function is called when the lwIP TCP/IP stack has received an
// acknowledgment for data that has been transmitted.
//
// \return This function will return an lwIP defined error code.
//
//*****************************************************************************
static err_t TCPSent(void *pvArg, struct tcp_pcb *psPcb, u16_t ui16Len)
{
    //
    // Return OK.
    //
    return (ERR_OK);
}

//*****************************************************************************
//
// Finalizes the TCP connection in client mode.
//
// \param pvArg is the state data for this connection.
// \param psPcb is the pointer to the TCP control structure.
// \param iErr is not used in this implementation.
//
// This function is called when the lwIP TCP/IP stack has completed a TCP
// connection.
//
// \return This function will return an lwIP defined error code.
//
//*****************************************************************************
static err_t TCPConnected(void *pvArg, struct tcp_pcb *psPcb, err_t iErr)
{
    //
    // Check if there was a TCP error.
    //
    if (iErr != ERR_OK)
    {
        //
        // Clear out all of the TCP callbacks.
        //
        tcp_sent(psPcb, NULL);
        tcp_recv(psPcb, NULL);
        tcp_err(psPcb, NULL);

        //
        // Close the TCP connection.
        //
        tcp_close(psPcb);

        if (psPcb == g_sEnet.psTCP)
        {
            g_sEnet.psTCP = 0;
        }

        //
        // And return.
        //
        return (ERR_OK);
    }

    //
    // Setup the TCP receive function.
    //
    tcp_recv(psPcb, TCPReceive);

    //
    // Setup the TCP error function.
    //
    tcp_err(psPcb, TCPError);

    //
    // Setup the TCP sent callback function.
    //
    tcp_sent(psPcb, TCPSent);

    //
    // Connection is complete.
    //
    g_sEnet.eState = iEthTCPConnectComplete;

    //
    // Return a success code.
    //
    return (ERR_OK);
}

//*****************************************************************************
//
// TCP connect
//
// This function attempts to connect to a TCP endpoint.
//
// \return None.
//
//*****************************************************************************
err_t EthClientTCPConnect(uint32_t ui32Port)
{
    err_t eTCPReturnCode;

    //
    // Enable the TCP timer function calls.
    //
    HWREGBITW(&g_sEnet.ui32Flags, FLAG_TIMER_TCP_EN) = 1;

    if (g_sEnet.psTCP)
    {
        //
        // Initially clear out all of the TCP callbacks.
        //
        tcp_sent(g_sEnet.psTCP, NULL);
        tcp_recv(g_sEnet.psTCP, NULL);
        tcp_err(g_sEnet.psTCP, NULL);

        //
        // Make sure there is no lingering TCP connection.
        //
        tcp_close(g_sEnet.psTCP);
    }

    //
    // Create a new TCP socket.
    //
    g_sEnet.psTCP = tcp_new();

    //
    // Check if you need to go through a proxy.
    //
    if (g_sEnet.pcProxyName != 0)
    {
        //
        // Attempt to connect through the proxy server.
        //
        eTCPReturnCode = tcp_connect(g_sEnet.psTCP, &g_sEnet.sServerIP,
                                     ui32Port, TCPConnected);
    }
    else
    {
        //
        // Attempt to connect to the server directly.
        //
        eTCPReturnCode = tcp_connect(g_sEnet.psTCP, &g_sEnet.sServerIP,
                                     ui32Port, TCPConnected);
    }

    return (eTCPReturnCode);
}

//*****************************************************************************
//
// TCP connect
//
// This function attempts to connect to a TCP endpoint.
//
// \return None.
//
//*****************************************************************************
void EthClientTCPDisconnect(void)
{
    //
    // No longer have a link.
    //
    g_sEnet.eState = iEthNoConnection;

    //
    // Deallocate the TCP structure if it was already allocated.
    //
    if (g_sEnet.psTCP)
    {
        //
        // Close the TCP connection.
        //
        tcp_close(g_sEnet.psTCP);
        g_sEnet.psTCP = 0;
    }
}

//*****************************************************************************
//
// Handler function when the DNS server gets a response or times out.
//
// \param pcName is DNS server name.
// \param psIPAddr is the DNS server's IP address.
// \param vpArg is the configurable argument.
//
// This function is called when the DNS server resolves an IP or times out.
// If the DNS server returns an IP structure that is not NULL, add the IP to
// to the g_sEnet.sServerIP IP structure.
//
// \return None.
//
//*****************************************************************************
static void DNSServerFound(const char *pcName, struct ip_addr *psIPAddr,
                           void *vpArg)
{
    //
    // Check if a valid DNS server address was found.
    //
    if (psIPAddr != NULL)
    {
        //
        // Copy the returned IP address into a global IP address.
        //
        g_sEnet.sServerIP = *psIPAddr;

        //
        // Tell the main program that a DNS address was found.
        //
        HWREGBITW(&g_sEnet.ui32Flags, FLAG_DNS_ADDRFOUND) = 1;
    }
    else
    {
        //
        // Disable the DNS timer.
        //
        HWREGBITW(&g_sEnet.ui32Flags, FLAG_TIMER_DNS_EN) = 0;
    }
}

//*****************************************************************************
//
// Required by lwIP library to support any host-related timer functions.
//
//*****************************************************************************
void lwIPHostTimerHandler(void)
{
}

//*****************************************************************************
//
// Send a request to the server
//
// \param pcRequest request to be sent
// \param ui32Size length of the request to be sent. this is usually the size
// of the request minus the termination character
//
// This function will send the request to the connected server
//
// \return the lwIP error code.
//
//*****************************************************************************
err_t EthClientSend(char *pcRequest, uint32_t ui32Size)
{
    err_t eError;

    eError = tcp_write(g_sEnet.psTCP, pcRequest, ui32Size,
    TCP_WRITE_FLAG_COPY);

    //
    //  Write data for sending (but does not send it immediately).
    //
    if (eError == ERR_OK)
    {
        //
        // Find out what we can send and send it
        //
        tcp_output(g_sEnet.psTCP);
    }

    return (eError);
}

//*****************************************************************************
//
// DHCP connect
//
// This function obtains the MAC address from the User registers, starts the
// DHCP timer and blocks until an IP address is obtained.
//
// \return None.
//
//*****************************************************************************
err_t EthClientDHCPConnect(void)
{
    //
    // Check if the DHCP has already been started.
    //
    if (HWREGBITW(&g_sEnet.ui32Flags, FLAG_DHCP_STARTED) == 0)
    {
        //
        // Set the DCHP started flag.
        //
        HWREGBITW(&g_sEnet.ui32Flags, FLAG_DHCP_STARTED) = 1;
    }
    else
    {
        //
        // If DHCP has already been started, we need to clear the IPs and
        // switch to static.  This forces the LWIP to get new IP address
        // and retry the DHCP connection.
        //
        lwIPNetworkConfigChange(0, 0, 0, IPADDR_USE_STATIC);

        //
        // Restart the DHCP connection.
        //
        lwIPNetworkConfigChange(0, 0, 0, IPADDR_USE_DHCP);
    }

    return ERR_OK;
}

//*****************************************************************************
//
// Handler function when the DNS server gets a response or times out.
//
// \param pcName is DNS server name.
//
// This function is called when the DNS server resolves an IP or times out.
// If the DNS server returns an IP structure that is not NULL, add the IP to
// to the g_sEnet.sServerIP IP structure.
//
// \return None.
//
//*****************************************************************************
int32_t EthClientDNSResolve(const char *pcName)
{
    err_t iRet;

    if (HWREGBITW(&g_sEnet.ui32Flags, FLAG_TIMER_DNS_EN))
    {
        return (ERR_INPROGRESS);
    }

    //
    // Set DNS config timer to true.
    //
    HWREGBITW(&g_sEnet.ui32Flags, FLAG_TIMER_DNS_EN) = 1;

    //
    // Initialize the host name IP address found flag to false.
    //
    HWREGBITW(&g_sEnet.ui32Flags, FLAG_DNS_ADDRFOUND) = 0;

    //
    // Resolve host name.
    //
    iRet = dns_gethostbyname(pcName, &g_sEnet.sServerIP, DNSServerFound, 0);

    //
    // If ERR_OK is returned, the local DNS table resolved the host name.  If
    // ERR_INPROGRESS is returned, the DNS request has been queued and will be
    // sent to the DNS server.
    //
    if (iRet == ERR_OK)
    {
        //
        // Stop calling the DNS timer function.
        //
        HWREGBITW(&g_sEnet.ui32Flags, FLAG_TIMER_DNS_EN) = 0;
    }

    //
    // Return host name not found.
    //
    return (iRet);
}

//*****************************************************************************
//
// Returns the IP address for this interface.
//
// This function will read and return the currently assigned IP address for
// the Tiva Ethernet interface.
//
// \return Returns the assigned IP address for this interface.
//
//*****************************************************************************
uint32_t EthClientAddrGet(void)
{
    //
    // Return IP.
    //
    return (lwIPLocalIPAddrGet());
}

//*****************************************************************************
//
// Returns the weather server IP address for this interface.
//
// This function will read and return the weather server IP address that is
// currently in use.  This could be the proxy server if the Internet proxy
// is enabled.
//
// \return Returns the weather server IP address for this interface.
//
//*****************************************************************************
uint32_t EthClientServerAddrGet(void)
{
    //
    // Return IP.
    //
    return ((uint32_t) g_sEnet.sServerIP.addr);
}

//*****************************************************************************
//
// Returns the MAC address for the Tiva Ethernet controller.
//
// \param pui8MACAddr is the 6 byte MAC address assigned to the Ethernet
// controller.
//
// This function will read and return the MAC address for the Ethernet
// controller.
//
// \return Returns the weather server IP address for this interface.
//
//*****************************************************************************
void EthClientMACAddrGet(uint8_t *pui8MACAddr)
{
    int32_t iIdx;

    for (iIdx = 0; iIdx < 6; iIdx++)
    {
        pui8MACAddr[iIdx] = g_sEnet.pui8MACAddr[iIdx];
    }
}

//*****************************************************************************
//
// Set the proxy string for the Ethernet connection.
//
// \param pcProxyName is the string used as the proxy server name.
//
// This function sets the current proxy used by the Ethernet connection.  The
// \e pcProxyName value can be 0 to indicate that no proxy is in use or it can
// be a pointer to a string that holds the name of the proxy server to use.
// The content of the pointer passed to \e pcProxyName should not be changed
// after this call as this function only stores the pointer and does not copy
// the data from this pointer.
//
// \return None.
//
//*****************************************************************************
void EthClientProxySet(const char *pcProxyName)
{
    //
    // Save the new proxy string.
    //
    g_sEnet.pcProxyName = pcProxyName;

    //
    // Reset the connection on any change to the proxy.
    //
    EthClientReset();
}

//*****************************************************************************
//
// Initialize the Ethernet client
//
// This function initializes all the Ethernet components to not configured.
// This tells the SysTick interrupt which timer modules to call.
//
// \return None.
//
//*****************************************************************************
void EthClientInit(tEventFunction pfnEvent)
{
    uint32_t ui32User0, ui32User1;

    //
    // Initialize all the Ethernet components to not configured.  This tells
    // the SysTick interrupt which timer modules to call.
    //
    HWREGBITW(&g_sEnet.ui32Flags, FLAG_TIMER_DHCP_EN) = 0;
    HWREGBITW(&g_sEnet.ui32Flags, FLAG_TIMER_DNS_EN) = 0;
    HWREGBITW(&g_sEnet.ui32Flags, FLAG_TIMER_TCP_EN) = 0;

    g_sEnet.eState = iEthNoConnection;

    g_sEnet.pfnEvent = pfnEvent;

    //
    // Convert the 24/24 split MAC address from NV ram into a 32/16 split MAC
    // address needed to program the hardware registers, then program the MAC
    // address into the Ethernet Controller registers.
    //
    FlashUserGet(&ui32User0, &ui32User1);

    g_sEnet.pui8MACAddr[0] = ((ui32User0 >> 0) & 0xff);
    g_sEnet.pui8MACAddr[1] = ((ui32User0 >> 8) & 0xff);
    g_sEnet.pui8MACAddr[2] = ((ui32User0 >> 16) & 0xff);
    g_sEnet.pui8MACAddr[3] = ((ui32User1 >> 0) & 0xff);
    g_sEnet.pui8MACAddr[4] = ((ui32User1 >> 8) & 0xff);
    g_sEnet.pui8MACAddr[5] = ((ui32User1 >> 16) & 0xff);

    lwIPInit(g_ui32SysClock, g_sEnet.pui8MACAddr, 0, 0, 0, IPADDR_USE_DHCP);

    //
    // Start lwIP tick interrupt.
    //
    HWREGBITW(&g_sEnet.ui32Flags, FLAG_TIMER_DHCP_EN) = 1;
}

//*****************************************************************************
//
// Periodic Tick for the Ethernet client
//
// This function is the needed periodic tick for the Ethernet client. It needs
// to be called periodically through the use of a timer or systick.
//
// \return None.
//
//*****************************************************************************
void EthClientTick(uint32_t ui32TickMS)
{
    uint32_t ui32IPAddr;
    int32_t i32Ret;

    if (HWREGBITW(&g_sEnet.ui32Flags, FLAG_TIMER_DHCP_EN))
    {
        lwIPTimer(ui32TickMS);
    }

    if (HWREGBITW(&g_sEnet.ui32Flags, FLAG_TIMER_DNS_EN))
    {
        dns_tmr();
    }

    if (HWREGBITW(&g_sEnet.ui32Flags, FLAG_TIMER_TCP_EN))
    {
        tcp_tmr();
    }

    //
    // Check for loss of link.
    //
    if ((g_sEnet.eState != iEthNoConnection)
            && (lwIPLocalIPAddrGet() == 0xffffffff))
    {
        //
        // Reset the connection due to a loss of link.
        //
        EthClientReset();

        //
        // Signal a disconnect event.
        //
        g_sEnet.pfnEvent(ETH_EVENT_DISCONNECT, 0, 0);
    }
    else if (g_sEnet.eState == iEthNoConnection)
    {
        //
        // Once link is detected start DHCP.
        //
        if (lwIPLocalIPAddrGet() != 0xffffffff)
        {
            EthClientDHCPConnect();

            g_sEnet.eState = iEthDHCPWait;
        }
    }
    else if (g_sEnet.eState == iEthDHCPWait)
    {
        //
        // Get IP address.
        //
        ui32IPAddr = lwIPLocalIPAddrGet();

        //
        // If IP Address has not yet been assigned, update the display
        // accordingly.
        //
        if ((ui32IPAddr != 0xffffffff) && (ui32IPAddr != 0))
        {
            //
            // Update the DHCP IP address.
            //
            g_sEnet.sLocalIP.addr = ui32IPAddr;

            g_sEnet.eState = iEthDHCPComplete;

            //
            // Stop DHCP timer since an address has been provided.
            //
            HWREGBITW(&g_sEnet.ui32Flags, FLAG_DHCP_STARTED) = 0;
        }
    }
    else if (g_sEnet.eState == iEthDHCPComplete)
    {
        if (g_sEnet.pcProxyName == 0)
        {
            //
            // Resolve the host by name.
            i32Ret = EthClientDNSResolve(hostName);

        }
        else
        {
            //
            // Resolve the proxy by name.
            //
            i32Ret = EthClientDNSResolve(g_sEnet.pcProxyName);
        }

        if (i32Ret == ERR_OK)
        {
            //
            // If the address was already returned then go to idle.
            //
            g_sEnet.eState = iEthIdle;

            //
            // Notify the main routine of the new Ethernet connection.
            //
            g_sEnet.pfnEvent(ETH_EVENT_CONNECT, &g_sEnet.sLocalIP.addr, 4);
        }
        else if (i32Ret == ERR_INPROGRESS)
        {
            //
            // If the request is pending the go to the iEthDNSWait state.
            //
            g_sEnet.eState = iEthDNSWait;
        }
    }
    else if (g_sEnet.eState == iEthDNSWait)
    {
        //
        // Check if the host name was resolved.
        //
        if (HWREGBITW(&g_sEnet.ui32Flags, FLAG_DNS_ADDRFOUND))
        {
            //
            // Stop calling the DNS timer function.
            //
            HWREGBITW(&g_sEnet.ui32Flags, FLAG_TIMER_DNS_EN) = 0;

            g_sEnet.eState = iEthIdle;

            //
            // Notify the main routine of the new Ethernet connection.
            //
            g_sEnet.pfnEvent(ETH_EVENT_CONNECT, &g_sEnet.sLocalIP.addr, 4);
        }
    }
    else if (g_sEnet.eState == iEthTCPConnectWait)
    {
    }
    else if (g_sEnet.eState == iEthTCPConnectComplete)
    {
        err_t eError;

        g_sEnet.eState = iEthTCPOpen;

        eError = EthClientSend(g_sUser.pcRequest, g_sUser.ui32RequestSize);

        if (eError == ERR_OK)
        {
            //
            // Waiting on a query response.
            //
            g_sEnet.eState = iEthQueryWait;
        }
        else
        {
            g_sEnet.eState = iEthIdle;
        }
    }
}

void userSourceSet()
{

    g_sUser.eFormat = iFormatJSON;
}

//*****************************************************************************
//
// Merges strings into the current request ast a given pointer and offset.
//
//*****************************************************************************
static int32_t MergeRequestVectorInt(int32_t i32Offset, const char *pcSrc,
                                     int32_t i32Size,
                                     bool bReplaceSpace)
{
    int32_t i32Idx;
    int32_t i32IdxCont = 0;

    //
    // Copy the base request to the buffer.
    //
    for (i32Idx = 0; i32Idx < i32Size; i32Idx++)
    {
        if ((pcSrc[i32Idx] == ' ') && (bReplaceSpace))
        {
            if ((i32Offset + 3) >= sizeof(g_sUser.pcRequest))
            {
                break;
            }
            g_sUser.pcRequest[i32Offset++] = '%';
            g_sUser.pcRequest[i32Offset++] = '2';
            g_sUser.pcRequest[i32Offset] = '0';
        }
        else
        {
            g_sUser.pcRequest[i32Offset] = pcSrc[i32Idx];
        }

        if ((i32Offset >= sizeof(g_sUser.pcRequest)) || (pcSrc[i32Idx] == 0))
        {
            break;
        }

        i32Offset++;
        i32IdxCont++;
        if (i32IdxCont < i32Size)
        {
            g_sUser.pcRequest[i32Offset] = ',';
            i32Offset++;
        }
    }

    return (i32Offset);
}

//*****************************************************************************
//
// Merges strings into the current request ast a given pointer and offset.
//
//*****************************************************************************
static int32_t MergeRequest(int32_t i32Offset, const char *pcSrc,
                            int32_t i32Size,
                            bool bReplaceSpace)
{
    int32_t i32Idx;

    //
    // Copy the base request to the buffer.
    //
    for (i32Idx = 0; i32Idx < i32Size; i32Idx++)
    {
        if ((pcSrc[i32Idx] == ' ') && (bReplaceSpace))
        {
            if ((i32Offset + 3) >= sizeof(g_sUser.pcRequest))
            {
                break;
            }
            g_sUser.pcRequest[i32Offset++] = '%';
            g_sUser.pcRequest[i32Offset++] = '2';
            g_sUser.pcRequest[i32Offset] = '0';
        }
        else
        {
            g_sUser.pcRequest[i32Offset] = pcSrc[i32Idx];
        }

        if ((i32Offset >= sizeof(g_sUser.pcRequest)) || (pcSrc[i32Idx] == 0))
        {
            break;
        }

        i32Offset++;
    }

    return (i32Offset);
}

//*****************************************************************************
//
// Function requestGET
//
// \param pcQuery is the name of the current user.
// \param psUserReport contains all about the current user.
// \param pfnEvent is the current event.
//
// This function is called when the communication needs to send a GET request to the server.
//
// \return none.
//
//*****************************************************************************
int32_t requestGET(const char *pcQuery, tUserReport *psUserReport,
                   tEventFunction pfnEvent)
{
    int32_t i32Idx = 0;

    //
    // If the requested source is not valid or there is no call back then
    // just fail.
    //
    if (g_sUser.pfnEvent)
    {
        return (-1);
    }

    g_sUser.pfnEvent = pfnEvent;
    g_sUser.psUserReport = psUserReport;

    //
    // Connect or reconnect to port 80.
    //
    g_sEnet.eState = iEthTCPConnectWait;

    //
    // "GET /api/usuarios/rfid/"
    //
    i32Idx = MergeRequest(0, getRequest, sizeof(getRequest), false);

    //
    // Append the rfid string
    //
    i32Idx = MergeRequest(i32Idx, psUserReport->rfid,
                          sizeof(psUserReport->rfid), false);

    //
    // Append the "HTTP:/1.1" string.
    //
    i32Idx = MergeRequest(i32Idx, g_cHTTP11, sizeof(g_cHTTP11), false);

    //
    // Append the "Cache-Control: no-cache" string.
    //
    i32Idx = MergeRequest(i32Idx, g_ControlCache, sizeof(g_ControlCache),
    false);

    //
    // Append the "zone: UTC-3" string.
    //
    i32Idx = MergeRequest(i32Idx, g_Zone, sizeof(g_Zone), false);

    //
    // Append the "Authorization: Bearer token" string.
    //
    i32Idx = MergeRequest(i32Idx, g_Authorization, sizeof(g_Authorization),
    false);

    //
    // Append the token
    //
    i32Idx = MergeRequest(i32Idx, psUserReport->token,
                          sizeof(psUserReport->token), false);

    //
    // Break line
    //
    i32Idx = MergeRequest(i32Idx, g_AfterLength, sizeof(g_AfterLength), false);

    //
    // Append the "UserAgent: Placa1 Version 1.0" string.
    //
    i32Idx = MergeRequest(i32Idx, g_UserAgent, sizeof(g_UserAgent), false);

    //
    // Append the "Accept: * /*" string.
    //
    i32Idx = MergeRequest(i32Idx, g_Accept, sizeof(g_Accept), false);

    //
    // Append the "Host: portaeletronica-api.herokuapp.com" string.
    //
    i32Idx = MergeRequest(i32Idx, g_host, sizeof(g_host), false);

    //
    // Append the "accept-encoding: gzip, deflate" string.
    //
    i32Idx = MergeRequest(i32Idx, g_AcceptEncoding, sizeof(g_AcceptEncoding),
    false);

    //
    // Append the "Connection: keep-alive" string.
    //
    i32Idx = MergeRequest(i32Idx, g_Conncection, sizeof(g_Conncection), false);

    g_sEnet.ulRequest = GET;

    if (EthClientTCPConnect(n_Port) != ERR_OK)
    {
        return (-1);
    }

    //
    // Save the size of the request.
    //
    g_sUser.ui32RequestSize = i32Idx;

    return (0);
}

//*****************************************************************************
//
// Function requestPOST
//
// \param pcQuery is the name of the current user.
// \param psUserReport contains all about the current user.
// \param pfnEvent is the current event.
//
// This function is called when the communication needs to send a POST request to the server.
//
// \return none.
//
//*****************************************************************************
int32_t requestPOST(const char *pcQuery, tUserReport *psUserReport,
                    tEventFunction pfnEvent, char* size)
{
    int32_t i32Idx;

    //
    // If the requested source is not valid or there is no call back then
    // just fail.
    //
    if (g_sUser.pfnEvent)
    {
        return (-1);
    }

    g_sUser.pfnEvent = pfnEvent;
    g_sUser.psUserReport = psUserReport;

    //
    // Connect or reconnect to port 80.
    //
    g_sEnet.eState = iEthTCPConnectWait;

    //
    // Copy the base forecast request to the buffer. This buffer is the buffer sent
    //
    i32Idx = MergeRequest(0, postRequest, sizeof(postRequest), false);

    //
    // Append the "HTTP:/1.1" string.
    //
    i32Idx = MergeRequest(i32Idx, g_cHTTP11, sizeof(g_cHTTP11), false);

    //
    // Append the g_ControlCache.
    //
    i32Idx = MergeRequest(i32Idx, g_ControlCache, sizeof(g_ControlCache),
    false);

    //
    // Append the "Content-Type: application/json" string.
    //

    i32Idx = MergeRequest(i32Idx, g_ContentType, sizeof(g_ContentType), false);

    //
    // Append the " User-Agent: Placa1 Version/1.0d" string.
    //

    i32Idx = MergeRequest(i32Idx, g_UserAgent, sizeof(g_UserAgent), false);

    //
    // Append the " g_Accept"string.
    //

    i32Idx = MergeRequest(i32Idx, g_Accept, sizeof(g_Accept), false);

    //
    // Append the " Host: localhost:portaeletronica-api.herokuapp.com" string.
    //

    i32Idx = MergeRequest(i32Idx, g_host, sizeof(g_host), false);

    //
    // Append the " Accept-Encoding:  gzip, deflate" string.
    //

    i32Idx = MergeRequest(i32Idx, g_AcceptEncoding, sizeof(g_AcceptEncoding),
    false);

    //
    // Append the " Content-Length: " string.
    //

    i32Idx = MergeRequest(i32Idx, g_ContentLength, sizeof(g_ContentLength),
    false);

    //
    // Append the size of the string.
    //

    i32Idx = MergeRequest(i32Idx, size, sizeof(size), false);

    //
    // Append the break line.
    //

    i32Idx = MergeRequest(i32Idx, g_AfterLength, sizeof(g_AfterLength), false);

    //
    // Append the " Connection: Keep-Alive" string.
    //

    i32Idx = MergeRequest(i32Idx, g_Conncection, sizeof(g_Conncection), false);

    //
    // Append the "User" string.
    //

    i32Idx = MergeRequest(i32Idx, g_Open, sizeof(g_Open), false);

    //
    // Append the "User" string.
    //

    i32Idx = MergeRequest(i32Idx, g_Code1, sizeof(g_Code1), false);

    //
    // Append the "User" string.
    //

    i32Idx = MergeRequest(i32Idx, psUserReport->idBoard,
                          sizeof(psUserReport->idBoard), false);

    //
    // Append the "User" string.
    //

    i32Idx = MergeRequest(i32Idx, g_Code2, sizeof(g_Code2), false);

    //
    // Append the "User" string.
    //

    i32Idx = MergeRequest(i32Idx, g_Key1, sizeof(g_Key1), false);

    //
    // Append the "User" string.
    //

    i32Idx = MergeRequest(i32Idx, psUserReport->keyBoard,
                          sizeof(psUserReport->keyBoard) + 1, false);

    //
    // Append the "User" string.
    //

    i32Idx = MergeRequest(i32Idx, g_Key2, sizeof(g_Key2), false);

    //
    // Append the "User" string.
    //

    i32Idx = MergeRequest(i32Idx, g_Close, sizeof(g_Close), false);

    //
    // Forcast ulr report request.
    //
    g_sEnet.ulRequest = POST;

    if (EthClientTCPConnect(n_Port) != ERR_OK)
    {
        return (-1);
    }

    //
    // Save the size of the request.
    //
    g_sUser.ui32RequestSize = i32Idx;

    return (0);
}

//*****************************************************************************
//
// Function requestPOSTKEY
//
// \param pcQuery is the name of the current user.
// \param psUserReport contains all about the current user.
// \param pfnEvent is the current event.
//
// This function is called when the communication needs to send a POST request to the server
// with key like a parameter
// \return none.
//
//*****************************************************************************
int32_t requestPOSTKEY(const char *pcQuery, tUserReport *psUserReport,
                       tEventFunction pfnEvent, char* size)
{
    int32_t i32Idx;

    //
    // If the requested source is not valid or there is no call back then
    // just fail.
    //
    if (g_sUser.pfnEvent)
    {
        return (-1);
    }

    g_sUser.pfnEvent = pfnEvent;
    g_sUser.psUserReport = psUserReport;

    //
    // Connect or reconnect to port 80.
    //
    g_sEnet.eState = iEthTCPConnectWait;

    //
    // Copy the base forecast request to the buffer. This buffer is the buffer sent
    //
    i32Idx = MergeRequest(0, postRequestKey, sizeof(postRequestKey), false);

    //
    // Append the "HTTP:/1.1" string.
    //
    i32Idx = MergeRequest(i32Idx, g_cHTTP11, sizeof(g_cHTTP11), false);

    //
    // Append the g_ControlCache.
    //
    i32Idx = MergeRequest(i32Idx, g_ControlCache, sizeof(g_ControlCache),
    false);

    //
    // Append the "zone: UTC-3" string.
    //
    i32Idx = MergeRequest(i32Idx, g_Zone, sizeof(g_Zone), false);

    //
    // Append the "Authorization: Bearer token" string.
    //
    i32Idx = MergeRequest(i32Idx, g_Authorization, sizeof(g_Authorization),
    false);

    //
    // Append the token
    //
    i32Idx = MergeRequest(i32Idx, psUserReport->token,
                          sizeof(psUserReport->token), false);

    //
    // Break line
    //
    i32Idx = MergeRequest(i32Idx, g_AfterLength, sizeof(g_AfterLength), false);

    //
    // Append the "Content-Type: application/json" string.
    //

    i32Idx = MergeRequest(i32Idx, g_ContentType, sizeof(g_ContentType), false);

    //
    // Append the " User-Agent: Placa1 Version/1.0d" string.
    //

    i32Idx = MergeRequest(i32Idx, g_UserAgent, sizeof(g_UserAgent), false);

    //
    // Append the " g_Accept"string.
    //

    i32Idx = MergeRequest(i32Idx, g_Accept, sizeof(g_Accept), false);

    //
    // Append the " Host: localhost:8090" string.
    //

    i32Idx = MergeRequest(i32Idx, g_host, sizeof(g_host), false);

    //
    // Append the " Accept-Encoding:  gzip, deflate" string.
    //

    i32Idx = MergeRequest(i32Idx, g_AcceptEncoding, sizeof(g_AcceptEncoding),
    false);

    //
    // Append the " Content-Length: " string.
    //

    i32Idx = MergeRequest(i32Idx, g_ContentLength, sizeof(g_ContentLength),
    false);

    //
    // Append the size of the string.
    //

    i32Idx = MergeRequest(i32Idx, size, sizeof(size), false);

    //
    // Append the break line.
    //

    i32Idx = MergeRequest(i32Idx, g_AfterLength, sizeof(g_AfterLength), false);

    //
    // Append the " Connection: Keep-Alive" string.
    //

    i32Idx = MergeRequest(i32Idx, g_Conncection, sizeof(g_Conncection), false);

    //
    // Append the "User" string.
    //

    i32Idx = MergeRequest(i32Idx, g_Open, sizeof(g_Open), false);

    //
    // Append the "User" string.
    //

    i32Idx = MergeRequest(i32Idx, g_rfid, sizeof(g_rfid), false);

    //
    // Append the "User" string.
    //

    i32Idx = MergeRequest(i32Idx, psUserReport->rfid,
                          sizeof(psUserReport->rfid), false);

    //
    // Append the "User" string.
    //

    i32Idx = MergeRequest(i32Idx, g_Code2, sizeof(g_Code2), false);

    //
    // Append the "User" string.
    //

    i32Idx = MergeRequest(i32Idx, g_Key1, sizeof(g_Key1), false);

    //
    // Append the "User" string.
    //

    i32Idx = MergeRequest(i32Idx, psUserReport->userKey,
                          sizeof(psUserReport->userKey), false);

    //
    // Append the "User" string.
    //

    i32Idx = MergeRequest(i32Idx, g_Key2, sizeof(g_Key2), false);

    //
    // Append the "User" string.
    //

    i32Idx = MergeRequest(i32Idx, g_Close, sizeof(g_Close), false);

    g_sEnet.ulRequest = POSTKEY;

    if (EthClientTCPConnect(n_Port) != ERR_OK)
    {
        return (-1);
    }

    //
    // Save the size of the request.
    //
    g_sUser.ui32RequestSize = i32Idx;

    return (0);
}

//*****************************************************************************
//
// Function requestPOSTACCESS
//
// \param pcQuery is the name of the current user.
// \param psUserReport contains all about the current user.
// \param pfnEvent is the current event.
//
// This function is called when the communication needs to send a POST request to the server
// to register a log
// \return none.
//
//*****************************************************************************
int32_t requestPOSTACCESS(const char *pcQuery, tUserReport *psUserReport,
                          tEventFunction pfnEvent, char* size)
{
    int32_t i32Idx;

    //
    // If the requested source is not valid or there is no call back then
    // just fail.
    //
    if (g_sUser.pfnEvent)
    {
        return (-1);
    }

    g_sUser.pfnEvent = pfnEvent;
    g_sUser.psUserReport = psUserReport;

    //
    // Connect or reconnect to port 80.
    //
    g_sEnet.eState = iEthTCPConnectWait;

    //
    // "POST /api/usuarios/confirmacaoAcesso/";
    //
    i32Idx = MergeRequest(0, postRequestAcess, sizeof(postRequestAcess), false);

    //
    // Append the rfid string
    //
    i32Idx = MergeRequest(i32Idx, psUserReport->rfid,
                          sizeof(psUserReport->rfid), false);

    //
    // Append the "HTTP:/1.1" string.
    //
    i32Idx = MergeRequest(i32Idx, g_cHTTP11, sizeof(g_cHTTP11), false);

    //
    // Append the "Cache-Control: no-cache" string.
    //
    i32Idx = MergeRequest(i32Idx, g_ControlCache, sizeof(g_ControlCache),
    false);

    //
    // Append the "zone: UTC-3" string.
    //
    i32Idx = MergeRequest(i32Idx, g_Zone, sizeof(g_Zone), false);

    //
    // Append the "Authorization: Bearer token" string.
    //
    i32Idx = MergeRequest(i32Idx, g_Authorization, sizeof(g_Authorization),
    false);

    //
    // Append the token
    //
    i32Idx = MergeRequest(i32Idx, psUserReport->token,
                          sizeof(psUserReport->token), false);

    //
    // Break line
    //
    i32Idx = MergeRequest(i32Idx, g_AfterLength, sizeof(g_AfterLength), false);

    //
    // Append the "UserAgent: Placa1 Version 1.0" string.
    //
    i32Idx = MergeRequest(i32Idx, g_UserAgent, sizeof(g_UserAgent), false);

    //
    // Append the "Accept: * /*" string.
    //
    i32Idx = MergeRequest(i32Idx, g_Accept, sizeof(g_Accept), false);

    //
    // Append the "Host: portaeletronica-api.herokuapp.com" string.
    //
    i32Idx = MergeRequest(i32Idx, g_host, sizeof(g_host), false);

    //
    // Append the "accept-encoding: gzip, deflate" string.
    //
    i32Idx = MergeRequest(i32Idx, g_AcceptEncoding, sizeof(g_AcceptEncoding),
    false);

    //
    // Append the "Connection: keep-alive" string.
    //
    i32Idx = MergeRequest(i32Idx, g_Conncection, sizeof(g_Conncection), false);

    g_sEnet.ulRequest = POSTACCESS;

    if (EthClientTCPConnect(n_Port) != ERR_OK)
    {
        return (-1);
    }

    //
    // Save the size of the request.
    //
    g_sUser.ui32RequestSize = i32Idx;

    return (0);
}

//*****************************************************************************
//
// Function requestPostSendAudio
//
// \param pcQuery is the name of the current user.
// \param psUserReport contains all about the current user.
// \param pfnEvent is the current event.
//
// This function is called when the communication needs to send a GET request to the server.
// Just used for test
// \return i32Idx.
//
//*****************************************************************************

int32_t requestPostSendAudio(const char *pcQuery, tUserReport *psUserReport,
                             tEventFunction pfnEvent, char* size)
{
    int32_t i32Idx;

    //
    // If the requested source is not valid or there is no call back then
    // just fail.
    //
    if (g_sUser.pfnEvent)
    {
        return (-1);
    }

    g_sUser.pfnEvent = pfnEvent;
    g_sUser.psUserReport = psUserReport;

    //
    // Connect or reconnect to port 80.
    //
    g_sEnet.eState = iEthTCPConnectWait;

    //
    // Append the POST /api/usuarios/audio
    //
    i32Idx = MergeRequest(0, postRequestSendAudio, sizeof(postRequestSendAudio),
    false);

    //
    // Append the "HTTP:/1.1" string.
    //
    i32Idx = MergeRequest(i32Idx, g_cHTTP11, sizeof(g_cHTTP11), false);

    //
    // Append the g_ControlCache.
    //
    i32Idx = MergeRequest(i32Idx, g_ControlCache, sizeof(g_ControlCache),
    false);

    //
    // Append the "zone: UTC-3" string.
    //
    i32Idx = MergeRequest(i32Idx, g_Zone, sizeof(g_Zone), false);

    //
    // Append the "Authorization: Bearer token" string.
    //
    i32Idx = MergeRequest(i32Idx, g_Authorization, sizeof(g_Authorization),
    false);

    //
    // Append the token
    //
    i32Idx = MergeRequest(i32Idx, psUserReport->token,
                          sizeof(psUserReport->token), false);

    //
    // Break line
    //
    i32Idx = MergeRequest(i32Idx, g_AfterLength, sizeof(g_AfterLength), false);

    //
    // Append the "Content-Type: application/json" string.
    //

    i32Idx = MergeRequest(i32Idx, g_ContentType, sizeof(g_ContentType), false);

    //
    // Append the " User-Agent: Placa1 Version/1.0d" string.
    //

    i32Idx = MergeRequest(i32Idx, g_UserAgent, sizeof(g_UserAgent), false);

    //
    // Append the " g_Accept"string.
    //

    i32Idx = MergeRequest(i32Idx, g_Accept, sizeof(g_Accept), false);

    //
    // Append the " Host: localhost:8090" string.
    //

    i32Idx = MergeRequest(i32Idx, g_host, sizeof(g_host), false);

    //
    // Append the " Accept-Encoding:  gzip, deflate" string.
    //

    i32Idx = MergeRequest(i32Idx, g_AcceptEncoding, sizeof(g_AcceptEncoding),
    false);

    //
    // Append the " Content-Length: " string.
    //

    i32Idx = MergeRequest(i32Idx, g_ContentLength, sizeof(g_ContentLength),
    false);

    //
    // Append the size of the string.
    //

    i32Idx = MergeRequest(i32Idx, size, sizeof(size)+1, false);

    //
    // Append the break line.
    //

    i32Idx = MergeRequest(i32Idx, g_AfterLength, sizeof(g_AfterLength), false);

    //
    // Append the " Connection: Keep-Alive" string.
    //

    i32Idx = MergeRequest(i32Idx, g_Conncection, sizeof(g_Conncection), false);

    //
    // Append the "{" string.
    //

    i32Idx = MergeRequest(i32Idx, g_Open, sizeof(g_Open), false);

    //
    // Append the "rfid" string.
    //

    i32Idx = MergeRequest(i32Idx, g_rfid, sizeof(g_rfid), false);

    //
    // Append the rfid number string.
    //

    i32Idx = MergeRequest(i32Idx, psUserReport->rfid,
                          sizeof(psUserReport->rfid), false);

    //
    // Append the "," string.
    //

    i32Idx = MergeRequest(i32Idx, g_Code2, sizeof(g_Code2), false);

    //
    // Append the "audio" string.
    //

    i32Idx = MergeRequest(i32Idx, g_audio, sizeof(g_audio), false);

    //20k amostras
    int j;
    for (j = 0; j < NUM_AMOSTRAS; j++)
    {
        bufferConversao[j] = '1';
    }

    //
    // Append the audio streaming string .
    //
    i32Idx = MergeRequest(i32Idx, bufferConversao,
                                   sizeof(bufferConversao), false);

    //
    // Append the "User" string.
    //
    i32Idx = MergeRequest(i32Idx, g_Key2, sizeof(g_Key2), false);

    //
    // Append the "User" string.
    //
    i32Idx = MergeRequest(i32Idx, g_Close, sizeof(g_Close), false);

    g_sEnet.ulRequest = PostSendAudio;

    if (EthClientTCPConnect(n_Port) != ERR_OK)
    {
        return (-1);
    }

    //
    // Save the size of the request.
    //
    g_sUser.ui32RequestSize = i32Idx;

    return (0);
}

