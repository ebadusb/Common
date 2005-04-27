/*
 *	Copyright (c) 2005 by Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: K:/BCT_Development/vxWorks/Common/cgui/rcs/cgui_text_item.cpp 1.19 2007/06/04 22:04:21Z wms10235 Exp adalusb $
 * $Log: cgui_text_item.cpp $
 * Revision 1.2  2005/02/21 17:17:12Z  cf10242
 * IT 133 - delete all allocated memory to avoid unrecovered memory
 * Revision 1.1  2005/01/31 17:36:38Z  rm10919
 * Initial revision
 *
 */

#include <vxWorks.h>
#include "cgui_text_item.h"
#include "cgui_string_data_container.h"

CGUIStringDataContainer CGUITextItem::_textMap;

int CGUITextItem::_defaultLanguageId = 0;

CGUITextItem::CGUITextItem()
:_id(NULL), _string(NULL)
{
}

CGUITextItem::CGUITextItem(const char * id, StylingRecord * stylingRecord)
:_id(id), _string(NULL)
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
         _string = new StringChar[strlen(string) + 1];
         
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
      
      _stringLength = ++stringLength;

      _string = new StringChar[stringLength+1];
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
