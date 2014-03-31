/*
* Copyright (c) 2007 by Gambro BCT, Inc.  All rights reserved.
* $Log: Comment.cpp $ 
* Initial revision
*
*/

/*! \file Comment.cpp
*/
#include "Comment.h"
using namespace BctXml;




Comment::~Comment(void)
{
}
Comment::Comment(void):_sValue("")
{
  _dataType=COMMENT;
}
Comment::Comment(const std::string& inValue):_sValue(inValue)
{
  _dataType=COMMENT;
}
std::string Comment::getDataString() const
{
  return _sValue;
}
int Comment::setDataString(const std::string& inValue)
{
  _sValue=inValue;
  return 1;
}
void Comment::toStream(std::ostream &output) const
{

  output<<std::endl<<"<!--"<<_sValue<<"-->";


}

std::string Comment::getName()const
{
  return "";
}

void Comment::toStream(int inTabCount, std::ostream &output) const
{


  output<<std::endl;
  for(int k=1;k<inTabCount;k++){
    output<<TAB;
  }
  output<<"<!--"<<_sValue<<"-->";


}


