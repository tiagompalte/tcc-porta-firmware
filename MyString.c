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
 * Realiza Oparações com Strings
 * v1.0
 * 2017
 * ************************************************************** */


#include "driverlib/MyString.h"
#include "stdint.h"

bool StrCompare(unsigned char* str1, unsigned char* str2, uint32_t length)
{
    while(length>0)
    {
        if(*str1 != *str2)
        {
            return false;
        }
        str1++;
        str2++;
        length--;
    }
    return true;
}
