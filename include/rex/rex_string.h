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

#ifndef _rexp_string_h_
#define _rexp_string_h_

#include <string>

#define RexpString(str) (_Exp(#str))

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

#endif


