/*
 *	Copyright (c) 2005 by Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: K:/BCT_Development/vxWorks/Common/cgui/rcs/cgui_text_item.cpp 1.19 2007/06/04 22:04:21Z wms10235 Exp adalusb $
 * $Log: cgui_text_item.cpp $
 *
 */

#include <vxWorks.h>
#include "cgui_text_item.h"

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
   delete _id;
   delete _string;
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

char * CGUITextItem::getAscii(LanguageId languageId = currentLanguage)
{
   char * string;
   int stringLength = 0;

   if (_string)
   {
      while (_string[stringLength])
      {
         stringLength ++;
      }
      
      stringLength++;
      
      string =  new char[stringLength];

      for (int i=0; i<=_stringLength; i++)
      {
         if (_string[i] > 0xff)
         {
            //  have an invalid character!!
            return NULL;
         }
         string[i] = _string[i];
      }

      return string;
   }
   else
   {
      return NULL;
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
