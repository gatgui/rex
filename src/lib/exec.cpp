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
#include <map>
#include <vector>

#ifdef _DEBUG
# define _DEBUG_PRINT(whatever) std::cerr << whatever
# define _DEBUG_ONLY(inst) inst
#else
# define _DEBUG_PRINT(whatever)
# define _DEBUG_ONLY(inst)
#endif

static void indent_line(size_t level) {
  for (size_t i=0; i<level; ++i) {
    std::cerr << "| ";
  }
}

static void print_substr(const char *from, const char *to) {
  const char *c = from;
  while (c <= to) {
    std::cerr << *c;
    ++c;
  }
}

bool rex_step(_Regexp&, ExecState&, _Match&);

// execuntil do no execute the "until" instruction
bool rex_execuntil(_Regexp &re, ExecState &es, _Match &m, const std::string &comment="") {
  ExecFlags ef = EXEC_FLAGS(es.opts);
  bool rev = ((ef & EXEC_REVERSE) != 0);
  
  _DEBUG_PRINT(std::endl);
  _DEBUG_ONLY(indent_line(es.indent));
  _DEBUG_PRINT("[rex_execuntil:" << (int)es.until << "] " << (comment.length() > 0 ? comment : ""));
  
  while (OP(re.cs[es.ci]) != es.until) {
    //if ((  rev && es.cur <= m.first ) ||
    if ((  rev && es.cur < m.beg ) ||
        ( !rev && es.cur > m.end ) ) {
      return false;
    }
    if (!rex_step(re,es,m)) {
      return false;
    }
    // update flags and rev state
    ef = EXEC_FLAGS(es.opts);
    rev = ((ef & EXEC_REVERSE) != 0);
  }
  return true;
}

bool rex_step(_Regexp &re, ExecState &es, _Match &m) {
  
  // reverse mode should be handled at this level (both cur char and instruction pointer)
  unsigned long op = OP(re.cs[es.ci]);
  unsigned long arg = ARG(re.cs[es.ci]);
  ExecFlags ef = EXEC_FLAGS(es.opts);
  int cg = CAPTURE_GROUP(es.opts);
  unsigned long rmin, rmax, rcount;
  char c0, c1, c = *(es.cur);
  bool rev = ((ef & EXEC_REVERSE) != 0);
  ExecState os;
  
  _DEBUG_PRINT(std::endl);
  _DEBUG_ONLY(indent_line(es.indent));
  _DEBUG_PRINT("[rex_step: @" << es.ci << " (" << int(op) << "): ");
  _DEBUG_PRINT("\"" << std::string(es.cur) << "\", all=\"" << m.beg << "\"] " << (rev ? "REV | " : "| "));
    
  //if ((  rev && es.cur <= m.first ) ||
  if ((  rev && es.cur < m.beg ) ||
      ( !rev && es.cur > m.end ) ||
      ( es.ci >= re.cs.size()     ) ) {
    return false;
  }
  
  //if (rev) {
  //  es.cur -= 1;
  //  c = *(es.cur);
  //}
  //
  //_DEBUG_PRINT("[\'" << *(es.cur) << "\'] ");
  
  switch(op) {
    //-------------------
    // Back substitution
    //-------------------
    case OP_BACKSUBST: {
      _DEBUG_PRINT("Back Substitution...");
      
      if (rev) {
        return false;
      }
      
      if (arg>=1 && m.mbeg[arg]!=0 && m.mend[arg]!=0) {
        const char *bsc = 0;
        const char *cc = es.cur;
        
        _DEBUG_ONLY(std::cerr << " (\""; print_substr(m.mbeg[arg], m.mend[arg]); std::cerr << "\") ");
        
        for (bsc=m.mbeg[arg]; bsc<=m.mend[arg] && cc != m.end; ++bsc, ++cc) {
          if (*bsc != *cc) {
            break;
          }
        }
        
        if (bsc == (m.mend[arg] + 1)) {
          es.ci++;
          if (ef & EXEC_CONSUME) {
            es.cur = cc;
          }
          _DEBUG_PRINT("Matched");
          return true;
        }
      }
      
      _DEBUG_PRINT("Failed");
      return false;
    }
    
    //--------
    // Branch
    //--------
    case OP_BRANCH: {
      _DEBUG_PRINT("Branch...(" << arg << " inst)");
      
      if (arg == 0) {
        es.ci++;
        return true; // should return false sometimes?... sucks a little
      }
      
      os = es;
      es.until = OP_BRANCH;
      es.ci++; // not if arg == 0 !!!
      es.indent++;
      
      //if (rev) {
      //  es.cur += 1;
      //  os.cur += 1;
      //}
      
      if (!rex_execuntil(re, es, m, "(Branch)")) {
        es = os;
        es.ci += arg;
        if (ARG(re.cs[es.ci]) == 0) {
          // no alternative !!
          return false;
        }
        es.indent++;
        bool r = rex_execuntil(re,es,m,"(Alternative)");
        es.indent--;
        return r;
      }
      
      es.indent--;
      es.until = os.until;
      
      // this does not necessarily means es.ci reached es.until... this should !
      if ((ef & EXEC_CONSUME) == 0) {
        es.cur = os.cur;
      }
      
      return true;
    }
    
    //---------------
    // Greedy repeat
    //---------------
    case OP_REPEAT: {
      
      std::vector<const char*> remain;
      std::vector<_Match> matches;
      
      rmin = ARG(re.cs[es.ci+1]);
      rmax = ARG(re.cs[es.ci+2]);
      rcount = 0;
      os = es;
      
      _DEBUG_PRINT("Repeat...");
      _DEBUG_PRINT("[" << rmin << ", " << rmax << "]");
      
      remain.push_back(es.cur);
      matches.push_back(m);
      
      es.opts = es.opts | EXEC_CONSUME;
      es.indent++;
      es.ci += 3; // 3 opcodes for repeat instruction
      es.until = OP_REPEAT_END;
      
      //if (rev) {
      //  es.cur += 1;
      //  os.cur += 1;
      //}
      
      while (rex_execuntil(re, es, m, "(Repeat match)")) {
        remain.push_back(es.cur);
        matches.push_back(m);
        if (++rcount == rmax) {
          break;
        }
        es.ci = os.ci + 3;
      }
      
      // at least one !!!!
      es.until = os.until;
      es.opts = os.opts;
      size_t i = remain.size() - 1;
      
      do {
        
        if (i < rmin) {
          break;
        }
        
        if (i > rmax) {
          --i;
          continue;
        }
        
        // Try matching remaining string !
        es.ci = os.ci + arg;
        es.cur = remain[i];
        
        if (rex_execuntil(re, es, matches[i], "(Match Remaining)")) {
          // Found match ! we're done !
          if ((ef & EXEC_CONSUME) == 0) {
            es.cur = os.cur;
          }
          
          _DEBUG_PRINT(std::endl);
          _DEBUG_ONLY(indent_line(es.indent));
          _DEBUG_PRINT("MATCHED REPEAT !!!");
          
          es.indent--;
          m = matches[i];
          return true;
        }
        
        if (i == 0) {
          break;
        }
        
        --i;
        
      } while (1);
      
      es = os;
      m = matches[0];
      return false;
    }
    
    //-------------
    // Lazy repeat
    //-------------
    case OP_REPEAT_LAZY: {
      
      rmin = ARG(re.cs[es.ci+1]);
      rmax = ARG(re.cs[es.ci+2]);
      rcount = 0;
      os = es;
      
      _DEBUG_PRINT("Repeat Lazy...");
      _DEBUG_PRINT("[" << rmin << ", " << rmax << "]");
      
      es.opts = es.opts | EXEC_CONSUME;
      es.indent++;
      es.ci += 3; // repeat is 3 opcodes
      es.until = OP_REPEAT_END;
      
      //if (rev) {
      //  es.cur += 1;
      //  os.cur += 1;
      //}
      
      while (rex_execuntil(re, es, m, "(Repeat match [Lazy])")) {
        ++rcount;
        
        if (rcount < rmin) {
          // not enough match, continue
          es.ci = os.ci + 3;
        }
        
        if (rcount > rmax) {
          // reach maximum number of repeats without having a match in remaining, we're done
          break;
        }
        
        es.ci = os.ci + arg;
        es.until = os.until;
        es.opts = os.opts;
        
        _DEBUG_PRINT(std::endl);
        _DEBUG_ONLY(indent_line(es.indent));
        _DEBUG_PRINT("Remains: \"" << es.cur << "\"");
        
        if (rex_execuntil(re, es, m, "(Match Remaining [Lazy])")) {
          // Found match ! we're done !
          if ((ef & EXEC_CONSUME) == 0) {
            es.cur = os.cur;
          }
          
          _DEBUG_PRINT(std::endl);
          _DEBUG_ONLY(indent_line(es.indent));
          _DEBUG_PRINT("MATCHED LAZY REPEAT !!!");
          
          es.indent--;
          return true;
        }
        
        es.ci = os.ci + 3;
        es.until = OP_REPEAT_END;
        es.opts = os.opts | EXEC_CONSUME;
      }
      
      if (rcount == 0 && rmin == 0) {
        // not a single match of repeat content by 0 is accepted
        es.ci = os.ci + arg;
        es.until = os.until;
        es.opts = os.opts;
        
        _DEBUG_PRINT(std::endl);
        _DEBUG_ONLY(indent_line(es.indent));
        _DEBUG_PRINT("Remains: \"" << es.cur << "\"");
        
        if (rex_execuntil(re, es, m, "(Match Remaining [Lazy])")) {
          // Found match ! we're done !
          if ((ef & EXEC_CONSUME) == 0) {
            es.cur = os.cur;
          }
          
          _DEBUG_PRINT(std::endl);
          _DEBUG_ONLY(indent_line(es.indent));
          _DEBUG_PRINT("MATCHED LAZY REPEAT !!!");
          
          es.indent--;
          return true;
        }
      }
      
      es = os;
      return false;
    }
    
    //---------------
    // End of repeat
    //---------------
    case OP_REPEAT_END: {
      _DEBUG_PRINT("Repeat End");
      es.ci++;
      return true;
    }
    
    //------------------------------------
    // Begining of sub-expression (group)
    //------------------------------------
    case OP_SUB_BEGIN: {
      const char *capb = 0;
      
      _DEBUG_PRINT("Sub...");
      
      os = es;
      es.until = OP_SUB_END;
      es.indent++;
      es.ci++;
      
#ifdef SUBEXPOPTS
      ExecFlags sef = (ExecFlags) ARG(re.cs[es.ci]);
      es.ci++;
      if ((sef & EXEC_INHERIT_ML) && (ef & EXEC_MULTILINE)) {
        sef = (ExecFlags) (sef | EXEC_MULTILINE);
      }
      if ((sef & EXEC_INHERIT_NC) && (ef & EXEC_NOCASE)) {
        sef = (ExecFlags) (sef | EXEC_NOCASE);
      }
      if ((sef & EXEC_INHERIT_DNL) && (ef & EXEC_DOT_NEWLINE)) {
        sef = (ExecFlags) (sef | EXEC_DOT_NEWLINE);
      }
      es.opts = EXEC_OPTS(sef, cg);
#else
      ExecFlags sef = ef;
#endif
      
      if (sef & EXEC_CAPTURE) {
        cg++;
        capb = es.cur;
        es.opts = EXEC_OPTS(sef, cg);
      }
      
      // force character consuming to advance in string while matching
      es.opts = es.opts | EXEC_CONSUME;
      
      //if (rev) {
      //  es.cur += 1;
      //  os.cur += 1;
      //}
      
      bool matched = rex_execuntil(re, es, m, "(Sub)");
      
      if (sef & EXEC_NOT) {
        matched = !matched;
      }
      
      if (!matched) {
        es = os;
        return false;
      
      } else {
        // because of the  possible EXEC_NOT, move es.ci to be sure we point to the OP_SUB_END
        if (sef & EXEC_NOT) {
          es.ci = os.ci + arg - 1;
        }
        
        if (sef & EXEC_CAPTURE) {
          m.mbeg[cg] = capb;
          m.mend[cg] = es.cur - 1;
          cg--;
          es.opts = EXEC_OPTS(sef, cg);
        }
        
        if (!(sef & EXEC_CONSUME)) {
          es.cur = os.cur;
        }
        es.opts = os.opts;
        es.indent--;
        es.until = os.until;
        
        return true;
      }
    }
    
    //-------------------------------
    // End of sub-expression (group)
    //-------------------------------
    case OP_SUB_END: {
      _DEBUG_PRINT("Sub end");
      es.ci++;
      return true;
    }
    
    //---------------
    // Start of word
    //---------------
    case OP_WORD_START: {
      _DEBUG_PRINT("Word begin...");
      if (rev) {
        if (CHAR_IS(c, WORD_CHAR) &&
            ((es.cur == m.beg) || !CHAR_IS(*(es.cur-1), WORD_CHAR))) {
          es.ci++;
          _DEBUG_PRINT("Matched");
          return true;
        }
      } else {
        if (!CHAR_IS(c, WORD_CHAR) &&
            ((es.cur == m.beg) || CHAR_IS(*(es.cur+1), WORD_CHAR))) {
          es.ci++;
          _DEBUG_PRINT("Matched");
          return true;
        }
      }
      _DEBUG_PRINT("Failed");
      return false;
    }
    
    //-------------
    // End of word
    //-------------
    case OP_WORD_END: {
      _DEBUG_PRINT("Word end...");
      if (rev) {
        if (!CHAR_IS(c, WORD_CHAR) &&
            (es.cur > m.beg) && CHAR_IS(*(es.cur-1), WORD_CHAR)) {
          es.ci++;
          _DEBUG_PRINT("Matched");
          return true;
        }
      } else {
        if (CHAR_IS(c, WORD_CHAR) &&
            ((es.cur == m.last) || !CHAR_IS(*(es.cur+1), WORD_CHAR))) {
          es.ci++;
          _DEBUG_PRINT("Matched");
          return true;
        }
      }
      _DEBUG_PRINT("Failed");
      return false;
    }
    
    //---------------
    // Word boundary
    //---------------
    case OP_WORD_BOUND: {
      _DEBUG_PRINT("Word bound...");
      if (rev) {
        if (((es.cur == m.beg) && CHAR_IS(c, WORD_CHAR)) ||
            ( CHAR_IS(c, WORD_CHAR) && !CHAR_IS(*(es.cur-1), WORD_CHAR)) ||
            (!CHAR_IS(c, WORD_CHAR) &&  CHAR_IS(*(es.cur-1), WORD_CHAR))) {
          es.ci++;
          _DEBUG_PRINT("Matched");
          return true;
        }
      } else {
        if (((es.cur == m.beg)  && CHAR_IS(c, WORD_CHAR)) ||
            ((es.cur == m.last) && CHAR_IS(c, WORD_CHAR)) ||
            ( CHAR_IS(c, WORD_CHAR) && !CHAR_IS(*(es.cur+1), WORD_CHAR)) ||
            (!CHAR_IS(c, WORD_CHAR) &&  CHAR_IS(*(es.cur+1), WORD_CHAR))) {
          es.ci++;
          _DEBUG_PRINT("Matched");
          return true;
        }
      }
      _DEBUG_PRINT("Failed");
      return false;
    }
    
    //-------------
    // Inside word (misnamed, this means !WORD_BOUNDARY not necessariy IN a word)
    //-------------
    case OP_WORD_IN: {
      _DEBUG_PRINT("Inside word...");
      if (rev) {
        if ((es.cur > m.beg) && (es.cur < m.last) &&
            (( CHAR_IS(c, WORD_CHAR) &&  CHAR_IS(*(es.cur-1), WORD_CHAR)) ||
             (!CHAR_IS(c, WORD_CHAR) && !CHAR_IS(*(es.cur-1), WORD_CHAR)))) {
          es.ci++;
          _DEBUG_PRINT("Matched");
          return true;
        }
      } else {
        if ((es.cur != m.beg) && (es.cur != m.last) &&
            (( CHAR_IS(c, WORD_CHAR) &&  CHAR_IS(*(es.cur+1), WORD_CHAR)) ||
             (!CHAR_IS(c, WORD_CHAR) && !CHAR_IS(*(es.cur+1), WORD_CHAR)))) {
          es.ci++;
          _DEBUG_PRINT("Matched");
          return true;
        }
      }
      _DEBUG_PRINT("Failed");
      return false;
    }
    
    //---------------
    // Start of line
    //---------------
    case OP_LINE_START: {
      _DEBUG_PRINT("Line start...");
      // NL: \r, \n or \r\n
      // same algorithm in reverse mode
      if (es.cur <= m.beg) {
        es.ci++;
        _DEBUG_PRINT("Matched");
        return true;
      } else if ((ef & EXEC_MULTILINE) != 0) {
        c0 = *(es.cur-1);
        if (c0 == '\n' || c0 == '\r') {
          es.ci++;
          _DEBUG_PRINT("Matched");
          return true;
        }
      }
      _DEBUG_PRINT("Failed");
      return false;
    }
    
    //-------------
    // End of line
    //-------------
    case OP_LINE_END: {
      _DEBUG_PRINT("Line end...");
      // NL: \r, \n or \r\n
      // same algorithm in reverse mode, but without EXEC_MULTILINE will never match
      if (es.cur == m.end) {
        es.ci++;
        _DEBUG_PRINT("Matched");
        return true;
      } else if ((ef & EXEC_MULTILINE) != 0) {
        if (c == '\n' || c == '\r') {
          es.ci++;
          _DEBUG_PRINT("Matched");
          return true;
        }
      }
      _DEBUG_PRINT("Failed");
      return false;
    }
    
    //-----------------
    // Start of string
    //-----------------
    case OP_STR_START: {
      _DEBUG_PRINT("String start...");
      if (es.cur == m.beg) {
        es.ci++;
        _DEBUG_PRINT("Matched");
        return true;
      }
      _DEBUG_PRINT("Failed");
      return false;
    }
    
    //---------------
    // End of string
    // \Z hum should match before \n if trailing one
    //---------------
    case OP_STR_END: {
      _DEBUG_PRINT("String end...");
      if (!rev) {
        if ((es.cur == m.end) ||
            (es.cur == m.last   && (c == '\n' || c == '\r')) ||
            (es.cur == m.last-1 && (c == '\r' && *(es.cur+1) == '\n'))) {
          es.ci++;
          _DEBUG_PRINT("Matched");
          return true;
        }
      }
      _DEBUG_PRINT("Failed");
      return false;
    }
    
    //---------------
    // End of string
    // \z
    //---------------
    case OP_BUF_END: {
      _DEBUG_PRINT("Buffer end...");
      if (!rev) {
        if (es.cur == m.end) {
          es.ci++;
          _DEBUG_PRINT("Matched");
          return true;
        }
      }
      _DEBUG_PRINT("Failed");
      return false;
    }
    
    //---------------------
    // Character range x-y
    //---------------------
    case OP_CHAR_RANGE: {
      _DEBUG_PRINT("Character range...");
      if (rev) {
        if (es.cur <= m.beg) {
          return false;
        }
        es.cur -= 1;
        c = *(es.cur);
      }
      c0 = (char)  (arg & 0x00FF);
      c1 = (char) ((arg & 0xFF00) >> 8);
      if (ef & EXEC_NOCASE) {
        int casediff = 'A' - 'a';
        // UPPER - casediff = lower
        // LOWER + casediff = upper
        char cc = c0;
        while (cc <= c1) {
          if ((c == cc) ||
              (CHAR_IS(cc, LOWER_CHAR) && (cc+casediff == c)) ||
              (CHAR_IS(cc, UPPER_CHAR) && (cc-casediff == c))) {
            es.ci++;
            if (((ef & EXEC_CONSUME) == 0) == rev) {
              es.cur += 1;
            }
            _DEBUG_PRINT("Matched");
            return true;         
          }
          ++cc;
        }
      } else {
        if (c >= c0 && c <= c1) {
          es.ci++;
          if (((ef & EXEC_CONSUME) == 0) == rev) {
            es.cur += 1;
          }
          _DEBUG_PRINT("Matched");
          return true;
        }
      }
      if (rev) {
        es.cur += 1;
      }
      _DEBUG_PRINT("Failed");
      return false;
    }
    
    //-----------------------
    // Character class [...]
    //-----------------------
    case OP_CHAR_CLASS: {
      _DEBUG_PRINT("Character class...");
      os = es;
      es.ci++;
      es.indent++;
      // do not consume characters
      es.opts = es.opts & ~EXEC_CONSUME;
      
      bool found = false;
      
      while (OP(re.cs[es.ci]) != OP_CHAR_ENDCLASS) {
        // if successful, will increment instruction counter
        // beware then of next test !!
        if (rex_step(re, es, m)) {
          found = true;
          break;
        } else {
          es.ci++;
        }
      }
      
      if (found == true) {
        if (ef & EXEC_CONSUME) {
          es.opts = es.opts | EXEC_CONSUME;
        }
        if (((ef & EXEC_CONSUME) == 0) == rev) {
          es.cur += 1;
        }
        es.until = os.until;
        es.indent--;
        while (OP(re.cs[es.ci]) != OP_CHAR_ENDCLASS) {
          es.ci++;
        }
        return true;
      }
      
      es = os;
      return false;
    }
    
    //----------------------------
    // Non-Character class [^...]
    //----------------------------
    case OP_CHAR_NCLASS: {
      _DEBUG_PRINT("Not-Character class...");
      os = es;
      es.indent++;
      es.ci++;
      // do not consume characters
      es.opts = es.opts & ~EXEC_CONSUME;
      
      bool found = false;
      while (OP(re.cs[es.ci]) != OP_CHAR_ENDCLASS) {
        if (rex_step(re, es, m)) {
          found = true;
          break;
        } else {
          es.ci++;
        }
      }
      
      if (found == false) {
        if (ef & EXEC_CONSUME) {
          es.opts = es.opts | EXEC_CONSUME;
        }
        if (((ef & EXEC_CONSUME) == 0) == rev) {
          es.cur += 1;
        }
        es.until = os.until;
        while (OP(re.cs[es.ci]) != OP_CHAR_ENDCLASS) {
          es.ci++;
        }
        es.indent--;
        return true;
      }
      
      es = os;
      return false;
    }
    
    //------------------------
    // End of character class
    //------------------------
    case OP_CHAR_ENDCLASS: {
      _DEBUG_PRINT("Character class End");
      es.ci++;
      return true;
    }
    
    //-----------------
    // Any character .
    //-----------------
    case OP_CHAR_ANY: {
      _DEBUG_PRINT("Any Character");
      if (rev) {
        if (es.cur <= m.beg) {
          return false;
        }
        es.cur -= 1;
        c = *(es.cur);
      }
      if ((ef & EXEC_DOT_NEWLINE) == 0) {
        // do not match \r, \n
        if (c == '\r' || c == '\n') {
          _DEBUG_PRINT("Failed");
          if (rev) {
            es.cur += 1;
          }
          return false;
        }
      }
      es.ci++;
      if (((ef & EXEC_CONSUME) == 0) == rev) {
        es.cur += 1;
      }
      return true;
    }
    
    //-----------------------------
    // Word character [a-zA-Z0-9_]
    //-----------------------------
    case OP_CHAR_WORD: {
      _DEBUG_PRINT("Word character...");
      if (rev) {
        if (es.cur <= m.beg) {
          return false;
        }
        es.cur -= 1;
        c = *(es.cur);
      }
      if (CHAR_IS(c, WORD_CHAR)) {
        es.ci++;
        if (((ef & EXEC_CONSUME) == 0) == rev) {
          es.cur += 1;
        }
        _DEBUG_PRINT("Matched");
        return true;
      }
      if (rev) {
        es.cur += 1;
      }
      _DEBUG_PRINT("Failed");
      return false;
    }
    
    //----------------------------------
    // Non-Word character [^a-zA-Z0-9_]
    //----------------------------------
    case OP_CHAR_NOTWORD: {
      _DEBUG_PRINT("Non-Word character...");
      if (rev) {
        if (es.cur <= m.beg) {
          return false;
        }
        es.cur -= 1;
        c = *(es.cur);
      }
      if (!CHAR_IS(c, WORD_CHAR)) {
        es.ci++;
        if (((ef & EXEC_CONSUME) == 0) == rev) {
          es.cur += 1;
        }
        _DEBUG_PRINT("Matched");
        return true;
      }
      if (rev) {
        es.cur += 1;
      }
      _DEBUG_PRINT("Failed");
      return false;
    }
    
    //-----------------------
    // Digit character [0-9]
    //-----------------------
    case OP_CHAR_DIGIT: {
      _DEBUG_PRINT("Digit character...");
      if (rev) {
        if (es.cur <= m.beg) {
          return false;
        }
        es.cur -= 1;
        c = *(es.cur);
      }
      if (CHAR_IS(c, DIGIT_CHAR)) {
        es.ci++;
        if (((ef & EXEC_CONSUME) == 0) == rev) {
          es.cur += 1;
        }
        _DEBUG_PRINT("Matched");
        return true;
      }
      if (rev) {
        es.cur += 1;
      }
      _DEBUG_PRINT("Failed");
      return false;
    }
    
    //----------------------------
    // Non-Digit character [^0-9]
    //----------------------------
    case OP_CHAR_NOTDIGIT: {
      _DEBUG_PRINT("Non-Digit character...");
      if (rev) {
        if (es.cur <= m.beg) {
          return false;
        }
        es.cur -= 1;
        c = *(es.cur);
      }
      if (!CHAR_IS(c, DIGIT_CHAR)) {
        es.ci++;
        if (((ef & EXEC_CONSUME) == 0) == rev) {
          es.cur += 1;
        }
        _DEBUG_PRINT("Matched");
        return true;
      }
      if (rev) {
        es.cur += 1;
      }
      _DEBUG_PRINT("Failed");
      return false;
    }
    
    //---------------------------
    // Letter character [a-zA-Z]
    //---------------------------
    case OP_CHAR_LETTER: {
      _DEBUG_PRINT("Letter...");
      if (rev) {
        if (es.cur <= m.beg) {
          return false;
        }
        es.cur -= 1;
        c = *(es.cur);
      }
      if (CHAR_IS(c, LETTER_CHAR)) {
        es.ci++;
        if (((ef & EXEC_CONSUME) == 0) == rev) {
          es.cur += 1;
        }
        _DEBUG_PRINT("Matched");
        return true;
      }
      if (rev) {
        es.cur += 1;
      }
      _DEBUG_PRINT("Failed");
      return false;
    }
    
    //--------------------------------
    // Non-Letter character [^a-zA-Z]
    //--------------------------------
    case OP_CHAR_NOTLETTER: {
      _DEBUG_PRINT("Non-Letter...");
      if (rev) {
        if (es.cur <= m.beg) {
          return false;
        }
        es.cur -= 1;
        c = *(es.cur);
      }
      if (!CHAR_IS(c, LETTER_CHAR)) {
        es.ci++;
        if (((ef & EXEC_CONSUME) == 0) == rev) {
          es.cur += 1;
        }
        _DEBUG_PRINT("Matched");
        return true;
      }
      if (rev) {
        es.cur += 1;
      }
      _DEBUG_PRINT("Failed");
      return false;
    }
    
    //---------------------------
    // Uppercase character [A-Z]
    //---------------------------
    case OP_CHAR_UPPER: {
      _DEBUG_PRINT("Upper case letter...");
      if (rev) {
        if (es.cur <= m.beg) {
          return false;
        }
        es.cur -= 1;
        c = *(es.cur);
      }
      if (CHAR_IS(c, UPPER_CHAR) || (((ef & EXEC_NOCASE) != 0) && CHAR_IS(c, LOWER_CHAR))) {
        es.ci++;
        if (((ef & EXEC_CONSUME) == 0) == rev) {
          es.cur += 1;
        }
        _DEBUG_PRINT("Matched");
        return true;
      }
      if (rev) {
        es.cur += 1;
      }
      _DEBUG_PRINT("Failed");
      return false;
    }
    
    //---------------------------
    // Lowercase character [a-z]
    //---------------------------
    case OP_CHAR_LOWER: {
      _DEBUG_PRINT("Lower case letter...");
      if (rev) {
        if (es.cur <= m.beg) {
          return false;
        }
        es.cur -= 1;
        c = *(es.cur);
      }
      if (CHAR_IS(c, LOWER_CHAR) || (((ef & EXEC_NOCASE) != 0) && CHAR_IS(c, UPPER_CHAR))) {
        es.ci++;
        if (((ef & EXEC_CONSUME) == 0) == rev) {
          es.cur += 1;
        }
        _DEBUG_PRINT("Matched");
        return true;
      }
      if (rev) {
        es.cur += 1;
      }
      _DEBUG_PRINT("Failed");
      return false;
    }
    
    //-----------------------
    // Hexadecimal character
    //-----------------------
    case OP_CHAR_HEXA: {
      _DEBUG_PRINT("Hexa character...");
      if (rev) {
        if (es.cur <= m.beg) {
          return false;
        }
        es.cur -= 1;
        c = *(es.cur);
      }
      if (CHAR_IS(c, HEXA_CHAR)) {
        es.ci++;
        if (((ef & EXEC_CONSUME) == 0) == rev) {
          es.cur += 1;
        }
        _DEBUG_PRINT("Matched");
        return true;
      }
      if (rev) {
        es.cur += 1;
      }
      _DEBUG_PRINT("Failed");
      return false;
    }
    
    //---------------------------
    // Non-Hexadecimal character
    //---------------------------
    case OP_CHAR_NOTHEXA: {
      _DEBUG_PRINT("Non-Hexa character...");
      if (rev) {
        if (es.cur <= m.beg) {
          return false;
        }
        es.cur -= 1;
        c = *(es.cur);
      }
      if (!CHAR_IS(c, HEXA_CHAR)) {
        es.ci++;
        if (((ef & EXEC_CONSUME) == 0) == rev) {
          es.cur += 1;
        }
        _DEBUG_PRINT("Matched");
        return true;
      }
      if (rev) {
        es.cur += 1;
      }
      _DEBUG_PRINT("Failed");
      return false;
    }
    
    //-----------------
    // Space character
    //-----------------
    case OP_CHAR_SPACE: {
      _DEBUG_PRINT("Space character...");
      if (rev) {
        if (es.cur <= m.beg) {
          return false;
        }
        es.cur -= 1;
        c = *(es.cur);
      }
      if (CHAR_IS(c, SPACE_CHAR)) {
        es.ci++;
        if (((ef & EXEC_CONSUME) == 0) == rev) {
          es.cur += 1;
        }
        _DEBUG_PRINT("Matched");
        return true;
      }
      if (rev) {
        es.cur += 1;
      }
      _DEBUG_PRINT("Failed");
      return false;
    }
    
    //---------------------
    // Non-Space character
    //---------------------
    case OP_CHAR_NOTSPACE: {
      _DEBUG_PRINT("Non-Space character...");
      if (rev) {
        if (es.cur <= m.beg) {
          return false;
        }
        es.cur -= 1;
        c = *(es.cur);
      }
      if (!CHAR_IS(c, SPACE_CHAR)) {
        es.ci++;
        if (((ef & EXEC_CONSUME) == 0) == rev) {
          es.cur += 1;
        }
        _DEBUG_PRINT("Matched");
        return true;
      }
      if (rev) {
        es.cur += 1;
      }
      _DEBUG_PRINT("Failed");
      return false;
    }
    
    //------------------
    // Single character
    //------------------
    case OP_CHAR_SINGLE: {
      _DEBUG_PRINT("Single character...");
      if (rev) {
        if (es.cur <= m.beg) {
          return false;
        }
        es.cur -= 1;
        c = *(es.cur);
      }
      c0 = (char)(arg & 0x00FF);
      if (ef & EXEC_NOCASE) {
        unsigned char diff = 'A' - 'a';
        if (CHAR_IS(c0, LOWER_CHAR)) {
          c1 = (char)(c0 + diff);
          if (c == c0 || c == c1) {
            es.ci++;
            if (((ef & EXEC_CONSUME) == 0) == rev) {
              es.cur += 1;
            }
            _DEBUG_PRINT("Matched");
            return true;
          }
        } else if (CHAR_IS(c0, UPPER_CHAR)) {
          c1 = (char)(c0 - diff);
          if (c == c0 || c == c1) {
            es.ci++;
            if (((ef & EXEC_CONSUME) == 0) == rev) {
              es.cur += 1;
            }
            _DEBUG_PRINT("Matched");
            return true;
          }
        } else {
          if (c == c0) {
            es.ci++;
            if (((ef & EXEC_CONSUME) == 0) == rev) {
              es.cur += 1;
            }
            _DEBUG_PRINT("Matched");
            return true;
          } 
        }
      } else {
        if (c == c0) {
          es.ci++;
          if (((ef & EXEC_CONSUME) == 0) == rev) {
            es.cur += 1;
          }
          _DEBUG_PRINT("Matched");
          return true;
        }
      }
      if (rev) {
        es.cur += 1;
      }
      _DEBUG_PRINT("Failed");
      return false;
    }
    
    //----------------
    // Unknown opcode
    //----------------
    default: {
      _DEBUG_PRINT("??????");
      return false;
    }
  }
}

bool rex_match(const std::string &str, _Regexp &re, _Match &m, unsigned short flags, size_t offset, size_t len) {
  
  bool match = false;
  ExecState es;
  
  if (offset >= str.length()) {
    return false;
  }
  if (len == size_t(-1)) {
    len = str.length();
  }
  if (offset + len > str.length()) {
    len = str.length() - offset;
  }
  
  // Initialize match
  m.str = str;
  //m.first = m.str.c_str();
  m.beg = m.str.c_str() + offset;
  m.end = m.str.c_str() + offset + len;
  m.last = m.end - 1;
  
  for (int i=0; i<10; ++i) {
    m.mbeg[i] = m.mend[i] = 0;
  }
  
  //es.opts = EXEC_OPTS(EXEC_CONSUME|EXEC_CAPTURE|EXEC_MULTILINE, 0);
  es.opts = EXEC_OPTS(EXEC_CONSUME|flags, 0);
  es.ci = 0;
  es.cur = 0;
  es.indent = 0;
  es.until = OP_BRANCH;
  
  match = rex_step(re, es, m);
  
  if (match) {
    m.mbeg[0] = m.beg;
    m.mend[0] = es.cur - 1;
  }
  
  return match;
}

bool rex_search(const std::string &str, _Regexp &re, _Match &m, unsigned short flags, size_t offset, size_t len) {
  
  bool match = false;
  ExecState es;
  
  if (offset >= str.length()) {
    return false;
  }
  if (len == size_t(-1)) {
    len = str.length();
  }
  if (offset + len > str.length()) {
    len = str.length() - offset;
  }
  
  // Initialize match
  m.str = str;
  //m.first = m.str.c_str();
  m.beg = m.str.c_str() + offset;
  m.end = m.str.c_str() + offset + len;
  m.last = m.end - 1;
  
  for (int i=0; i<10; ++i) {
    m.mbeg[i] = m.mend[i] = 0;
  }
  
  //es.opts = EXEC_OPTS(EXEC_CONSUME|EXEC_CAPTURE|EXEC_MULTILINE, 0);
  es.opts = EXEC_OPTS(EXEC_CONSUME|flags, 0);
  es.ci = 0;
  
  size_t off = 0;
  
  while (!match && off!=m.str.length()) {
    _DEBUG_PRINT(std::endl << std::endl << "TRY at offset(" << off << ")" << std::endl);
    
    es.cur = m.beg + off;
    es.ci = 0;
    es.indent = 0;
    es.until = OP_BRANCH;
    match = rex_step(re, es, m);
    
    if (match) {
      m.mbeg[0] = m.beg + off;
      m.mend[0] = es.cur - 1;
    }
    
    off += 1;
  }
  return match;
}
