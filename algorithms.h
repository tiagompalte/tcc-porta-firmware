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
 * algorithms.h - Driver algorithms to compression.
 * ************************************************************** */

#ifndef ALGORITHMS_H_
#define ALGORITHMS_H_

enum {
    dictionarySize = (71+20) + (97+29), // maximum number of entries defined for the dictionary
    codeLength = 8, // the codes which are taking place of the substrings
    maxValue = dictionarySize - 1
};

extern void compress();

#endif /* ALGORITHMS_H_ */
