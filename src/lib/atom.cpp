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

// sub begin/ sub end opcodes !

#ifdef SUBEXPOPTS
static bool reverse_code(const CodeSegment &cs, size_t firstOp, size_t lastOp, CodeSegment &rcs) {
  // care of repeat, sub, branch => recursive call
  size_t curOp = firstOp;
  
  while (curOp <= lastOp) {
    
    size_t code = OP(cs[curOp]);
    
    if (code == OP_REPEAT || code == OP_REPEAT_LAZY) {
      // arg is number of instructions to start of repeat?
      size_t arg  = ARG(cs[curOp]);
      size_t rmin = ARG(cs[curOp+1]);
      size_t rmax = ARG(cs[curOp+2]);
      
      CodeSegment rrcs;
      // curOp+arg   = inst after OP_REPEAT_END
      // curOp+arg-1 = OP_REPEAT_END
      // curOp+arg-2 = last inst of repeat
      if (!reverse_code(cs, curOp+3, curOp+arg-2, rrcs)) {
        return false;
      }
      // re-insert in same order
      for (size_t i=0; i<rrcs.size(); ++i) {
        rcs.insert(rcs.begin()+i, rrcs[i]);
      }
      rcs.insert(rcs.begin()+rrcs.size(), cs[curOp+arg-1]);
      
      rcs.insert(rcs.begin(), OPARG(OP_PARAM, rmax));
      rcs.insert(rcs.begin(), OPARG(OP_PARAM, rmin));
      rcs.insert(rcs.begin(), OPARG(code, arg));
      curOp += arg;
      
    } else if (code == OP_SUB_BEGIN) {
      // arg is number of instructions to start of sub?
      size_t arg  = ARG(cs[curOp]);
      size_t opts = ARG(cs[curOp+1]);
      
      // modify opts? -> no capture
      CodeSegment srcs;
      // curOp+arg   = inst after OP_SUB_END
      // curOp+arg-1 = OP_SUB_END
      // curOp+arg-2 = last inst of sub
      if (!reverse_code(cs, curOp+2, curOp+arg-2, srcs)) {
        return false;
      }
      // re-insert in same order
      for (size_t i=0; i<srcs.size(); ++i) {
        rcs.insert(rcs.begin()+i, srcs[i]);
      }
      rcs.insert(rcs.begin()+srcs.size(), cs[curOp+arg-1]);
      
      rcs.insert(rcs.begin(), OPARG(OP_PARAM, opts));
      rcs.insert(rcs.begin(), OPARG(code, arg));
      curOp += arg;
      
    } else if (code == OP_BRANCH) {
      // arg is number of instructions in branch?
      size_t arg = ARG(cs[curOp]);
      if (arg == 0) {
        std::cerr << "*** Found empty branch before open branch, THIS SHOULD NEVER HAPPEN!" << std::endl;
        rcs.insert(rcs.begin(), OPARG(code, 0));
        ++curOp;
        
      } else {
        CodeSegment brcs;
        // curOp+arg = OP_BRANCH (next branch)
        if (!reverse_code(cs, curOp+1, curOp+arg-1, brcs)) {
          return false;
        }
        // re-insert in same order
        for (size_t i=0; i<brcs.size(); ++i) {
          rcs.insert(rcs.begin()+i, brcs[i]);
        }
        
        size_t nbl = ARG(cs[curOp+arg]);
        
        if (nbl == 0) {
          rcs.insert(rcs.begin()+brcs.size(), cs[curOp+arg]);
          curOp += arg + 1;
        } else {
          curOp += arg;
        }
        
        rcs.insert(rcs.begin(), OPARG(code, arg));
      }
      
    } else if (code == OP_CHAR_CLASS || code == OP_CHAR_NCLASS) {
      size_t arg = ARG(cs[curOp]);
      size_t n = 0;
      ++curOp;
      while (OP(cs[curOp]) != OP_CHAR_ENDCLASS) {
        rcs.insert(rcs.begin()+n, cs[curOp]);
        ++curOp;
        ++n;
      }
      rcs.insert(rcs.begin()+n, cs[curOp]);
      rcs.insert(rcs.begin(), OPARG(code, arg));
      ++curOp;
      
    } else {
      rcs.insert(rcs.begin(), cs[curOp]);
      ++curOp;
    }
  }
  return true;
  
}
#endif

// Atom ::= "(" Expression ")" | "[" [^] Range "]" | Characters
bool parse_atom(const char **ppc, _Regexp &re) {
#ifdef _DEBUG
  std::cerr << std::endl << "Regexp::parse_atom...";
#endif
  const char *pc = *ppc;
  switch(*pc) {
    case '(': {
#ifdef _DEBUG
      std::cerr << "Subexp";
#endif
      size_t beg, end, n;
      ++pc;
      beg = re.cs.size();
      re.cs.push_back( OP_SUB_BEGIN );
      
#ifdef SUBEXPOPTS
      size_t opts = EXEC_CONSUME|EXEC_CAPTURE|EXEC_INHERIT_ML|EXEC_INHERIT_NC|EXEC_INHERIT_DNL;
      re.cs.push_back( 0 );
      
      if (*pc == '?') {
        ++pc;
        if (*pc == ':') {
          // non-capturing
          opts = opts & ~EXEC_CAPTURE;
          ++pc;
        } else if (*pc == '=') {
          // positive lookahead
          opts = opts & ~EXEC_CONSUME & ~EXEC_CAPTURE;
          ++pc;
        } else if (*pc == '!') {
          // negative lookahead
          opts = ((opts & ~EXEC_CONSUME) & ~EXEC_CAPTURE) | EXEC_NOT;
          ++pc;
        } else if (*pc == '<') {
          ++pc;
          if (*pc == '=') {
            // positive lookbehind
            opts = ((opts & ~EXEC_CONSUME) & ~EXEC_CAPTURE) | EXEC_REVERSE;
            ++pc;
          } else if (*pc == '!') {
            // negative lookbehind
            opts = ((opts & ~EXEC_CONSUME) & ~EXEC_CAPTURE) | EXEC_REVERSE | EXEC_NOT;
            ++pc;
          } else {
            std::cerr << "*** Invalid group format" << std::endl;
            return false;
          }
        } else if (*pc == 'i' || *pc == 'm' || *pc == 's' || *pc == '-') {
          if (*pc == 'i') {
            // case sensitive off
            opts = (opts & ~EXEC_INHERIT_NC) | EXEC_NOCASE;
            ++pc;
          }
          if (*pc == 'm') {
            // multiline on (. matches \r\n)
            opts = (opts & ~EXEC_INHERIT_ML) | EXEC_MULTILINE;
            ++pc;
          }
          if (*pc == 's') {
            // dot matches new line on
            opts = (opts & ~EXEC_INHERIT_DNL) | EXEC_DOT_NEWLINE;
            ++pc;
          }
          if (*pc == '-') {
            ++pc;
            if (*pc == 'i') {
              // case sensitive on
              opts = opts & ~EXEC_INHERIT_NC & ~EXEC_NOCASE;
              ++pc;
            }
            if (*pc == 'm') {
              // multiline off
              opts = opts & ~EXEC_INHERIT_ML & ~EXEC_MULTILINE;
              ++pc;
            }
            if (*pc == 's') {
              // dot matches newline off
              opts = opts & ~EXEC_INHERIT_DNL & ~EXEC_DOT_NEWLINE;
              ++pc;
            }
          }
          if (*pc != ':' && *pc != ')') {
            // either followed by : or group end (meaning we just want to change 
            //                                    exp exec flags)
            std::cerr << "*** Invalid group format" << std::endl;
            return false;
          }
        }
      }
#endif
      
      if (!parse_expression(&pc, re)) {
        return false;
      }
      if (*pc != ')') {
        return false;
      }
      end = re.cs.size();
      n = end - beg;
      // in fact this could be empty, just for changing exp flags
      if (n > 1) {
        re.cs[beg] = OPARG(OP_SUB_BEGIN,n+1);
#ifdef SUBEXPOPTS
        re.cs[beg+1] = OPARG(OP_PARAM, opts);
#endif
        re.cs.push_back(OPARG(OP_SUB_END,n+1));
        
#ifdef SUBEXPOPTS
        if ((opts & EXEC_REVERSE) != 0) {
          // accepts only fixed width opcodes
          size_t firstOp = beg + 2;
          size_t lastOp = re.cs.size() - 2;
          size_t curOp;
          for (curOp=firstOp; curOp<=lastOp; ++curOp) {
            size_t opCode = OP(re.cs[curOp]);
            if (opCode == OP_BACKSUBST) {
              std::cout << "*** No backsubstitution in lookbehind" << std::endl;
              return false;
            }
            /* Un-necessary limitation of lookbehind !
            else if (opCode == OP_REPEAT ||
                     opCode == OP_REPEAT_LAZY) {
              size_t rmin = ARG(re.cs[curOp+1]);
              size_t rmax = ARG(re.cs[curOp+2]);
              if (rmin != rmax) {
                cout << "*** Variable length match in lookbehind" << std::endl;
                return false;
              }
            }
            */
          }
          // need to reverse sequence
          CodeSegment rcs;
          reverse_code(re.cs, firstOp, lastOp, rcs);
          if (rcs.size() != (lastOp - firstOp + 1)) {
            std::cout << "*** Reversed code has not the same size" << std::endl;
            return false;
          }
          
#ifdef _DEBUG
          std::cerr << std::endl << "=== Before reversing:";
          print_partial_expcode(re.cs, firstOp, lastOp, &std::cerr);
#endif
          
          for (size_t i=0, j=firstOp; i<rcs.size(); ++i, ++j) {
            re.cs[j] = rcs[i];
          }
          
#ifdef _DEBUG
          std::cerr << "=== After reversing:";
          print_partial_expcode(re.cs, firstOp, lastOp, &std::cerr);
          std::cerr <<  "=== Done reversing" << std::endl;
#endif
        }
#endif
      } else {
        // only OP_SUB_BEG instruction pushed --> empty group. remove it
        re.cs.pop_back();
#ifdef SUBEXPOPTS
        re.cs.pop_back();
#endif
      }
      ++pc;
      break;
    }
    case '[': {
#ifdef _DEBUG
      std::cerr << "Range";
#endif
      bool inv = false;
      ++pc;
      if (*pc == '^') {
        inv = true;
        ++pc;
      }
      if (!parse_range(&pc,re,inv)) {
        return false;
      }
      if (*pc != ']') {
        return false;
      }
      ++pc;
      break;
    }
    default:
#ifdef _DEBUG
      std::cerr << "Characters...";
#endif
      if (!parse_characters(&pc,re)) {
        if (!parse_zerowidth(&pc, re.cs)) {
          return false;
        }
      }
      // Catch special chars here
      // Zero width and repeat have been tested
      // need to check against ) ?
  }
  *ppc = pc;
  return true;
}
