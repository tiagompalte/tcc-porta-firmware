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
 * Sketch do Algoritmo
 * ************************************************************** */

#include <driverlib/audioSample.h>
#include <math.h>

// Retornar o maior valor positivo de um vetor
uint8_t scaleSample(uint32_t sample) {
	return (uint8_t)((sample/4096)*255);
}

