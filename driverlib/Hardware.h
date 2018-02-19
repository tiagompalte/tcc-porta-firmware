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
 * Header Controlador Geral do Hardware
 * v1.0
 * 2017
 * ************************************************************** */


#ifndef HARDWARE_H_
#define HARDWARE_H_

typedef enum CardStatus
{
    CardDetected,
    CardNotDetected
}CardStatus;

typedef enum UserStatus
{
    EntryAllowed,
    EntryNotAllowed,
    UserNotRegistered,
    UserBlocked
}UserStatus;

typedef enum UserOptionsStatus
{
    KeyPassWord,
    VoicePassWord,
    none
}UserOptionsStatus;

#include "stdint.h"
uint32_t ui32SysClock;
uint32_t ulDelayms;
extern void HardwareInit();
extern void HardwareLoop();
extern void HardwareRFID();
extern void buscaCadastro();
extern void HardwareControl();
extern void HardwarePassWordControl();
extern void InitConsole();
extern void PasswordValidate();
extern void VerificaTentativas();
void LCDIntHandler(void);
void BuzzerIntHandler(void);
void KeyBoardIntHandler(void);
#endif /* HARDWARE_H_ */
