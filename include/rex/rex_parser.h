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

#include <vector>
#include <string>
#include <iostream>
#include "class_table.h"
#include "opcodes.h"
#include "rex_string.h"

typedef std::vector<unsigned long> CodeSegment;

struct Regexp
{
  std::string  exp;
  CodeSegment  cs;
};

enum ExecFlags
{
  EXEC_NOCASE      = 0x0001, // ignore case
  EXEC_CONSUME     = 0x0002, // increment input pointer
  EXEC_CAPTURE     = 0x0004, // group are capturing
  EXEC_REVERSE     = 0x0008, // apply backwards (no group capture)
  EXEC_MULTILINE   = 0x0010, // makes ^ and $ match at line boundaries (instead of buffer like \A \Z)
  EXEC_NOT         = 0x0020, // no match is match
  EXEC_DOT_NEWLINE = 0x0040, // dot matches new line chars \r and/or \n
  
  // special values for sub options
  EXEC_INHERIT_ML  = 0x0100, // group inherit multiline flag
  EXEC_INHERIT_NC  = 0x0200, // group inherit nocase flags
  EXEC_INHERIT_DNL = 0x0400  // group inherit dot match newline
};

typedef unsigned long ExecOpts;

#define CAPTURE_GROUP(_eo) (((_eo) & 0xFFFF0000) >> 16)
#define EXEC_FLAGS(_eo)    (ExecFlags)((_eo) & 0x0000FFFF)
// combination of flags and group
#define EXEC_OPTS(_ef,_cg) (((_ef) & 0x0000FFFF) | (((_cg) & 0x0000FFFF) << 16))

struct Match
{
  std::string str;
  //const char* first; // very first char [keep for lookbehind]
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

extern bool parse_expression(const char **ppc, Regexp &re);
extern bool parse_branch(const char **ppc, Regexp &re);
extern bool parse_piece(const char **ppc, Regexp &re);
extern bool parse_atom(const char **ppc, Regexp &re);
extern bool parse_characters(const char **ppc, Regexp &re);
extern bool parse_range(const char **ppc, Regexp &re, bool inv);
extern bool parse_repeat(const char **ppc, Regexp &re);
extern bool parse_zerowidth(const char **ppc, CodeSegment &cs);

extern void print_expcode(CodeSegment &cs);
extern void print_partial_expcode(CodeSegment &cs, size_t from, size_t to, std::ostream *os=0);

extern bool rex_search(const std::string &str, Regexp &re, Match &m, unsigned short flags=EXEC_CAPTURE);
extern bool rex_match(const std::string &str, Regexp &re, Match &m, unsigned short flags=EXEC_CAPTURE);

#endif 

