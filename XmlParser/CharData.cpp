/*
* Copyright (c) 2007 by Gambro BCT, Inc.  All rights reserved.
* $Log: CharData.cpp $ 
* Initial revision
*
*/

/*! \file CharData.cpp
*/
#include "CharData.h"
using namespace BctXml;

CharData::CharData(void):_sValue("")
{
  _dataType=DATA;
}
CharData::CharData(const std::string& inValue):_sValue(inValue)
{
  _dataType=DATA;
}

void CharData::toStream(std::ostream &output) const
{
  output<<_sValue;
}
std::string CharData::getName()const
{
  return "";
}
CharData::~CharData(void)
{
}
std::string CharData::getDataString() const
{
  return _sValue;
}
int CharData::setDataString(const std::string& inValue)
{
  _sValue=inValue;
  return 1;
}


