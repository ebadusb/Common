/*
 *	Copyright (c) 2004 by Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: H:/BCT_Development/vxWorks/Common/cgui/rcs/cgui_bitmap.h 1.4 2004/11/18 22:34:09Z rm10919 Exp wms10235 $
 * $Log: cgui_bitmap.h $
 * Revision 1.1  2004/09/20 18:18:12Z  rm10919
 * Initial revision
 *
 */

#ifndef _CGUI_BITMAP_INCLUDE
#define _CGUI_BITMAP_INCLUDE


#include <vxworks.h>
#include <ugl/ugl.h>
#include <ugl/uglwin.h>
#include <ugl/uglinput.h>

#include "cgui_graphics.h"
#include "cgui_window_object.h"

class CGUIBitmap : public CGUIWindowObject
{
public:
   CGUIBitmap(CGUIDisplay & display);
   CGUIBitmap(CGUIDisplay & display, BITMAP_ID guiId);
   virtual ~CGUIBitmap(void);

   virtual void setRegion(const CGUIRegion & newRegion);

   void setBitmap(BITMAP_ID guiId);

protected:
   virtual void draw(UGL_GC_ID gc);

private:
   void initializeData(void);
   virtual bool clipSiblings(void) const { return true;}

private:
   BITMAP_ID      _guiId;
   CGUIBitmapId   _uglId;
   unsigned short _width;
   unsigned short _height;
};

#endif /* #ifndef _CGUI_BITMAP_INCLUDE */

