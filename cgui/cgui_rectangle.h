/*
 *	Copyright (c) 2004 by Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: K:/BCT_Development/vxWorks/Common/cgui/rcs/cgui_rectangle.h 1.1 2004/09/20 18:18:07Z rm10919 Exp jl11312 $
 * $Log: cgui_rectangle.h $
 *
 */

#ifndef _CGUI_RECTANGLE_INCLUDE
#define _CGUI_RECTANGLE_INCLUDE


#include <vxworks.h>
#include <ugl/ugl.h>
#include <ugl/uglwin.h>
#include <ugl/uglinput.h>

#include "cgui_graphics.h"
#include "cgui_window_object.h"

class CGUIRectangle : public CGUIWindowObject
{
public:
   CGUIRectangle(OSDisplay & display, CGUIColor color = 0);
   CGUIRectangle(OSDisplay & display, const OSRegion & region, CGUIColor color = 0);
   virtual ~CGUIRectangle();

   void setColor(CGUIColor color);
   CGUIColor getColor(void) { return _color;}

protected:
   virtual void draw(UGL_GC_ID gc);

private:
   virtual bool clipSiblings(void) const { return true;}

private:
   CGUIColor  _color;
};
#endif /* #ifndef _CGUI_RECTANGLE_INCLUDE */
