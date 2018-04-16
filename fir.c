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
	float coefs[] = {-0.0067, -0.01477, -0.02145, -0.02554, -0.02596, 0.02198, -0.013277, -0.000019,
					 0.0171, 0.03695, 0.05801, 0.07856, 0.09684, 0.11121, 0.1204, 0.12357,
					 0.1204, 0.11121, 0.09684, 0.07856, 0.05801, 0.03695, 0.0171,
					 -0.000019, -0.013277, -0.02198, -0.02596, -0.02554, -0.02145, -0.01477, -0.0067};
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
		bufferConversao[i] = y;
	}

	return;
}

