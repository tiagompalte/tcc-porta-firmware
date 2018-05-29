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
 * Sketch do Filtro FIR
 * ************************************************************** */

#include "driverlib/fir.h"
#include <driverlib/audioSample.h>
#include <math.h>

// Sobrescreve o vetor de conversao com os valores filtrados
void FIR(uint8_t sinal[]) {
	float coefs[] = {-0.014937, -0.022705, -0.01733, -0.0000579, 0.020381, 0.031755, 0.024963, 0.000000022,
					 -0.032109, -0.052976, -0.044935, 0.00005786, 0.075042, 0.159124, 0.225011, 0.249918,
					 0.225011, 0.159124, 0.075042, 0.00005786, -0.044935, -0.052976, -0.032109,
					 0.000000022, 0.024963, 0.031755, 0.020381, -0.0000579, -0.01733, -0.022705, -0.014937};
	int i, j;
	float y = 0;

	for (i = 0; i <= ORDEM; i++) {
		bufferFiltro[i] = bufferConversao[2*i];
	}

	for (i = 0; i < NUM_AMOSTRAS; i++) {
		y = 0;
		//int y2 = 0;
		for (j = 0; j <= ORDEM; j++) {
			y += bufferFiltro[j]*coefs[j];
			bufferFiltro[ORDEM - j] = bufferFiltro[ORDEM - j - 1];
		}
		bufferFiltro[0] = bufferConversao[ORDEM + 2*i];

		//y2 = (int)y/2;

		// caso gere " ' \ / ou espaço intermediário, repor com o caracter anterior
		/*if (y2 <= 32) {
			bufferConversao[2*i] = 33;
		} else if (y2 == 34 || y2 == 47 || y2 == 92 || y2 == 127) {
		    bufferConversao[2*i] = (y2 - 1);
		} else if (y2 == 128) {
			bufferConversao[2*i] = (y2 - 2);
		} else {
			bufferConversao[2*i] = y2;
		} */

		bufferConversao[2*i] = (uint8_t)y;

	}

	int media = 0;

	for (i = 0; i < NUM_AMOSTRAS; i++) {
		media += bufferConversao[2*i];
	}
	media = media/NUM_AMOSTRAS;

	for (i = 0; i < NUM_AMOSTRAS; i++) {
		if(((bufferConversao[2*i] <= media+26) && (bufferConversao[2*i] >= media-26)) || ((bufferConversao[2*i] <= 132) && (bufferConversao[2*i] >= 120))) {
		//if((bufferConversao[2*i] <= 154) && (bufferConversao[2*i] >= 102)) {
			bufferConversao[2*i] = 0;
		}
	}

	return;
}

