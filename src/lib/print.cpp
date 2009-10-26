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
#include <iostream>
using namespace std;

static void indent(int level, std::ostream *os=0) {
  std::ostream &sout = (os == 0 ? cout : *os);
  for (int i=0; i<level; ++i) {
    sout << " ";
  }
}

static void line_header(size_t ndigits, size_t ino, std::ostream *os=0) {
  std::ostream &sout = (os == 0 ? cout : *os);
  size_t cdigits=1;
  size_t divider=10;
  while ( (ino % divider) != ino ){
    divider *= 10;
    cdigits += 1;
  }
  for (size_t i=ndigits; i>cdigits; --i) {
    sout << "0";
  }
  sout << ino << ": ";
}

static void print_hexa(unsigned char c, std::ostream *os=0) {
  std::ostream &sout = (os == 0 ? cout : *os);
  unsigned char hd[2] = {c/16, c%16};
  sout << "0x";
  for (int i=0; i<2; ++i) {
    if (hd[i]>=10) {
      sout << (char)((unsigned char)'A' + (hd[i] - 10));
    } else {
      sout << (char)((unsigned char)'0' + hd[i]);
    }
  }
}

void print_partial_expcode(CodeSegment &cs, size_t from, size_t to, std::ostream *os) {
  int il = 0;
  
  size_t ci = from;
  size_t cssize = (to - from + 1);
  
  size_t ndigits=1;
  size_t divider=10;
  
  std::ostream &sout = (os == 0 ? cout : *os);
  
  while ( (cssize % divider) != cssize ) {
    divider *= 10;
    ndigits += 1;
  }

  while (ci <= to) {
    OpCode code = (OpCode)OP(cs[ci]);
    unsigned long arg = ARG(cs[ci]);
    
    line_header(ndigits, ci, os);
    
    switch(code) {
      case OP_CHAR_ANY: {
        indent(il, os);
        sout << "AnyChar" << endl;
        ++ci;
        break;
      }
      case OP_CHAR_SINGLE: {
        indent(il, os);
        sout << "SingleChar(";
        print_hexa(arg, os);
        sout << ":" << char(arg&0x00FF);
        sout << ")" << endl;
        ++ci;
        break;
      }
      case OP_CHAR_SPACE: {
        indent(il, os);
        sout << "SpaceChar" << endl;
        ++ci;
        break;
      }
      case OP_CHAR_NOTSPACE: {
        indent(il, os);
        sout << "!SpaceChar" << endl;
        ++ci;
        break;
      }
      case OP_CHAR_WORD: {
        indent(il, os);
        sout << "WordChar" << endl;
        ++ci;
        break;
      }
      case OP_CHAR_NOTWORD: {
        indent(il, os);
        sout << "!WordChar" << endl;
        ++ci;
        break;
      }
      case OP_CHAR_DIGIT: {
        indent(il, os);
        sout << "DigitChar" << endl;
        ++ci;
        break;
      }
      case OP_CHAR_NOTDIGIT: {
        indent(il, os);
        sout << "!DigitChar" << endl;
        ++ci;
        break;
      }
      case OP_CHAR_LOWER: {
        indent(il, os);
        sout << "LowerCaseChar" << endl;
        ++ci;
        break;
      }
      case OP_CHAR_UPPER: {
        indent(il, os);
        sout << "UpperCaseChar" << endl;
        ++ci;
        break;
      }
      case OP_CHAR_LETTER: {
        indent(il, os);
        sout << "LetterChar" << endl;
        ++ci;
        break;
      }
      case OP_CHAR_NOTLETTER: {
        indent(il, os);
        sout << "!LetterChar" << endl;
        ++ci;
        break;
      }
      case OP_CHAR_HEXA: {
        indent(il, os);
        sout << "HexaChar" << endl;
        ++ci;
        break;
      }
      case OP_CHAR_NOTHEXA: {
        indent(il, os);
        sout << "!HexaChar" << endl;
        ++ci;
        break;
      }
      case OP_CHAR_CLASS: {
        indent(il, os);
        sout << "CharClass[:next+=" << arg << endl;
        ++il;
        ++ci;
        break;
      }
      case OP_CHAR_NCLASS: {
        indent(il, os);
        sout << "!CharClass[:next+=" << arg << endl;
        ++il;
        ++ci;
        break;
      }
      case OP_CHAR_ENDCLASS: {
        --il;
        indent(il, os);
        sout << "]" << endl;
        ++ci;
        break;
      }
      case OP_CHAR_RANGE: {
        unsigned long c0 = (arg & 0x00FF);
        unsigned long c1 = ((arg & 0xFF00)>>8);
        indent(il, os);
        sout << "CharRange(";
        print_hexa(c0, os);
        sout << "-";
        print_hexa(c1, os);
        sout << ":";
        sout << char(c0 & 0x00FF);
        sout << "-";
        sout << char(c1 & 0x00FF);
        sout << ")" << endl;
        ++ci;
        break;
      }
      case OP_REPEAT: {
        unsigned long rmin = ARG(cs[ci+1]);
        unsigned long rmax = ARG(cs[ci+2]);
        indent(il, os);
        if (rmax==ARGMAX) {
          sout << "GreedyRep{[" << rmin << ", INF]:next=+" << arg << endl;
        } else {
          sout << "GreedyRep{[" << rmin << ", " << rmax << "]:next=+" << arg << endl;
        }
        il++;
        ci+=3;
        break;
      }
      case OP_REPEAT_LAZY: {
        unsigned long rmin = ARG(cs[ci+1]);
        unsigned long rmax = ARG(cs[ci+2]);
        indent(il, os);
        if (rmax==ARGMAX) {
          sout << "LazyRep{[" << rmin << ", INF]:next=+" << arg << endl;
        } else {
          sout << "LazyRep{[" << rmin << ", " << rmax << "]:next=+" << arg << endl;
        }
        il++;
        ci+=3;
        break;
      }
      case OP_REPEAT_END: {
        --il;
        indent(il, os);
        sout << "}" << endl;
        ++ci;
        break;
      }
      case OP_BRANCH: {
        if (ARG(cs[ci]) == 0) {
          --il;
          indent(il, os);
          sout << "|:empty" << endl;
        } else {
          indent(il, os);
          if (ci+arg > to) {
            sout << "Branch|:next=END" << endl;
          } else {
            sout << "Branch|:next=+" << arg << endl;
          }
          ++il;
        }
        ++ci;
        break;
      }
      case OP_SUB_BEGIN: {
        indent(il, os);
        sout << "Sub(:next=+" << arg << endl;
        ++il;
#ifdef SUBEXPOPTS
        ci += 2;
#else
        ++ci;
#endif
        break;
      }
      case OP_SUB_END: {
        --il;
        indent(il, os);
        sout << "):prev=-" << arg << endl;
        ++ci;
        break;
      }
      case OP_WORD_START: {
        indent(il, os);
        sout << "WordBeg" << endl;
        ++ci;
        break;
      }
      case OP_WORD_END: {
        indent(il, os);
        sout << "WordEnd" << endl;
        ++ci;
        break;
      }
      case OP_WORD_BOUND: {
        indent(il, os);
        sout << "WordBnd" << endl;
        ++ci;
        break;
      }
      case OP_WORD_IN: {
        indent(il, os);
        sout << "InWord" << endl;
        ++ci;
        break;
      }
      case OP_LINE_START: {
        indent(il, os);
        sout << "LineBeg" << endl;
        ++ci;
        break;
      }
      case OP_LINE_END: {
        indent(il, os);
        sout << "LineEnd" << endl;
        ++ci;
        break;
      }
      case OP_STR_START: {
        indent(il, os);
        sout << "StrBeg" << endl;
        ++ci;
        break;
      }
      case OP_STR_END: {
        indent(il, os);
        sout << "StrEnd" << endl;
        ++ci;
        break;
      }
      case OP_BUF_END: {
        indent(il, os);
        sout << "BufEnd" << endl;
        ++ci;
        break;
      }
      case OP_BACKSUBST: {
        indent(il, os);
        sout << "Backsubst(" << arg << ")" << endl;
        ++ci;
        break;
      }
      default: {
        indent(il, os);
        sout << "UNKNOWN INSTRUCTION !!!" << endl;
        ++ci;
      }
    }
  }
}

void print_expcode(CodeSegment &cs) {
  if (cs.size() == 0) {
    return;
  }
  print_partial_expcode(cs, 0, cs.size()-1, &cout);
}
