/*
 *	Copyright (c) 2005 by Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: J:/BCT_Development/vxWorks/Common/cgui/rcs/cgui_data_item.cpp 1.9 2007/06/04 22:04:20Z wms10235 Exp rm10919 $
 * $Log: cgui_data_item.cpp $
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


const size_t textBlockSize = 16;  // minimum StringChar string allocation in bytes.

const StringChar null_char = '\0';
const StringChar zero_char = '0';
const StringChar space_char = ' ';
const StringChar space_string[2] = { space_char , null_char }; 
const StringChar newline_string[2] = { '\n', null_char }; 

// Return the length of a StringChar string.
size_t CGUIDataItemDouble::stringCharLen( const StringChar * s )
{
    size_t l=0;
    if( s != NULL ) while( *s++!=null_char ) l++;
    return l;
} 

// Compute allocations needed for StringChar strings.
size_t CGUIDataItemDouble::stringCharAllocation(const size_t len) { return ((len+textBlockSize)/textBlockSize)*textBlockSize; };
size_t CGUIDataItemDouble::stringCharAllocation(const StringChar * s) { return (s == NULL) ? 0 : stringCharAllocation(stringCharLen(s)); };

// Form a StringChar string from a const char string.
StringChar * CGUIDataItemDouble::stringChar( const char * string )
{
   StringChar * s =  NULL;
   if (string != NULL)
   {
      int stringLength = strlen(string);
      s = new StringChar[stringCharAllocation(stringLength)];  
      for (int i=0; i<=stringLength; i++) s[i] = (unsigned char)(StringChar)string[i];
   }
   return s;
}

// Copy a StringChar string into another, reallocating the destination if necessary.
void CGUIDataItemDouble::stringCharCpy( StringChar ** ps1, const StringChar * s2 )
{
    if( ps1 != NULL )  // Do nothing if we have no pointer to an s1.
    {
        StringChar * s1 = *ps1;
        size_t s1size = stringCharAllocation( s1 );
        size_t s2size = stringCharAllocation( s2 );
        if( s1 != NULL && s1size == s2size )
        {
            // Don't need to (re)allocate, just copy the string.
            if( s2 != NULL ) while( *s2 != null_char && --s1size > 0) { *s1++ = *s2++; }
            *s1 = null_char;
        }
        else
        {
            //(Re)allocate *ps1.  Degenerative case is s1==NULL;
            if( s1 != NULL ) delete[] s1;
            *ps1 = new StringChar[s2size];
            stringCharCpy( ps1, s2 );
        }
    }
}

// Destructively Cat s2 to the end of *ps1, reallocating *ps1 if necessary.
void CGUIDataItemDouble::stringCharCat( StringChar ** ps1, const StringChar * s2 )
{
    if( ps1 != NULL )  // Do nothing if we have no pointer to an s1.
    {
        StringChar * s1 = *ps1;
        if( s1 != NULL )
        {
            size_t s1len = stringCharLen(s1);
            size_t s1size = stringCharAllocation( s1len );
            size_t newSize = stringCharAllocation( s1len + stringCharLen(s2));
            if( newSize != s1size )
            {
                // Reallocate to fit cat'ed string
                StringChar * sNew = new StringChar[newSize]; // This does the new allocation
                stringCharCpy( &sNew, s1 );  // Copy s1 into the new allocation.
                delete[] s1;  // Get rid of old allocation.
                s1 = sNew;    // Point to new.
                *ps1 = sNew;  //   "    "  "
            }
            s1 += s1len;
            stringCharCpy( &s1, s2);
        }
        else
            // Degenerate to a newly allocated string copy.
            stringCharCpy( ps1, s2 );
    }
}

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
CGUIDataItemDouble::CGUIDataItemDouble() : _separator(NULL)
{
}

CGUIDataItemDouble::CGUIDataItemDouble(double value, int precision): CGUIDataItem(true),
_value(value), _precision(precision), _separator(NULL)
{

}

CGUIDataItemDouble::CGUIDataItemDouble(double value, int precision, CGUITextItem * separator): CGUIDataItem(true),
_value(value), _precision(precision), _separator(separator)
{

}

CGUIDataItemDouble::~CGUIDataItemDouble()
{
}

StringChar * CGUIDataItemDouble::convertToString(void)
{
   if (_valueChanged || _string)
   {

      ostringstream textStream;

      textStream.setf(ios::fixed);
      if (_precision < 0)
         textStream.precision(0);
      else
         textStream.precision(_precision);

      textStream << _value;
      const char * string = textStream.str().c_str();
      
      char * cString = new char[strlen(string)+1];
      strcpy( cString, string );


      //
      // Clear _string
      //
      if (_string) delete _string;

      //
      // Copy value (cString) into _string
      //
      convertToStringChar(cString, &_string);
      delete cString;

      if( _separator != NULL )
      {
        // replace 1st period with the given separator.
        size_t pos=0;
        // Find first non-numeric character or end of string.
        while (_string[pos] != '\0' && _string[pos] <= '9' && _string[pos] >= '0' ) pos++;
        // Is it the "normal" decimal point (period)?
        if( _string[pos] == '.' )
        {
             // Yes, replace it.
             StringChar * right=NULL;
             stringCharCpy( &right, &_string[pos+1] ); // save characters to right of decimal point.
             _string[pos] = '\0';                      // split the string.
             stringCharCat( &_string, _separator->getText() );  // insert the given separator.
             stringCharCat( &_string, right );          // restore rightmost characters.
             delete right;
        }
      }

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
   if (_valueChanged && _value != NULL)
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

void CGUIDataItemTextItem::setValue(StringChar * value)
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

