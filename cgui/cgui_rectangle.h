/*
 *	Copyright (c) 2004 by Gambro BCT, Inc.  All rights reserved.
 *
 * $Header$
 * $Log: cgui_rectangle.h $
 * Revision 1.5  2008/11/06 22:24:16Z  rm10919
 * Add transparent and shaded bitmaps and shaded buttons.
 * Revision 1.4  2005/04/15 21:35:19Z  cf10242
 * override region method for rectangle
 * Revision 1.3  2005/02/21 17:17:12Z  cf10242
 * IT 133 - delete all allocated memory to avoid unrecovered memory
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
   virtual void setRegion(const CGUIRegion & newRegion);

protected:
   virtual void draw(UGL_GC_ID gc);

private:
   virtual ClippingType clipSiblings(void) const { return Clipped;}

private:
   CGUIColor  _color;

private:
	CGUIRectangle(void);
	CGUIRectangle (const CGUIRectangle &obj);
	CGUIRectangle operator=(const CGUIRectangle &obj);

};
#endif /* #ifndef _CGUI_RECTANGLE_INCLUDE */
