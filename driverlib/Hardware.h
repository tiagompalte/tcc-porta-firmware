/*
 * Hardware.h
 *
 *  Created on: 31 de ago de 2017
 *      Author: thfax
 */

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
    UserNotRegistered
}UserStatus;

#include "stdint.h"
uint32_t ui32SysClock;
uint32_t ulDelayms;
extern void HardwareInit();
extern void TesteRFID();
extern void buscaCadastro();
extern void LCDControl();
extern void InitConsole();
#endif /* HARDWARE_H_ */
