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

#include <math.h>
#include "driverlib/algorithm.h"

float _maxFloat(float vector[]) {
	int i;
	float maior = 0;

	for (i=0; i<2*DELAY_MAX + 1; i++) {
		if (vector[i] > maior) {
			maior = vector[i];
		}
	}

	return maior;
}

void _normalizar() {
	uint32_t maxValue = 0;
	int i;

	maxValue = _maxFloat(bufferDatabase);

	for (i=0; i<NUM_AMOSTRAS + 2*DELAY_MAX; i++) {
		bufferDatabase[i] = (bufferDatabase[i])/(maxValue);
	}

	maxValue = _maxFloat(bufferConversao);

	for (i=0; i<NUM_AMOSTRAS + 2*DELAY_MAX; i++) {
		bufferConversao[i] = (bufferConversao[i])/(maxValue);
	}

	return;
}

float autoCorrelate(float signal[]) {
	int n;
	rAuto = 0;

	for (n = 0; n < (NUM_AMOSTRAS + 2*DELAY_MAX) - 1; n++) {
		rAuto += signal[n]*signal[n];
	}

	return rAuto;
}

float crossCorrelate(float signal1[], float signal2[]) {
	int n, k;

	for (k = -DELAY_MAX; k <= DELAY_MAX; k++) {
		for (n = 0; n < (NUM_AMOSTRAS + 2*DELAY_MAX) - 1; n++) {
			rCross[DELAY_MAX + k] += signal1[n]*signal2[n-k];
		}
	}

	return _maxFloat(rCross);
}

bool validate() {
	float coefCorrelacao;
	float auto1, auto2, cross12;

	_normalizar();

	auto1 = autoCorrelate(bufferDatabase);
	auto2 = autoCorrelate(bufferConversao);
	cross12 = crossCorrelate(bufferDatabase, bufferConversao);

	coefCorrelacao = (cross12*cross12)/(auto1*auto2);

	return (coefCorrelacao >= MINIMO_VALIDACAO ? true : false);
}
