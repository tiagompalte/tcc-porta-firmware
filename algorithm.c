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

/*
// Retornar o maior valor positivo de um vetor de float
float _maxFloat(int n, float vector[]) {
	int i;
	float maior = 0;

	for (i=0; i<n; i++) { // Roda o vetor
		if (vector[i] > maior) {		// Se for maior que o maior armazenado, troca
			maior = vector[i];
		}
	}

	return maior;						// Retorna o maior valor
}

// Efetua a escala dos valores dos vetores armazenados (divide todos os valores pelo maior de cada)
void _normalizar() {
	uint32_t maxValue = 0;
	int i;

	maxValue = _maxFloat(NUM_AMOSTRAS + 2*DELAY_MAX, bufferDatabase);		// Obter o maior valor do vetor DB

	for (i=0; i<NUM_AMOSTRAS + 2*DELAY_MAX; i++) {				// Roda os valores
		bufferDatabase[i] = (bufferDatabase[i])/(maxValue);		// Realiza a divisão
	}

	maxValue = _maxFloat(NUM_AMOSTRAS + 2*DELAY_MAX, bufferConversao);	// Obter o maior valor do vetor AQ

	for (i=0; i<NUM_AMOSTRAS + 2*DELAY_MAX; i++) {				// Roda os valores
		bufferConversao[i] = (bufferConversao[i])/(maxValue);	// Realiza a divisão
	}

	return;
} */

// Retorna o valor central da autocorrelação de um vetor (Rxx[0])
int autoCorrelate(int signal[]) {
	int n;
	rAuto = 0;

	for (n = 0; n < (NUM_AMOSTRAS + 2*DELAY_MAX) - 1; n++) {	// Roda as posições
		rAuto += signal[n]*signal[n];							// Efetua a soma dos quadrados
	}

	return rAuto;												// Retorna o valor
}

// Retorna o maior valor de crosscorrelação entre dois vetores (max(Rxy[k]))
int crossCorrelate(int signal1[], int signal2[]) {
	int n, k;

	for (k = -DELAY_MAX; k <= DELAY_MAX; k++) {						// Para os atrasos relevantes
		for (n = 0; n < (NUM_AMOSTRAS + 2*DELAY_MAX) - 1; n++) {	// Roda as posições
			rCross[DELAY_MAX + k] += signal1[n]*signal2[n-k];		// Efetua a operação
		}
	}

	return _max(2*DELAY_MAX + 1, rCross);
	//return _maxFloat(2*DELAY_MAX + 1, rCross);						// Retorna o maior valor
}

// Validar se o sinal AQ é parecido com o DB, considerando a tolerância
bool validate() {
	float coefCorrelacao;
	//float
	int auto1, auto2, cross12;

	//_normalizar();													// Escala os vetores

	auto1 = autoCorrelate(bufferDatabase);							// Calcula Autocorrelação do sinal DB
	auto2 = autoCorrelate(bufferConversao);							// Calcula Autocorrelação do sinal AQ
	cross12 = crossCorrelate(bufferDatabase, bufferConversao);		// Calcula Crosscorrelação DB/AQ

	coefCorrelacao = (cross12*cross12)/sqrt(auto1*auto2);			// Calcula o coeficiente de correlação (rho)

	return (coefCorrelacao >= MINIMO_VALIDACAO ? true : false);		// Retorna true se o coeficiente é maior que o mínimo tolerado
}
