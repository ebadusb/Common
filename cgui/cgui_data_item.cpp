/*
 *	Copyright (c) 2005 by Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: J:/BCT_Development/vxWorks/Common/cgui/rcs/cgui_data_item.cpp 1.9 2007/06/04 22:04:20Z wms10235 Exp rm10919 $
 * $Log: cgui_data_item.cpp $
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
   const char * string = NULL;
   ostringstream textStream;
   
   textStream.setf(ios::fixed);
   textStream.precision(0);
   textStream << _value;

   //
   // Get string length
   //
   int stringLength = strlen(string);

   //
   // Clear _string
   //
   if (_string) delete _string;

   _string = new StringChar(stringLength + 1);

   string = textStream.str().c_str();

   //
   // Copy value (string) into _string
   //
   memcpy(_string, string, stringLength * sizeof(StringChar));

   _valueChanged = false;

   return _string;
}

void CGUIDataItemInteger::setValue(int value)
{
   _value = value;
   _valueChanged = true;
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
   const char * string = NULL;
   ostringstream textStream;
   
   textStream.setf(ios::fixed);
   textStream.precision(_precision);
   textStream << _value;

   //
   // Get string length
   //
   int stringLength = strlen(string);

   //
   // Clear _string
   //
   if (_string) delete _string;

   _string = new StringChar(stringLength + 1);

   string = textStream.str().c_str();

   //
   // Copy value (string) into _string
   //
   memcpy(_string, string, stringLength * sizeof(StringChar));

   _valueChanged = false;

   return _string;
}

void CGUIDataItemDouble::setValue(double value)
{
   _value = value;
   
   _valueChanged = true;
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

}

StringChar * CGUIDataItemTextItem::convertToString()
{
   if (_string) delete _string;
   _string =  new StringChar(_value->getLength());

   memcpy (_string, _value->getText(), _value->getLength() * sizeof(StringChar));

   _valueChanged = false;
   
   return _string;
}

void CGUIDataItemTextItem::setValue(CGUITextItem * value)
{
   _value = value;

   _valueChanged = true;
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

}

StringChar * CGUIDataItemText::convertToString()
{
   //
   // Find CGUITextItem for _value.
   // Currently not done since CGUIStringData::findString
   // doesn't exsist!
//   CGUITextItem * textItem = findString(_value);
  
   if (_string) delete _string;

   // This doesn't mean anything for _string!
   // _string = textItem->getText();
   _string = convertToStringChar(_value);

   _valueChanged = false;
   
   return _string;
}

void CGUIDataItemText::setValue(char * value)
{
   _value = value;

   _valueChanged = true;
}

