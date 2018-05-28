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
#include <driverlib/audioSample.h>
#include "dictionary.h"
#include "algorithms.h"
#include "array.h" // a faster array for decompression

// compression
void compress() {
    int prefix = bufferConversao[0];
    int nextCode;
    int index;

    nextCode = 71; // next code is the next available string code

    dictionaryInit();

    // while (there is still data to be read)
    int i;
    for(i = 1; i < sizeof(bufferConversao); i++) { // ch = read a character;
        // if (dictionary contains prefix+character)
        if ((index = dictionaryLookup(prefix, bufferConversao[i])) != -1) prefix = index; // prefix = prefix+character
        else { // ...no, try to add it
            insertArray(&bufferComprimido, prefix);
            // add prefix+character to dictionary
            if (nextCode < dictionarySize) dictionaryAdd(prefix, bufferConversao[i], nextCode++);
            //if ((nextCode >= 48 && nextCode < 71) || nextCode == 92) nextCode++; //Evitar barra invertida
            if (nextCode == 92) nextCode++; //Evitar barra invertida
            prefix = bufferConversao[i]; //... output the last string after adding the new one
        }
    }
    insertArray(&bufferComprimido, prefix); // output the last code
    // free the dictionary here
    dictionaryDestroy();
}
