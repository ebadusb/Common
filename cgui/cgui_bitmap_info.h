/*
* $Header: //BCTquad3/home/BCT_Development/vxWorks/Common/cgui/rcs/cgui_bitmap_info.h 1.5 2005/02/21 10:17:10 cf10242 Exp pn02526 $ 
* This file defines the class that defines the bitmaps compiled into the application.
* Each bitmap will have automatically generated an object of this type via the
* build_bitmap_info file.
*
* $Log: cgui_bitmap_info.h $
* Revision 1.1  2004/10/14 14:26:52Z  cf10242
* Initial revision
*
*/
#ifndef _CGUIBITMAPINFO_INCLUDE
#define _CGUIBITMAPINFO_INCLUDE

#include <vxworks.h>
#include <stdio.h>

#include <ugl/ugl.h>
#include <ugl/uglwin.h>
#include <ugl/uglinput.h>

#include "cgui_graphics.h"

class CGUIBitmapInfo
{

	public:

		enum CGUIBitmapState
		{
			UNLOADED,
			LOADED
		};

		typedef UGL_DDB_ID   CGUIBitmapId;  // create a CGUI type for the UGL ID associated with a bitmap

		// CONSTRUCTOR - accepts a pointer to the input bitmap.  Expects that the data is in
		// compressed format.
		CGUIBitmapInfo (const unsigned char bmp_data[],
							  unsigned short size,
			                  unsigned short height,
							  unsigned short width) : 	_myHeight (height),
														_myWidth (width),
														_loadCount (0),
														_loadState (UNLOADED)
		{
			_mySize = size;
			_myBitmap = (unsigned char *)bmp_data; 
		};

		// CONSTRUCTOR - accepts a file path and name as input.  This constructor will open and
		// read in the file to _myBitmap, set _myHeight and _myWidth as well.  THIS IS A FUTURE
		// CAPABILITY AND IS NOT YET AVAILABLE!!!
		CGUIBitmapInfo (char * fileName); 
 
		// DESTRUCTOR - doesn't need to do anything at this point
		virtual ~CGUIBitmapInfo()  {};

		// getRawData -  return the pointer to the raw compressed data
		unsigned char * getRawData (void)
		{
			return _myBitmap;
		};

		// getSize - return the width and height of the bitmap
		void getSize(unsigned short & width, unsigned short & height)  
		{ 
			width = _myWidth;
			height = _myHeight;
		};
        
        int getWidth(void) { return _myWidth;}
        int getHeight(void) { return _myHeight;}

		// createDisplay - decompress the bitmap loaded at construction and display the image
		virtual void createDisplay (CGUIDisplay & display);

		// removeDisplay - remove the image from the display
		virtual void removeDisplay(CGUIDisplay & display);

		// getId - return my CGUIBitmapId
		CGUIBitmapId getId()   { return _myId;  };

	private:
		unsigned char * _myBitmap;
		unsigned long _mySize;
		unsigned short _myHeight,_myWidth;
		CGUIBitmapState _loadState;
		unsigned short _loadCount;
		CGUIBitmapId _myId;
};
#endif




