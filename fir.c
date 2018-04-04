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

#include <driverlib/fir.h>
#include <driverlib/audioSample.h>
#include <math.h>

// Sobrescreve o vetor de conversao com os valores filtrados
void FIR(uint8_t sinal) {
	int i, j;
	float y = 0;

	for (i = 0; i <= ORDEM; i++) {
		bufferFiltro[i] = bufferConversao[i];
	}

	for (i = 0; i < NUM_AMOSTRAS; i++) {
		y = 0;
		for (j = 0; j <= ORDEM; j++) {
			y += bufferFiltro[j]*coefFiltro[j];
			bufferFiltro[ORDEM - j] = bufferFiltro[ORDEM - j - 1];
		}
		bufferFiltro[0] = bufferConversao[ORDEM + i];
		bufferConversao[i] = y;
	}

	return;
}

