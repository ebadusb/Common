/*
* $Header: J:/BCT_Development/vxWorks/Common/cgui/rcs/cgui_bitmap_info.h 1.9 2007/04/14 18:04:41Z jl11312 Exp rm10919 $
* This file defines the class that defines the bitmaps compiled into the application.
* Each bitmap will have automatically generated an object of this type via the
* build_bitmap_info file.
*
* $Log: cgui_bitmap_info.h $
* Revision 1.9  2007/04/14 18:04:41Z  jl11312
* - delete uncompressed data for unreferenced bitmaps (common IT 80)
* Revision 1.8  2007/03/28 15:18:28Z  wms10235
* IT2888 - Correcting GUI memory leak
* Revision 1.7  2006/05/15 21:47:36Z  rm10919
* Update to handle trima bitmaps.
* Revision 1.6  2005/07/07 19:33:12Z  pn02526
* Add dynamic bitmaps.
* Revision 1.5  2005/02/21 10:17:10  cf10242
* IT 133 - delete all allocated memory to avoid unrecovered memory
* Revision 1.4  2004/11/11 17:44:39Z  cf10242
* Size of bitmap change to unsigned long to handle large bitmaps.
* Revision 1.3  2004/11/03 18:10:58Z  cf10242
* Swap height and width in constructor to be cnsistent with CGUIRegion ordering.
* Revision 1.2  2004/10/22 20:13:09Z  rm10919
* Added methods to get height and width.
* Revision 1.1  2004/10/14 14:26:52Z  cf10242
* Initial revision
*
*/

#ifndef _CGUIBITMAPINFO_INCLUDE
#define _CGUIBITMAPINFO_INCLUDE

#include <ugl/ugl.h>
#include <ugl/uglwin.h>
#include <ugl/uglinput.h>

#include "cgui_graphics.h"

class CGUIBitmapInfo
{
public:
	typedef UGL_DDB_ID	CGUIBitmapId;  // create a CGUI type for the UGL ID associated with a bitmap

	// CONSTRUCTOR - accepts a pointer to the input bitmap.  Expects that the data is in
	// compressed format.
	CGUIBitmapInfo (const unsigned char bmp_data[],
						 unsigned long size,
						 unsigned short width,
						 unsigned short height);

	CGUIBitmapInfo (const unsigned char bmp_data[],
						 unsigned long size,
						 const unsigned char mask_data[],
                   unsigned long sizeMask,
						 unsigned short width,
						 unsigned short height);

	// CONSTRUCTOR - accepts a pointer to the input bitmap.  Expects that the data is in
	// uncompressed format.
	CGUIBitmapInfo (const unsigned short bmp_data[],
						 unsigned long size,
						 unsigned short width,
						 unsigned short height);

	CGUIBitmapInfo (const unsigned short bmp_data[],
						 unsigned long size,
						 const unsigned short mask_data[],
						 unsigned long sizeMask,
						 unsigned short width,
						 unsigned short height);

	virtual ~CGUIBitmapInfo();

	// createDisplay - decompress the bitmap loaded at construction and display the image
	virtual void createDisplay (CGUIDisplay & display);

	// removeDisplay - remove the image from the display
	virtual void removeDisplay(CGUIDisplay & display);

	// getRawData -  return the pointer to the raw compressed data
	inline unsigned char * getRawData (void){ return _myBitmap; }

	// getSize - return the width and height of the bitmap
	inline void getSize(unsigned short & width, unsigned short & height) { width = _myWidth;height = _myHeight; }
	inline int getWidth(void) { return _myWidth; }
	inline int getHeight(void) { return _myHeight; }

	// getId - return my CGUIBitmapId
	inline CGUIBitmapId getId() { return _myId; }

	inline bool getTransparency(void) { return _transparent; }

private:
	// Declare copy constructor and assigned operator private to avoid use
	//
	CGUIBitmapInfo(const CGUIBitmapInfo& rhs);
	CGUIBitmapInfo& operator=(const CGUIBitmapInfo& rhs);

private:
	CGUIBitmapInfo(void);

	unsigned char *	_myBitmap;
	unsigned char *	_myMaskBitmap;
	unsigned long  	_mySize;
	unsigned long  	_mySizeMask;
	unsigned short 	_myHeight,_myWidth;
	unsigned short 	_loadCount;
	CGUIBitmapId		_myId;
	bool  				_compressed;
	bool  				_transparent;
};

#endif /* ifndef _CGUIBITMAPINFO_INCLUDE */

