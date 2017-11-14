/*
 * criptografy.h
 *
 *  Created on: 13 de out de 2017
 *      Author: tauanmarinho
 */

#ifndef CRIPTOGRAFY_H_
#define CRIPTOGRAFY_H_



extern int encode(char *palavra, char *chave, char *output);
extern int decode(char *input, char *chave, char *output);
extern int detectorLetra(char *key);


#endif /* CRIPTOGRAFY_H_ */





