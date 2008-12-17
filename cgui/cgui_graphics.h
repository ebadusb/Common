/*
 *	Copyright (c) 2004 by Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: J:/BCT_Development/vxWorks/Common/cgui/rcs/cgui_graphics.h 1.19 2007/06/04 22:04:20Z wms10235 Exp $
 * $Log: cgui_graphics.h $
 * Revision 1.19  2007/06/04 22:04:20Z  wms10235
 * IT83 - Updates for the common GUI project to use the unicode string class
 * Revision 1.18  2007/05/10 16:35:46Z  jl11312
 * - add option to specify timeout in waiting for UGL events in message loop (Taos IT 3315)
 * Revision 1.17  2007/04/10 20:47:34Z  adalusb
 * IT2354 - Fixed prototype
 * Revision 1.16  2007/04/10 15:12:02Z  wms10235
 * IT2354 - Removing postscript generation
 * Revision 1.15  2007/04/09 21:12:31Z  wms10235
 * IT2354 - Added postscript save capability
 * Revision 1.14  2007/04/05 18:39:38Z  wms10235
 * IT2354 - Added a preliminary version of the off-screen flush
 * Revision 1.13  2006/05/15 21:51:42Z  rm10919
 * Fix memory bug in convertToStringChar and handle trima palette.
 * Revision 1.12  2005/04/26 23:16:47Z  rm10919
 * Made changes to cgui_text and cgui_text_item, plus added
 * classes for variable substitution in text strings.
 * Revision 1.11  2005/01/28 23:52:17Z  rm10919
 * CGUITextItem class changed and put into own file.
 * Revision 1.10  2005/01/12 20:06:09Z  rm10919
 * Deleted convertToAscii().
 * Revision 1.9  2004/12/27 22:48:51Z  rm10919
 * Added CGUITextItem::getAscii for use with sprintf().
 * Revision 1.8  2004/11/19 18:14:47Z  cf10242
 * Integration checkin
 * Revision 1.7  2004/11/12 14:53:11Z  rm10919
 * Modify some TextItem class rountines.
 * Revision 1.6  2004/11/01 17:27:21Z  cf10242
 * Change TextItem to CGUITextItem
 * Revision 1.5  2004/10/29 15:11:14Z  rm10919
 * Revision 1.4  2004/10/07 22:47:02Z  cf10242
 * Revision 1.3  2004/10/07 18:52:19Z  cf10242
 * Revision 1.2  2004/09/30 17:00:52Z  cf10242
 * Correct for initial make to work
 * Revision 1.1  2004/09/20 18:18:06Z  rm10919
 * Initial revision
 *
 */

#ifndef _CGUI_GRAPHICS_INCLUDE
#define _CGUI_GRAPHICS_INCLUDE

//
// This file contains the classes used to wrap the underlying graphics
// windowing system.  The intent is to limit dependencies on this graphics
// system to this header file and to the implementation files associated
// with it. All interaction with the graphics system from the application
// should be through the publically accessible functions and data defined
// in this file.
//
#include <vxworks.h>
#include <map>
#include <list>
#include <string>
#include <stdio.h>

#include <ugl/ugl.h>
#include <ugl/uglwin.h>
#include <ugl/uglinput.h>

#include "callback.h"
#include "unicode_string/unicode_string_types.h"

//
// Window Attributes
//
typedef UGL_UINT32 WIN_ATTRIB;

//
// Color (RGB565 format)
//
typedef unsigned short CGUIColor;
inline unsigned short MakeCGUIColor(int red, int green, int blue) { return((red&0xf8)<<8) + ((green&0xfc)<<3) + ((blue&0xf8)>>3);}

//
// Font management
//
typedef UGL_FONT_ID  CGUIFontId;

//
// Bitmap management
//
typedef UGL_DDB_ID   CGUIBitmapId;

//
// Color management
//
// Palette Size
enum { CGUIPaletteSize = 256 };
struct CGUIPaletteEntry
{
	unsigned char	red;
	unsigned char	green;
	unsigned char	blue;
};
#if CPU==SIMNT
extern UGL_ARGB deviceClut[CGUIPaletteSize];
#endif /* if CPU==SIMNT */

enum
{
   MAX_FONTS = 1200
};

//
// String Management

typedef int LanguageId;

extern int currentLanguage;

void convertToStringChar(const char * string, StringChar ** stringChar);

//
// The CGUIDisplay class handles the underlying UGL graphics context
// and event loop.  Currently, only one instance of the CGUIDisplay
// class should be created by the application.  The intent of making
// this a instantiable class is to provide a framework for future
// support for other types of displays such as printers.
//
class CGUIWindow;
class CGUIDisplay
{
public:

   CGUIFontId     _font[MAX_FONTS];

   CGUIDisplay(const CallbackBase & startCB=CallbackBase(), const CallbackBase & wakeupCB=CallbackBase(), unsigned int idleTimeoutMSec = 100);
   virtual ~CGUIDisplay();

   //
   // The event handler must be called by the application to process
   // graphics input events and to update the display.  The flush
   // function can be called to flush pending graphics output to the
   // display.
   //
   void flush(void);

	// The off screen flush function is used to save the screen to an off-screen bitmap
	// then allocate a device independent bitmap to store the resulting screen. Note
	// That the caller is responsible for freeing the memory allocated.
	UGL_DIB * offscreenFlush(void);

   //
   // Font management
   //
   CGUIFontId createFont(const char * familyName, unsigned char pixelSize, unsigned char weight = 0);
   void deleteFont(CGUIFontId font);
   //
   // The top left of the display is fixed at 0,0.  The following
   // functions report the width and height of the display (in pixels).
   //
   short width(void) const { return _width;}
   short height(void) const { return _height;}

   //
   // The CGUIDisplay class maintains a list of active windows for use in
   // the flush() function.
   //
   void addWindow(CGUIWindow * window) { _windowList.push_front(window);}
   void removeWindow(CGUIWindow * window) { _windowList.remove(window);}

   //
   // These functions are declared public to allow access by other
   // classes defined in the osgraphics interface.  They should not
   // be used by application code.
   //
   UGL_DEVICE_ID display(void) const { return _uglDisplay;}
   UGL_GC_ID gc(void) const { return _uglGc;}
   WIN_APP_ID app(void) const { return _uglApp;}
   UGL_WINDOW_ID rootWindow(void) const { return _uglRootWindow;}

   //
   // These functions get and set the cursor position.
   // Only used in Service Mode.
   //
   void setCursorPos(int x, int y);
   void getCursorPos(int &x, int &y);

   void setPaletteColor(CGUIColor index, const CGUIPaletteEntry & entry);
   void getPaletteColor(CGUIColor index, CGUIPaletteEntry * entry);

private:
   void cursorInit(void);
   void drawRootWindow(void);

private:
   UGL_DEVICE_ID        _uglDisplay;
   UGL_INPUT_SERVICE_ID _uglEventService;
   WIN_APP_ID           _uglApp;
   UGL_GC_ID            _uglGc;
	UGL_GC_ID            _uglOffscreenGc;
   UGL_WINDOW_ID        _uglRootWindow;
   UGL_FONT_DRIVER_ID   _uglFontDriver;

   short _width;
   short _height;

   list<CGUIWindow *> _windowList;

   // copy constructor and copy assignment are not legal for this class
   CGUIDisplay (CGUIDisplay &);
   CGUIDisplay& operator= (const CGUIDisplay&);

//   UGL_ARGB colorPallette [256];
};


class CGUIRegion
{
public:
   short x, y;    // top left corner pixel coordinates
   short width;   // width in pixels
   short height;  // height in pixels

   CGUIRegion(void) { x=0; y=0; width=0; height=0;}
   CGUIRegion(short argX, short argY, short argWidth, short argHeight)
   { x = argX; y=argY; width=argWidth; height=argHeight;}

   void horizShift(short horizShift) { x+=horizShift;}
   void vertShift (short vertShift)  { y+=vertShift;}

   void convertToUGLRect(UGL_RECT & rect) const;

private:
   // copy constructor and copy assignment are not legal for this class
//	CGUIRegion (CGUIRegion &);
//	CGUIRegion& operator= (const CGUIRegion&);

};

class CGUIPalette
{
public:
   CGUIPalette(void);
   ~CGUIPalette(void);

   UGL_ARGB colorPallette [256];

   void setPaletteColor(CGUIColor index, const CGUIPaletteEntry & entry);
   void getPaletteColor(int red, int green, int blue);
};

#endif /* #ifndef _CGUI_GRAPHICS_INCLUDE */

