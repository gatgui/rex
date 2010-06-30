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

// Rep    ::= ( "*" | "+" | "?" | Counts ) [ "?" ]
// Counts ::= "{" digits ["," [ digits] ] "}"
bool parse_repeat(const char **ppc, _Regexp &re) {
#ifdef _DEBUG
  std::cerr << std::endl << "Regexp::parse_repeat...";
#endif
  const char *pc = *ppc;
  
  unsigned long rmin = 0;
  unsigned long rmax = ARGMAX;
  bool greedy = true;
  
  switch(*pc) {
    case '*':
      ++pc;
      if (*pc == '?') {
        ++pc;
        greedy = false;
      }
      break;
    case '+':
      ++pc;
      if (*pc == '?') {
        ++pc;
        greedy = false;
      }
      rmin = 1;
      break;
    case '?':
      ++pc;
      if (*pc == '?') {
        ++pc;
        greedy = false;
      }
      rmax = 1;
      break;
    case '{': {
      unsigned long r;
      ++pc;
      if (read_decimal(&pc,r)) {
        rmin = r;
        if (*pc == '}') {
          // matched {m}
          ++pc;
          rmax=rmin;
          break;
        }
        if (*pc != ',') {
          return false;
        }
        ++pc;
        if (*pc == '}') {
          // matched {m,}
          ++pc;
          break;
        }
        if (!read_decimal(&pc,r)) {
          return false;
        }
        if (*pc != '}') {
          return false;
        }
        ++pc;
        rmax = r; // matched {m,n}
      
      } else {
        if (*pc == '}') {
          // matched {}
          ++pc;
          break;
        }
        if (*pc != ',') {
          return false;
        }
        ++pc;
        if (!read_decimal(&pc,r)) {
          return false;
        }
        if (*pc != '}') {
          return false;
        }
        // matched {,m}
        ++pc;
        rmax = r;
      }
      if (*pc == '?') {
        ++pc;
        greedy = false;
      }
      break;
    }
    default:
      return true;
  }
  
  re.cs.push_back( greedy ? OP_REPEAT : OP_REPEAT_LAZY );
  re.cs.push_back( OPARG(OP_PARAM, rmin) );
  re.cs.push_back( OPARG(OP_PARAM, rmax) );
  re.cs.push_back( OP_REPEAT_END );
  
  *ppc = pc;
  return true;
}

