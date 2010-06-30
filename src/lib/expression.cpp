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

// OP_BRANCH / OP_JUMP ?
// OP_BRANCH [next branch offset]
// OP_BRANCH [next branch offset]
// OP_BRANCH [0] --> last branch

// Expression ::= Branch { "|" Branch }*
bool parse_expression(const char **ppc, _Regexp &re) {
#ifdef _DEBUG
  std::cerr << std::endl << "Regexp::parse_expression...";
#endif
  size_t pbeg = re.cs.size();
  size_t nbeg = 0;
  
  re.cs.push_back(OPARG(OP_BRANCH,0));//0 means no next branch
  
  if (parse_branch(ppc,re)) {
    while (**ppc == '|') {
      nbeg = re.cs.size();
      re.cs.push_back(OPARG(OP_BRANCH,0));
      re.cs[pbeg] = OPARG(OP_BRANCH, nbeg-pbeg);
      pbeg = nbeg;
      ++(*ppc);
      if (!parse_branch(ppc,re)) {
        return false;
      }
    }
  }
  
  nbeg = re.cs.size();
  re.cs[pbeg] = OPARG(OP_BRANCH, nbeg-pbeg);
  re.cs.push_back(OPARG(OP_BRANCH,0)); // last branch is empty
  return true;
}
