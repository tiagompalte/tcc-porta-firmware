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
		bufferFiltro[i] = bufferConversao[i];
	}

	for (i = 0; i < NUM_AMOSTRAS; i++) {
		y = 0;
		for (j = 0; j <= ORDEM; j++) {
			y += bufferFiltro[j]*coefs[j];
			bufferFiltro[ORDEM - j] = bufferFiltro[ORDEM - j - 1];
		}
		bufferFiltro[0] = bufferConversao[ORDEM + i];

		// caso gere " ' \ / ou espaço intermediário, repor com o caracter anterior
		if (y <= 32 || y == 34 || y ==  47 || y == 92 || y == 127 || y == 255 ) {
			if(y <= 32)
			{
			    bufferConversao[i] = 33;
			}
			else
			{
			    bufferConversao[i] = y - 1;
			}

		} else {
			bufferConversao[i] = y;
		}
	}

	return;
}

