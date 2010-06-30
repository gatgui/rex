/*

Copyright (C) 2010  Gaetan Guidet

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

#ifndef __rex_rex_h_
#define __rex_rex_h_

#include <string>
#include <vector>

// ignore escape character in string
#define IEC(str) (_Exp(#str))

struct _Exp {
  inline _Exp(const char *s)
    :e(s) {
    e.erase(0,1);
    e.erase(e.length()-1,1);
  }
  inline operator std::string () const {
    return e;
  }
  private:
    std::string e;
};

enum ExecFlags {
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

class Regexp {
  public:
    
    class Match {
      public:

        friend class Regexp;

        Match();
        Match(const Match &rhs);
        ~Match();

        Match& operator=(const Match &rhs);

        std::string pre() const;
        std::string post() const;
        std::string group(size_t i) const;
        // offset in matched string (group(0))
        size_t offset(size_t i) const;
        size_t length(size_t i) const;
        size_t numGroups() const;
        bool hasGroup(size_t i) const;

      protected:

        std::string mStr;
        int mBeg;
        int mLast;
        int mGrpBeg[10];
        int mGrpEnd[10];
    };
  
  public:
    
    Regexp();
    Regexp(const std::string &exp);
    Regexp(const Regexp &rhs);
    ~Regexp();
    
    Regexp& operator=(const Regexp &rhs);
    
    bool isValid() const;
    
    void setExpression(const std::string &exp);
    const std::string getExpression() const;
    
    bool search(const std::string &s, Match &m, unsigned short execflags=EXEC_CAPTURE, size_t offset=0, size_t len=size_t(-1)) const;
    bool search(const std::string &s, unsigned short execflags=0, size_t offset=0, size_t len=size_t(-1)) const;
    
    bool match(const std::string &s, Match &m, unsigned short execflags=EXEC_CAPTURE, size_t offset=0, size_t len=size_t(-1)) const;
    bool match(const std::string &s, unsigned short execflags=0, size_t offset=0, size_t len=size_t(-1)) const;
    
    void printCode() const;
    
  protected:
    
    bool mValid;
    
    struct ExpData
    {
      std::string str;
      std::vector<unsigned long> code;
    };
    
    ExpData mExp;
};


#endif


