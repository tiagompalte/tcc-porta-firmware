/* ************************************************************** *
 * UTFPR - Universidade Tecnologica Federal do Paraná
 * Engenharia Eletrônica
 * Trabalho de Conclusão de Curso
 * ************************************************************** *
 * Sistema de Seguranca baseado em Reconhecimento de Senha Falada
 * ************************************************************** *
 * Equipe:
 * Luiz Felipe Kim Evaristo
 * Tauan Marinho
 * Tiago Henrique Faxina
 * Tiago Mariani Palte
 * ************************************************************** *
 * algorithms.c - Driver algorithms to compression.
 * ************************************************************** */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dictionary.h"
#include "algorithms.h"
#include "array.h" // a faster array for decompression

void compress(char *inputFile, char *outputFile) {
    int prefix = sizeof(inputFile);

    int character;

    int nextCode;
    int index;
    
    // LZW starts out with a dictionary of 256 characters (in the case of 8 codeLength) and uses those as the "standard"
    //  character set.
    //nextCode = 256; // next code is the next available string code
    //dictionaryInit();
    
    // while (there is still data to be read)
    int i;
    for (i = 0; i < sizeof(inputFile); i++) { // ch = read a character;
        
        // if (dictionary contains prefix+character)
        if ((index = dictionaryLookup(prefix, sizeof(inputFile))) != -1) prefix = index; // prefix = prefix+character
        else { // ...no, try to add it
            // encode s to output file
            //writeBinary(outputFile, prefix);
            
            // add prefix+character to dictionary
            if (nextCode < dictionarySize) dictionaryAdd(prefix, character, nextCode++);
            
            // prefix = character
            prefix = character; //... output the last string after adding the new one
        }
    }
    // encode s to output file
    //writeBinary(outputFile, prefix); // output the last code
    
    //if (leftover > 0) fputc(leftoverBits << 4, outputFile);

    // free the dictionary here
    dictionaryDestroy();
}


void writeBinary(FILE * output, int code) {
    if (leftover > 0) {
        int previousCode = (leftoverBits << 4) + (code >> 8);

        fputc(previousCode, output);
        fputc(code, output);

        leftover = 0; // no leftover now
    } else {
        leftoverBits = code & 0xF; // save leftover, the last 00001111
        leftover = 1;

        fputc(code >> 4, output);
    }
}

int readBinary(FILE * input) {
    int code = fgetc(input);
    if (code == EOF) return 0;

    if (leftover > 0) {
        code = (leftoverBits << 8) + code;

        leftover = 0;
    } else {
        int nextCode = fgetc(input);

        leftoverBits = nextCode & 0xF; // save leftover, the last 00001111
        leftover = 1;

        code = (code << 4) + (nextCode >> 4);
    }
    return code;
}


