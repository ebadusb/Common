/*
 *	Copyright (c) 2005 by Gambro BCT, Inc.  All rights reserved.
 *
 * $Header$
 * $Log: cgui_variable_database.h $
 * Revision 1.3  2007/05/03 16:19:14Z  jl11312
 * - added semaphore protection for map structures
 * Revision 1.2  2005/09/30 22:40:54Z  rm10919
 * Get the variable database working!
 * Revision 1.1  2005/04/27 13:40:49Z  rm10919
 * Initial revision
 *
 */

#ifndef _VARIABLE_DATABASE_INCLUDE
#define _VARIABLE_DATABASE_INCLUDE

#include <semLib.h>
#include "link_element.h"
#include "link_group.h"
#include "cgui_graphics.h"
#include "cgui_data_item.h"


class CGUIVariableDatabase : public LinkElement
{
public:
   CGUIVariableDatabase(unsigned int linkLevel);
   ~CGUIVariableDatabase(void);

   const StringChar * variableLookUp(const char * name);

   void addDataItem(const char * name, CGUIDataItem * dataItem);
   void deleteDataItem (const char * name);

   CGUIDataItem * getDataItem(const char * name);

protected:

private:
	SEM_ID	_lock;
   map<string, CGUIDataItem *> _variableDictionary;
};

#endif /* ifndef _VARIABLE_DATABASE_INCLUDE */

