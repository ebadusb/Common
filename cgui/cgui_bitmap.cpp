/*
* $Header: H:/BCT_Development/vxWorks/Common/cgui/rcs/cgui_bitmap.cpp 1.3 2005/01/25 21:45:40Z cf10242 Exp wms10235 $ 
*  Implement bitmap display manager for the common GUI.  Uses the CGUIBitmapInfo class to keep track
*  of the bitmap information and do the actual display of the graphics.
*
* $Log: cgui_bitmap.cpp $
 */

#include <vxWorks.h>
#include "cgui_bitmap.h"
#include "cgui_bitmap_info.h"
#include "cgui_window.h"

CGUIBitmap::CGUIBitmap(CGUIDisplay & display, const CGUIRegion region, CGUIBitmapInfo & bitmapObj, bool realize)
: CGUIWindowObject(display, region),	// initialize the parent
	_display(display),							// set display reference
	_bitmapObj (bitmapObj),					// set the bitmap data object
	_region (region)							// initialize the region
{
	if(realize)
		setBitmap();
}

CGUIBitmap::~CGUIBitmap()
{
	unsigned short width, height;
   if (_owner)
   {
      _owner->deleteObject(this);
      _owner = NULL;
   }

   _bitmapObj.removeDisplay(_display);
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
	_bitmapObj.createDisplay(_display);

   CGUIRegion newRegion = _region;
	_bitmapObj.getSize(_width, _height);
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

void CGUIBitmap::unloadBitmap()
{
	
	_bitmapObj.removeDisplay(_display);

	unsigned short height, width;
	CGUIRegion newRegion = _region;
	_bitmapObj.getSize(width, height);
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
	CGUIBitmapId uglId = _bitmapObj.getId();
   if (uglId != UGL_NULL_ID)
   {
      uglBitmapBlt(gc, uglId, 0, 0, _width-1, _height-1, UGL_DEFAULT_ID, _region.x, _region.y);
   }
}
