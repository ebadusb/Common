/*
 *	Copyright (c) 2004 by Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: K:/BCT_Development/vxWorks/Common/cgui/rcs/cgui_rectangle.h 1.5 2008/11/06 22:24:16Z rm10919 Exp wms10235 $
 * $Log: cgui_rectangle.h $
 * Revision 1.2  2004/09/30 16:10:29Z  jl11312
 * - updated old-style class names
 * Revision 1.1  2004/09/20 18:18:07Z  rm10919
 * Initial revision
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
   CGUIRectangle(CGUIDisplay & display, CGUIColor color = 0);
   CGUIRectangle(CGUIDisplay & display, const CGUIRegion & region, CGUIColor color = 0);
   virtual ~CGUIRectangle();

   void setColor(CGUIColor color);
   CGUIColor getColor(void) { return _color;}

protected:
   virtual void draw(UGL_GC_ID gc);

private:
   virtual bool clipSiblings(void) const { return true;}

private:
   CGUIColor  _color;

private:
	//CGUIRectangle();
	//CGUIRectangle (const CGUIRectangle &obj);
	//CGUIRectangle operator=(CGUIRectangle &obj);

};
#endif /* #ifndef _CGUI_RECTANGLE_INCLUDE */
