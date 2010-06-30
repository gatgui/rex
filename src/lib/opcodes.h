/*

Copyright (C) 2009, 2010  Gaetan Guidet

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

#ifndef __rex_opcodes_h__
#define __rex_opcodes_h__

enum OpCode
{
  OP_CHAR_SINGLE = 0,
  OP_CHAR_ANY, //1
  OP_CHAR_WORD, //2
  OP_CHAR_NOTWORD, //3
  OP_CHAR_DIGIT, //4
  OP_CHAR_NOTDIGIT, //5
  OP_CHAR_LOWER, //6
  OP_CHAR_UPPER, //7
  OP_CHAR_LETTER, //8
  OP_CHAR_NOTLETTER, //9
  OP_CHAR_HEXA, //10
  OP_CHAR_NOTHEXA, //11
  OP_CHAR_SPACE, //12
  OP_CHAR_NOTSPACE, //13
  OP_CHAR_CLASS, // [...] //14
  OP_CHAR_NCLASS, //[^...] //15
  OP_CHAR_ENDCLASS, //16
  OP_CHAR_RANGE, // a-z //17
  OP_REPEAT, //3 args: min, max //18
  OP_REPEAT_LAZY, //19
  OP_REPEAT_END, //20
  OP_BRANCH, //21
  OP_SUB_BEGIN, //22
  OP_SUB_END, //23
  OP_WORD_START, //24
  OP_WORD_END, //25
  OP_WORD_BOUND, //26
  OP_WORD_IN, //27
  OP_LINE_START, //28
  OP_LINE_END, //29
  OP_STR_START, //30
  OP_STR_END, //31
  OP_PARAM, //32
  OP_PARAM_BEGIN, //33
  OP_PARAM_END, //34
  OP_BACKSUBST, // +1 ... +9 //35
  OP_BUF_END,
  OP_MAX //36
};
//~30 opcodes... lets say 256 this leaves us with 0x00FFFFFF for arg indices

// up to 3 bytes argument can be packed in opcode
#define OPARG(_o,_a) (((_o) & 0x000000FF) | (((_a) & 0x00FFFFFF) << 8))
#define OP(_c)       ((OpCode)((_c) & 0x000000FF))
#define ARG(_c)      (((_c) & 0xFFFFFF00) >> 8)
#define ARGMAX       0x00FFFFFF

#endif
