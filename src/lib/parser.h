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

#ifndef _rex_parser_h_
#define _rex_parser_h_

#include "charclass.h"
#include "opcodes.h"
#include <rex/rex.h>
#include <vector>
#include <string>
#include <iostream>

typedef unsigned long ExecOpts;

#define CAPTURE_GROUP(_eo) (((_eo) & 0xFFFF0000) >> 16)
#define EXEC_FLAGS(_eo)    (ExecFlags)((_eo) & 0x0000FFFF)
#define EXEC_OPTS(_ef,_cg) (((_ef) & 0x0000FFFF) | (((_cg) & 0x0000FFFF) << 16))

typedef std::vector<unsigned long> CodeSegment;

struct _Regexp
{
  std::string  exp;
  CodeSegment  cs;
};

struct _Match
{
  std::string str;
  const char* beg;   // first char
  const char* last;  // last char
  const char* end;   // last + 1
  const char* mbeg[10]; // 0-9
  const char* mend[10]; // 0-9
};

struct ExecState
{
  const char* cur;     // current character in input
  ExecOpts    opts;    // exec flag and current capture group
  size_t      ci;      // current instruction
  OpCode      until;   // instruction to execute until
  size_t      indent;  // pretty print info
};

extern bool read_decimal(const char **ppc, unsigned long &v);
extern bool read_char_hexa(const char **ppc, unsigned char &v);
extern bool read_char_octal(const char **ppc, unsigned char &v);

extern bool parse_expression(const char **ppc, _Regexp &re);
extern bool parse_branch(const char **ppc, _Regexp &re);
extern bool parse_piece(const char **ppc, _Regexp &re);
extern bool parse_atom(const char **ppc, _Regexp &re);
extern bool parse_characters(const char **ppc, _Regexp &re);
extern bool parse_range(const char **ppc, _Regexp &re, bool inv);
extern bool parse_repeat(const char **ppc, _Regexp &re);
extern bool parse_zerowidth(const char **ppc, CodeSegment &cs);

extern void print_expcode(const CodeSegment &cs);
extern void print_partial_expcode(const CodeSegment &cs, size_t from, size_t to, std::ostream *os=0);

extern bool rex_search(const std::string &str, _Regexp &re, _Match &m, unsigned short flags=EXEC_CAPTURE, size_t offset=0, size_t len=size_t(-1));
extern bool rex_match(const std::string &str, _Regexp &re, _Match &m, unsigned short flags=EXEC_CAPTURE, size_t offset=0, size_t len=size_t(-1));

#endif 

