/* ************************************************************** *
 * UTFPR - Universidade Tecnologica Federal do Paran�
 * Engenharia Eletr�nica
 * Trabalho de Conclus�o de Curso
 * ************************************************************** *
 * Sistema de Seguran�a baseado em Reconhecimento de Senha Falada
 * ************************************************************** *
 * Equipe:
 * Luiz Felipe Kim Evaristo
 * Tauan Marinho
 * Tiago Henrique Faxina
 * Tiago Mariani Palte
 * ************************************************************** *
 * Header para Parametros de Audio e Variaveis
 * ************************************************************** */

#ifndef DRIVERLIB_AUDIOSAMPLE_H_
#define DRIVERLIB_AUDIOSAMPLE_H_

#define NUM_AMOSTRAS 22000

#include <stdint.h>
#include <stdbool.h>

// Vari�veis
int conversionEnd;
uint16_t indiceAmostra;
uint32_t bufferCapture[1];
uint8_t bufferConversao[NUM_AMOSTRAS];

#endif /* ALGORITHM_ALGORITHM_H_ */
