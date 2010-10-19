/*
 *	Copyright (c) 2005 by Gambro BCT, Inc.  All rights reserved.
 *
 * $Header$
 * $Log: cgui_variable_db_container.cpp $
 * Revision 1.4  2007/06/04 22:04:22Z  wms10235
 * IT83 - Updates for the common GUI project to use the unicode string class
 * Revision 1.3  2005/11/22 00:34:43Z  rm10919
 * Get data item database to work with software layers.
 * Revision 1.2  2005/09/30 22:40:55Z  rm10919
 * Get the variable database working!
 * Revision 1.1  2005/04/27 13:40:50Z  rm10919
 * Initial revision
 *
 */

#include <vxWorks.h>
#include <error.h>
#include "cgui_variable_db_container.h"
#include "cgui_variable_database.h"
#include "datalog_levels.h"

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

	if( name == NULL ) _FATAL_ERROR(__FILE__, __LINE__, "Name parameter is NULL.");

   variableDatabase = _variableDatabase;

   while(variableDatabase  && !result)
   {
      result = variableDatabase->variableLookUp(name);
      variableDatabase = (CGUIVariableDatabase *)variableDatabase->child();
   }

	if( result == NULL )
	{
		DataLog( log_level_cgui_error ) << "Data item could not be found: '" << name << "'" << endmsg;
	}

   // if can't find variable return name.
   return result;
}

CGUIDataItem * CGUIVariableDatabaseContainer::getDataItem(const char * name)
{
   CGUIVariableDatabase * variableDatabase;
   CGUIDataItem * result = NULL;

	if( name == NULL ) _FATAL_ERROR(__FILE__, __LINE__, "Name parameter is NULL.");

   variableDatabase = _variableDatabase;

   while(variableDatabase  && !result)
   {
      result = variableDatabase->getDataItem(name);
      variableDatabase = (CGUIVariableDatabase *)variableDatabase->child();
   }

	if( result == NULL )
	{
		DataLog( log_level_cgui_error ) << "Data item could not be found: '" << name << "'" << endmsg;
	}

   // if can't find variable return name.
   return result;
}

