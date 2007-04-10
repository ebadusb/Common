/*
 *	Copyright (c) 2004 by Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: J:/BCT_Development/vxWorks/Common/cgui/rcs/cgui_graphics.cpp 1.27 2007/05/10 16:35:46Z jl11312 Exp rm10919 $
 * $Log: cgui_graphics.cpp $
 * Revision 1.24  2007/04/09 21:12:31Z  wms10235
 * IT2354 - Added postscript save capability
 * Revision 1.23  2007/04/05 18:39:38Z  wms10235
 * IT2354 - Added a preliminary version of the off-screen flush
 * Revision 1.22  2006/05/15 21:51:42Z  rm10919
 * Fix memory bug in convertToStringChar and handle trima palette.
 * Revision 1.21  2005/10/21 16:45:37Z  rm10919
 * Correct use of _stringSize and adding null string terminator.
 * Revision 1.20  2005/09/30 22:40:52Z  rm10919
 * Get the variable database working!
 * Revision 1.19  2005/07/27 22:27:57Z  cf10242
 * increase winApp stack size
 * Revision 1.18  2005/04/26 23:16:47Z  rm10919
 * Made changes to cgui_text and cgui_text_item, plus added
 * classes for variable substitution in text strings.
 * Revision 1.17  2005/01/28 23:52:17Z  rm10919
 * CGUITextItem class changed and put into own file.
 * Revision 1.16  2005/01/17 17:47:06Z  jl11312
 * - implemented setId function
 * Revision 1.15  2005/01/12 20:06:08Z  rm10919
 * Deleted convertToAscii().
 * Revision 1.14  2004/12/27 22:48:51Z  rm10919
 * Added CGUITextItem::getAscii for use with sprintf().
 * Revision 1.13  2004/12/09 00:20:41Z  cf10242
 * Put guard in for message system being created
 * Revision 1.12  2004/11/19 18:14:46Z  cf10242
 * Integration checkin
 * Revision 1.11  2004/11/12 14:59:44Z  cf10242
 * Add message system to UGL so GUi can handle/send messages.
 * Revision 1.9  2004/11/01 17:27:20Z  cf10242
 * Change TextItem to CGUITextItem
 * Revision 1.8  2004/10/29 15:11:14Z  rm10919
 * Revision 1.7  2004/10/22 20:14:34Z  rm10919
 * CGUIButton updates and changes.
 * Revision 1.6  2004/10/07 22:47:02Z  cf10242
 * Revision 1.5  2004/10/07 18:52:19Z  cf10242
 * Revision 1.4  2004/09/30 19:47:27Z  jl11312
 * - temporary add of bitmap data array
 * Revision 1.3  2004/09/30 17:00:53Z  cf10242
 * Correct for initial make to work
 * Revision 1.2  2004/09/28 19:47:27Z  rm10919
 * Missed naming changes to CGUI.
 * Revision 1.1  2004/09/20 18:18:05Z  rm10919
 * Initial revision
 *
 */

#include <vxWorks.h>
#include <ugl/uglInput.h>
#include "zlib.h"

#include "cgui_graphics.h"
#include "cgui_window.h"
#include "bmepsoe.h"
//#include "datalogger.h"

#if CPU==SIMNT
UGL_ARGB deviceClut[CGUIPaletteSize];
#endif /* if CPU==SIMNT */

//
// String Management
//
void convertToStringChar(const char * string, StringChar ** stringChar)
{
   if (string)
   {
      int stringLength = strlen(string) + 1;   // add 1 for the NULL

      *stringChar = new UGL_WCHAR[stringLength];

      for (int i=0; i<stringLength; i++)
         (*stringChar)[i] = (unsigned char)(UGL_WCHAR)string[i];
   }
   else *stringChar =  NULL;
}

// START MESSAGE_SYSTEM_IN_WIN_MGR
#include "messagesystem.h"
extern void (* winAppStartupTask)(void);
extern void (* winAppIdleTask)(void);
MessageSystem * msgSys = NULL;

UGL_DDB_ID drawBitMap = UGL_NULL_ID;
UGL_DDB_ID offscreenBitMap = UGL_NULL_ID;
UGL_GC_ID uglOffscreenGc;

static CallbackBase guiStartCB;
static CallbackBase guiWakeupCB;

static void cguiWinAppStartupTask(void)
{
//	if ( !msgSys )
//	{

   msgSys = new MessageSystem();
   printf("startuptask: msgSys = 0x%x\n",msgSys);
   msgSys->initNonBlock();
   guiStartCB();
//	}
}

static void cguiWinAppIdleTask(void)
{
   if (msgSys)
   {
      msgSys->dispatchMessages();
      guiWakeupCB();
   }
}
// END MESSAGE_SYSTEM_IN_WIN_MGR

CGUIDisplay::CGUIDisplay(const CallbackBase & startCB, const CallbackBase & wakeupCB)
{
   //
   // Perform basic UGL initialization
   //
   UGL_STATUS status = uglInitialize();
   if (status != UGL_STATUS_OK)
   {
      fprintf(stderr, "uglInitialize failed status=%d\n", status);
      taskSuspend(taskIdSelf());
   }

   // set statics to input callback functions
   guiStartCB = startCB;
   guiWakeupCB = wakeupCB;
   //
   // Get an ID for the display driver, then check that we are in the proper
   // color mode.
   //
   uglDriverFind(UGL_DISPLAY_TYPE, 0, (UGL_UINT32 *)&_uglDisplay);

   UGL_MODE_INFO modeInfo;
   uglInfo(_uglDisplay, UGL_MODE_INFO_REQ, &modeInfo);
   _height = modeInfo.height;
   _width = modeInfo.width;

   if ((modeInfo.colorModel != UGL_DIRECT) && (modeInfo.colorModel != UGL_INDEXED_8))
   {
      fprintf(stderr, "UGL reports color model %d - expected %d or %d \n", (int)modeInfo.colorModel, (int)UGL_DIRECT, (int)UGL_INDEXED_8);
      taskSuspend(taskIdSelf());
   }

   _uglGc = uglGcCreate(_uglDisplay);

   uglDriverFind(UGL_EVENT_SERVICE_TYPE, 0, (UGL_UINT32 *)&_uglEventService);

   // START MESSAGE_SYSTEM_IN_WIN_MGR
   winAppStartupTask = cguiWinAppStartupTask;
   winAppIdleTask = cguiWinAppIdleTask;
   // END MESSAGE_SYSTEM_IN_WIN_MGR


   _uglApp = winAppCreate("winApp", 0, 64000, 0, UGL_NULL);

   _uglRootWindow = winCreate(_uglApp, UGL_NULL_ID,
                              WIN_ATTRIB_VISIBLE,             // list window attributes
                              0, 0, _width, _height, UGL_NULL, 0, UGL_NULL);

   winDataSet(_uglRootWindow, NULL, NULL, 0);

   winAttach(_uglRootWindow, UGL_NULL_ID, UGL_NULL_ID);

   uglDriverFind(UGL_FONT_ENGINE_TYPE, 0, (UGL_UINT32 *)&_uglFontDriver);



   cursorInit();
}

CGUIDisplay::~CGUIDisplay()
{
}

void CGUIDisplay::flush(void)
{
   //
   // Redraw all dirty windows
   //
   list<CGUIWindow *>::iterator windowIter = _windowList.begin();
   while (windowIter != _windowList.end())
   {
      (*windowIter)->draw();
      ++windowIter;
   }

   drawRootWindow();
}

/*
int CGUIDisplay::offscreenBitmapFlush(const char * filename)
{
	int retVal = -1;
	uglCursorOff(_uglDisplay);   // Remove mouse from display for snapshot.

	typedef unsigned char  BMP_BYTE;
	typedef unsigned short BMP_WORD;
	typedef unsigned long  BMP_DWORD;

	const int bmpWidth = 800;
	const int bmpHeight = 600;

	struct BITMAPFILEHEADER
	{
		BMP_WORD    bfType;
		BMP_DWORD   bfSize;
		BMP_WORD    bfReserved1;
		BMP_WORD    bfReserved2;
		BMP_DWORD   bfOffBits;
	} __attribute__((packed));

	struct BITMAPINFOHEADER
	{
		BMP_DWORD   biSize;
		BMP_DWORD   biWidth;
		BMP_DWORD   biHeight;
		BMP_WORD    biPlanes;
		BMP_WORD    biBitCount;
		BMP_DWORD   biCompression;
		BMP_DWORD   biSizeImage;
		BMP_DWORD   biXPelsPerMeter;
		BMP_DWORD   biYPelsPerMeter;
		BMP_DWORD   biClrUsed;
		BMP_DWORD   biClrImportant;
	} __attribute__((packed));

	BITMAPFILEHEADER bmpFileHeader;
	BITMAPINFOHEADER bmpInfoHeader;

	size_t bmpSize = bmpWidth * bmpHeight;

	//
	//  Set fileheader information.
	//
	bmpFileHeader.bfType      = 0x4D42;
	bmpFileHeader.bfSize      = sizeof(bmpFileHeader) + sizeof(bmpInfoHeader) + bmpSize * 3;
	bmpFileHeader.bfReserved1 = 0x0000;
	bmpFileHeader.bfReserved2 = 0x0000;
	bmpFileHeader.bfOffBits   = sizeof(bmpFileHeader) + sizeof(bmpInfoHeader);

	bmpInfoHeader.biSize          = sizeof(bmpInfoHeader);
	bmpInfoHeader.biWidth         = bmpWidth;
	bmpInfoHeader.biHeight        = bmpHeight;
	bmpInfoHeader.biPlanes        = 1;
	bmpInfoHeader.biBitCount      = 24;
	bmpInfoHeader.biCompression   = 0;
	bmpInfoHeader.biSizeImage     = bmpSize * 3;
	bmpInfoHeader.biXPelsPerMeter = 0x0EC4;
	bmpInfoHeader.biYPelsPerMeter = 0x0EC4;
	bmpInfoHeader.biClrUsed       = 0;
	bmpInfoHeader.biClrImportant  = 0;

	//
	//  Put current screen image information
	//  into a dib structure. (Device Independent Bitmap)
	//
	UGL_DIB *pDib     = (UGL_DIB *)UGL_MALLOC(sizeof(UGL_DIB));
	pDib->width       = bmpWidth;
	pDib->height      = bmpHeight;
	pDib->stride      = bmpWidth;
	pDib->colorFormat = UGL_DEVICE_COLOR_32; //ARGB8888;//DEVICE_COLOR_32;
	pDib->imageFormat = UGL_DIRECT;
	pDib->clutSize    = UGL_NULL;
	pDib->pClut       = NULL;
	pDib->pImage      = UGL_MALLOC( bmpSize * 4 );

	memset( pDib->pImage, 0, bmpSize * 4 );

	if( drawBitMap == UGL_NULL_ID )
	{
		// Create an offscreen bitmap and graphics context
		uglOffscreenGc = uglGcCreate( _uglDisplay );
		uglGcCopy( _uglGc, uglOffscreenGc );
		drawBitMap = uglBitmapCreate(_uglDisplay, pDib, UGL_DIB_INIT_DATA, 0, UGL_NULL);
	}

	// Enable the offscreen bitmap
	offscreenBitMap = drawBitMap;

	// Draw the screen
	list<CGUIWindow *>::iterator windowIter = _windowList.begin();
	while (windowIter != _windowList.end())
	{
		(*windowIter)->draw( uglOffscreenGc );
		++windowIter;
		//DataLog( log_level_cgui_info ) << "Drawing window." << endmsg;
	}

	// Disable the offscreen bitmap
	offscreenBitMap = UGL_NULL_ID;

	// Read the offscreen bitmap into a DIB
	uglBitmapRead(_uglDisplay, drawBitMap, 0, 0, bmpWidth-1, bmpHeight-1, pDib, 0, 0);

	//
	//  Create and open the bmp file.
	//
	FILE * bmpFile = fopen(filename, "wb");

	if( bmpFile != NULL )
	{
		fwrite(&bmpFileHeader, sizeof(bmpFileHeader), 1, bmpFile);
		fwrite(&bmpInfoHeader, sizeof(bmpInfoHeader), 1, bmpFile);

		//
		// Now convert color to RGB value and invert bitmap.
		//
		unsigned int stride = bmpWidth * 4;

		unsigned char * buffer = new unsigned char [bmpSize * 3];
		unsigned char * copyto = buffer;
		unsigned char * copyfrom = (unsigned char *)pDib->pImage + bmpWidth * (bmpHeight - 1) * 4;
		unsigned char * placeholder = copyfrom;

		for (int row =  0; row <= bmpHeight-1; row++)
		{
			for (int column = 0; column <= bmpWidth - 1; column++)
			{
				int color = *((int *)copyfrom);

				copyto[2] = (color & 0xf800) >> 8;
				copyto[1] = (color & 0x7e0) >> 3;
				copyto[0] = (color & 0x1f) << 3;

				copyto += 3;
				copyfrom += 4;
			}
			placeholder -= stride;
			copyfrom = placeholder;
		}

		//  Write bitmap image to file.
		fwrite(buffer, bmpSize * 3, 1, bmpFile);

		fclose(bmpFile);  //Close file.

		UGL_FREE(pDib->pImage);
		UGL_FREE(pDib);

		delete buffer;

		retVal = 0;
	}
	else
	{
		DataLog( log_level_cgui_error ) << "Off screen bitmap error. Could not open file:'" << filename << "'" << endmsg;
	}

	// Shouldn't need if running on a taos machine.
	uglCursorOn(_uglDisplay);  //Reactivate mouse.

	return retVal;
}
*/

UGL_DIB * CGUIDisplay::offscreenFlush(void)
{
	//
	//  Put current screen image information
	//  into a dib structure. (Device Independent Bitmap)
	//
	UGL_DIB *pDib     = (UGL_DIB *)UGL_MALLOC(sizeof(UGL_DIB));
	pDib->width       = _width;
	pDib->height      = _height;
	pDib->stride      = _width;
	pDib->colorFormat = UGL_DEVICE_COLOR_32; //ARGB8888;//DEVICE_COLOR_32;
	pDib->imageFormat = UGL_DIRECT;
	pDib->clutSize    = UGL_NULL;
	pDib->pClut       = NULL;
	pDib->pImage      = UGL_MALLOC( pDib->width * pDib->height * 4 );

	if( drawBitMap == UGL_NULL_ID )
	{
		// Create an offscreen bitmap and graphics context
		uglOffscreenGc = uglGcCreate( _uglDisplay );
		uglGcCopy( _uglGc, uglOffscreenGc );
		drawBitMap = uglBitmapCreate(_uglDisplay, pDib, UGL_DIB_INIT_DATA, 0, UGL_NULL);
	}

	// Enable the offscreen bitmap
	offscreenBitMap = drawBitMap;

	// Draw the screen
	list<CGUIWindow *>::iterator windowIter = _windowList.begin();
	while (windowIter != _windowList.end())
	{
		(*windowIter)->draw( uglOffscreenGc );
		++windowIter;
	}

	// Disable the offscreen bitmap
	offscreenBitMap = UGL_NULL_ID;

	// Read the offscreen bitmap into a DIB
	uglBitmapRead(_uglDisplay, drawBitMap, 0, 0, _width-1, _height-1, pDib, 0, 0);

	return pDib;
}

/*
	int retVal = -1;

	uglCursorOff(_uglDisplay);   // Remove mouse from display for snapshot.

	typedef unsigned char  BMP_BYTE;
	typedef unsigned short BMP_WORD;
	typedef unsigned long  BMP_DWORD;

	const int bmpWidth = 800;
	const int bmpHeight = 600;

	struct bmepsoe
	{
		Output_Encoder * oeP;
		int * rleBuffer;
		Byte * flateInBuffer;
		Byte * flateOutBuffer;
	};

	struct BITMAPFILEHEADER
	{
		BMP_WORD    bfType;
		BMP_DWORD   bfSize;
		BMP_WORD    bfReserved1;
		BMP_WORD    bfReserved2;
		BMP_DWORD   bfOffBits;
	} __attribute__((packed));

	struct BITMAPINFOHEADER
	{
		BMP_DWORD   biSize;
		BMP_DWORD   biWidth;
		BMP_DWORD   biHeight;
		BMP_WORD    biPlanes;
		BMP_WORD    biBitCount;
		BMP_DWORD   biCompression;
		BMP_DWORD   biSizeImage;
		BMP_DWORD   biXPelsPerMeter;
		BMP_DWORD   biYPelsPerMeter;
		BMP_DWORD   biClrUsed;
		BMP_DWORD   biClrImportant;
	} __attribute__((packed));

	BITMAPFILEHEADER bmpFileHeader;
	BITMAPINFOHEADER bmpInfoHeader;

	size_t bmpSize = bmpWidth * bmpHeight;

	//
	//  Set fileheader information.
	//
	bmpFileHeader.bfType      = 0x4D42;
	bmpFileHeader.bfSize      = sizeof(bmpFileHeader) + sizeof(bmpInfoHeader) + bmpSize * 3;
	bmpFileHeader.bfReserved1 = 0x0000;
	bmpFileHeader.bfReserved2 = 0x0000;
	bmpFileHeader.bfOffBits   = sizeof(bmpFileHeader) + sizeof(bmpInfoHeader);

	bmpInfoHeader.biSize          = sizeof(bmpInfoHeader);
	bmpInfoHeader.biWidth         = bmpWidth;
	bmpInfoHeader.biHeight        = bmpHeight;
	bmpInfoHeader.biPlanes        = 1;
	bmpInfoHeader.biBitCount      = 24;
	bmpInfoHeader.biCompression   = 0;
	bmpInfoHeader.biSizeImage     = bmpSize * 3;
	bmpInfoHeader.biXPelsPerMeter = 0x0EC4;
	bmpInfoHeader.biYPelsPerMeter = 0x0EC4;
	bmpInfoHeader.biClrUsed       = 0;
	bmpInfoHeader.biClrImportant  = 0;

	//
	//  Put current screen image information
	//  into a dib structure. (Device Independent Bitmap)
	//
	UGL_DIB *pDib     = (UGL_DIB *)UGL_MALLOC(sizeof(UGL_DIB));
	pDib->width       = bmpWidth;
	pDib->height      = bmpHeight;
	pDib->stride      = bmpWidth;
	pDib->colorFormat = UGL_DEVICE_COLOR_32; //ARGB8888;//DEVICE_COLOR_32;
	pDib->imageFormat = UGL_DIRECT;
	pDib->clutSize    = UGL_NULL;
	pDib->pClut       = NULL;
	pDib->pImage      = UGL_MALLOC( bmpSize * 4 );

	if( drawBitMap == UGL_NULL_ID )
	{
		// Create an offscreen bitmap and graphics context
		uglOffscreenGc = uglGcCreate( _uglDisplay );
		uglGcCopy( _uglGc, uglOffscreenGc );
		drawBitMap = uglBitmapCreate(_uglDisplay, pDib, UGL_DIB_INIT_DATA, 0, UGL_NULL);
	}

	// Enable the offscreen bitmap
	offscreenBitMap = drawBitMap;

	// Draw the screen
	list<CGUIWindow *>::iterator windowIter = _windowList.begin();
	while (windowIter != _windowList.end())
	{
		(*windowIter)->draw( uglOffscreenGc );
		++windowIter;
		//DataLog( log_level_cgui_info ) << "Drawing window." << endmsg;
	}

	// Disable the offscreen bitmap
	offscreenBitMap = UGL_NULL_ID;

	// Read the offscreen bitmap into a DIB
	uglBitmapRead(_uglDisplay, drawBitMap, 0, 0, bmpWidth-1, bmpHeight-1, pDib, 0, 0);

	//
	//  Create and open the bmp file.
	//
	FILE * psFile = fopen(filename, "wb");

	if( psFile != NULL )
	{
		fprintf(psFile, "%%!PS-Adobe-2.0 EPSF-2.0\n");
		fprintf(psFile, "%%%%Creator: pnmtops\n");
		fprintf(psFile, "%%%%Title: pnmScreen.ps\n");
		fprintf(psFile, "%%%%Pages: 1\n");
		fprintf(psFile, "%%%%BoundingBox: 9 0 603 792\n");
		fprintf(psFile, "%%%%EndComments\n");
		fprintf(psFile, "%%%%Page: Image0 1\n");
		fprintf(psFile, "/picstr\n");
		fprintf(psFile, "   2400 string\n");
		fprintf(psFile, "def\n");
		fprintf(psFile, "   /inputf\n");
		fprintf(psFile, "   currentfile\n");
		fprintf(psFile, "   /ASCII85Decode filter\n");
		fprintf(psFile, "   /FlateDecode filter\n");
		fprintf(psFile, "def\n");
		fprintf(psFile, "%%%%EndProlog\n");
		fprintf(psFile, "%%%%Page: 1 1\n");
		fprintf(psFile, "gsave\n");
		fprintf(psFile, "9 0 translate\n");
		fprintf(psFile, "594 792 scale\n");
		fprintf(psFile, "0.5 0.5 translate  90 rotate  -0.5 -0.5 translate\n");
		fprintf(psFile, "800 600 8\n");
		fprintf(psFile, "[ 800 0 0 -600 0 600 ]\n");
		fprintf(psFile, "{ inputf picstr readstring pop }\n");
		fprintf(psFile, "false 3\n");
		fprintf(psFile, "colorimage\n");

		unsigned int stride = bmpWidth * 4;
		const char* const hexits = "0123456789abcdef";
		unsigned char * copyfrom = (unsigned char *)pDib->pImage;;
		unsigned char * placeholder = copyfrom;
		int color;
		struct bmepsoe *bmepsoeP;
		unsigned int const FLATE_IN_SIZE = 16384;
		unsigned int const FLATE_OUT_SIZE = 17408;
		int mode;

		bmepsoeP = (struct bmepsoe *)malloc(sizeof(struct bmepsoe));
		bmepsoeP->oeP = (Output_Encoder *)malloc(sizeof(Output_Encoder));
		bmepsoeP->rleBuffer = (int *)malloc(129);
		bmepsoeP->flateInBuffer = (Byte *)malloc(FLATE_IN_SIZE);
		bmepsoeP->flateOutBuffer = (Byte *)malloc(FLATE_OUT_SIZE);

		mode = OE_FLATE | OE_ASC85;

		oe_init(bmepsoeP->oeP, psFile, mode, 9, bmepsoeP->rleBuffer, bmepsoeP->flateInBuffer, FLATE_IN_SIZE,
				  bmepsoeP->flateOutBuffer, FLATE_OUT_SIZE);

		for (int row=0; row<bmpHeight; row++)
		{
			for (int column=0; column<bmpWidth; column++)
			{
				color = *((int *)copyfrom);

				oe_byte_add(bmepsoeP->oeP, (unsigned char)((color & 0xf800) >> 8));
				oe_byte_add(bmepsoeP->oeP, (unsigned char)((color & 0x7e0) >> 3));
				oe_byte_add(bmepsoeP->oeP, (unsigned char)((color & 0x1f) << 3));

				copyfrom += 4;
			}
			placeholder += stride;
			copyfrom = placeholder;
		}

		oe_byte_flush(bmepsoeP->oeP);

		free(bmepsoeP->rleBuffer);
		free(bmepsoeP->flateInBuffer);
		free(bmepsoeP->flateOutBuffer);
		free(bmepsoeP);

		fprintf(psFile, "\n~>\n");
		fprintf(psFile, "grestore\n" );
		fprintf(psFile, "showpage\n" );
		fprintf(psFile, "%%%%Trailer\n" );

		fclose(psFile);  //Close file.

		UGL_FREE(pDib->pImage);
		UGL_FREE(pDib);

		retVal = 0;
	}
	else
	{
		DataLog( log_level_cgui_error ) << "Off screen bitmap error. Could not open file:'" << filename << "'" << endmsg;
	}

	// Shouldn't need if running on a taos machine.
	uglCursorOn(_uglDisplay);  //Reactivate mouse.

	return retVal;
}
*/
CGUIFontId CGUIDisplay::createFont(const char * familyName, unsigned char pixelSize)
{
   static const char fontStringTemplate[] = "familyName=%s; pixelSize=%d";
   char * fontString = new char[strlen(familyName)+sizeof(fontStringTemplate)+3];
   UGL_FONT_DEF fontDef;
   CGUIFontId font;

   sprintf(fontString, fontStringTemplate, familyName, pixelSize);
   uglFontFindString(_uglFontDriver, fontString, &fontDef);
   fontDef.pixelSize = pixelSize;
   font = uglFontCreate(_uglFontDriver, &fontDef);

   delete[] fontString;
   return font;
}


void CGUIDisplay::deleteFont(CGUIFontId font)
{
   uglFontDestroy(font);
}

void CGUIDisplay::cursorInit(void)
{
   uglCursorInit(_uglDisplay, 20, 20, _width/2, _height/2);

#if CPU==SIMNT

   UGL_CDIB pointerDib;

   /* bitmap of cursor image */
   static UGL_UINT8 pointerData[] =
   {
#define T UGL_CURSOR_COLOR_TRANSPARENT,
#define B 0,
#define W 1,
      B T T T T T T T T T T
      B B T T T T T T T T T
      B W B T T T T T T T T
      B W W B T T T T T T T
      B W W W B T T T T T T
      B W W W W B T T T T T
      B W W W W W B T T T T
      B W W W W W W B T T T
      B W W W W W W W B T T
      B W W W W W W W W B T
      B W W W W W B B B B B
      B W W B W W B T T T T
      B W B T B W W B T T T
      B B T T B W W B T T T
      B T T T T B W W B T T
      T T T T T B W W B T T
      T T T T T T B W W B T
      T T T T T T B W W B T
      T T T T T T T B B T T
#undef T
#undef B
#undef W
   };

   /* color of cursor */
   static UGL_ARGB cursorClut[] =
   {
      UGL_MAKE_ARGB (0xff,0x00, 0x00, 0x00),
      UGL_MAKE_ARGB (0xff,0xFF, 0xFF, 0xFF)
   };


   pointerDib.width = 11;
   pointerDib.stride = 11;
   pointerDib.height = 19;
   pointerDib.hotSpot.x = 0;
   pointerDib.hotSpot.y = 0;
   pointerDib.pClut = cursorClut;
   pointerDib.clutSize = 2;
   pointerDib.pImage = pointerData;

   UGL_CDDB_ID pointerImage = uglCursorBitmapCreate(_uglDisplay, &pointerDib);
   uglCursorImageSet(_uglDisplay, pointerImage);
   uglCursorOn(_uglDisplay);

#endif /* if CPU==SIMNT */

}


void CGUIDisplay::drawRootWindow(void)
{
   if (_uglRootWindow != UGL_NULL_ID)
   {
      winDrawStart(_uglRootWindow, _uglGc, true);
      uglLineWidthSet(_uglGc, 0);
      uglBackgroundColorSet(_uglGc, 0);
      uglRectangle(_uglGc, 0, 0, _width-1, _height-1);
      winDrawEnd(_uglRootWindow, _uglGc, true);
   }
}

void CGUIDisplay::setCursorPos(int x, int y)
{
   uglCursorMove (_uglDisplay, x, y);
}


void CGUIDisplay::getCursorPos(int &x, int &y)
{
   uglCursorPositionGet (_uglDisplay, &x, &y);
}

void CGUIDisplay::setPaletteColor(CGUIColor index, const CGUIPaletteEntry & entry)
{
	//
	// Free color first, in case it has already been allocated.  If not allocated,
	// the function will simply do nothing and return an error status.
	//
	UGL_COLOR	uglColor = index;
	uglColorFree(_uglDisplay, &uglColor, 1);

//    printf("Old Palette Entry %d  Red - %d,  Green - %d,  Blue - %d\n ", index, entry.red, entry.green, entry.blue);
	//
	// Allocate the color for the associated palette entry
	//
	UGL_ARGB uglARGB = UGL_MAKE_RGB(entry.red, entry.green, entry.blue);
	UGL_ORD	uglColorIndex = index;
	uglColorAlloc(_uglDisplay, &uglARGB, &uglColorIndex, &uglColor, 1);
    uglClutSet(_uglDisplay, uglColorIndex, &uglARGB, 1);

#if CPU==SIMNT
	deviceClut[index] = uglARGB;
#endif

//    UGL_ARGB newARGB; // = UGL_MAKE_RGB(entry.red, entry.green, entry.blue);

//    int red = UGL_ARGB_RED(uglARGB);
//    int green = UGL_ARGB_GREEN(uglARGB);
//    int blue = UGL_ARGB_BLUE(uglARGB);

//    printf("UGL_RGB Palette Entry %d  Red - %d,  Green - %d,  Blue - %d\n ", index, red, green, blue);

//    uglClutGet(_uglDisplay, uglColorIndex, &newARGB, 1);

//    red = UGL_ARGB_RED(newARGB);
//    green = UGL_ARGB_GREEN(newARGB);
//    blue = UGL_ARGB_BLUE(newARGB);

//    printf("New Palette Entry %d  Red - %d,  Green - %d,  Blue - %d\n\n ", index, red, green, blue);
}

void CGUIDisplay::getPaletteColor(CGUIColor index, CGUIPaletteEntry * entry)
{
   UGL_COLOR	uglColor = index;
   UGL_ARGB uglARGB; // = UGL_MAKE_RGB(entry.red, entry.green, entry.blue);
   UGL_ORD	uglColorIndex = index;
   uglClutGet(_uglDisplay, uglColorIndex, &uglARGB, 1);

//   printf("Old Palette Entry %d  Red - %d,  Green - %d,  Blue - %d\n ", index, entry->red, entry->green, entry->blue);

//   int red = UGL_ARGB_RED(uglARGB);
//   int green = UGL_ARGB_GREEN(uglARGB);
//   int blue = UGL_ARGB_BLUE(uglARGB);

//   printf("New Palette Entry %d  Red - %d,  Green - %d,  Blue - %d\n\n ", index, red, green, blue);
}

void CGUIRegion::convertToUGLRect(UGL_RECT & rect) const
{
   rect.left = x;
   rect.top = y;
   rect.right = x + width - 1;
   rect.bottom = y + height - 1;
}

