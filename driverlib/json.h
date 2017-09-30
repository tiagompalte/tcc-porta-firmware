/* ************************************************************** *
 * UTFPR - Universidade Tecnologica Federal do Paran�
 * Engenharia Eletr�nica
 * Trabalho de Conclus�o de Curso
 * ************************************************************** *
 * Sistema de Seguran�a baseado em Reconhecimento de Senha Falada
 * ************************************************************** *
 * Equipe:
 * Luiz Felipe Kim Evaristo
 * Tauan Marinho
 * Tiago Henrique Faxina
 * Tiago Mariani Palte
 * ************************************************************** *
 * Header Sketch JSON
 * ************************************************************** */

#ifndef __JSON_H__
#define __JSON_H__

#ifndef JSON_H_
#define JSON_H_

#define INVALID_INT             ((int32_t)(0x80000000))

extern int32_t JSONParse(uint32_t ui32Index,
                                 tUserReport *psWeatherReport,
                                 struct pbuf *psBuf);
#endif

#endif
