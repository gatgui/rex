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
#include <algorithm>
#include <iostream>

// ignore escape character in string
#define RAW(str) (_RawString(#str))

struct _RawString {
  inline _RawString(const char *s)
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


class Rex {
  public:
    
    // Remove Consume, Capture, Not and Inherit stuffs
    enum Flags {
      NoCase          = 0x0001, // ignore case
      //Consume         = 0x0002, // increment input pointer
      //Capture         = 0x0004, // group are capturing
      Reverse         = 0x0008, // apply backwards (no group capture)
      Multiline       = 0x0010, // makes ^ and $ match at line boundaries (instead of buffer like \A \Z)
      //Not             = 0x0020, // no match is match
      DotMatchNewline = 0x0040, // dot matches new line chars \r and/or \n
      // special values for sub options
      //InheritMultiline       = 0x0100, // group inherit multiline flag
      //InheritNoCase          = 0x0200, // group inherit nocase flags
      //InheritDotMatchNewline = 0x0400  // group inherit dot match newline
    };
    
    class Match {
      public:
        
        friend class Rex;
        
        typedef std::pair<int,int> Range;
        
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
        Range mRange;
        std::vector<Range> mGroups; // 0 if full match
    };
  
  public:
    
    Rex();
    Rex(const std::string &exp);
    Rex(const Rex &rhs);
    ~Rex();
    
    Rex& operator=(const Rex &rhs);
    
    bool valid() const;
    
    void set(const std::string &exp);
    const std::string get() const;
    
    bool search(const std::string &s, Match &m, unsigned short flags=0, size_t offset=0, size_t len=size_t(-1)) const;
    bool search(const std::string &s, unsigned short execflags=0, size_t offset=0, size_t len=size_t(-1)) const;
    
    bool match(const std::string &s, Match &m, unsigned short flags=0, size_t offset=0, size_t len=size_t(-1)) const;
    bool match(const std::string &s, unsigned short execflags=0, size_t offset=0, size_t len=size_t(-1)) const;
    
    std::string substitute(const Match &m, const std::string &in, bool *failed=0) const;
    std::string substitute(const std::string &str, const std::string &in, bool *failed=0) const;
    
    friend std::ostream& operator<<(std::ostream &os, const Rex &r);
    
  protected:
    
    bool mValid;
    std::string mExp;
    class Instruction *mCode;
    int mNumGroups;
};


#endif


