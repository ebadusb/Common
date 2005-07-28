/*
 *	Copyright (c) 2004 by Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: H:/BCT_Development/vxWorks/Common/cgui/rcs/cgui_graphics.cpp 1.22 2006/05/15 21:51:42Z rm10919 Exp wms10235 $
 * $Log: cgui_graphics.cpp $
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
//#include "datalogger.h"

//
// String Management
//  
StringChar * convertToStringChar(const char * string)
{
   if (string)
   {
      int stringLength = strlen(string) + 1;   // add 1 for the NULL

      StringChar * textString = new UGL_WCHAR[stringLength];  

   for (int i=0; i<stringLength; i++)
      textString[i] = string[i];

   return textString;
   }
   else return NULL;
}
            
            
// START MESSAGE_SYSTEM_IN_WIN_MGR
#include "messagesystem.h"
extern void (* winAppStartupTask)(void);
extern void (* winAppIdleTask)(void);
MessageSystem * msgSys = NULL;

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
   if (modeInfo.colorModel != UGL_DIRECT)
   {
      fprintf(stderr, "UGL reports color model %d - expected %d\n", (int)modeInfo.colorModel, (int)UGL_DIRECT);
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


void CGUIRegion::convertToUGLRect(UGL_RECT & rect) const
{
   rect.left = x;
   rect.top = y;
   rect.right = x + width - 1;
   rect.bottom = y + height - 1;
}


