/*
 *	Copyright (c) 2005 by Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: K:/BCT_Development/vxWorks/Common/cgui/rcs/cgui_variable_db_container.cpp 1.4 2007/06/04 22:04:22Z wms10235 Exp wms10235 $
 * $Log: cgui_variable_db_container.cpp $
 *
 */

#include <vxworks.h>
#include "cgui_variable_db_container.h"
#include "cgui_variable_database.h"


CGUIVariableDatabaseContainer::CGUIVariableDatabaseContainer()
{

}

CGUIVariableDatabaseContainer::~CGUIVariableDatabaseContainer()
{

}

StringChar * CGUIVariableDatabaseContainer::variableLookUp(const char * name)
{
   CGUIVariableDatabase * variableDatabase;
   StringChar * result = NULL;

   variableDatabase = _variableDatabase;
   
   while(variableDatabase  && !result)
   {
      result = variableDatabase->variableLookUp(name);
      variableDatabase = (CGUIVariableDatabase *)variableDatabase->child();      
   }

   // if can't find variable return name.
   return result;
}

