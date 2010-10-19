/*
 *	Copyright (c) 2005 by Gambro BCT, Inc.  All rights reserved.
 *
 * $Header$
 * $Log: cgui_text_item.cpp $
 * Revision 1.19  2007/06/04 22:04:21Z  wms10235
 * IT83 - Updates for the common GUI project to use the unicode string class
 * Revision 1.18  2007/05/03 04:21:20Z  cf10242
 * IT 3218: clean up string lengths in several places
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
#include "datalog_levels.h"

CGUIStringDataContainer CGUITextItem::_textMap;

int CGUITextItem::_defaultLanguageId = 0;

CGUITextItem::CGUITextItem(void) :
	_languageId(0),
	_hasVariables(false)
{
}

CGUITextItem::CGUITextItem(const char * id, StylingRecord * stylingRecord) :
	_languageId(0),
	_hasVariables(false)
{
   setId(id);

   if (stylingRecord)
   {
      _stylingRecord = *stylingRecord;
   }
}

CGUITextItem::CGUITextItem(const CGUITextItem& textItem) :
	_id(textItem._id),
   _string(textItem._string),
	_template(textItem._template),
   _languageId(textItem._languageId),
	_stylingRecord(textItem._stylingRecord),
	_hasVariables(textItem._hasVariables)
{
}

CGUITextItem& CGUITextItem::operator= (const CGUITextItem& textItem)
{
	if( this != &textItem ) // Protect against self assignment
	{
		_id = textItem._id;
		_string = textItem._string;
		_template = textItem._template;
		_languageId = textItem._languageId;
		_stylingRecord = textItem._stylingRecord;
		_hasVariables = textItem._hasVariables;
	}

   return *this;
}

CGUITextItem::~CGUITextItem()
{
}

void CGUITextItem::setText(const char * string, LanguageId languageId)
{
   if( string )
   {
      if( _id.size() > 0 )
      {
			_template = string;
			UnicodeString startTok( "#!{" );
			_string = _template;
			int startIndex = _string.find( startTok );
			if( startIndex >= 0 )
				_hasVariables = true;
		}
   }
}

void CGUITextItem::setText(const StringChar * string, LanguageId languageId)
{
   if( _id.size() > 0 )
   {
		_template = string;
		UnicodeString startTok( "#!{" );
		_string = _template;
		int startIndex = _string.find( startTok );
		if( startIndex >= 0 )
			_hasVariables = true;
   }
}

const StringChar * CGUITextItem::getText(LanguageId languageId)
{
	handleVariableSubstitution();
	return _string.getString();
}

const UnicodeString& CGUITextItem::getTextObj(LanguageId languageId)
{
	handleVariableSubstitution();
	return _string;
}

void CGUITextItem::getAscii(string &myString, LanguageId languageId)
{
	handleVariableSubstitution();
	myString = _string.getUTF8();
}

CGUITextItem * CGUITextItem::getTextItem(const char * id, LanguageId languageId)
{
   return _textMap.findString(id);
}

void CGUITextItem::setId(const char * id)
{
	_id.erase();

	if( id )
	{
		_id = id;
	}
}

const char * CGUITextItem::getId(void) const
{
	return _id.c_str();
}

int CGUITextItem::getLength(void) const
{
	return _string.getLength();
}

bool CGUITextItem::isInitialized(void) const
{
	return !_id.empty();
}

int CGUITextItem::getStringCharVariableLength(void)
{
   handleVariableSubstitution();
   return _string.getLength();
}

void CGUITextItem::handleVariableSubstitution(void)
{
	if( _hasVariables )
	{
		UnicodeString startTok( "#!{" );

		_string = _template;

		int startIndex = _string.find( startTok );

		if( startIndex >= 0 )
		{
			UnicodeString endTok( "}" );
			UnicodeString variableName;
			UnicodeString variableValue;
			string name;
			int endIndex;

			while( startIndex >= 0 )
			{
				endIndex = _string.find( endTok, startIndex );

				if( endIndex >= 0 )
				{
					variableName = _string.mid( startIndex, endIndex - startIndex + 1 );
					name = variableName.mid(3,endIndex - startIndex - 3).getUTF8();
					variableValue = CGUIText::_variableDictionary.variableLookUp( name.c_str() );

					if( variableValue.getLength() > 0 )
					{
						if( _string.replace( variableName, variableValue ) < 0 )
						{
							DataLog( log_level_cgui_info ) << "Variable replacement failed for " << getId() << endmsg;
						}
					}
					else
					{
						DataLog( log_level_cgui_info ) << "Variable lookup returned NULL for " << name.c_str() << endmsg;
					}
				}

				startIndex = _string.find( startTok, startIndex + 1 );
			}
		}
	}
}

