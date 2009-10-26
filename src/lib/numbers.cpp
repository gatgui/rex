/*

Copyright (C) 2009  Gaetan Guidet

This file is part of rex.

rex is free software; you can redistribute it and/or modify it
under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation; either version 2.1 of the License, or (at
your option) any later version.

rex is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
USA.

*/

#include <rex/rex_parser.h>

bool read_decimal(const char **ppc, unsigned long &v) {
  const char *pc = *ppc;
  unsigned char digits[32];
  int i = 0;
  while (CHAR_IS(*pc,DIGIT_CHAR)) {
    digits[i++] = *pc - '0';
    ++pc;
  }
  if (i != 0) {
    unsigned long f = 1;
    v = 0;
    while (--i >= 0) {
      v += digits[i] * f;
      f *= 10;
    }
    *ppc = pc;
    return true;
  }
  return false;
}

bool read_char_hexa(const char **ppc, unsigned char &v) {
  const char *pc = *ppc;
  unsigned char digits[2] = {0,0};
  int i = 0;
  while (CHAR_IS(*pc,HEXA_CHAR) && i<2) {
    if (CHAR_IS(*pc,DIGIT_CHAR)) {
      digits[i++] = *pc - '0';
    } else if (CHAR_IS(*pc,LOWER_CHAR)) {
      digits[i++] = 10 + (*pc - 'a');
    } else {
      digits[i++] = 10 + (*pc - 'A');
    }
    ++pc;
  }
  if (i != 0) {
    if (i == 1) {
      v = digits[0];
    } else {
      v = digits[0]*16 + digits[1];
    }
    *ppc = pc;
    return true;
  }
  return false;
}

bool read_char_octal(const char **ppc, unsigned char &v) {
  const char *pc = *ppc;
  unsigned char digits[3] = {0,0,0};
  int i = 0;
  while (CHAR_IS(**ppc, DIGIT_CHAR) && i<3) {
    if (*pc > '7') return false;
    digits[i++] = *pc - '0';
    ++pc;
  }
  if (i != 0) {
    unsigned long lv;
    
    if (i == 1) {
      lv = digits[0];
    } else if (i == 2) {
      lv = digits[0]*8 + digits[1];
    } else {
      lv = digits[0]*64 + digits[1]*8 + digits[2];
    }
    
    if (lv >= 256) {
      return false;
    }
    v = (unsigned char)lv;
    *ppc = pc;
    return true;
  }
  return false;
}
