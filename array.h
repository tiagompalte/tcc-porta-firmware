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

typedef struct{
    int prefix; // prefix for byte > 255
    int character; // the last byte of the string
} DictElement;

extern void dictionaryArrayAdd(int prefix, int character, int value);
extern int dictionaryArrayPrefix(int value);
extern int dictionaryArrayCharacter(int value);

#endif /* ARRAY_H_ */
