/*
* Copyright (c) 2007 by Gambro BCT, Inc.  All rights reserved.
* $Log: XMLMemberType.cpp $ 
* Initial revision
*
*/

/*! \file XMLMemberType.cpp
*/
#include "XMLMemberType.h"

using namespace BctXml;
XMLMemberType::XMLMemberType(void):_dataType(ERR)
{
}


XMLMemberType::~XMLMemberType(void)
{
}
bool XMLMemberType::isData(void)
{
  return (_dataType==DATA);
}		
bool XMLMemberType::isElement(void)
{
  return (_dataType==ELEMENT);
}
bool XMLMemberType::isComment(void)
{
  return (_dataType==COMMENT);
}	

std::string XMLMemberType::print(int inTabCount) const
{
  std::ostringstream oStr;
  toStream(inTabCount,oStr);
  return oStr.str();

}
std::string XMLMemberType::print() const
{

  std::ostringstream oStr;
  toStream(oStr);
  return oStr.str();

}
void XMLMemberType::toStream(int inTabCount,std::ostream &output) const
{
  toStream(output);
}

std::ostream& operator<<(std::ostream&s,const BctXml::XMLMemberType&e)
{
  e.toStream(s);
  return s;
}

std::string XMLMemberType::trim(const std::string& inString)
{
  size_t found;
  std::string output = inString;
  std::string whitespaces (" \t\f\v\n\r");
  found=output.find_last_not_of(whitespaces);
  if (found!=std::string::npos)
  {
    output.erase(found+1);
    found = output.find_first_not_of(whitespaces);
    if(found != std::string::npos)
      output.erase(0, found);

  }
  else
    output="";

  return output;
}
