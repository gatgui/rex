#include <rex/rex.h>
#include "parse.h"
#include "instruction.h"

Rex::Match::Match()
  : mStr("")
{
}

Rex::Match::Match(const Rex::Match &rhs)
{
  operator=(rhs);
}

Rex::Match::~Match()
{
}

Rex::Match& Rex::Match::operator=(const Rex::Match &rhs)
{
  if (this != &rhs)
  {
    mStr = rhs.mStr;
    mGroups = rhs.mGroups;
  }
  return *this;
}

bool Rex::Match::hasGroup(size_t i) const
{
  return (i < mGroups.size() && mGroups[i].first>=0 && mGroups[i].second>=0);
}

std::string Rex::Match::pre() const
{
  //return mStr.substr(mRange.first, mGroups[0].first - mRange.first + 1);
  return mStr.substr(0, mRange.first);
}

std::string Rex::Match::post() const
{
  //return mStr.substr(mGroups[0].second + 1, mRange.second - mGroups[0].second);
  return mStr.substr(mRange.second);
}

std::string Rex::Match::group(size_t i) const
{
  if (!hasGroup(i))
  {
    return "";
  }
  return mStr.substr(mGroups[i].first, mGroups[i].second - mGroups[i].first);
}

size_t Rex::Match::offset(size_t i) const
{
  if (!hasGroup(i))
  {
    return 0;
  }
  return mGroups[i].first - mRange.first;
}

size_t Rex::Match::length(size_t i) const
{
  if (!hasGroup(i))
  {
    return 0;
  }
  return (mGroups[i].second - mGroups[i].first + 1);
}

size_t Rex::Match::numGroups() const
{
  return mGroups.size();
}

// ---

Rex::Rex()
  : mValid(false), mCode(0), mNumGroups(0)
{
}

Rex::Rex(const std::string &exp)
  : mValid(false), mCode(0), mNumGroups(0)
{
  set(exp);
}

Rex::Rex(const Rex &rhs)
  : mValid(false), mCode(0), mNumGroups(0)
{
  operator=(rhs);
}

Rex::~Rex()
{
  if (mCode)
  {
    delete mCode;
  }
}

Rex& Rex::operator=(const Rex &rhs)
{
  if (this != &rhs)
  {
    mValid = rhs.mValid;
    mExp = rhs.mExp;
    mNumGroups = rhs.mNumGroups;
    if (mCode)
    {
      delete mCode;
    }
    //mCode = rhs.mCode->clone();
    mCode = Instruction::CloneList(rhs.mCode);
  }
  return *this;
}

bool Rex::valid() const
{
  return mValid;
}

void Rex::set(const std::string &exp)
{
  ParseInfo info;
  info.numGroups = 0;
  
  mExp = exp;
  
  const char *pc = mExp.c_str();
  
  mCode = ParseExpression(&pc, info);
  
  mNumGroups = info.numGroups;
}

const std::string Rex::get() const
{
  return mExp;
}

bool Rex::search(const std::string &s, Rex::Match &m, unsigned short flags, size_t offset, size_t len) const
{
  if (!mCode)
  {
    return false;
  }
  
  if (len == size_t(-1))
  {
    len = s.length();
  }
  
  if (offset >= s.length())
  {
    return false;
  }
  
  if (offset+len > s.length())
  {
    return false;
  }
  
  //MatchInfo info(s.c_str()+offset, s.c_str()+offset+len, flags, mNumGroups+1);
  //const char *cur = info.beg;
  
  MatchInfo info(s.c_str(), s.c_str()+s.length(), flags, mNumGroups+1);
  
  const char *beg = info.beg + offset;
  const char *end = beg + len;
  const char *cur = beg;
  size_t step = 1;
  Instruction *code = mCode;
  
  if (flags & Rex::Reverse)
  {
    cur = end;
    step = -1;
    while (code->next())
    {
      code = code->next();
    }
  }
  
  //while (cur < info.end)
  //while (cur < end)
  do
  {
#ifdef _DEBUG
    std::cout << "Try match with \"" << cur << "\"" << std::endl;
#endif
    const char *rv = code->match(cur, info);
    if (rv != 0)
    {
      std::swap(info.gmatch, m.mGroups);
      if (flags & Rex::Reverse)
      {
        m.mRange.first = rv - info.beg;
        m.mRange.second = cur - info.beg;
      }
      else
      {
        m.mRange.first = cur - info.beg;
        m.mRange.second = rv - info.beg;
      }
      m.mGroups[0].first = m.mRange.first;
      m.mGroups[0].second = m.mRange.second;
      m.mStr = s;
#ifdef _DEBUG
      std::cout << "  Matched string: \"" << m.mStr << "\"" << std::endl;
      std::cout << "  Matched range: [" << m.mRange.first << ", " << m.mRange.second << "]" << std::endl;
      for (size_t i=0; i<m.mGroups.size(); ++i)
      {
        std::cout << "  Matched group " << i << ": [" << m.mGroups[i].first << ", " << m.mGroups[i].second << "]: \"" << m.group(i) << "\"" << std::endl;
      }
      std::cout << "  Pre: \"" << m.pre() << "\"" << std::endl;
      std::cout << "  Post: \"" << m.post() << "\"" << std::endl;
#endif
      return true;
    }
    //++cur;
    cur += step;
  } while (cur >= beg && cur < end);
  
  return false;
}

bool Rex::search(const std::string &s, unsigned short flags, size_t offset, size_t len) const
{
  Rex::Match m;
  return search(s, m, flags, offset, len);
}

bool Rex::match(const std::string &s, Rex::Match &m, unsigned short flags, size_t offset, size_t len) const
{
  if (!mCode)
  {
    return false;
  }
  
  if (len == size_t(-1))
  {
    len = s.length();
  }
  
  if (offset >= s.length())
  {
    return false;
  }
  
  if (offset+len > s.length())
  {
    return false;
  }
  
  //MatchInfo info(s.c_str()+offset, s.c_str()+offset+len, flags, mNumGroups+1);
  //const char *cur = info.beg;
  MatchInfo info(s.c_str(), s.c_str()+s.length(), flags, mNumGroups+1);
  
  const char *cur = info.beg + offset;
  Instruction *code = mCode;
  
  if (flags & Rex::Reverse)
  {
    cur += len;
    while (code->next())
    {
      code = code->next();
    }
  }
  
  const char *rv = mCode->match(cur, info);
  if (rv != 0)
  {
    std::swap(info.gmatch, m.mGroups);
    if (flags & Rex::Reverse)
    {
      m.mRange.first = rv - info.beg;
      m.mRange.second = cur - info.beg;
    }
    else
    {
      m.mRange.first = cur - info.beg;
      m.mRange.second = rv - info.beg;
    }
    m.mGroups[0].first = m.mRange.first;
    m.mGroups[0].second = m.mRange.second;
    m.mStr = s;
    return true;
  }
  else
  {
    return false;
  }
}

bool Rex::match(const std::string &s, unsigned short flags, size_t offset, size_t len) const
{
  Rex::Match m;
  return match(s, m, flags, offset, len);
}

std::ostream& operator<<(std::ostream &os, const Rex &r)
{
  if (r.mCode != 0)
  {
    r.mCode->toStream(os);
  }
  return os;
}

