/*
 *	Copyright (c) 2005 by Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: K:/BCT_Development/vxWorks/Common/cgui/rcs/cgui_text_item.cpp 1.19 2007/06/04 22:04:21Z wms10235 Exp adalusb $
 * $Log: cgui_text_item.cpp $
 * Revision 1.4  2005/08/01 23:31:39Z  cf10242
 * Revision 1.3  2005/04/26 23:16:48Z  rm10919
 * Made changes to cgui_text and cgui_text_item, plus added 
 * classes for variable substitution in text strings.
 * Revision 1.2  2005/02/21 17:17:12Z  cf10242
 * IT 133 - delete all allocated memory to avoid unrecovered memory
 * Revision 1.1  2005/01/31 17:36:38Z  rm10919
 * Initial revision
 *
 */

#include <vxWorks.h>
#include "cgui_text_item.h"
#include "cgui_string_data_container.h"

const int textBlockSize = 64;

CGUIStringDataContainer CGUITextItem::_textMap;

int CGUITextItem::_defaultLanguageId = 0;

CGUITextItem::CGUITextItem()
:_id(NULL), _string(NULL)
{
}

CGUITextItem::CGUITextItem(const char * id, StylingRecord * stylingRecord)
:_id(id), _string(NULL), _stringSize(0), _stringLength(0)
{
   if (stylingRecord)
   {
      _stylingRecord = *stylingRecord;
   }
}

CGUITextItem::~ CGUITextItem()
{
   if(_string) 
	{
		delete[] _string;
		_string = NULL;
	}
}


void CGUITextItem::setText(const char * string, LanguageId = currentLanguage)
{
   if (string)
   {
      if (_id)
      {
         int newLength = strlen(string);
         if(!_string)
         {         
            _stringSize = newLength;
            if(newLength < textBlockSize)
               _stringSize = textBlockSize;
            _string = new StringChar[_stringSize + 1];
         }
         else
         {
            if(newLength > _stringSize)
            {
           		delete[] _string;
               _string = new StringChar[newLength + textBlockSize + 1];
               _stringSize = newLength + textBlockSize;
            }
         }
         
         int stringLength = 0;

         while (string[stringLength] != '\0')
         {
            _string[stringLength] = string[stringLength];
            stringLength++;
         }
         _stringLength = stringLength;
      }
   }
}

void CGUITextItem::setText(const StringChar * string, LanguageId = currentLanguage)
{
   int stringLength = 0;

   if (_id)
   {
      while (string[stringLength])
      {
         stringLength += 1;
      }

      if(!string)
      {         
         _stringSize = stringLength;
         if(stringLength < textBlockSize)
            _stringSize = textBlockSize;
         _string = new StringChar[_stringSize + 1];
      }
      else
      {
         if(stringLength > _stringSize)
         {
        		delete[] _string;
            _string = new StringChar[stringLength + textBlockSize + 1];
            _stringSize = stringLength + textBlockSize;
         }
      }
      
      _stringLength = ++stringLength;

      memcpy(_string, string, stringLength * sizeof(StringChar));
   }
}

const StringChar * CGUITextItem::getText(LanguageId languageId = currentLanguage)
{
	return _string;
}

void CGUITextItem::getAscii( char * myString, LanguageId languageId = currentLanguage)
{
   int stringLength = 0;

	myString[0] = '\0';

   if (_string)
   {
      while (_string[stringLength])
      {
         stringLength ++;
      }
      
      stringLength++;
      
      for (int i=0; i<=_stringLength; i++)
      {
         if (_string[i] > 0xff)
         {
            //  have an invalid character!! force an exit to the loop with no joy
            myString[0] = '\0';
				i = _stringLength;
         }
			else
				myString[i] = _string[i];
      }
	}
}

void CGUITextItem::setId(const char * id)
{
	_id = id;
}

bool CGUITextItem::isInitialized(void)
{
   return (_id != NULL);
}
