/*
 *	Copyright (c) 2004 by Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: K:/BCT_Development/vxWorks/Common/cgui/rcs/cgui_rectangle.cpp 1.3 2005/04/15 21:35:18Z cf10242 Exp jl11312 $
 * $Log: cgui_rectangle.cpp $
 * Revision 1.3  2005/04/15 21:35:18Z  cf10242
 * override region method for rectangle
 * Revision 1.2  2004/09/28 19:47:27Z  rm10919
 * Missed naming changes to CGUI.
 * Revision 1.1  2004/09/20 18:18:06Z  rm10919
 * Initial revision
 *
 */

#include <vxWorks.h>
#include "cgui_rectangle.h"
#include "cgui_window.h"


CGUIRectangle::CGUIRectangle(CGUIDisplay & display, CGUIColor color)
: CGUIWindowObject(display), _color(color)
{
}

CGUIRectangle::CGUIRectangle(CGUIDisplay & display, const CGUIRegion & region, CGUIColor color)
: CGUIWindowObject(display, region), _color(color)
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

void CGUIRectangle::setRegion(const CGUIRegion & newRegion)
{
  if (_owner)
  {
    _owner->setObjectRegion(this, newRegion);
  } else
  {
    _region = newRegion;
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
