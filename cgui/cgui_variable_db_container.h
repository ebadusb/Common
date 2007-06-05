/*
 *	Copyright (c) 2005 by Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: H:/BCT_Development/vxWorks/Common/cgui/rcs/cgui_variable_db_container.h 1.2 2005/11/22 00:34:43Z rm10919 Exp wms10235 $
 * $Log: cgui_variable_db_container.h $
 * Revision 1.2  2005/11/22 00:34:43Z  rm10919
 * Get data item database to work with software layers.
 * Revision 1.1  2005/04/27 13:40:43Z  rm10919
 * Initial revision
 *
 */

#ifndef _VARIABLE_DB_CONTAINER_INCLUDE
#define _VARIABLE_DB_CONTAINER_INCLUDE

#include "cgui_graphics.h"
#include "link_group.h"

class CGUIVariableDatabase;
class CGUIDataItem;

class CGUIVariableDatabaseContainer: public LinkGroup
{
public:

   CGUIVariableDatabaseContainer(void);
   ~CGUIVariableDatabaseContainer(void);

   const StringChar * variableLookUp(const char * name);

   CGUIDataItem * getDataItem(const char * name);

   CGUIVariableDatabase * _variableDatabase;
};

#endif /* ifndef _VARIABLE_DB_CONTAINER_INCLUDE */
