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
 * Codec G.711
 * ************************************************************** */

#include <stdint.h>
#include "driverlib/g711.h"

int8_t ALaw_Encode(uint16_t number) {
   const uint16_t ALAW_MAX = 0xFFF;
   uint16_t mask = 0x800;
   uint8_t sign = 0;
   uint8_t position = 11;
   uint8_t lsb = 0;

   if (number < 0) {
      number = -number;
      sign = 0x80;
   }

   if (number > ALAW_MAX) {
      number = ALAW_MAX;
   }

   for (; ((number & mask) != mask && position >= 5); mask >>= 1, position--);
   lsb = (number >> ((position == 4) ? (1) : (position - 4))) & 0x0f;
   return (sign | ((position - 4) << 4) | lsb) ^ 0x55;
}

/*
 * Description:
 *  Decodes an 8-bit unsigned integer using the A-Law.
 * Parameters:
 *  number - the number who will be decoded
 * Returns:
 *  The decoded number
 */
int16_t ALaw_Decode(int8_t number) {
   uint8_t sign = 0x00;
   uint8_t position = 0;
   int16_t decoded = 0;
   number ^= 0x55;
   if(number & 0x80) {
      number &= ~(1<<7);
      sign = -1;
   }
   position = ((number & 0xF0) >>4) + 4;
   if (position != 4) {
      decoded = ((1<<position)|((number&0x0F)<<(position-4))
                |(1<<(position-5)));
   } else {
      decoded = (number<<1)|1;
   }

   return (sign==0)?(decoded):(-decoded);
}
