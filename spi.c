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
 * Controlador do SPI
 * v1.0
 * 2017
 * ************************************************************** */


#include "driverlib/spi.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_ssi.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include"driverlib/pin_map.h"
#include "driverlib/rom_map.h"
#include "driverlib/ssi.h"
//*****************************************************************************
//
// Number of bytes to send and receive.
//
//*****************************************************************************
#define NUM_SSI_DATA            3

uint8_t SPITransfer(uint16_t data)
{
    uint32_t rxData;

    MAP_SSIDataPut(SSI2_BASE, data);

    while(MAP_SSIBusy(SSI2_BASE));

    MAP_SSIDataGet(SSI2_BASE, &rxData);

    return (uint8_t) rxData;
}

void SPIWrite(uint32_t data)
{
    MAP_SSIDataPut(SSI2_BASE, data);

}
void SPIBusy()
{
    while(MAP_SSIBusy(SSI2_BASE));
}
void SPIDataGetNonBlocking()
{
    uint32_t data32 = 0;
    while(MAP_SSIDataGetNonBlocking(SSI2_BASE,&data32));
}
void SPIDataGet(uint32_t ui32Base, uint32_t *pui32Data)
{
    MAP_SSIDataGet(SSI2_BASE, pui32Data);
}

uint8_t SPIRead()
{
    uint32_t value;
    MAP_SSIDataGet(SSI2_BASE,&value);
    return value & 0x00FF;
}


