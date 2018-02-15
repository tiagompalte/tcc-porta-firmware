
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
 * Header do Controlador do LCD
 * v1.0
 * 2017
 * ************************************************************** */

#ifndef LCD_H_
#define LCD_H_

#include <stdbool.h>
#include <stdint.h>
#include "../inc/hw_ints.h"
#include "../inc/hw_memmap.h"
#include "../inc/hw_sysctl.h"
#include "../inc/hw_types.h"

#include "sysctl.h"
#include "debug.h"
#include "gpio.h"

/*|---------------|
 *| ENABLE | PP3  |
 *|---------------|
 *| R/W    | PQ1  |
 *|---------------|
 *| RS     | PM6  |
 *|---------------|
 *| DATA 0 | PK5  |
 *|---------------|
 *| DATA 1 | PM0  |
 *|---------------|
 *| DATA 2 | PM1  |
 *|---------------|
 *| DATA 3 | PM2  |
 *|---------------|
 *| DATA 4 | PH0  |
 *|---------------|
 *| DATA 5 | PH1  |
 *|---------------|
 *| DATA 6 | PK6  |
 *|---------------|
 *| DATA 7 | PK7  |
 *|---------------|
 */

// Define Pinos LCD
#define     LCD_DATA             GPIO_PORTK_BASE
#define     DATA0                GPIO_PIN_0
#define     DATA1                GPIO_PIN_1
#define     DATA2                GPIO_PIN_2
#define     DATA3                GPIO_PIN_3
#define     DATA4                GPIO_PIN_4
#define     DATA5                GPIO_PIN_5
#define     DATA6                GPIO_PIN_6
#define     DATA7                GPIO_PIN_7
#define     LCD_CMD              GPIO_PORTM_BASE
#define     LCD_ENABLE           GPIO_PIN_0
#define     LCD_RW               GPIO_PIN_1
#define     LCD_RS               GPIO_PIN_2

// => Define Comandos e configuraçôes do display LCD <=
#define     LCDCOMMAND           0
#define     LCDDATA              1
#define     LCD_CLR              0x01
#define     RETURN_HOME          0x02

//********************************************
// => Define Comandos ENTRY_SET_MODE <=
#define     ENTRY_SET_MODE       0x04

//Direção do cursor
#define     INCREMENT            0x02
#define     DECREMENT            0x00

//Deslocamento do display
#define     EN_SHIFT             0x01
#define     NO_SHIFT             0x00

//********************************************
// => Define Comandos DISPLAY_ON_CONTROL <=
#define     DISPLAY_ON_CONTROL   0x08

// Liga ou desliga display
#define     DISPLAY_ON           0x04
#define     DISPLAY_OFF          0x00

// Liga ou desliga cursor
#define     CURSOR_ON            0x02
#define     CURSOR_OFF           0x00

// Pisca cursor
#define     CURSOR_BLINK_ON      0x01
#define     CURSOR_BLINK_OFF     0x00

//********************************************
// => Define comandos CURSOR_DISPLAY_SHIFT <=
#define     CURSOR_DISPLAY_SHIFT 0x10

// Bits de configuração do deslocamento
#define     CURSOR_LEFT_AC_DECR  0x00
#define     CURSOR_RIGHT_AC_INCR 0x04
#define     SHIFT_DISPLAY_LEFT   0x08
#define     SHIFT_DISPLAY_RIGHT  0x0C

//********************************************
// => Define comandos FUNCTION_SET <=
#define     FUNCTION_SET         0x20

// Tamanho do BUS de dados
#define     BUS_8_BITS           0x10
#define     BUS_4_BITS           0x00

// 2 ou mais linhas do display
#define     DISPLAY_LINHAS       0x08

// Tamanho da fonte
#define     FONTE_5x11           0x04
#define     FONTE_5x8            0x00

//********************************************
// => Define Endereços <=
#define     SET_CGRAM_ADDRESS    0x40
#define     SET_DRAM_ADDRESS     0x80

#define     END_INICIAL_LINHA_1  0x00
#define     END_INICIAL_LINHA2   0x40

// => Busy Flag <=
#define     BUSY_FLAG            GPIO_PIN_7

// Address positions
#define LCD_LINE_1  0x80    // Position of start of line 1
#define LCD_LINE_2  0xC0    // Position of start of line 2
#define LCD_LINE_3  0x94    // Position of start of line 3
#define LCD_LINE_4  0xD4    // Position of start of line 4

//uint8_t i = 0;

extern void LCDInit();
extern void CheckBusyFlag();
extern void LCDWriteData(unsigned char data);
extern void LCDWriteCmd(unsigned char command);
extern void LCDMoveCursorToXY(uint8_t line, uint8_t column);
extern void LCDMoveCursorLeft();
extern void LCDMoveCursorRight();
extern void LCDMoveDisplayLeft();
extern void LCDMoveDisplayRight();
extern void LCDNextLine();
extern void LCDClear();
extern void LCDWriteString(char *str);
extern void LCDInicio();
extern void LCDPassRFID();
extern void LCDAnimation(char *str);
extern void LCDPassword();
extern void LCDKeyPassword();
extern void LCDShowKeyPassword(uint8_t count);
extern void LCDNotAllowed();
extern void LCDAllowed();
extern void LCDUserBlocked();
extern void LCDError();
extern void LCDBlinkDisplay(uint16_t time);
extern uint8_t getAddress();
extern void LCDNotRegister();
extern void LCDRecordingSound();
#endif /* LCD_H_ */
