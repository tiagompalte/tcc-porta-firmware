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
 * Realiza Operações com Strings
 * v1.0
 * 2017
 * ************************************************************** */


#ifndef MYSTRING_H_
#define MYSTRING_H_
#include "stdbool.h"
#include "stdint.h"
bool StrCompare(unsigned char* str1, unsigned char* str2, uint32_t length);
unsigned char* strrev(unsigned char* str);
int printString(unsigned char* str);
int strSep(unsigned char* strIn, unsigned char* strOut);
#endif /* MYSTRING_H_ */
