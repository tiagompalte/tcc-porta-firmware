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
#include <stdio.h>
#include <stdint.h>
#include <string.h>
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

unsigned char* strrev(unsigned char* str)
{
    char strRev[5] = {0, 0, 0, 0, '\0'};
    char *iterator = str + strlen(str) - 2;
    int i = 0;
    for(i = 0; i < strlen(str); i++)
    {
        strRev[i] = *iterator--;
    }
    return strRev;
}

int printString(unsigned char* str)
{
    int i = 0;
    for(i = 0; i < strlen(str); i++)
    {

       printf("\n RFID: %02x",  ( unsigned int)str[i]);

    }
}

int strSep(unsigned char* strIn, unsigned char* strOut)
{
    unsigned char byte1;
    unsigned char byte2;
    int i = 0;
    for(i = 0; i < strlen(strIn); i++)
    {
       byte1 = (strIn[i]>>4)&0x0F;
       byte2 = (strIn[i])&0x0F;
       strOut[2*i] = byte1;
       strOut[2*i + 1] = byte2;
    }

    return 1;
}

int strSep2(unsigned char* strIn, unsigned char* strOut)
{
    unsigned char nibbleL;
    unsigned char nibbleM;
    int i = 0;
    for(i = 0; i < 22000; i++)
    {
    	nibbleM = (strIn[2*i]>>4)&0x0F;
    	nibbleL = (strIn[2*i])&0x0F;
    	strOut[2*i] = nibbleM;
    	strOut[2*i + 1] = nibbleL;
    }

    return 1;
}

int hex2str(unsigned char* strIn, unsigned char* strOut)
{
    int i = 0;
    unsigned char c;
    for(i = 0; i < 8; i++)
    {
        c = strIn[i];
        strOut[i] = (c < 10) ? (c+48) : (c+55);

    }
}

int hex2str2(unsigned char* strIn, unsigned char* strOut)
{
    int i = 0;
    unsigned char c;
    for(i = 0; i < 44000; i++)
    {
        c = strIn[i];
        strOut[i] = (c < 10) ? (c+48) : (c+55);

    }
}

int strInsertSpace(unsigned char* strIn, unsigned char* strOut)
{
    int i = 0;
    int j = 0;
    for(i = 0; i < strlen(strIn); i++)
    {
        strOut[2*i + j] = strIn[2*i];
        strOut[2*i + j + 1] = strIn[2*i + 1];
        strOut[2*i + j + 2] = ' ';
        j++;
    }
}
