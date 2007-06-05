/*
 *	Copyright (c) 2005 by Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: K:/BCT_Development/vxWorks/Common/cgui/rcs/cgui_variable_db_container.cpp 1.4 2007/06/04 22:04:22Z wms10235 Exp wms10235 $
 * $Log: cgui_variable_db_container.cpp $
 * Revision 1.3  2005/11/22 00:34:43Z  rm10919
 * Get data item database to work with software layers.
 * Revision 1.2  2005/09/30 22:40:55Z  rm10919
 * Get the variable database working!
 * Revision 1.1  2005/04/27 13:40:50Z  rm10919
 * Initial revision
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

const StringChar * CGUIVariableDatabaseContainer::variableLookUp(const char * name)
{
   CGUIVariableDatabase * variableDatabase;
   const StringChar * result = NULL;

   variableDatabase = _variableDatabase;

   while(variableDatabase  && !result)
   {
      result = variableDatabase->variableLookUp(name);
      variableDatabase = (CGUIVariableDatabase *)variableDatabase->child();
   }

   // if can't find variable return name.
   return result;
}

CGUIDataItem * CGUIVariableDatabaseContainer::getDataItem(const char * name)
{
   CGUIVariableDatabase * variableDatabase;
   CGUIDataItem * result = NULL;

   variableDatabase = _variableDatabase;

   while(variableDatabase  && !result)
   {
      result = variableDatabase->getDataItem(name);
      variableDatabase = (CGUIVariableDatabase *)variableDatabase->child();
   }

   // if can't find variable return name.
   return result;
}

