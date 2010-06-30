#include <rex/rex.h>
#include "parser.h"

Regexp::Match::Match()
  : mStr(""), mBeg(0), mLast(0)
{
  for (size_t i=0; i<10; ++i)
  {
    mGrpBeg[i] = -1;
    mGrpEnd[i] = -1;
  }
}

Regexp::Match::Match(const Regexp::Match &rhs)
{
  operator=(rhs);
}

Regexp::Match::~Match()
{
}

Regexp::Match& Regexp::Match::operator=(const Regexp::Match &rhs)
{
  if (this != &rhs)
  {
    mStr = rhs.mStr;
    mBeg = rhs.mBeg;
    mLast = rhs.mLast;
    for (size_t i=0; i<10; ++i)
    {
      mGrpBeg[i] = rhs.mGrpBeg[i];
      mGrpEnd[i] = rhs.mGrpEnd[i];
    }
  }
  return *this;
}

bool Regexp::Match::hasGroup(size_t i) const
{
  return (i < 10 && mGrpBeg[i] > 0 && mGrpEnd[i] > 0);
}

std::string Regexp::Match::pre() const
{
  return mStr.substr(mBeg, mGrpBeg[0]-mBeg+1);
}

std::string Regexp::Match::post() const
{
  return mStr.substr(mGrpEnd[0]+1, mLast-mGrpEnd[0]);
}

std::string Regexp::Match::group(size_t i) const
{
  if (!hasGroup(i))
  {
    return "";
  }
  return mStr.substr(mGrpBeg[i], mGrpEnd[i] - mGrpBeg[i] + 1);
}

size_t Regexp::Match::offset(size_t i) const
{
  if (!hasGroup(i))
  {
    return 0;
  }
  return mGrpBeg[i] - mBeg;
}

size_t Regexp::Match::length(size_t i) const
{
  if (!hasGroup(i))
  {
    return 0;
  }
  return (mGrpEnd[i] - mGrpBeg[i] + 1);
}

size_t Regexp::Match::numGroups() const
{
  size_t numGrp = 0;
  for (size_t i=0; i<10; ++i)
  {
    if (mGrpBeg[i] > 0 && mGrpEnd[i] > 0)
    {
      ++numGrp;
    }
  }
  return numGrp;
}

// ---

Regexp::Regexp()
  : mValid(false)
{
}

Regexp::Regexp(const std::string &exp)
  : mValid(false)
{
  setExpression(exp);
}

Regexp::Regexp(const Regexp &rhs)
{
  operator=(rhs);
}

Regexp::~Regexp()
{
}

Regexp& Regexp::operator=(const Regexp &rhs)
{
  if (this != &rhs)
  {
    mValid = rhs.mValid;
    mExp = rhs.mExp;
  }
  return *this;
}

bool Regexp::isValid() const
{
  return mValid;
}

void Regexp::setExpression(const std::string &exp)
{
  mExp.str = exp;
  const char *pc = mExp.str.c_str();
  mValid = parse_expression(&pc, *((_Regexp*)&mExp));
}

const std::string Regexp::getExpression() const
{
  return mExp.str;
}

bool Regexp::search(const std::string &s, Regexp::Match &m, unsigned short execflags, size_t offset, size_t len) const
{
  if (!mValid)
  {
    return false;
  }
  
  _Match _m;
  
  if (rex_search(s, *((_Regexp*)&mExp), _m, execflags, offset, len))
  {
    m.mStr  = s;
    m.mBeg  = int(_m.beg  - _m.str.c_str());
    m.mLast = int(_m.last - _m.str.c_str());
    for (int i=0; i<10; ++i)
    {
      m.mGrpBeg[i] = int(_m.mbeg[i] == 0 ? -1 : _m.mbeg[i] - _m.str.c_str());
      m.mGrpEnd[i] = int(_m.mend[i] == 0 ? -1 : _m.mend[i] - _m.str.c_str());
    }
    return true;
  }
  
  return false;
}

bool Regexp::search(const std::string &s, unsigned short execflags, size_t offset, size_t len) const
{
  if (!mValid)
  {
    return false;
  }
  
  _Match _m;
  
  return rex_search(s, *((_Regexp*)&mExp), _m, execflags, offset, len);
}

bool Regexp::match(const std::string &s, Regexp::Match &m, unsigned short execflags, size_t offset, size_t len) const
{
  if (!mValid)
  {
    return false;
  }
  
  _Match _m;
  
  if (rex_search(s, *((_Regexp*)&mExp), _m, execflags, offset, len))
  {
    m.mStr  = s;
    m.mBeg  = int(_m.beg  - _m.str.c_str());
    m.mLast = int(_m.last - _m.str.c_str());
    for (int i=0; i<10; ++i)
    {
      m.mGrpBeg[i] = int(_m.mbeg[i] == 0 ? -1 : _m.mbeg[i] - _m.str.c_str());
      m.mGrpEnd[i] = int(_m.mend[i] == 0 ? -1 : _m.mend[i] - _m.str.c_str());
    }
    return true;
  }
  
  return false;
}

bool Regexp::match(const std::string &s, unsigned short execflags, size_t offset, size_t len) const
{
  if (!mValid)
  {
    return false;
  }
  
  _Match _m;
  
  return rex_match(s, *((_Regexp*)&mExp), _m, execflags, offset, len);
}

void Regexp::printCode() const
{
  print_expcode(mExp.code);
}

