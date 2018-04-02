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
 * Header Sketch Algoritmo
 * ************************************************************** */

#ifndef DRIVERLIB_AUDIOSAMPLE_H_
#define DRIVERLIB_AUDIOSAMPLE_H_

#define NUM_AMOSTRAS 22000

#include <stdint.h>
#include <stdbool.h>

// Vari�veis
uint16_t indiceAmostra;
uint32_t bufferCapture[1];
uint8_t bufferConversao[NUM_AMOSTRAS];

extern uint8_t scaleSample(uint32_t sample);

#endif /* ALGORITHM_ALGORITHM_H_ */
