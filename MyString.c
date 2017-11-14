/*
 * MyString.C
 *
 *  Created on: 5 de nov de 2017
 *      Author: thfax
 */

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
