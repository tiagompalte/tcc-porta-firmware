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
 * Header do Controlador do Teclado
 * v1.0
 * 2017
 * ************************************************************** */

#ifndef KEYBOARD_H_
#define KEYBOARD_H_
#include <stdint.h>
#include <stdbool.h>

extern uint8_t KeyboardGetKey();
extern bool KeyboardIsPressed();
extern void testeTeclado();

#endif /* KEYBOARD_H_ */
