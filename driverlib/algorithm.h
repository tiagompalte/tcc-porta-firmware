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

#ifndef DRIVERLIB_ALGORITHM_H_
#define DRIVERLIB_ALGORITHM_H_

#define NUM_AMOSTRAS 48000
#define DELAY_MAX 75
#define MINIMO_VALIDACAO 0.9

#include <stdint.h>
#include <stdbool.h>

// Variáveis
uint16_t indiceAmostra;
uint32_t bufferCapture[1];
uint8_t bufferConversao[NUM_AMOSTRAS + 2*DELAY_MAX];
uint8_t bufferDatabase[NUM_AMOSTRAS + 2*DELAY_MAX];

int rAuto;
uint8_t rCross[2*DELAY_MAX + 1];

/*extern float autoCorrelate(float *signal);
extern float crossCorrelate(float *signal1, float *signal2); */
extern int autoCorrelate(uint8_t *signal);
extern int crossCorrelate(uint8_t *signal1, uint8_t *signal2);
extern int validate();

#endif /* ALGORITHM_ALGORITHM_H_ */
