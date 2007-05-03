/*
 *	Copyright (c) 2005 by Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: K:/BCT_Development/vxWorks/Common/cgui/rcs/cgui_text_item.cpp 1.19 2007/06/04 22:04:21Z wms10235 Exp adalusb $
 * $Log: cgui_text_item.cpp $
 * Revision 1.17  2007/04/26 16:47:20Z  wms10235
 * IT2354 - Updated the assignment operator and copy contructor
 * Revision 1.16  2007/04/23 23:54:37Z  wms10235
 * IT2354 - Found text item bug while debugging reports
 * Revision 1.15  2007/02/21 21:06:52Z  rm10919
 * Add methods for variable substitution and stringLength.  Fix copy constructor and equals operator.
 * Revision 1.14  2006/12/01 17:00:42Z  pn02526
 * Change Id allocation so that destructor does not crash GUI.
 * Null-teminate string buffer in setText(StringChar ...).
 * Remove extraneous code from constructor.
 * Revision 1.13  2006/08/29 13:14:16  rm10919
 * Check for null strings in copy.
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
#include "cgui_text.h"
#include "cgui_string_data_container.h"

const int textBlockSize = 64;
const int idSize = 256;  // Limit text id to 255 characters + a null byte;

CGUIStringDataContainer CGUITextItem::_textMap;

int CGUITextItem::_defaultLanguageId = 0;

CGUITextItem::CGUITextItem()
:_id(NULL), _string(NULL), _stringSize(0), _stringLength(0), _languageId(0)
{
}

CGUITextItem::CGUITextItem(const char * id, StylingRecord * stylingRecord)
:_id(NULL), _string(NULL), _stringSize(0), _stringLength(0), _languageId(0)
{
   setId(id);

   if (stylingRecord)
   {
      _stylingRecord = *stylingRecord;
   }
}

CGUITextItem::CGUITextItem(const CGUITextItem& textItem) :
   _stringLength(textItem._stringLength),
	_languageId(textItem._languageId),
	_stylingRecord(textItem._stylingRecord)
{
	_id = NULL;

	setId(textItem._id);

	if( textItem._string )
	{
		_stringSize = _stringLength;
		if( _stringSize < textBlockSize )
			_stringSize = textBlockSize;

      _string = new StringChar[_stringSize + 1];
		memcpy( _string, textItem._string, (_stringLength + 1) * sizeof(StringChar) );
	}
	else
	{
		_string = NULL;
		_stringSize = 0;
		_stringLength = 0;
	}
}

CGUITextItem& CGUITextItem::operator= (const CGUITextItem& textItem)
{
	if( this != &textItem ) // Protect against self assignment
	{
		if (_id != textItem._id)
		{
			_languageId = textItem._languageId;
			_stringLength = textItem._stringLength;
			_stylingRecord = textItem._stylingRecord;

			setId(textItem._id);

			// check to see if initialized to NULL
			if( textItem._string && _stringLength > 0 )
			{
				// Check the current size against what is needed
				if( _stringSize < _stringLength )
				{
					if( _string )
						delete [] _string;
					_stringSize = _stringLength;
					if( _stringSize < textBlockSize )
						_stringSize = textBlockSize;

					_string = new StringChar[_stringSize + 1];
				}

				memcpy( _string, textItem._string, _stringLength * sizeof(StringChar) );
				_string[_stringLength] = 0;
			}
			else
			{
				if( _string )
					delete [] _string;
				_string = NULL;
				_stringSize = 0;
				_stringLength = 0;
			}
		}
	}

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
      _string[stringLength] = (StringChar)0;

      _stringLength = stringLength;
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
   if( id == NULL )  // test for NULL input.
   {
        if( _id != NULL ) delete [] _id;
        _id = NULL;
   }
   else
   {
       if( _id == NULL ) _id = new char[idSize]; // must allocate memory

       char *textId = (char *)_id; // purposely cast away the const.

       int idLen = idSize-1;
        // copy and truncate to max length + 1 null.
       strncpy(textId, id, idLen);
       textId[idLen] = '\0';  // make sure string is terminated.
   }
}

bool CGUITextItem::isInitialized(void)
{
   return (_id != NULL);
}

int CGUITextItem::getStringCharLength(void)
{
   int stringLength = 0;

   while( _string != NULL && _string[stringLength] != '\0')
   {
//      _string[stringLength] = (unsigned char)(UGL_WCHAR)string[stringLength];
      stringLength++;
   }

   return stringLength;
}

int CGUITextItem::getStringCharVariableLength(void)
{
   int stringLength = 0;

   handleVariableSubstitution();

   while( _string != NULL && _string[stringLength] != '\0')
   {
      stringLength++;
   }

   return stringLength;
}

void CGUITextItem::handleVariableSubstitution()
{
   StringChar null_char = '\0';
   bool changedText = false;
   size_t newStringSize = _stringLength+1;
   StringChar * newTextString = (StringChar *)malloc(newStringSize * sizeof(StringChar));
   size_t newStringLength = 0;
   size_t idx = 0;

   while( idx < _stringLength && _string[idx] != null_char )
   {
      // Check for start of variable substitution string
      //  check for valid length before accessing next elements of the array
      if (_string[idx] == (wchar_t)'#' &&  idx+2 < _stringLength)
	  {
		 if( _string[idx+1] == (wchar_t)'!' && _string[idx+2] == (wchar_t)'{')
		 {
			 // Find ending '}' character if any
			 //
			 size_t subStartIdx = idx+3;
			 size_t subEndIdx = subStartIdx;
	
			 while (_string[subEndIdx] != null_char &&
					_string[subEndIdx] != (wchar_t)'}' &&
					subEndIdx < _stringLength)
			 {
				subEndIdx += 1;
			 }
	
			 if (_string[subEndIdx] == '}' &&
				 subEndIdx-subStartIdx > 0)
			 {
				// Have a valid variable substitution string - lookup the value
				//
				changedText = true;
	
				char  * variableName = new char[subEndIdx-subStartIdx+1];
	
				for (int i=0; i<subEndIdx-subStartIdx; i++)
				{
				   variableName[i] = (char)(_string[subStartIdx+i] & 0x00ff);
	
				}
	
				variableName[subEndIdx-subStartIdx] = '\0';
				StringChar * variableText = CGUIText::_variableDictionary.variableLookUp(variableName);
				delete[] variableName;
	
				int variableTextLength = 0;
				if (variableText)
				{
				   while (variableText[variableTextLength])
					  variableTextLength += 1;
	
				   // Value is present, copy to the string and setup to continue with
				   // next character after substitution string
				   //
				   //newStringSize += (variableTextLength + textBlockSize);
				   newStringSize += variableTextLength;
				   newTextString = (StringChar *)realloc(newTextString, (newStringSize * sizeof(StringChar) ));
	
				   for (int i=0; i<variableTextLength; i++)
				   {
					  newTextString[newStringLength++] = (StringChar)variableText[i];
				   }
	
				   idx = subEndIdx + 1;
				   continue;
				}
			 }
		  }
	  }
      newTextString[newStringLength++] = _string[idx++];
   }

   if (changedText)
   {
      if (_string) delete[] _string;

      _stringSize = newStringLength + textBlockSize;
      _string = new StringChar[_stringSize+1];

      memcpy(_string, newTextString, newStringLength * sizeof(StringChar));
      _string[newStringLength] = null_char;
      _stringLength = newStringLength;
   }

   free(newTextString);
   newTextString = NULL;
}

