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
 * json.h - File to handle JSON formatted data from websites.
 * ************************************************************** */

#ifndef __JSON_H__
#define __JSON_H__

#ifndef JSON_H_
#define JSON_H_

#define INVALID_INT             ((int32_t)(0x80000000))

extern int32_t JSONParseGET(uint32_t ui32Index,
                                 tUserReport *psWeatherReport,
                                 struct pbuf *psBuf);
extern int32_t JSONParseGETteste(uint32_t ui32Index,
                                 tUserReport *psWeatherReport,
                                 struct pbuf *psBuf);
extern int32_t JSONParsePOST(uint32_t ui32Index,
                                 tUserReport *psWeatherReport,
                                 struct pbuf *psBuf);
extern int32_t JSONParsePOSTKEY(uint32_t ui32Index,
                                 tUserReport *psWeatherReport,
                                 struct pbuf *psBuf);
extern int32_t JSONParsePOSTACCESS(uint32_t ui32Index,
                                 tUserReport *psWeatherReport,
                                 struct pbuf *psBuf);
#endif

#endif
