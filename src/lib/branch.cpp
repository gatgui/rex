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

// Branch ::= { Piece }*
bool parse_branch(const char **ppc, Regexp &re) {
  
#ifdef _DEBUG
  cerr << endl << "Regexp::parse_branch...";
#endif
  while (**ppc!='\0' && **ppc!='|' && **ppc!=')') {
    if (!parse_piece(ppc,re)) {
      return false;
    }
  }
  return true;
}
