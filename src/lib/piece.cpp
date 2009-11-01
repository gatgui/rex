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

// Piece ::= Atom [ Rep ]
bool parse_piece(const char **ppc, Regexp &re) {
#ifdef _DEBUG
  cerr << endl << "Regexp::parse_piece...";
#endif
  size_t as = re.cs.size();
  if (parse_atom(ppc,re)) {
    size_t ae = re.cs.size();
#ifdef _DEBUG
    cerr << ae-as << " instruction in atom...";
#endif
    if (parse_repeat(ppc,re)) {
      if (re.cs.size() > ae) {
        unsigned long rinst[4];
        for (int i=3; i>=0; --i) {
          rinst[i] = re.cs.back();
          re.cs.pop_back();
        }
        
        vector<unsigned long>::iterator it;
        it = re.cs.begin() + as;
        
        // add three instruction, Repeat opcode, rmin, rmax
        re.cs.insert(it, 3, 0);
        
        //re.cs[as] = rinst[0]; // add next instruction idx as argument
        re.cs[as+1] = rinst[1];
        re.cs[as+2] = rinst[2];
        
        //add repeat end marker
        re.cs.push_back(rinst[3]);
        // if repeat block size is needed in OP_REPEAT_END, could use:
        //re.cs.push_back(OPARG(OP(rinst[3]), re.cs.size()-as));
        
        re.cs[as] = OPARG(rinst[0], re.cs.size()-as);
        
      } else {
#ifdef _DEBUG
        cerr << "...no repeat";
#endif
      }
      return true;
    }
  }
  return false;
}

