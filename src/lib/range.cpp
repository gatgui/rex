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
using namespace std;

// Range ::= { character | character "-" character }+
bool parse_range(const char **ppc, Regexp &re, bool inv) {
#ifdef _DEBUG
  cerr << endl << "Regexp::parse_range... ";
#endif
  // push the class on the stack ???
  CodeSegment cs;
  const char *pc = *ppc;
  unsigned char v;
  
  if (*pc == ']') {
    cs.push_back(OPARG(OP_CHAR_SINGLE,']'));
    ++pc;
  }
  if (*pc == '-') {
    cs.push_back(OPARG(OP_CHAR_SINGLE,'-'));
    ++pc;
  }
  // - can also be juste before the ] char
  
  bool hasPrev = false; // prev character was an escape sequence
  
  while (*pc!=']' && *pc!='\0') {
    switch(*pc) {
      case '-': {
        unsigned char fc, lc;
        unsigned long arg = 0;
        
        if (*(pc+1) == ']') {
          cs.push_back(OPARG(OP_CHAR_SINGLE,'-'));
          break;
        }
        
        if (!hasPrev) {
          return false;
        }
        fc = *(pc-1);
        ++pc;
        switch(*pc) {
          case ']':
          case '\0':
          case '\\': return false;
          default: {
            lc = *pc;
            // 2 bytes arg
            arg = (unsigned long)fc | ((unsigned long)lc << 8);
            cs.pop_back(); // remove last instruction [reconize fc only]
            cs.push_back(OPARG(OP_CHAR_RANGE,arg));
          }
        }
        hasPrev = false;
        break;
      }
      case '\\': {
        ++pc;
        switch(*pc) {
          // Predefined character class
          case 's': cs.push_back(OP_CHAR_SPACE); break;
          case 'S': cs.push_back(OP_CHAR_NOTSPACE); break;
          case 'w': cs.push_back(OP_CHAR_WORD); break;
          case 'W': cs.push_back(OP_CHAR_NOTWORD); break;
          case 'l': cs.push_back(OP_CHAR_LETTER); break;
          case 'L': cs.push_back(OP_CHAR_NOTLETTER); break;
          case 'u': cs.push_back(OP_CHAR_UPPER); break;
          case 'U': cs.push_back(OP_CHAR_LOWER); break;
          case 'd': cs.push_back(OP_CHAR_DIGIT); break;
          case 'D': cs.push_back(OP_CHAR_NOTDIGIT); break;
          case 'h': cs.push_back(OP_CHAR_HEXA); break;
          case 'H': cs.push_back(OP_CHAR_NOTHEXA); break;
          // Escape sequence
          case 't': cs.push_back(OPARG(OP_CHAR_SINGLE,'\t')); break;
          case 'v': cs.push_back(OPARG(OP_CHAR_SINGLE,'\v')); break;
          case 'n': cs.push_back(OPARG(OP_CHAR_SINGLE,'\n')); break;
          case 'r': cs.push_back(OPARG(OP_CHAR_SINGLE,'\r')); break;
          case 'f': cs.push_back(OPARG(OP_CHAR_SINGLE,'\f')); break;
          case 'b': cs.push_back(OPARG(OP_CHAR_SINGLE,'\b')); break;
          case 'a': cs.push_back(OPARG(OP_CHAR_SINGLE,'\a')); break;
          //case 'e': cs.push_back(OPARG(OP_CHAR_SINGLE,'\e'); break;
          case '\\': cs.push_back(OPARG(OP_CHAR_SINGLE,'\\')); break;
          // Control char
          case 'c': //control char: \cD
            ++pc;
            switch (*pc) {
              case '@': cs.push_back(OPARG(OP_CHAR_SINGLE,'\x00')); break; // '\0'
              case 'A': cs.push_back(OPARG(OP_CHAR_SINGLE,'\x01')); break;
              case 'B': cs.push_back(OPARG(OP_CHAR_SINGLE,'\x02')); break;
              case 'C': cs.push_back(OPARG(OP_CHAR_SINGLE,'\x03')); break;
              case 'D': cs.push_back(OPARG(OP_CHAR_SINGLE,'\x04')); break;
              case 'E': cs.push_back(OPARG(OP_CHAR_SINGLE,'\x05')); break;
              case 'F': cs.push_back(OPARG(OP_CHAR_SINGLE,'\x06')); break;
              case 'G': cs.push_back(OPARG(OP_CHAR_SINGLE,'\x07')); break; // '\a'
              case 'H': cs.push_back(OPARG(OP_CHAR_SINGLE,'\x08')); break; // '\b'
              case 'I': cs.push_back(OPARG(OP_CHAR_SINGLE,'\x09')); break; // '\t'
              case 'J': cs.push_back(OPARG(OP_CHAR_SINGLE,'\x0A')); break; // '\n'
              case 'K': cs.push_back(OPARG(OP_CHAR_SINGLE,'\x0B')); break; // '\v'
              case 'L': cs.push_back(OPARG(OP_CHAR_SINGLE,'\x0C')); break; // '\f'
              case 'M': cs.push_back(OPARG(OP_CHAR_SINGLE,'\x0D')); break; // '\r'
              case 'N': cs.push_back(OPARG(OP_CHAR_SINGLE,'\x0E')); break;
              case 'O': cs.push_back(OPARG(OP_CHAR_SINGLE,'\x0F')); break;
              case 'P': cs.push_back(OPARG(OP_CHAR_SINGLE,'\x10')); break;
              case 'Q': cs.push_back(OPARG(OP_CHAR_SINGLE,'\x11')); break;
              case 'R': cs.push_back(OPARG(OP_CHAR_SINGLE,'\x12')); break;
              case 'S': cs.push_back(OPARG(OP_CHAR_SINGLE,'\x13')); break;
              case 'T': cs.push_back(OPARG(OP_CHAR_SINGLE,'\x14')); break;
              case 'U': cs.push_back(OPARG(OP_CHAR_SINGLE,'\x15')); break;
              case 'V': cs.push_back(OPARG(OP_CHAR_SINGLE,'\x16')); break;
              case 'W': cs.push_back(OPARG(OP_CHAR_SINGLE,'\x17')); break;
              case 'X': cs.push_back(OPARG(OP_CHAR_SINGLE,'\x18')); break;
              case 'Y': cs.push_back(OPARG(OP_CHAR_SINGLE,'\x19')); break;
              case 'Z': cs.push_back(OPARG(OP_CHAR_SINGLE,'\x1A')); break;
              case '[': cs.push_back(OPARG(OP_CHAR_SINGLE,'\x1B')); break;
              case '\\': cs.push_back(OPARG(OP_CHAR_SINGLE,'\x1C')); break;
              case ']': cs.push_back(OPARG(OP_CHAR_SINGLE,'\x1D')); break;
              case '^': cs.push_back(OPARG(OP_CHAR_SINGLE,'\x1E')); break;
              case '_': cs.push_back(OPARG(OP_CHAR_SINGLE,'\x1F')); break;
              default: return false;
            }
            break;
          // Hexa character
          case 'x':
            ++pc;
            if (!read_char_hexa(&pc,v)) {
              return false;
            }
            cs.push_back(OPARG(OP_CHAR_SINGLE,v));
            break;
          // Octal character [no back-substitute in char class]
          case '0':
          case '1':
          case '2':
          case '3':
          case '4':
          case '5':
          case '6':
          case '7':
          case '8':
          case '9':
            ++pc;
            if (!read_char_octal(&pc,v)) {
              return false;
            }
            cs.push_back(OPARG(OP_CHAR_SINGLE,v));
            break;
          case ']':
          case '^':
          case '-':
          case '*':
          case '+':
          case '?':
          case '{':
          case '}':
          case '(':
          case ')':
          case '$':
          case '[':
            cs.push_back(OPARG(OP_CHAR_SINGLE,*pc));
            ++pc;
            break;
          default:
            // Not recognized !! [no zero width assertion for example]
            return false;
        }
        hasPrev = false;
        break;
      }
      default:
        cs.push_back(OPARG(OP_CHAR_SINGLE,*pc));
        hasPrev = true;
    }
    ++pc;
  }
  
  if (*pc=='\0' || cs.size()==0) {
    return false;
  }
  // add CHAR_CLASS instruction, arg = num instruction in class [for jump]
  // doesn't include the OP_CHAR_CLASS instruction
  unsigned long jmp = cs.size() + 1;
  re.cs.push_back( OPARG((inv?OP_CHAR_NCLASS:OP_CHAR_CLASS), jmp) );
  re.cs.insert(re.cs.end(), cs.begin(), cs.end());
  re.cs.push_back( OPARG(OP_CHAR_ENDCLASS, jmp) );
  *ppc = pc;
  
  return true;
}
