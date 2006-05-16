/*
 *	Copyright (c) 2005 by Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: J:/BCT_Development/vxWorks/Common/cgui/rcs/cgui_data_item.cpp 1.9 2007/06/04 22:04:20Z wms10235 Exp rm10919 $
 * $Log: cgui_data_item.cpp $
 * Revision 1.3  2005/11/22 00:34:42Z  rm10919
 * Get data item database to work with software layers.
 * Revision 1.2  2005/09/30 22:40:43Z  rm10919
 * Get the variable database working!
 * Revision 1.1  2005/04/27 13:40:50Z  rm10919
 * Initial revision
 *
 */

#include <vxworks.h>
#include "cgui_data_item.h"
#include <iostream>
#include <sstream>

//
// Void Constructor
//
CGUIDataItem::CGUIDataItem() : _string(NULL), _valueChanged(false)
{
}

CGUIDataItem::CGUIDataItem(bool valueChanged) : _string(NULL), _valueChanged(valueChanged)
{

}


CGUIDataItem::~CGUIDataItem()
{
   delete _string;
}

//
// Void Constructor
//
CGUIDataItemInteger::CGUIDataItemInteger()
{

}

CGUIDataItemInteger::CGUIDataItemInteger(int value):  CGUIDataItem(true), _value(value) 
{

}

CGUIDataItemInteger::~CGUIDataItemInteger()
{

}

StringChar * CGUIDataItemInteger::convertToString(void)
{
   if (_valueChanged || !_string)
   {
      const char * string = NULL;
      ostringstream textStream;

      textStream.setf(ios::fixed);
      textStream.precision(0);
      textStream << _value;
      
      string = textStream.str().c_str();

      //
      // Clear _string
      //
      if (_string) delete _string;

      //
      // Copy value (string) into _string
      //
      convertToStringChar(string, &_string);
      
      _valueChanged = false;
   }
   return _string;
}

void CGUIDataItemInteger::setValue(int value)
{
   if (_value != value)
   {
      _value = value;
      _valueChanged = true;
   }
}

//
// Void Constructor
//
CGUIDataItemDouble::CGUIDataItemDouble()
{
}

CGUIDataItemDouble::CGUIDataItemDouble(double value, int precision): CGUIDataItem(true),
_value(value), _precision(precision)
{

}

CGUIDataItemDouble::~CGUIDataItemDouble()
{
}

StringChar * CGUIDataItemDouble::convertToString(void)
{
   if (_valueChanged || _string)
   {
      const char * string = NULL;

      ostringstream textStream;

      textStream.setf(ios::fixed);
      textStream.precision(_precision);
      textStream << _value;
      
      string = textStream.str().c_str();

      //
      // Clear _string
      //
      if (_string) delete _string;

      //
      // Copy value (string) into _string
      //
      convertToStringChar(string, &_string);

      _valueChanged = false;
   }
   return _string;
}

void CGUIDataItemDouble::setValue(double value)
{
   if (_value != value)
   {
      _value = value;
      _valueChanged = true;
   }
}

void CGUIDataItemDouble::setPrecision(int precision)
{
   _precision = precision;
   _valueChanged = true;
}


//
// Void Constructor
//
CGUIDataItemTextItem::CGUIDataItemTextItem()
{

}


CGUIDataItemTextItem::CGUIDataItemTextItem(CGUITextItem * value): CGUIDataItem(true), _value(value)
{

}

CGUIDataItemTextItem::~CGUIDataItemTextItem()
{
   delete _value;
}

StringChar * CGUIDataItemTextItem::convertToString()
{
   if (_valueChanged || !_string)
   {
      if (_string) delete _string;
      _string =  new StringChar[_value->getLength()+1];

      memcpy (_string, _value->getText(), _value->getLength() * sizeof(StringChar));

      _string[_value->getLength()] = '\0';

      _valueChanged = false;
   }
   return _string;
}

void CGUIDataItemTextItem::setValue(CGUITextItem * value)
{
   if (_value != value)
   {
      _value = value;

      _valueChanged = true;
   }
}

//
// Void Constructor
//
CGUIDataItemText::CGUIDataItemText()
{

}


CGUIDataItemText::CGUIDataItemText(char * value): CGUIDataItem(true), _value(value)
{

}

CGUIDataItemText::~CGUIDataItemText()
{
   delete _value;
}

StringChar * CGUIDataItemText::convertToString()
{  
   if (_valueChanged || !_string)
   {
      if (_string) delete _string;

      convertToStringChar(_value, &_string);

      _valueChanged = false;
   }
   return _string;
}

void CGUIDataItemText::setValue(char * value)
{
   if (_value != value)
   {
      _value = value;

      _valueChanged = true;
   }
}

