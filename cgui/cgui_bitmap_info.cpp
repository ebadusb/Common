/*
* $Header: //BCTquad3/home/BCT_Development/vxWorks/Common/cgui/rcs/cgui_bitmap_info.cpp 1.3 2005/01/25 14:45:40 cf10242 Exp pn02526 $ 
* This file implements the class that manages the bitmaps compiled into the application.
* Each bitmap will have automatically generated an object of this type via the
* build_bitmap_info file.
*
* $Log: cgui_bitmap_info.cpp $
* Revision 1.2  2004/11/11 17:44:38Z  cf10242
* Size of bitmap change to unsigned long to handle large bitmaps.
* Revision 1.1  2004/10/14 14:26:53Z  cf10242
* Initial revision
*
*/

#include "cgui_bitmap_info.h"
#include "zlib.h"


// constructor with a file name presented
CGUIBitmapInfo::CGUIBitmapInfo(char * fileName) :	_myHeight (0),
																	_myWidth (0),
																	_loadCount (0),
																	_loadState (UNLOADED),
																	_myBitmap (NULL),
																	_mySize(0),
																	_myId(UGL_NULL_ID)
{
	// future capability - for now it does nothing but initialize some variables
}

// Uncompress the data loaded into the object at construction and create a UGL bitmap.
void CGUIBitmapInfo::createDisplay (CGUIDisplay & dispObj)
{
	// load bitmap if not already loaded.  Should only occur for first time bitmap info is referenced
	// inside of cgui_bitmap object.
	if(_loadCount == 0)
	{
		// bitmap not loaded yet
		unsigned long   bmpSize = _myHeight * _myWidth * sizeof(unsigned short);   // the USHORT is for RGB565 representation of color for bmp
		unsigned char * bmpImage = new unsigned char[bmpSize];
		UGL_DIB dib;

		if(uncompress(bmpImage, &bmpSize, _myBitmap, _mySize) != Z_OK)
		{
			// log mesage to error log level and set a null ID
			_myId = UGL_NULL_ID;
		}
		else
		{
			dib.height = _myHeight;
			dib.width = _myWidth;
			dib.stride = _myWidth;
			dib.pImage = (void *)bmpImage;

#if CPU==SIMNT

			dib.clutSize = 0;
			dib.pClut = NULL;

			dib.colorFormat = UGL_DEVICE_COLOR;
			dib.imageFormat = UGL_DIRECT;

#else /* if CPU!=SIMNT */

			dib.clutSize = 0;
			dib.pClut = NULL;

			dib.colorFormat = UGL_DEVICE_COLOR;
			dib.imageFormat = UGL_DIRECT;

#endif /* if CPU==SIMNT */

			UGL_DEVICE_ID display = dispObj.display();
			_myId = uglBitmapCreate(display, &dib, UGL_DIB_INIT_DATA, 0, UGL_DEFAULT_MEM);
			delete[] bmpImage;

			// get actual height and width
	      UGL_SIZE width, height;
		   uglBitmapSizeGet(_myId, &width, &height);
			_myWidth = width;
			_myHeight = height;
			_loadCount = 1;
		}
	}
}

void CGUIBitmapInfo::removeDisplay(CGUIDisplay & dispObj)
{

	if (_loadCount > 0 && _myId != UGL_NULL_ID)
	{
		UGL_DEVICE_ID display = dispObj.display();
		uglBitmapDestroy(display, _myId);
		_myId = UGL_NULL_ID;
	}

}
