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
 * Header Sketch Algoritmo
 * ************************************************************** */

#ifndef DRIVERLIB_AUDIOSAMPLE_H_
#define DRIVERLIB_AUDIOSAMPLE_H_

#define NUM_AMOSTRAS 32000

#include <stdint.h>
#include <stdbool.h>

// Variáveis
uint16_t indiceAmostra;
uint32_t bufferCapture[1];
uint8_t bufferConversao[NUM_AMOSTRAS];

extern uint8_t scaleSample(uint32_t sample);

#endif /* ALGORITHM_ALGORITHM_H_ */
