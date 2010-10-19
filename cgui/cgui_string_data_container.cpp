/*
 *	Copyright (c) 2005 by Gambro BCT, Inc.  All rights reserved.
 *
 * $Header$
 * $Log: cgui_string_data_container.cpp $
 * Revision 1.1  2005/04/27 13:40:47Z  rm10919
 * Initial revision
 *
 */

#include <vxworks.h>
#include <error.h>
#include "cgui_text_item.h"
#include "cgui_string_data.h"
#include "cgui_string_data_container.h"
#include "datalog_levels.h"

CGUIStringDataContainer::CGUIStringDataContainer() :
	_textTable(NULL)
{
}

CGUIStringDataContainer::~CGUIStringDataContainer()
{
}

CGUITextItem * CGUIStringDataContainer::findString(const char * name)
{
   CGUIStringData * textTable;
   CGUITextItem * result = NULL;

	if( name == NULL ) _FATAL_ERROR(__FILE__, __LINE__, "Name parameter is NULL.");

   textTable = _textTable;

   while(textTable  && !result)
   {
      result = textTable->findString(name);
      textTable = (CGUIStringData *)textTable->child();
   }

	if( result == NULL )
	{
		DataLog( log_level_cgui_error ) << "String item could not be found: '" << name << "'" << endmsg;
	}

   // if can't find variable return NULL.
   return result;
}

