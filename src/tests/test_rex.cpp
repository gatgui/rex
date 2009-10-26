#include <rex/rex_parser.h>
#include <iostream>
using namespace std;

#if 0
# define PARSE_ONLY
#endif

int main(int, char **)
{
  init_table();
  
  Regexp re;
  //re.exp = RexpString("^\s*([a-z]+_\d{3})*?\1$");
  //re.exp = RexpString("([a-z]+_\d{3})\1$");
  //re.exp = RexpString("^(.*?,){11}P");
  //re.exp = RexpString("^([^,\r\n]*,){11}P");
  //re.exp = RexpString("(?<=\d{3}[a-z0-9_-]o)taro");
  //re.exp = RexpString("(?<=\bt)a");
  //re.exp = RexpString("$.*hello");
  //re.exp = "^hello$";
  //re.exp = "(?<=^hello)\\staro";
  //re.exp = "(?<=hello$)\r?\n\\staro";
  re.exp = "(?<=frame\\.\\d+\\.exr$)\r?\n\\staro";
  
  // "  mypicture_000abut_111" matches
  
  const char *pc = re.exp.c_str();

  cout << "Parsing expresion: \"" << re.exp << "\"" << endl;
  if (!parse_expression(&pc, re)) {
    cout << "\tfailed" << endl;
  } else {
    cout << endl << endl;
    print_expcode(re.cs);
  }
  
#ifndef PARSE_ONLY
  Match m;
  unsigned short flags = EXEC_CAPTURE|EXEC_MULTILINE;
  //unsigned short flags = EXEC_CAPTURE|EXEC_MULTILINE;
  //unsigned short flags = EXEC_CAPTURE;
  //bool success = rex_search("  mypicture_000tata_007tata_007", re, m);
  //bool success = rex_execute("1,2,3,4,5,6,7,8,9,10,11,P", re, m);
  //bool success = rex_execute("  taro, 001_otaro", re, m);
  //bool success = rex_execute(" 001_otaro,taro", re, m);
  //bool success = rex_search(" yotaro taro", re, m);
  bool success = rex_search("frame.1.exr\n taro", re, m, flags);
  if (success) {
    size_t len0 = m.mend[0] - m.mbeg[0] + 1;
    
    cout << endl << "Matched Expression(" << m.mbeg[0]-m.beg << ":" << len0 << ")" << endl;
    cout << "-> \"";
    
    for (const char *cc=m.mbeg[0]; cc<=m.mend[0]; ++cc)
    {
      cout << *cc;
    }
    cout << "\"" << endl;
    
    for (int i=1; i<10; ++i)
    {
      if (m.mbeg[i]!=0 && m.mend[i]!=0)
      {
        cout << "Match " << i << ":" << endl;
        cout << "-> \"";
        for (const char *cc=m.mbeg[i]; cc<=m.mend[i]; ++cc)
        {
          cout << *cc;
        }
        cout << "\"" << endl;
      }
    }
  }
  else
  {
    cout << endl << "Unmatched" << endl;
  }
#endif
  
  return 0;
}
