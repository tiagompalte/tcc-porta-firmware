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
 * Header do Controlador do SPI
 * v1.0
 * 2017
 * ************************************************************** */


#ifndef SPI_LIBRARY_H_
#define SPI_LIBRARY_H_
#include <stdint.h>

#define SSI2_DR_R               (*((volatile uint32_t *)0x4000A008))
#define SSI2_SR_R               (*((volatile uint32_t *)0x4000A00C))

extern void SPIInit(uint32_t ui32SysClock);
extern uint8_t SPITransfer(uint16_t data);
extern void SPIWrite(uint32_t data);
extern uint8_t SPIRead();
extern void SPIDataGet(uint32_t ui32Base, uint32_t *pui32Data);
extern void SPIBusy();
extern void SPIDataGetNonBlocking();
#endif /* SPI_LIBRARY_H_ */
