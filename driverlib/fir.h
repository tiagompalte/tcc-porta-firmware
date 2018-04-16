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
 * Header do Filtro FIR (Constrained Least-Squares)
 * ************************************************************** */
#include <stdint.h>

#define ORDEM 30

uint8_t bufferFiltro[ORDEM + 1];

void FIR(uint8_t sinal[]);
