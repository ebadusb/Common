/*
 *	Copyright (c) 2005 by Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: K:/BCT_Development/vxWorks/Common/cgui/rcs/cgui_variable_database.cpp 1.4 2007/06/04 22:04:21Z wms10235 Exp wms10235 $
 * $Log: cgui_variable_database.cpp $
 * Revision 1.1  2005/04/27 13:40:48Z  rm10919
 * Initial revision
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
   
   if (iter!=_variableDictionary.end())
   {
      CGUIDataItem * dataItem = iter->second;

      result= dataItem->convertToString();
   }
   
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


CGUIDataItem * CGUIVariableDatabase::getDataItem(const char * name)
{
   CGUIDataItem * result = NULL;
   
   map<string, CGUIDataItem *>::iterator iter;
   
   //
   // Do the lookup thing for dataItem.
   //
   iter = _variableDictionary.find(name);
   
   if (iter!=_variableDictionary.end())
   {
      result= iter->second;
   }
   
   // if can't find name ruturn NULL.
   return result;
}

