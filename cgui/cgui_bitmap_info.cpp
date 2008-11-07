/*
 * $Header: J:/BCT_Development/vxWorks/Common/cgui/rcs/cgui_bitmap_info.cpp 1.7 2007/04/14 18:04:40Z jl11312 Exp rm10919 $
 *
 * This file implements the class that manages the bitmaps compiled into the application.
 * Each bitmap will have automatically generated an object of this type via the
 * build_bitmap_info file.
 *
 * $Log: cgui_bitmap_info.cpp $
 * Revision 1.7  2007/04/14 18:04:40Z  jl11312
 * - delete uncompressed data for unreferenced bitmaps (common IT 80)
 * Revision 1.6  2007/03/28 15:18:28Z  wms10235
 * IT2888 - Correcting GUI memory leak
 * Revision 1.5  2006/05/15 21:47:36Z  rm10919
 * Update to handle trima bitmaps.
 * Revision 1.4  2005/07/20 17:22:04Z  pn02526
 * Add dynamic bitmaps.
 * Revision 1.3  2005/01/25 14:45:40  cf10242
 * uncompress bitmaps once to improve speed
 * Revision 1.2  2004/11/11 17:44:38Z  cf10242
 * Size of bitmap change to unsigned long to handle large bitmaps.
 * Revision 1.1  2004/10/14 14:26:53Z  cf10242
 * Initial revision
 *
 */

#include <vxWorks.h>
#include "cgui_bitmap_info.h"
#include "zlib.h"

CGUIBitmapInfo::CGUIBitmapInfo(const unsigned char bmp_data[],
					 unsigned long size,
					 unsigned short width,
					 unsigned short height) :
					 _myHeight (height),
					 _myWidth (width),
					 _loadCount (0),
					 _myId(UGL_NULL_ID),
					 _myMaskBitmap(NULL),
					 _transparent(false),
					 _compressed(true)
{
	_mySize = size;
	_mySizeMask = 0;
	_myBitmap = (unsigned char *)bmp_data;
}

CGUIBitmapInfo::CGUIBitmapInfo(const unsigned char bmp_data[], 
                               unsigned long size,
                               const unsigned char mask_data[],
                               unsigned long sizeMask,
                               unsigned short width,
                               unsigned short height ) :
					 _myHeight (height),
					 _myWidth (width),
					 _loadCount (0),
					 _myId(UGL_NULL_ID),
					 _transparent(true),	
					 _compressed(true)
{
	_mySize = size;
	_mySizeMask = sizeMask;
	_myBitmap = (unsigned char *)bmp_data;
	_myMaskBitmap = (unsigned char *)mask_data;
}

// CONSTRUCTOR - accepts a pointer to the input bitmap.  Expects that the data is in
// uncompressed format.
CGUIBitmapInfo::CGUIBitmapInfo(const unsigned short bmp_data[],
                               unsigned long size,
                               unsigned short width,
                               unsigned short height) :
					 _myHeight (height),
					 _myWidth (width),
					 _loadCount (0),
					 _myId(UGL_NULL_ID),
					 _myMaskBitmap(NULL),
					 _transparent(false),	
					 _compressed(false)
{
	_mySize = size;
	_mySizeMask = 0;
	_myBitmap = (unsigned char *)bmp_data;
}

CGUIBitmapInfo::CGUIBitmapInfo(const unsigned short bmp_data[], 
										 unsigned long size,
                               const unsigned short mask_data[],
                               unsigned long sizeMask,
                               unsigned short width,
                               unsigned short height) :
					 _myHeight (height),
					 _myWidth (width),
					 _loadCount (0),
					 _myId(UGL_NULL_ID),
					 _transparent(true),
					 _compressed(false)
{
	_mySize = size;
	_mySizeMask = sizeMask;
	_myBitmap = (unsigned char *)bmp_data;
	_myMaskBitmap = (unsigned char *)mask_data;
}

CGUIBitmapInfo::~CGUIBitmapInfo()
{
}

// Uncompress the data loaded into the object at construction and create a UGL bitmap.
void CGUIBitmapInfo::createDisplay (CGUIDisplay & dispObj)
{
   // load bitmap if not already loaded.  Should only occur for first time bitmap info is referenced
   // inside of cgui_bitmap object.
   if ( _myId == UGL_NULL_ID )
   {
      // bitmap not loaded yet
      unsigned long   bmpSize = _myHeight * _myWidth * sizeof(unsigned short);   // the USHORT is for RGB565 representation of color for bmp
      unsigned long   maskSize = _myHeight * _myWidth * sizeof(unsigned char);   // the U CHAR is for monchrome representation of the mask bitmap
      unsigned char * bmpImage = _compressed ? new unsigned char[bmpSize] : _myBitmap;
		unsigned char * maskImage = (_compressed && _transparent) ? new unsigned char[maskSize] : _myMaskBitmap;

		bool haveTransparentMask = _transparent;
		
      UGL_DIB dib;
      UGL_MDIB tdib;

      UGL_DEVICE_ID display = dispObj.display();
      UGL_MODE_INFO modeInfo;

      uglInfo(display, UGL_MODE_INFO_REQ, &modeInfo);

		int uncp = uncompress( maskImage, &maskSize, _myMaskBitmap, _mySizeMask);

		if ( _compressed &&  uncp != Z_OK ) 
			haveTransparentMask = false;
		
      if (_compressed && uncompress(bmpImage, &bmpSize, _myBitmap, _mySize) != Z_OK)
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

			if( _transparent )
			{
				int byteWidth = (_myWidth+7)/8;
				
            tdib.height = _myHeight;
            tdib.width = _myWidth;
            tdib.stride = 8 * byteWidth;
            tdib.pImage = (UGL_UINT8 *)maskImage;
			}

         if (modeInfo.colorModel == UGL_DIRECT)
         {
            dib.clutSize = 0;
            dib.pClut = NULL;

            dib.colorFormat = UGL_DEVICE_COLOR;
            dib.imageFormat = UGL_DIRECT;
         }
			else if (modeInfo.colorModel == UGL_INDEXED_8)
         {
#if CPU==SIMNT
				dib.clutSize = CGUIPaletteSize;
				dib.pClut = deviceClut;

            dib.colorFormat = UGL_ARGB8888;
            dib.imageFormat = UGL_INDEXED_8;
#else /* if CPU==SIMNT */
				dib.clutSize = 0;
				dib.pClut = NULL;

				dib.colorFormat = UGL_DEVICE_COLOR;
				dib.imageFormat = UGL_DIRECT;
#endif /* if CPU==SIMNT */
         }
			else
         {
            dib.clutSize = 0;
            dib.pClut = NULL;

            dib.colorFormat = UGL_DEVICE_COLOR;
            dib.imageFormat = UGL_DIRECT;
         }

			if( !haveTransparentMask )
            _myId = uglBitmapCreate(display, &dib, UGL_DIB_INIT_DATA, 0, UGL_DEFAULT_MEM);
			else
				_myId = uglTransBitmapCreate(display, &dib, &tdib, UGL_DIB_INIT_DATA, 0, UGL_DEFAULT_MEM);

         // get actual height and width
         UGL_SIZE width, height;
         uglBitmapSizeGet(_myId, &width, &height);
         _myWidth = width;
         _myHeight = height;
      }

      if ( _compressed ) delete[] bmpImage;
      if ( _compressed && _transparent ) delete[] maskImage;
   }

	_loadCount += 1;
}

void CGUIBitmapInfo::removeDisplay(CGUIDisplay & dispObj)
{
	if ( _loadCount > 0 )
	{
		_loadCount -= 1;
	}
 
	if ( _loadCount == 0 &&
		  _myId != UGL_NULL_ID )
	{
		UGL_DEVICE_ID display = dispObj.display();
		uglBitmapDestroy(display, _myId);
		_myId = UGL_NULL_ID;
   }
}
