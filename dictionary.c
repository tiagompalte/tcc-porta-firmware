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
 * dictionary.c - The dictionary for the compression.
 * ************************************************************** */

#include <stdio.h>
#include <stdlib.h>
#include "dictionary.h"

// the dictionary
struct DictNode *dictionary, *tail;

// initialize the dictionary
void dictionaryInit() {
    int i;
    struct DictNode *node;
    for (i = 48; i < 58; i++) { // ASCII
		node = (struct DictNode *)malloc(sizeof(struct DictNode));
		node->prefix = emptyPrefix;
		node->character = i;
		appendNode(node);
    }

    for (i = 65; i < 71; i++) {
		node = (struct DictNode *)malloc(sizeof(struct DictNode));
		node->prefix = emptyPrefix;
		node->character = i;
		appendNode(node);
    }
}

// add node to the list
void appendNode(struct DictNode *node) {
    if (dictionary != NULL) tail->next = node;
    else dictionary = node;
    tail = node;
    node->next = NULL;
}

// destroy the whole dictionary down to NULL
void dictionaryDestroy() {
    while (dictionary != NULL) {
        dictionary = dictionary->next; /* the head now links to the next element */
    }
}

// is prefix + character in the dictionary?
int dictionaryLookup(int prefix, int character) {
    struct DictNode *node;
    for (node = dictionary; node != NULL; node = node->next) { // ...traverse forward
        if (node->prefix == prefix && node->character == character) return node->value;
    }
    return emptyPrefix;
}

int dictionaryPrefix(int value) {
    struct DictNode *node;
    for (node = dictionary; node != NULL; node = node->next) { // ...traverse forward
        if (node->value == value) return node->prefix;
    }
    return -1;
}

int dictionaryCharacter(int value) {
    struct DictNode *node;
    for (node = dictionary; node != NULL; node = node->next) { // ...traverse forward
        if (node->value == value) {
            //printf("\nNODE %i %i %i\n", node->value, node->prefix, node->character);
            return node->character;
        }
    }
    return -1;
}

// add prefix + character to the dictionary
void dictionaryAdd(int prefix, int character, int value) {
    struct DictNode *node;
    node = (struct DictNode *)malloc(sizeof(struct DictNode));
    node->value = value;
    node->prefix = prefix;
    node->character = character;
    //printf("\n(%i) = (%i) + (%i)\n", node->value, node->prefix, node->character);
    appendNode(node);
}
