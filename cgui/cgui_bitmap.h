/*
 *	Copyright (c) 2004 by Gambro BCT, Inc.  All rights reserved.
 *
 *  Bitmap display manager for the common GUI.  Uses the CGUIBitmapInfo class to keep track
 *  of the bitmap information and do the actual display of the graphics.
 *
 * $Header: H:/BCT_Development/vxWorks/Common/cgui/rcs/cgui_bitmap.h 1.4 2004/11/18 22:34:09Z rm10919 Exp wms10235 $
 * $Log: cgui_bitmap.h $
 * Revision 1.2  2004/09/30 17:00:52Z  cf10242
 * Correct for initial make to work
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

#include "cgui_bitmap_info.h"
#include "cgui_graphics.h"
#include "cgui_window_object.h"

class CGUIBitmap : public CGUIWindowObject
{
public:
	// CONSTRUCTOR - must provide a display reference, region, and bitmap data reference.  Can elect
	// to realize the graphics on the display or not.  Note that for bitmaps the region only defines the
	// start location (x,y) of the bitmap.  The height and width are set by the size of the bitmap itself.
   CGUIBitmap(CGUIDisplay & display, const CGUIRegion region, CGUIBitmapInfo & bitmapObj,  bool realize=true);
	//DESTRUCTIOR
	virtual ~CGUIBitmap(void);

	// setRegion - override of the parent setRegion to maintain a fixed height and width
	//  for the bitmap since that can not change (although location of the bitmap may change).
   virtual void setRegion(const CGUIRegion & newRegion);

	// setBitmap - realize the bitmap at the currently set region
   void setBitmap(void);

	// unloadBitmap - remove the bitmap from the display at the currently set region
	void unloadBitmap (void);

protected:
   virtual void draw(UGL_GC_ID gc);

private:
   virtual bool clipSiblings(void) const { return true;}

private:
	CGUIRegion _region;
	unsigned short _width;
   unsigned short _height;
	CGUIBitmapInfo & _bitmapObj;
	CGUIDisplay & _display;
};

#endif /* #ifndef _CGUI_BITMAP_INCLUDE */

