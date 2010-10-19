/*
 * Copyright (c) 2004 by Gambro BCT, Inc.  All rights reserved.
 *
 * $Header$
 * $Log: cgui_string_data.cpp $
 * Revision 1.12  2007/06/14 19:34:11Z  wms10235
 * Taos IT3439 - Discovered a bug where variable substitution was not functioning
 * Revision 1.11  2007/05/03 16:19:13Z  jl11312
 * - added semaphore protection for map structures
 * Revision 1.10  2007/04/30 18:26:07Z  jl11312
 * - additional error checking when reading string info files (Taos IT 3102)
 * Revision 1.9  2007/02/08 19:28:05Z  rm10919
 * Updates to add languages to string data.
 * Revision 1.8  2006/12/01 19:20:07Z  pn02526
 * Use CGUIStringInfo class to read string.info files.
 * Revision 1.7  2006/07/25 15:42:37  cf10242
 * IT 54: decode UTF8 into unicode
 * Revision 1.6  2006/07/12 23:37:57Z  rm10919
 * Update for reading in unicode strings.
 * Revision 1.5  2006/05/15 21:52:17Z  rm10919
 * Add debug lines.
 * Revision 1.4  2005/10/19 15:42:22Z  rm10919
 * Fix file read to not skip over text if only one space.
 * Revision 1.3  2005/06/09 15:00:12Z  rm10919
 * Fix the read data file routine.
 * Revision 1.2  2005/04/28 14:40:32Z  rm10919
 * Fixed bug in reading eol characters.
 * Revision 1.1  2005/04/27 13:40:45Z  rm10919
 * Initial revision
 *
 */

#include <vxWorks.h>
#include "cgui_string_data.h"
#include "cgui_string_data_container.h"
#include "cgui_string_info.h"
#include "datalog_levels.h"


CGUIStringData::CGUIStringData(unsigned int linkLevel): LinkElement()
{
	_lock = semMCreate(SEM_Q_PRIORITY | SEM_INVERSION_SAFE);
	do_link((LinkGroup *)&CGUITextItem::_textMap, (LinkElement**)&CGUITextItem::_textMap._textTable, linkLevel, LT_Exclusive);
}

CGUIStringData::~CGUIStringData(void)
{
}

bool CGUIStringData::readDatabaseFile (const char * filename, CGUIFontId * fontId, LanguageId languageId = currentLanguage, int fontIndex = 0)
{
	CGUITextItem textItem;
	CGUIStringInfo stringInfo( filename );
	bool  result = true;

	DataLog( log_level_cgui_info ) << "Entering CGUIStringData::readDatabaseFile for " << filename << endmsg;

	// Loop reading string info records, converting them to CGUTextItems,
	// and putting them in their proper places in the text map.
	while ( stringInfo.get(fontId, textItem, NULL, fontIndex) )
	{
		// Define a pointer for a map entry's CGUITextItem.
		CGUITextItem * pTextItem;

		// Define an iterator for the text map.
		map<string, CGUITextItem *>::iterator iter;

		// Do the lookup thing for the textItem.
		//
		semTake(_lock, WAIT_FOREVER);

		iter = _textMap.find(textItem.getId());

		// If found the corresponding text item in map,
		// correct the formatting sequences in the text.
		if (iter != _textMap.end())
		{
			pTextItem = iter->second;

			if( pTextItem )
			{
				// Update the text item information in corresponding _textMap[id].
				//
				*pTextItem = textItem;
				pTextItem->setLanguageId(languageId);
			}
			else
			{
				DataLog( log_level_cgui_error ) << "line " << stringInfo.line() << ": Text item is NULL for string ID " << textItem.getId() << " in map!!!!! - " << filename << endmsg;
				result = false;
			}
		}
		else
		{
			DataLog( log_level_cgui_error ) << "line " << stringInfo.line() << ": Can't find string Id " << textItem.getId() << " in map!!!!! - " << filename << endmsg;
			result = false;
		}

		semGive(_lock);
	}

	// Check for premature end.
	if( !stringInfo.endOfFile() )
	{
		DataLog( log_level_cgui_error ) << "line " << stringInfo.line() << ": bad entry in string info file - " << filename << endmsg;
		result = false;
	}

	// Close file.
	DataLog( log_level_cgui_info ) << "line " << stringInfo.line() << ": closing string info file - " << filename << endmsg;
	stringInfo.close();

	return result;
}

bool CGUIStringData::readDatabaseItem (CGUITextItem * LanguageName) //, Language language[0]);
{
	return true;
}

CGUITextItem * CGUIStringData::findString(const char * id)
{
	CGUITextItem * result = NULL;
	map<string, CGUITextItem *>::iterator iter;

	semTake(_lock, WAIT_FOREVER);
	iter = _textMap.find(id);

	if (iter != _textMap.end())
	{
		result = iter->second;

		if (strcmp(result->getId(), id) != 0)
		{
			result = NULL;
		}

	}

	semGive(_lock);
	return result;
}

void CGUIStringData::addTextItem(const char * name, CGUITextItem * textItem)
{
	semTake(_lock, WAIT_FOREVER);
	_textMap[name] = textItem;
	semGive(_lock);
}

void CGUIStringData::deleteTextItem(const char * name)
{
	semTake(_lock, WAIT_FOREVER);
	_textMap.erase(name);
	semGive(_lock);
}
