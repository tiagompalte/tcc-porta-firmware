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

// Retornar o maior valor positivo de um vetor
int _max(int n, int vector[]) {
	int i;
	int maior = 0;

	for (i=0; i<n; i++) { // Roda o vetor
		if (vector[i] > maior) {		// Se for maior que o maior armazenado, troca
			maior = vector[i];
		}
	}

	return maior;						// Retorna o maior valor
}

// Retorna o valor central da autocorrelação de um vetor (Rxx[0])
int autoCorrelate(uint8_t signal[]) {
	int n;
	rAuto = 0;

	for (n = 0; n < (NUM_AMOSTRAS + 2*DELAY_MAX) - 1; n++) {	// Roda as posições
		rAuto += signal[n]*signal[n];							// Efetua a soma dos quadrados
	}

	return rAuto;												// Retorna o valor
}

// Retorna o maior valor de crosscorrelação entre dois vetores (max(Rxy[k]))
int crossCorrelate(uint8_t signal1[], uint8_t signal2[]) {
	int n, k;

	for (k = -DELAY_MAX; k <= DELAY_MAX; k++) {						// Para os atrasos relevantes
		for (n = 0; n < (NUM_AMOSTRAS + 2*DELAY_MAX) - 1; n++) {	// Roda as posições
			rCross[DELAY_MAX + k] += signal1[n]*signal2[n-k];		// Efetua a operação
		}
	}

	return _max(2*DELAY_MAX + 1, rCross);
}

// Validar se o sinal AQ é parecido com o DB, considerando a tolerância
int validate() {
	float coefCorrelacao;
	int auto1, auto2, cross12;

	auto1 = autoCorrelate(bufferDatabase);							// Calcula Autocorrelação do sinal DB
	auto2 = autoCorrelate(bufferConversao);							// Calcula Autocorrelação do sinal AQ
	cross12 = crossCorrelate(bufferDatabase, bufferConversao);		// Calcula Crosscorrelação DB/AQ

	coefCorrelacao = cross12/sqrt(auto1*auto2);						// Calcula o coeficiente de correlação (rho)

	return (coefCorrelacao >= MINIMO_VALIDACAO ? 1 : 0);		// Retorna true se o coeficiente é maior que o mínimo tolerado
}
