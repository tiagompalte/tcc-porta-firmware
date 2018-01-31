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
 * Header do Controlador da Trava Elétrica
 * v1.0
 * 2017
 * ************************************************************** */


#ifndef TRAVA_ELETRICA_H_
#define TRAVA_ELETRICA_H_

#include <stdbool.h>
extern void TravaInit();
extern void AcionarTrava();
extern void DesacionarTrava();
bool TravaAcionada();
#endif /* TRAVA_ELETRICA_H_ */
