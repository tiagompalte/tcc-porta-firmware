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
 * Header do Controlador do Buzzer
 * v1.0
 * 2017
 * ************************************************************** */


#ifndef BUZZER_H_
#define BUZZER_H_

extern void BuzzerInit();
extern void BuzzerEnable();
extern void BuzzerDisable();
extern void BuzzerEntradaPermitida();
extern void BuzzerNaoPermitida();

#endif /* BUZZER_H_ */
