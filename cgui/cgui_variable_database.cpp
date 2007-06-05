/*
 * Copyright (c) 2005 by Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: K:/BCT_Development/vxWorks/Common/cgui/rcs/cgui_variable_database.cpp 1.4 2007/06/04 22:04:21Z wms10235 Exp wms10235 $
 * $Log: cgui_variable_database.cpp $
 * Revision 1.3  2007/05/03 16:19:14Z  jl11312
 * - added semaphore protection for map structures
 * Revision 1.2  2005/09/30 22:40:54Z  rm10919
 * Get the variable database working!
 * Revision 1.1  2005/04/27 13:40:48Z  rm10919
 * Initial revision
 *
 */

#include <vxworks.h>
#include "cgui_variable_database.h"
#include "cgui_variable_db_container.h"
#include "cgui_text.h"
#include "cgui_data_item.h"


CGUIVariableDatabase::CGUIVariableDatabase(unsigned int linkLevel) : LinkElement()
{
	_lock = semMCreate(SEM_Q_PRIORITY | SEM_INVERSION_SAFE);
	do_link((LinkGroup *)&CGUIText::_variableDictionary, (LinkElement**)&CGUIText::_variableDictionary._variableDatabase, linkLevel, LT_Exclusive);
}

CGUIVariableDatabase::~CGUIVariableDatabase()
{
	_variableDictionary.clear();
}

const StringChar * CGUIVariableDatabase::variableLookUp(const char * name)
{
	const StringChar * result = NULL;
	map<string, CGUIDataItem *>::iterator iter;

	semTake(_lock, WAIT_FOREVER);
	iter = _variableDictionary.find(name);

	if (iter!=_variableDictionary.end())
	{
		CGUIDataItem * dataItem = iter->second;
		result = dataItem->convertToString();
	}

	semGive(_lock);

	return result;
}

void CGUIVariableDatabase::addDataItem(const char * name, CGUIDataItem * dataItem)
{
	semTake(_lock, WAIT_FOREVER);
	_variableDictionary[name] = dataItem;
	semGive(_lock);
}

void CGUIVariableDatabase::deleteDataItem(const char * name)
{
	semTake(_lock, WAIT_FOREVER);
	_variableDictionary.erase(name);
	semGive(_lock);
}


CGUIDataItem * CGUIVariableDatabase::getDataItem(const char * name)
{
	CGUIDataItem * result = NULL;
	map<string, CGUIDataItem *>::iterator iter;

	semTake(_lock, WAIT_FOREVER);
	iter = _variableDictionary.find(name);

	if (iter!=_variableDictionary.end())
	{
		result= iter->second;
	}

	semGive(_lock);
	return result;
}
