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

#include <stdlib.h>
#include <string.h>
#include <rex/class_table.h>

unsigned char char_class[256];

void init_table() {
  unsigned char c;
  
  memset(char_class, 0, 256);
  
  for (c='a'; c<='z'; ++c) {
    char_class[c] = LOWER_CHAR|WORD_CHAR;
  }
  for (c='A'; c<='Z'; ++c) {
    char_class[c] = UPPER_CHAR|WORD_CHAR;
  }
  char_class[(unsigned char)'_'] = WORD_CHAR;
  
  for (c='0'; c<='9'; ++c) {
    char_class[c] = DIGIT_CHAR|HEXA_CHAR;
  }
  
  for (c='a'; c<='f'; ++c) {
    char_class[c] |= HEXA_CHAR;
  }
  for (c='A'; c<='F'; ++c) {
    char_class[c] |= HEXA_CHAR;
  }
  
  char_class[(unsigned char)'\t'] = SPACE_CHAR|ESCAPE_CHAR;
  char_class[(unsigned char)'\n'] = SPACE_CHAR|ESCAPE_CHAR;
  char_class[(unsigned char)'\r'] = SPACE_CHAR|ESCAPE_CHAR;
  char_class[(unsigned char)'\f'] = SPACE_CHAR|ESCAPE_CHAR;
  char_class[(unsigned char)'\v'] = SPACE_CHAR|ESCAPE_CHAR;
  char_class[(unsigned char)' ']  = SPACE_CHAR;
  
  //char_class[(unsigned char)'\t'] = ESCAPE_CHAR;
  //char_class[(unsigned char)'\n'] = ESCAPE_CHAR;
  //char_class[(unsigned char)'\r'] = ESCAPE_CHAR;
  //char_class[(unsigned char)'\f'] = ESCAPE_CHAR;
  //char_class[(unsigned char)'\v'] = ESCAPE_CHAR;
  char_class[(unsigned char)'\b'] = ESCAPE_CHAR;
  char_class[(unsigned char)'\a'] = ESCAPE_CHAR;
  //char_class[(unsigned char)'\e'] = ESCAPE_CHAR;
  
  char_class[(unsigned char)'('] = SPECIAL_CHAR;
  char_class[(unsigned char)')'] = SPECIAL_CHAR;
  char_class[(unsigned char)'['] = SPECIAL_CHAR;
  char_class[(unsigned char)']'] = SPECIAL_CHAR;
  char_class[(unsigned char)'{'] = SPECIAL_CHAR;
  char_class[(unsigned char)'}'] = SPECIAL_CHAR;
  char_class[(unsigned char)'*'] = SPECIAL_CHAR;
  char_class[(unsigned char)'+'] = SPECIAL_CHAR;
  char_class[(unsigned char)'?'] = SPECIAL_CHAR;
  char_class[(unsigned char)'.'] = SPECIAL_CHAR;
  char_class[(unsigned char)'^'] = SPECIAL_CHAR;
  char_class[(unsigned char)'$'] = SPECIAL_CHAR;
  char_class[(unsigned char)'|'] = SPECIAL_CHAR;
  //char_class[(unsigned char)'\\'] = SPECIAL_CHAR;
}


