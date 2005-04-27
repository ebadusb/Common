/*
 *	Copyright (c) 2005 by Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: H:/BCT_Development/vxWorks/Common/cgui/rcs/cgui_variable_database.h 1.3 2007/05/03 16:19:14Z jl11312 Exp wms10235 $
 * $Log: cgui_variable_database.h $
 *
 */

#ifndef _VARIABLE_DATABASE_INCLUDE
#define _VARIABLE_DATABASE_INCLUDE

#include "link_element.h"
#include "link_group.h"
#include "cgui_graphics.h"
#include "cgui_data_item.h"


class CGUIVariableDatabase : public LinkElement
{
public:
   CGUIVariableDatabase(unsigned int linkLevel);
   ~CGUIVariableDatabase(void);

   StringChar * variableLookUp(const char * name);

   void addDataItem(const char * name, CGUIDataItem * dataItem);
   void deleteDataItem (const char * name);

protected:

private:

   map<string, CGUIDataItem *> _variableDictionary;
};

#endif /* ifndef _VARIABLE_DATABASE_INCLUDE */

