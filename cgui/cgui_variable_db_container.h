/*
 *	Copyright (c) 2005 by Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: H:/BCT_Development/vxWorks/Common/cgui/rcs/cgui_variable_db_container.h 1.2 2005/11/22 00:34:43Z rm10919 Exp wms10235 $
 * $Log: cgui_variable_db_container.h $
 *
 */

#ifndef _VARIABLE_DB_CONTAINER_INCLUDE
#define _VARIABLE_DB_CONTAINER_INCLUDE

#include "link_group.h"
#include "cgui_graphics.h"

class CGUIVariableDatabase;
   
class CGUIVariableDatabaseContainer: public LinkGroup
{
public:

   CGUIVariableDatabaseContainer(void);
   ~CGUIVariableDatabaseContainer(void);
   
   StringChar * variableLookUp(const char * name);

   CGUIVariableDatabase * _variableDatabase;

};

#endif /* ifndef _VARIABLE_DB_CONTAINER_INCLUDE */
