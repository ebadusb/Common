/*
* Copyright (c) 2007 by Gambro BCT, Inc.  All rights reserved.
* $Log: Attribute.cpp $
* Revision 1.1  2007/11/15 20:42:42Z  estausb
* Initial revision
*
*/

/*! \file Attribute.cpp
*/

#include "Attribute.h"

using namespace BctXml;

Attribute::Attribute(const std::string& name, 
                     const std::string& val) : _name(name), _val(val)
{
}

Attribute::Attribute(const std::string& name) : _name(name), _val("")
{
}

Attribute::~Attribute()
{
}

std::string Attribute::getName() const
{
  return _name;
}

std::string Attribute::getValue() const
{
  return _val;
}

void Attribute::changeValue(const std::string& newValue)
{
  _val = newValue;
}

