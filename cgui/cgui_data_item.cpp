/*
 *	Copyright (c) 2005 by Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: J:/BCT_Development/vxWorks/Common/cgui/rcs/cgui_data_item.cpp 1.9 2007/06/04 22:04:20Z wms10235 Exp rm10919 $
 * $Log: cgui_data_item.cpp $
 * Revision 1.8  2007/04/05 17:36:06Z  pn02526
 * Make StringChar * arg a const StringChar *
 * Revision 1.7  2007/03/28 12:39:07  pn02526
 * Add ability to specify a decimal separator.
 * Revision 1.6  2007/03/01 12:09:54  rm10919
 * Add DataItemText to file reader and setText(char *) to DITextItem.
 * Revision 1.5  2006/07/31 16:03:30Z  rm10919
 * Update presicion in double to handle numbers less than 0.
 * Revision 1.4  2006/05/15 21:49:43Z  rm10919
 * update for change in convertToStringChar in cgui_graphics.
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
CGUIDataItem::CGUIDataItem(void) :
	_valueChanged(false)
{
}

CGUIDataItem::CGUIDataItem(bool valueChanged) :
	_valueChanged(valueChanged)
{
}


CGUIDataItem::~CGUIDataItem()
{
}

//
// Void Constructor
//
CGUIDataItemInteger::CGUIDataItemInteger(void) :
	_value(0)
{
}

CGUIDataItemInteger::CGUIDataItemInteger(int value) :
	CGUIDataItem(true),
	_value(value)
{
}

CGUIDataItemInteger::~CGUIDataItemInteger()
{
}

const StringChar * CGUIDataItemInteger::convertToString(void)
{
	if( _valueChanged )
	{
		const char * intString = NULL;
		ostringstream textStream;

		textStream.setf(ios::fixed);
		textStream.precision(0);
		textStream << _value;

		intString = textStream.str().c_str();

		//
		// Copy value (string) into _string
		//
		if( intString )
			_string = intString;
		else
			_string.empty();

		_valueChanged = false;
	}

	return _string.getString();
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
CGUIDataItemDouble::CGUIDataItemDouble(void) :
	_separator(NULL),
	_value(0.0),
	_precision(0)
{
}

CGUIDataItemDouble::CGUIDataItemDouble(double value, int precision) :
	CGUIDataItem(true),
	_value(value),
	_precision(precision),
	_separator(NULL)
{
}

CGUIDataItemDouble::CGUIDataItemDouble(double value, int precision, CGUITextItem * separator) :
	CGUIDataItem(true),
	_value(value),
	_precision(precision),
	_separator(separator)
{
}

CGUIDataItemDouble::~CGUIDataItemDouble()
{
}

const StringChar * CGUIDataItemDouble::convertToString(void)
{
	if( _valueChanged )
	{
		ostringstream textStream;

		textStream.setf(ios::fixed);
		if (_precision < 0)
			textStream.precision(0);
		else
			textStream.precision(_precision);

		textStream << _value;

		_string = textStream.str().c_str();

		if( _separator != NULL )
		{
			// replace 1st period with the given separator.
			int seperatorPos = _string.find( (StringChar)'.' );

			if( seperatorPos >= 0 )
			{
				_string.deleteChar( seperatorPos, 1 );
				_string.insert( _separator->getTextObj(), seperatorPos );
			}
		}

		_valueChanged = false;
	}

	return _string.getString();
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
CGUIDataItemTextItem::CGUIDataItemTextItem(void) :
	_value(NULL)
{
}

CGUIDataItemTextItem::CGUIDataItemTextItem(CGUITextItem * value) :
	CGUIDataItem(true),
	_value(value)
{
}

CGUIDataItemTextItem::~CGUIDataItemTextItem()
{
}

const StringChar * CGUIDataItemTextItem::convertToString()
{
	if (_valueChanged && _value != NULL)
	{
		_string = _value->getTextObj();
		_valueChanged = false;
	}

	return _string.getString();
}

void CGUIDataItemTextItem::setValue(CGUITextItem * value)
{
   if (_value != value)
   {
      _value = value;

      _valueChanged = true;
   }
}

void CGUIDataItemTextItem::setValue(const StringChar * value)
{
   if (value != NULL && _value != NULL)
   {
      _value->setText(value);

      _valueChanged = true;
   }
}

void CGUIDataItemTextItem::setValue(char * value)
{
   if (value != NULL && _value != NULL)
   {
      _value->setText(value);

      _valueChanged = true;
   }
}

//
// Void Constructor
//
CGUIDataItemText::CGUIDataItemText(void)
{
}

CGUIDataItemText::CGUIDataItemText(const char * value) :
	CGUIDataItem(true)
{
	_string = value;
}

CGUIDataItemText::~CGUIDataItemText()
{
}

const StringChar * CGUIDataItemText::convertToString()
{
   return _string.getString();
}

void CGUIDataItemText::setValue(const char * value)
{
	UnicodeString uValue = value;

   if( _string != uValue )
   {
      _string = value;
      _valueChanged = true;
   }
}

