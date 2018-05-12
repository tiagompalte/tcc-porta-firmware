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
 * dictionary.h - The dictionary for the compression.
 * ************************************************************** */

#ifndef DICTIONARY_H_
#define DICTIONARY_H_

enum {
    emptyPrefix = -1 // empty prefix for ASCII characters
};

// the "string" in the dictionary consists of the last byte of the string and an index to a prefix for that string
struct DictNode {
    int value; // the position in the list
    int prefix; // prefix for byte > 255
    int character; // the last byte of the string
    struct DictNode *next;
};

extern void dictionaryInit();
extern void appendNode(struct DictNode *node);
extern void dictionaryDestroy();
extern int dictionaryLookup(int prefix, int character);
extern int dictionaryPrefix(int value);
extern int dictionaryCharacter(int value);
extern void dictionaryAdd(int prefix, int character, int value);


#endif /* DICTIONARY_H_ */
