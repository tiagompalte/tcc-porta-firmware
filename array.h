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
 * array.h - Driver array used on compression.
 * ************************************************************** */

#ifndef ARRAY_H_
#define ARRAY_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  char *array;
  size_t used;
  size_t size;
} Array;

extern void initArray(Array *a, size_t initialSize);
extern void insertArray(Array *a, char element);
extern void freeArray(Array *a);

#endif /* ARRAY_H_ */
