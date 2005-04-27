/*
 *	Copyright (c) 2005 by Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: K:/BCT_Development/vxWorks/Common/cgui/rcs/cgui_variable_database.cpp 1.4 2007/06/04 22:04:21Z wms10235 Exp wms10235 $
 * $Log: cgui_variable_database.cpp $
 *
 */

#include <vxworks.h>
#include "cgui_variable_database.h"
#include "cgui_variable_db_container.h"
#include "cgui_text.h"


CGUIVariableDatabase::CGUIVariableDatabase(unsigned int linkLevel) : LinkElement()
{
   do_link((LinkGroup *)&CGUIText::_variableDictionary, (LinkElement**)&CGUIText::_variableDictionary._variableDatabase, linkLevel, LT_Exclusive);
}

CGUIVariableDatabase::~CGUIVariableDatabase()
{
   _variableDictionary.clear();
}

StringChar * CGUIVariableDatabase::variableLookUp(const char * name)
{
   StringChar * result = NULL;
   
   map<string, CGUIDataItem *>::iterator iter;
   
   //
   // Do the lookup thing for dataItem.
   //
   iter = _variableDictionary.find(name);
   result = iter->second->convertToString();
   // if can't find name ruturn NULL.
   return result;

}

void CGUIVariableDatabase::addDataItem(const char * name, CGUIDataItem * dataItem)
{
   _variableDictionary[name] = dataItem;

}

void CGUIVariableDatabase::deleteDataItem(const char * name)
{

   _variableDictionary.erase(name);
}


