/*
 *	Copyright (c) 2004 by Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: L:/vxWorks/Common/cgui/rcs/cgui_graphics.cpp 1.8 2004/10/29 15:11:14Z rm10919 Exp cf10242 $
 * $Log: cgui_graphics.cpp $
 *
 */

#include <vxWorks.h>
#include <ugl/uglInput.h>
#include "zlib.h"

#include "bitmap_info.h"
#include "cgui_graphics.h"
#include "cgui_window.h"
//#include "datalogger.h"

CGUIDisplay::CGUIDisplay(void)
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

   //
   // Get an ID for the display driver, then check that we are in the proper
   // color mode.
   //
   uglDriverFind(UGL_DISPLAY_TYPE, 0, (UGL_UINT32 *)&_uglDisplay);

   UGL_MODE_INFO modeInfo;
   uglInfo(_uglDisplay, UGL_MODE_INFO_REQ, &modeInfo);
   _height = modeInfo.height;
   _width = modeInfo.width;
   if (modeInfo.colorModel != UGL_DIRECT)
   {
      fprintf(stderr, "UGL reports color model %d - expected %d\n", (int)modeInfo.colorModel, (int)UGL_DIRECT);
      taskSuspend(taskIdSelf());
   }

   _uglGc = uglGcCreate(_uglDisplay);

   uglDriverFind(UGL_EVENT_SERVICE_TYPE, 0, (UGL_UINT32 *)&_uglEventService);

   _uglApp = winAppCreate("winApp", 0, 0, 0, UGL_NULL);     

   _uglRootWindow = winCreate(_uglApp, UGL_NULL_ID,         
                              WIN_ATTRIB_VISIBLE,             // list window attributes 
                              0, 0, _width, _height, UGL_NULL, 0, UGL_NULL);

   winDataSet(_uglRootWindow, NULL, NULL, 0);

   winAttach(_uglRootWindow, UGL_NULL_ID, UGL_NULL_ID);

   uglDriverFind(UGL_FONT_ENGINE_TYPE, 0, (UGL_UINT32 *)&_uglFontDriver);

   for (int i=0; i<BITMAP_ID_COUNT; i++)
   {
      _bitmapStatus[i]._uglId = UGL_NULL_ID;
      _bitmapStatus[i]._loadCount = 0;
   }
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


CGUIFontId CGUIDisplay::createFont(const char * familyName, unsigned char pixelSize)
{
   static const char fontStringTemplate[] = "familyName=%s; pixelSize=%d";
   char * fontString = new char[strlen(familyName)+sizeof(fontStringTemplate)+3];
   UGL_FONT_DEF fontDef;
   CGUIFontId font; 

   sprintf(fontString, fontStringTemplate, familyName, pixelSize);
   uglFontFindString(_uglFontDriver, fontString, &fontDef);
   font = uglFontCreate(_uglFontDriver, &fontDef);

   delete[] fontString;
   return font;
}


void CGUIDisplay::deleteFont(CGUIFontId font)
{
   uglFontDestroy(font);
}


OSBitmapId CGUIDisplay::loadBitmap(BITMAP_ID guiId)
{
   OSBitmapId  result = UGL_NULL_ID;

   if (guiId <= BITMAP_NULL || guiId >= BITMAP_ID_COUNT)
   {
      fprintf(stderr, "Invalid bitmap ID %d [%d %d]\n", (int)guiId, (int)BITMAP_NULL, (int)BITMAP_ID_COUNT);
   }
   else if (_bitmapStatus[guiId]._loadCount > 0)
   {
      // bitmap already loaded
      result = _bitmapStatus[guiId]._uglId;
      _bitmapStatus[guiId]._loadCount += 1;
   }
   else if (bitmap_data_table[guiId].dataSize > 0)
   {
      // compressed image data for bitmap is already in memory
      loadBitmapFromCompressedData(guiId);

      result = _bitmapStatus[guiId]._uglId;
      _bitmapStatus[guiId]._loadCount += 1;
   }
   else
   {
      // dynamic file must be loaded at runtime
      loadBitmapFromFile(guiId);

      result = _bitmapStatus[guiId]._uglId;
      _bitmapStatus[guiId]._loadCount += 1;
   }

   if (result == UGL_NULL_ID)
   {
      fprintf(stderr, "Bitmap load failure %d\n", (int)guiId);
   }

   return result;
}


void CGUIDisplay::unloadBitmap(BITMAP_ID guiId)
{
   if (guiId <= BITMAP_NULL || guiId >= BITMAP_ID_COUNT)
   {
      // requested bitmap does not exist
      fprintf(stderr, "Invalid bitmap ID %d [%d %d]\n", (int)guiId, (int)BITMAP_NULL, (int)BITMAP_ID_COUNT);
   }
   else
   {
      if (_bitmapStatus[guiId]._loadCount > 0)
      {
         _bitmapStatus[guiId]._loadCount -= 1;
      }

      if (_bitmapStatus[guiId]._loadCount == 0 && _bitmapStatus[guiId]._uglId != UGL_NULL_ID)
      {
         uglBitmapDestroy(_uglDisplay, _bitmapStatus[guiId]._uglId);
         _bitmapStatus[guiId]._uglId = UGL_NULL_ID;
      }
   }
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


void CGUIDisplay::loadBitmapFromCompressedData(BITMAP_ID guiId)
{
   unsigned long   bmpSize = bitmap_data_table[guiId].height * bitmap_data_table[guiId].width * sizeof(OSColor);
   unsigned char * bmpImage = new unsigned char[bmpSize];

   uncompress(bmpImage, &bmpSize, bitmap_data_table[guiId].data, bitmap_data_table[guiId].dataSize);

   UGL_DIB dib;
   dib.height = bitmap_data_table[guiId].height;
   dib.width = bitmap_data_table[guiId].width;
   dib.stride = bitmap_data_table[guiId].width;
   dib.pImage = (void *)bmpImage;

#if CPU==SIMNT

   dib.clutSize = 0;
   dib.pClut = NULL;

   dib.colorFormat = UGL_DEVICE_COLOR;
   dib.imageFormat = UGL_DIRECT;

#else /* if CPU==SIMNT */

   dib.clutSize = 0;
   dib.pClut = NULL;

   dib.colorFormat = UGL_DEVICE_COLOR;
   dib.imageFormat = UGL_DIRECT;

#endif /* if CPU==SIMNT */

   _bitmapStatus[guiId]._uglId = uglBitmapCreate(_uglDisplay, &dib, UGL_DIB_INIT_DATA, 0, UGL_DEFAULT_MEM);
   delete[] bmpImage;
}


void CGUIDisplay::loadBitmapFromFile(BITMAP_ID guiId)
{
   typedef unsigned char    BMP_BYTE;
   typedef unsigned short   BMP_WORD;
   typedef unsigned long    BMP_DWORD;

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
      BMP_WORD    biWidth;
      BMP_WORD    biHeight;
      BMP_WORD    biPlanes;
      BMP_WORD    biBitCount;
   } __attribute__((packed));

   BITMAPFILEHEADER bmpFileHeader;
   BITMAPINFOHEADER bmpInfoHeader;

   /* Open the bmp file. */
   FILE * bmpFile = fopen((char *)bitmap_data_table[guiId].data, "rb");
   if (!bmpFile) return;

   int readSize = fread((void *)&bmpFileHeader, sizeof(bmpFileHeader), 1, bmpFile);
   if (readSize != 1 ||
       bmpFileHeader.bfType != 0x4d42)
   {
      fclose(bmpFile);
      return;
   }

   fread((void *)&bmpInfoHeader, sizeof(bmpInfoHeader), 1, bmpFile);
   if (bmpInfoHeader.biBitCount != 8)
   {
      fclose(bmpFile);
      return;
   }

   size_t   bmpSize = 2*bmpInfoHeader.biWidth * bmpInfoHeader.biHeight;
   UGL_DIB *pDib = (UGL_DIB *)UGL_MALLOC(sizeof(UGL_DIB)+bmpSize);
   pDib->width = bmpInfoHeader.biWidth;
   pDib->height = bmpInfoHeader.biHeight;
   pDib->stride = bmpInfoHeader.biWidth;

   pDib->colorFormat = UGL_DEVICE_COLOR;
   pDib->imageFormat = UGL_DIRECT;
   pDib->clutSize = 0;
   pDib->pClut = NULL;
   pDib->pImage = (UGL_UINT8 *)pDib + sizeof(UGL_DIB);

   //
   // BMP data is aligned on 32-bit words and is upside down
   // compared to WindMl data
   //
   unsigned int stride = (bmpInfoHeader.biWidth + 3) / 4 * 4;
   BMP_WORD * pPix = (BMP_WORD *)pDib->pImage;
   pPix += bmpInfoHeader.biWidth * (bmpInfoHeader.biHeight - 1);

   fseek(bmpFile, bmpFileHeader.bfOffBits, SEEK_SET);
   for (unsigned int y = 1; y <= bmpInfoHeader.biHeight; y++)
   {
      unsigned char  rgbData[3];
      for (unsigned int x = 0; x < bmpInfoHeader.biWidth; x++)
      {
         /* read RGB data (1 byte for each color) and convert to RGB565 format */
         fread((void *)rgbData, 3, 1, bmpFile);
         pPix[x] = (rgbData[2] & 0xf8) << 8;
         pPix[x] |= (rgbData[1] & 0xfc) << 3;
         pPix[x] |= (rgbData[0] & 0xf8) >> 3;
      }

      if (stride > bmpInfoHeader.biWidth)
      {
         fseek(bmpFile, stride-bmpInfoHeader.biWidth, SEEK_CUR);
      }

      pPix -= bmpInfoHeader.biWidth;
   }

   fclose(bmpFile);

   _bitmapStatus[guiId]._uglId = uglBitmapCreate(_uglDisplay, pDib, UGL_DIB_INIT_DATA, 0, UGL_DEFAULT_MEM);
   UGL_FREE(pDib);
}


void CGUIDisplay::setCursorPos(int x, int y)
{
   uglCursorMove (_uglDisplay, x, y);
}


void CGUIDisplay::getCursorPos(int &x, int &y)
{
   uglCursorPositionGet (_uglDisplay, &x, &y);
}


void OSRegion::convertToUglRect(UGL_RECT & rect) const
{
   rect.left = x;
   rect.top = y;
   rect.right = x + width - 1;
   rect.bottom = y + height - 1;
}


int TextItem::_defaultLanguageId = 0;

TextItem::TextItem()
:_id(NULL), _string(NULL)
{
}

TextItem::TextItem(const char * id)
:_id(id), _string(NULL)
{
}

TextItem::~ TextItem()
{
   delete _id;
   delete _string;
}

const StringChar * TextItem::getText(LanguageId languageId = currentLanguage)
{
   const StringChar * string;
   StringChar * stringNonsense;

   string = (StringChar *) "\x41\x00\x63\x00\x63\x00\x65\x00\x73\x00" "\x73\x00\x20\x00\x50\x00\x72\x00\x65\x00" "\x73\x00\x73\x00\x75\x00\x72\x00\x65\x00\x00";

   if (string)
   {
      int length = 0;
      while (string[length] != '\0')
      {
         length += 1;
      }

      stringNonsense = new UGL_WCHAR [length+1];
      memcpy(stringNonsense, string, length * sizeof(StringChar));
   }
   
   _languageId = languageId;
   _string = stringNonsense;

   return stringNonsense;
}


void TextItem::setId(const char * id)
{
}

bool TextItem::isInitialized(void)
{
   if (_id)
   {
      return true;
   }
   else
   {
      return false;
   }
}
