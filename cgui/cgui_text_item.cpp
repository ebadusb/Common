/*
 *	Copyright (c) 2005 by Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: K:/BCT_Development/vxWorks/Common/cgui/rcs/cgui_text_item.cpp 1.19 2007/06/04 22:04:21Z wms10235 Exp adalusb $
 * $Log: cgui_text_item.cpp $
 * Revision 1.12  2006/07/12 23:36:08Z  rm10919
 * Updates from adding cguiListBox class.
 * Revision 1.11  2006/05/31 19:51:09Z  rm10919
 * Fix _stringLength in setChar(StringChar), and the getAscii method.
 * Revision 1.10  2006/05/15 21:54:41Z  rm10919
 * Fix bug to handle latin char (accented vowels) as unsigned char to unsigned wide char correctly.
 * Revision 1.9  2005/11/22 00:34:42Z  rm10919
 * Get data item database to work with software layers.
 * Revision 1.8  2005/09/30 22:40:53Z  rm10919
 * Get the variable database working!
 * Revision 1.7  2005/08/13 20:55:17Z  cf10242
 * TAOS IT 842 - string handling
 * Revision 1.6  2005/08/02 14:42:32Z  cf10242
 * Revision 1.5  2005/08/02 00:05:45Z  cf10242
 * fix text of existing string
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
:_id(NULL), _string(NULL), _stringSize(0), _stringLength(0)
{
}

CGUITextItem::CGUITextItem(const char * id, StylingRecord * stylingRecord)
:_id(id), _string(NULL), _stringSize(0), _stringLength(0)
{
   if (stylingRecord)
   {
      _stylingRecord = *stylingRecord;
   }
   int newLength = 0;

   if (_string)
   {
      while (_string[newLength])
         newLength += 1;

      _stringLength = newLength;
   }
}

CGUITextItem::CGUITextItem(const CGUITextItem& textItem)
{
   (*this) = textItem;
}

CGUITextItem CGUITextItem::operator= (const CGUITextItem& textItem)
{
   _defaultLanguageId = textItem._defaultLanguageId;
   _languageId = textItem._languageId;
   _stringLength = textItem._stringLength;
   _stringSize = textItem._stringSize;
   _stylingRecord = textItem._stylingRecord;

   // must allocate memory
   char * textId = new char[strlen(textItem._id)+1];

   // check to see if initialized to NULL
   if (_string == NULL)
      _string = new StringChar[_stringLength];
   
   strcpy(textId, textItem._id);
   _id = textId;

   memcpy(_string, textItem._string, _stringLength * sizeof(StringChar));
   return (*this);
}

CGUITextItem::~ CGUITextItem()
{
   if(_string) 
	{
		delete[] _string;
		_string = NULL;
	}

   if (_id)
   {
      delete [] _id;
      _id = NULL;
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
            _string[stringLength] = (unsigned char)(UGL_WCHAR)string[stringLength];
            stringLength++;
         }
         _stringLength = stringLength;
         _string[_stringLength] = (StringChar)0;
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

      if(!_string)
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
      
      memcpy(_string, string, stringLength * sizeof(StringChar));
      _stringLength = stringLength + textBlockSize;
   }
}

const StringChar * CGUITextItem::getText(LanguageId languageId = currentLanguage)
{
	return _string;
}

void CGUITextItem::getAscii( char * myString, LanguageId languageId = currentLanguage)
{
   // anyone using this function needs to confirm that myString can hold the full
   // string length of the text item
   if (_string)
   {
      int i = 0;
      while (i < _stringLength  && _string[i] <= 127 && _string[i] != '\0')
      {
         myString[i] = _string[i];
         i++;
      }
      // terminate myString
      myString[i] = '\0';
	}
}

CGUITextItem * CGUITextItem::getTextItem(const char * id, LanguageId languageId = currentLanguage)
{
//   CGUITextItem textItem;
   return _textMap.findString(id);
}

void CGUITextItem::setId(const char * id)
{
	_id = id;
}

bool CGUITextItem::isInitialized(void)
{
   return (_id != NULL);
}
