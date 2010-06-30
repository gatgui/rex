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

#include "parser.h"

bool parse_zerowidth(const char **ppc, CodeSegment &cs) {
#ifdef _DEBUG
  std::cerr << std::endl << "Regexp::parse_zerowidth...";
#endif
  const char *pc = *ppc;
  
  switch(*pc) {
    case '^':
      cs.push_back(OP_LINE_START);
      break;
    case '$':
      cs.push_back(OP_LINE_END);
      break;
    case '\\':
      ++pc;
      switch(*pc) {
        case 'b': cs.push_back(OP_WORD_BOUND); break;
        case 'B': cs.push_back(OP_WORD_IN); break;
        case '<': cs.push_back(OP_WORD_START); break;
        case '>': cs.push_back(OP_WORD_END); break;
        case 'A': cs.push_back(OP_STR_START); break;
        case 'Z': cs.push_back(OP_STR_END); break;
        case 'z': cs.push_back(OP_BUF_END); break;
        //case 'z':
        default:
          return false;
      }
      break;
    default:
      return false;
  }
  ++pc;
  *ppc = pc;
  return true;
}

