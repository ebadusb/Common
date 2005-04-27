/*
 *	Copyright (c) 2005 by Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: K:/BCT_Development/vxWorks/Common/cgui/rcs/cgui_string_data_container.cpp 1.1 2005/04/27 13:40:47Z rm10919 Exp wms10235 $
 * $Log: cgui_string_data_container.cpp $
 *
 */

#include <vxworks.h>

#include "cgui_text_item.h"
#include "cgui_string_data.h"
#include "cgui_string_data_container.h"

CGUIStringDataContainer::CGUIStringDataContainer()
{

}

CGUIStringDataContainer::~CGUIStringDataContainer()
{

}

CGUITextItem * CGUIStringDataContainer::findString(const char * name)
{
   CGUIStringData * textTable;
   CGUITextItem * result = NULL;

   textTable = _textTable;
   
   while(textTable  && !result)
   {
      result = textTable->findString(name);
      textTable = (CGUIStringData *)textTable->child();      
   }

   // if can't find variable return NULL.
   return result;
}

