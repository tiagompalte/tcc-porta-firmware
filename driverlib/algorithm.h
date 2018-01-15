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

#define N_MEDIAS 6
#define NUM_AMOSTRAS 144000/N_MEDIAS
#define DELAY_MAX 75
#define MINIMO_VALIDACAO 0.9

#include <stdint.h>
#include <stdbool.h>

// Variáveis
uint16_t indiceAmostra;
uint32_t bufferCapture[1], bufferTemp[N_MEDIAS];
float bufferConversao[NUM_AMOSTRAS + 2*DELAY_MAX];
//float bufferConversaoNorm[NUM_AMOSTRAS + 2*DELAY_MAX];
float bufferDatabase[NUM_AMOSTRAS];
//float bufferDatabaseNorm[NUM_AMOSTRAS + 2*DELAY_MAX];

float rAuto;
float rCross[2*DELAY_MAX + 1];

extern float autoCorrelate(float *signal);
extern float crossCorrelate(float *signal1, float *signal2);
extern bool validate();

#endif /* ALGORITHM_ALGORITHM_H_ */
