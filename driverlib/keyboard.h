/*
 * teclado.h
 *
 *  Created on: 1 de set de 2017
 *      Author: thfax
 */

#ifndef KEYBOARD_H_
#define KEYBOARD_H_
#include <stdint.h>
#include <stdbool.h>

extern void KeyboardInit();
extern uint8_t KeyboardGetKey();
extern bool KeyboardIsPressed();
extern void testeTeclado();

#endif /* KEYBOARD_H_ */
