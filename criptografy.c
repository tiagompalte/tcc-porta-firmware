/*
 * criptografy.c
 *
 *  Created on: 13 de out de 2017
 *      Author: tauanmarinho
 */




#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define OK 1
#define ERROR 0

int encode(char *palavra, char *chave, char *output) {
    int sizePalavra = strlen(palavra);
    int sizeChave = strlen(chave);
    char senhaMaior = 'T';
    int Var [1024];
    int i = 0;


    if(strlen(palavra) > strlen(chave)){

        for (i = strlen(chave); i < strlen(palavra); i++){
            chave[i] = senhaMaior;
        }

        } else if(strlen(palavra) < strlen(chave)) {

                for (i = sizeChave; i > sizePalavra; i--){
                    palavra[i] = ' ';
                }
    }

    for(i = 0; i < strlen(palavra); i++){
        Var[i] = ((palavra[i] + 32 + chave[i] )% 159) + 30;

    }


    for(i = 0; i < strlen(palavra); i++){
            output[i] = (char)Var[i];
        }

    return OK;
}

int decode(char *input, char *chave, char *output){
    char senhaMaior = 'T';
    char mFinal[1024];
    int i = 0;

    if(strlen(input) > strlen(chave)){
        for (i = strlen(chave); i < strlen(input); i++){
            chave[i] = senhaMaior;
        }

        } else if(sizeof(input) < strlen(chave)) {
                for (i = strlen(chave); i > strlen(input); i--){
                    input[i] = ' ';
                }
    }
    for(i = 0; i < strlen(input); i++){
        mFinal[i] = (input[i] - 30 + 159 - 32 - chave[i]);
    }

    for(i = 0; i < strlen(input); i++){
            output[i] = (char)mFinal[i];
        }
    return OK;
}


int detectorLetra(char *key){
    int i = 0;
    for( i = 0; i < strlen(key); i++){

        if ((char)key[i] > 97){
            key[i] = key[i] - 32;
        }
    }
    return OK;
}
