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
 * Comunicação com MFRC522, Card MIFARE S50 e ISO 14443 baseado no
 * código para arduino  de Miguel Balboa (https://github.com/miguelbalboa/rfid)
 * v1.0
 * 2017
 * ************************************************************** */


#ifndef RFID_H_
#define RFID_H_

#include "stdio.h"
#include "stdbool.h"

/******************************************************************************
 * Definições
 ******************************************************************************/
#define MAX_LEN 16

//Comandos do MFRC522
#define PCD_IDLE              0x00               // Sem ação, cancela a execução do comando atual
#define PCD_AUTHENT           0x0E               // Executa a autenticação padrão MIFARE como um leitor
#define PCD_RECEIVE           0x08               // Ativa o circuito receptor
#define PCD_TRANSMIT          0x04               // Transmite dados do buffer FIFO
#define PCD_TRANSCEIVE        0x0C               // Transmite os dados do buffer FIFO para a antena e
                                                 // automaticamente ativa o receptor após a transmissão
#define PCD_RESETPHASE        0x0F               // Reseta o MFRC522
#define PCD_CALCCRC           0x03               // Ativa o coprocessador CRC or reaiza um auto teste

//COMANDOS ENVIADOS AO PICC (Cartão  Mifare_One)
// Comandos usados pelo MFRC522 para gerenciar a comunicação com vários cartões (ISO 14443-3, Type A, Seção 6.4)
#define PICC_REQIDL           0x26               // Comando Request, Tipo A. Convida PICCs no estado IDLE ir para o estado READY e preparar para anticolisão ou seleção
#define PICC_REQALL           0x52               // Comando Wake-Up, Tipo A. Convida PICCs no estado IDLE e HALT ir para o estado READY e preparar para anticolisão ou seleção
#define PICC_ANTICOLL         0x93               // Anti collision/Select, Cascade Level 1
#define PICC_SElECTTAG        0x93               //
#define PICC_HALT             0x50               // Comando HALT. Instrui um PICC ATIVO ir para o estado HALT

//Comandos usados para o MIFARE Classic (http://www.nxp.com/documents/data_sheet/MF1S503x.pdf, Section 9)
#define PICC_AUTHENT1A        0x60               // Executa uma autenticação com a chave A
#define PICC_AUTHENT1B        0x61               // Executa uma autenticação com a chave B
#define PICC_READ             0x30               // Lê um bloco de 16 bytes do setor autentidado do PICC
#define PICC_WRITE            0xA0               // Escreve um bloco de 16 bytes do setor autentidado do PICC
#define PICC_DECREMENT        0xC0               // Decrementa o conteúdo de um bloco e armazena o resultado em um registrador de dados interno
#define PICC_INCREMENT        0xC1               // Incrementa o conteúdo de um bloco e armazena o resultado em um registrador de dados interno
#define PICC_RESTORE          0xC2               // Lê os conteudos dobloco em um registrador de dados interno
#define PICC_TRANSFER         0xB0               // Escreve  os conteudos do bloco em um registrador de dados interno


//Código de erro do MFRC522
#define MI_OK                 0
#define MI_NOTAGERR           1
#define MI_ERR                2

//------------------ Registradores MFRC522---------------
//Page 0:Command and Status
#define     Reserved00            0x00
#define     CommandReg            0x01
#define     CommIEnReg            0x02
#define     DivlEnReg             0x03
#define     CommIrqReg            0x04
#define     DivIrqReg             0x05
#define     ErrorReg              0x06
#define     Status1Reg            0x07
#define     Status2Reg            0x08
#define     FIFODataReg           0x09
#define     FIFOLevelReg          0x0A
#define     WaterLevelReg         0x0B
#define     ControlReg            0x0C
#define     BitFramingReg         0x0D
#define     CollReg               0x0E
#define     Reserved01            0x0F
//Page 1:Command
#define     Reserved10            0x10
#define     ModeReg               0x11
#define     TxModeReg             0x12
#define     RxModeReg             0x13
#define     TxControlReg          0x14
#define     TxAutoReg             0x15
#define     TxSelReg              0x16
#define     RxSelReg              0x17
#define     RxThresholdReg        0x18
#define     DemodReg              0x19
#define     Reserved11            0x1A
#define     Reserved12            0x1B
#define     MifareReg             0x1C
#define     Reserved13            0x1D
#define     Reserved14            0x1E
#define     SerialSpeedReg        0x1F
//Page 2:CFG
#define     Reserved20            0x20
#define     CRCResultRegM         0x21
#define     CRCResultRegL         0x22
#define     Reserved21            0x23
#define     ModWidthReg           0x24
#define     Reserved22            0x25
#define     RFCfgReg              0x26
#define     GsNReg                0x27
#define     CWGsPReg              0x28
#define     ModGsPReg             0x29
#define     TModeReg              0x2A
#define     TPrescalerReg         0x2B
#define     TReloadRegH           0x2C
#define     TReloadRegL           0x2D
#define     TCounterValueRegH     0x2E
#define     TCounterValueRegL     0x2F
//Page 3:TestRegister
#define     Reserved30            0x30
#define     TestSel1Reg           0x31
#define     TestSel2Reg           0x32
#define     TestPinEnReg          0x33
#define     TestPinValueReg       0x34
#define     TestBusReg            0x35
#define     AutoTestReg           0x36
#define     VersionReg            0x37
#define     AnalogTestReg         0x38
#define     TestDAC1Reg           0x39
#define     TestDAC2Reg           0x3A
#define     TestADCReg            0x3B
#define     Reserved31            0x3C
#define     Reserved32            0x3D
#define     Reserved33            0x3E
#define     Reserved34            0x3F
//-----------------------------------------------

unsigned char serNum[5];       // Constante para guardar el numero de serie leido.

bool MFRC522IsCard();
bool MFRC522ReadCardSerial();
void MFRC522Init();
void MFRC522Reset();
void MFRC522Write(unsigned char addr, unsigned char val);
void MFRC522AntennaOn(void);
unsigned char MFRC522Read(unsigned char addr);
void MFRC522SetBitMask(unsigned char reg, unsigned char mask);
void MFRC522ClearBitMask(unsigned char reg, unsigned char mask);
void MFRC522CalculateCRC(unsigned char *pIndata, unsigned char len, unsigned char *pOutData);
unsigned char MFRC522Request(unsigned char reqMode, unsigned char *TagType);
unsigned char MFRC522ToCard(unsigned char command, unsigned char *sendData, unsigned char sendLen, unsigned char *backData, unsigned int *backLen);
unsigned char MFRC522Anticoll(unsigned char *serNum);
unsigned char MFRC522Halt();

#endif /* RFID_H_ */
