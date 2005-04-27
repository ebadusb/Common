/*
 *	Copyright (c) 2005 by Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: K:/BCT_Development/vxWorks/Common/cgui/rcs/cgui_string_data_container.h 1.1 2005/04/27 13:40:47Z rm10919 Exp wms10235 $
 * $Log: cgui_string_data_container.h $
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
};

#endif /* ifndef _STRING_DATA_CONTAINER_INCLUDE */
