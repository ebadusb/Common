/*
 *	Copyright (c) 2004 by Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: K:/BCT_Development/vxWorks/Common/cgui/rcs/cgui_rectangle.cpp 1.3 2005/04/15 21:35:18Z cf10242 Exp jl11312 $
 * $Log: cgui_rectangle.cpp $
 *
 */

#include <vxWorks.h>
#include "cgui_rectangle.h"
#include "cgui_window.h"


CGUIRectangle::CGUIRectangle(CGUIDisplay & display, CGUIColor color)
: OSWindowObject(display), _color(color)
{
}

CGUIRectangle::CGUIRectangle(CGUIDisplay & display, const CGUIRegion & region, CGUIColor color)
: OSWindowObject(display, region), _color(color)
{
}

CGUIRectangle::~CGUIRectangle()
{
}

void CGUIRectangle::setColor(CGUIColor color)
{
   if (color != _color)
   {
      _color = color;
      if (_owner)
      {
         _owner->invalidateObjectRegion(this);
      }
   }
}

void CGUIRectangle::draw(UGL_GC_ID gc)
{
   if (_region.width > 0 && _region.height > 0)
   {
      uglLineWidthSet(gc, 0);
      uglBackgroundColorSet(gc, _color);
      uglRectangle(gc, _region.x, _region.y, _region.x+_region.width-1, _region.y+_region.height-1);
   }
}
