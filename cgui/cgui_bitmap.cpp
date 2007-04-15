/*
 * $Header: J:/BCT_Development/vxWorks/Common/cgui/rcs/cgui_bitmap.cpp 1.5 2007/04/14 18:04:40Z jl11312 Exp rm10919 $
 *
 * Implement bitmap display manager for the common GUI.  Uses the CGUIBitmapInfo class to keep track
 * of the bitmap information and do the actual display of the graphics.
 *
 * $Log: cgui_bitmap.cpp $
 * Revision 1.4  2007/03/28 15:18:28Z  wms10235
 * IT2888 - Correcting GUI memory leak
 * Revision 1.3  2005/01/25 21:45:40Z  cf10242
 * uncompress bitmaps once to improve speed
 * Revision 1.2  2004/11/18 22:34:09Z  rm10919
 * Naming conventions.
 * Revision 1.1  2004/10/14 14:30:04Z  cf10242
 * Initial revision
 */

#include <vxWorks.h>
#include "cgui_bitmap.h"
#include "cgui_bitmap_info.h"
#include "cgui_window.h"

CGUIBitmap::CGUIBitmap(CGUIDisplay & display, const CGUIRegion region, CGUIBitmapInfo & bitmapObject, bool realize) :
	CGUIWindowObject(display, region),	// initialize the parent
	_display(display),               	// set display reference
	_bitmapObject (&bitmapObject),    	// set the bitmap data object
	_region (region)              		// initialize the region
{
   if (realize)
	{
      setBitmap();
	}
}

CGUIBitmap::~CGUIBitmap()
{
   _bitmapObject->removeDisplay(_display);
}

void CGUIBitmap::setRegion(const CGUIRegion & newRegion)
{
   CGUIRegion clippedRegion = newRegion;
   clippedRegion.width = _width;
   clippedRegion.height = _height;

   CGUIWindowObject::setRegion(clippedRegion);
}

void CGUIBitmap::setBitmap()
{
   // tell bitmap object to display the image
   _bitmapObject->createDisplay(_display);

   CGUIRegion newRegion = _region;
   _bitmapObject->getSize(_width, _height);
   newRegion.width = _width;
   newRegion.height = _height;

   CGUIWindowObject::setRegion(newRegion);

   //
   // The explicit call to invalidate the bitmap's region is needed
   // in case we changed to an identically sized bitmap (if the old and
   // new regions are the same, CGUIWindowObject::set_region simply returns
   // without forcing a redraw).
   //
   if (_owner)
   {
      _owner->invalidateObjectRegion(this);
   }
}

void CGUIBitmap::setBitmap(CGUIBitmapInfo & bitmapObject)
{
   if (_bitmapObject->getId() != 0)
   {
      //_display.unloadBitmap(_bitmapObject);
      unloadBitmap();
   }

   _bitmapObject = &bitmapObject;
   setBitmap();

   //
   // The explicit call to invalidate the bitmap's region is needed
   // in case we changed to an identically sized bitmap (if the old and
   // new regions are the same, OSWindowObject::set_region simply returns
   // without forcing a redraw).
   //
   if (_owner)
   {
      _owner->invalidateObjectRegion(this);
   }
}

void CGUIBitmap::unloadBitmap()
{

   _bitmapObject->removeDisplay(_display);

   unsigned short height, width;
   CGUIRegion newRegion = _region;
   _bitmapObject->getSize(width, height);
   newRegion.width = width;
   newRegion.height = height;
   CGUIWindowObject::setRegion(newRegion);

   //
   // The explicit call to invalidate the bitmap's region is needed
   // in case we changed to an identically sized bitmap (if the old and
   // new regions are the same, CGUIWindowObject::set_region simply returns
   // without forcing a redraw).
   //
   if (_owner)
   {
      _owner->invalidateObjectRegion(this);
   }
}

void CGUIBitmap::draw(UGL_GC_ID gc)
{
   CGUIBitmapId uglId = _bitmapObject->getId();
   if (uglId != UGL_NULL_ID)
   {
      uglBitmapBlt(gc, uglId, 0, 0, _width-1, _height-1, UGL_DEFAULT_ID, _region.x, _region.y);
   }
}

