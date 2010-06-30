#include <rex/rex.h>
#include <iostream>

int main(int, char **)
{
  //Regexp re(IEC("^\s*([a-z]+_\d{3})*?\1$"));
  //Regexp re(IEC("([a-z]+_\d{3})\1$"));
  //Regexp re(IEC("^(.*?,){11}P"));
  //Regexp re(IEC("^([^,\r\n]*,){11}P"));
  //Regexp re(IEC("(?<=\d{3}[a-z0-9_-]o)taro"));
  //Regexp re(IEC("(?<=\bt)a"));
  //Regexp re(IEC("$.*hello"));
  //Regexp re("^hello$");
  //Regexp re("(?<=^hello)\\staro");
  //Regexp re("(?<=hello$)\r?\n\\staro");
  Regexp re("(?<=frame\\.\\d+\\.exr$)\r?\n\\staro");
  // $ does not get recognized as LineEnd ... why???
  // that was the case in old code
  
  re.printCode();
  
  Regexp::Match m;
  
  unsigned short flags = EXEC_CAPTURE|EXEC_MULTILINE;
  //unsigned short flags = EXEC_CAPTURE|EXEC_MULTILINE;
  //unsigned short flags = EXEC_CAPTURE;
  
  //bool success = re.search("  mypicture_000tata_007tata_007", re, m);
  //bool success = re.search("1,2,3,4,5,6,7,8,9,10,11,P", re, m);
  //bool success = re.search("  taro, 001_otaro", re, m);
  //bool success = re.search(" 001_otaro,taro", re, m);
  //bool success = re.search(" yotaro taro", re, m);
  bool success = re.search("frame.1.exr\n taro", m, flags);
  
  if (success)
  {
    std::cout << std::endl << "Matched Expression(" << m.offset(0) << ":" << m.length(0) << ")" << std::endl;
    std::cout << "-> \"" << m.group(0) << "\"" << std::endl;
    
    std::cout << "Pre: \"" << m.pre() << "\"" << std::endl;
    std::cout << "Post: \"" << m.post() << "\"" << std::endl;
    
    for (size_t i=1; i<m.numGroups(); ++i)
    {
      std::cout << "Match " << i << ":" << std::endl;
      std::cout << "-> \"" << m.group(i) << "\"" << std::endl;
    }
  }
  else
  {
    std::cout << std::endl << "Unmatched" << std::endl;
  }
  
  return 0;
}
