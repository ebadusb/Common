/*
 *	Copyright (c) 2005 by Gambro BCT, Inc.  All rights reserved.
 *
 * $Header$
 * $Log: cgui_string_data_container.h $
 * Revision 1.1  2005/04/27 13:40:47Z  rm10919
 * Initial revision
 *
 */

#ifndef _STRING_DATA_CONTAINER_INCLUDE
#define _STRING_DATA_CONTAINER_INCLUDE

#include "link_group.h"

class CGUITextItem;
class CGUIStringData;

class CGUIStringDataContainer: public LinkGroup
{
public:

   CGUIStringDataContainer(void);
   ~CGUIStringDataContainer(void);

   CGUITextItem * findString(const char * name);

   CGUIStringData * _textTable;

private:
	CGUIStringDataContainer(const CGUIStringDataContainer & copy);
	CGUIStringDataContainer & operator=(const CGUIStringDataContainer & copy);
};

#endif /* ifndef _STRING_DATA_CONTAINER_INCLUDE */
