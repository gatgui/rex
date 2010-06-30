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

// Charactes ::= { character }*
bool parse_characters(const char **ppc, _Regexp &re) {
  
#ifdef _DEBUG
  std::cerr << std::endl << "Regexp::parse_characters...";
#endif
  const char *pc = *ppc;
  //bool done = false;
  //CodeSegment cs;
  unsigned char v;
  // SHOULD TAKE ONLY ONE !
  //while (!done)
  //{
    //while (*pc!='\0' && !CHAR_IS(*pc, SPECIAL_CHAR))
    if (*pc!='\0' && !CHAR_IS(*pc, SPECIAL_CHAR)) {
      if (*pc == '\\') {
        ++pc;
        switch(*pc) {
          // Predefined character class
          case 's': re.cs.push_back(OP_CHAR_SPACE); break;
          case 'S': re.cs.push_back(OP_CHAR_NOTSPACE); break;
          case 'w': re.cs.push_back(OP_CHAR_WORD); break;
          case 'W': re.cs.push_back(OP_CHAR_NOTWORD); break;
          case 'l': re.cs.push_back(OP_CHAR_LETTER); break;
          case 'L': re.cs.push_back(OP_CHAR_NOTLETTER); break;
          case 'u': re.cs.push_back(OP_CHAR_UPPER); break;
          case 'U': re.cs.push_back(OP_CHAR_LOWER); break;
          case 'd': re.cs.push_back(OP_CHAR_DIGIT); break;
          case 'D': re.cs.push_back(OP_CHAR_NOTDIGIT); break;
          case 'h': re.cs.push_back(OP_CHAR_HEXA); break;
          case 'H': re.cs.push_back(OP_CHAR_NOTHEXA); break;
          // Escape sequence
          // i have a doubt on those ones in fact
          case 't': re.cs.push_back(OPARG(OP_CHAR_SINGLE,'\t')); break;
          case 'v': re.cs.push_back(OPARG(OP_CHAR_SINGLE,'\v')); break;
          case 'n': re.cs.push_back(OPARG(OP_CHAR_SINGLE,'\n')); break;
          case 'r': re.cs.push_back(OPARG(OP_CHAR_SINGLE,'\r')); break;
          case 'f': re.cs.push_back(OPARG(OP_CHAR_SINGLE,'\f')); break;
          // THIS ONE IS ANNOYING, GET CAUGHT BEFORE CHECKING ZEROWIDTH ASSERTION
          //case 'b': re.cs.push_back(OPARG(OP_CHAR_SINGLE,'\b')); break;
          case 'a': re.cs.push_back(OPARG(OP_CHAR_SINGLE,'\a')); break;
          //case 'e': cs.push_back(OPARG(OP_CHAR_SINGLE,'\e'); break;
          // Control char
          case 'c': //control char: \cD
            ++pc;
            switch (*pc) {
              case '@': re.cs.push_back(OPARG(OP_CHAR_SINGLE,'\x00')); break; // '\0'
              case 'A': re.cs.push_back(OPARG(OP_CHAR_SINGLE,'\x01')); break;
              case 'B': re.cs.push_back(OPARG(OP_CHAR_SINGLE,'\x02')); break;
              case 'C': re.cs.push_back(OPARG(OP_CHAR_SINGLE,'\x03')); break;
              case 'D': re.cs.push_back(OPARG(OP_CHAR_SINGLE,'\x04')); break;
              case 'E': re.cs.push_back(OPARG(OP_CHAR_SINGLE,'\x05')); break;
              case 'F': re.cs.push_back(OPARG(OP_CHAR_SINGLE,'\x06')); break;
              case 'G': re.cs.push_back(OPARG(OP_CHAR_SINGLE,'\x07')); break; // '\a'
              case 'H': re.cs.push_back(OPARG(OP_CHAR_SINGLE,'\x08')); break; // '\b'
              case 'I': re.cs.push_back(OPARG(OP_CHAR_SINGLE,'\x09')); break; // '\t'
              case 'J': re.cs.push_back(OPARG(OP_CHAR_SINGLE,'\x0A')); break; // '\n'
              case 'K': re.cs.push_back(OPARG(OP_CHAR_SINGLE,'\x0B')); break; // '\v'
              case 'L': re.cs.push_back(OPARG(OP_CHAR_SINGLE,'\x0C')); break; // '\f'
              case 'M': re.cs.push_back(OPARG(OP_CHAR_SINGLE,'\x0D')); break; // '\r'
              case 'N': re.cs.push_back(OPARG(OP_CHAR_SINGLE,'\x0E')); break;
              case 'O': re.cs.push_back(OPARG(OP_CHAR_SINGLE,'\x0F')); break;
              case 'P': re.cs.push_back(OPARG(OP_CHAR_SINGLE,'\x10')); break;
              case 'Q': re.cs.push_back(OPARG(OP_CHAR_SINGLE,'\x11')); break;
              case 'R': re.cs.push_back(OPARG(OP_CHAR_SINGLE,'\x12')); break;
              case 'S': re.cs.push_back(OPARG(OP_CHAR_SINGLE,'\x13')); break;
              case 'T': re.cs.push_back(OPARG(OP_CHAR_SINGLE,'\x14')); break;
              case 'U': re.cs.push_back(OPARG(OP_CHAR_SINGLE,'\x15')); break;
              case 'V': re.cs.push_back(OPARG(OP_CHAR_SINGLE,'\x16')); break;
              case 'W': re.cs.push_back(OPARG(OP_CHAR_SINGLE,'\x17')); break;
              case 'X': re.cs.push_back(OPARG(OP_CHAR_SINGLE,'\x18')); break;
              case 'Y': re.cs.push_back(OPARG(OP_CHAR_SINGLE,'\x19')); break;
              case 'Z': re.cs.push_back(OPARG(OP_CHAR_SINGLE,'\x1A')); break;
              case '[': re.cs.push_back(OPARG(OP_CHAR_SINGLE,'\x1B')); break;
              case '\\': re.cs.push_back(OPARG(OP_CHAR_SINGLE,'\x1C')); break;
              case ']': re.cs.push_back(OPARG(OP_CHAR_SINGLE,'\x1D')); break;
              case '^': re.cs.push_back(OPARG(OP_CHAR_SINGLE,'\x1E')); break;
              case '_': re.cs.push_back(OPARG(OP_CHAR_SINGLE,'\x1F')); break;
              default: return false;
            }
            break;
          // Hexa character
          case 'x':
            ++pc;
            if (!read_char_hexa(&pc,v)) {
              return false;
            }
            re.cs.push_back(OPARG(OP_CHAR_SINGLE,v));
            break;
          // this will be an octal
          case '0':
            ++pc;
            if (!read_char_octal(&pc,v)) {
              return false;
            }
            re.cs.push_back(OPARG(OP_CHAR_SINGLE,v));
            break;
          // Back substitution, or octal if not possible
          case '1':
          case '2':
          case '3':
          case '4':
          case '5':
          case '6':
          case '7':
          case '8':
          case '9':
            v = *pc - '0';
            re.cs.push_back(OPARG(OP_BACKSUBST,v));
            break;
          // special char
          case '\\':
          case '(':
          case ')':
          case '{':
          case '}':
          case '[':
          case ']':
          case '*':
          case '+':
          case '?':
          case '.':
          case '^':
          case '$':
          case '|':
            re.cs.push_back(OPARG(OP_CHAR_SINGLE,*pc));
            break;
          default:
            // Not recognized !! [no zero width assertion for example]
            return false;
        }
        //break;
      } else {
        re.cs.push_back(OPARG(OP_CHAR_SINGLE,*pc));
      }
      ++pc;
    }
    if (*pc == '.') {
      re.cs.push_back(OP_CHAR_ANY);
      ++pc;
      
    } else if (pc == *ppc) {
       return false;
    }
    //else {
    //  done = true;
    //}
    /*
    else if (!parse_zerowidth(&pc, cs)) {
      // other special chars "(){}[]*+?^$|"
      done = true;
    }
    */
  //}
  
  //if (cs.size() == 0) {
  //  return false;
  //}
  
  // no special instruction to add like in range
  //re.cs.insert(re.cs.end(), cs.begin(), cs.end());
  *ppc = pc;
  return true;
}

