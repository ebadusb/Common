/*
 *	Copyright (c) 2004 by Gambro BCT, Inc.  All rights reserved.
 *
 * $Header$
 * $Log: cgui_window.h $
 * Revision 1.12  2007/04/20 14:58:26Z  wms10235
 * IT2354 - Removing function bringToTop() since it is a duplicate of raiseScreenPriority()
 * Revision 1.11  2007/04/18 16:28:30Z  wms10235
 * IT2354 - Added screen/display functions for reports
 * Revision 1.10  2006/06/17 17:42:32Z  cf10242
 * IT 52: insure button release event is sent to window even if disabled
 * Revision 1.9  2005/08/11 16:26:13Z  cf10242
 * TAOS IT 674 - ungrab when deleting to prevent page fault
 * Revision 1.8  2005/04/12 18:13:40Z  cf10242
 * remove changes from 1.7
 * Revision 1.6  2005/02/21 17:17:13Z  cf10242
 * IT 133 - delete all allocated memory to avoid unrecovered memory
 * Revision 1.5  2005/01/28 23:52:19Z  rm10919
 * CGUITextItem class changed and put into own file.
 * Revision 1.4  2004/11/04 20:19:09Z  rm10919
 * Common updates and changes.
 * Revision 1.3  2004/10/22 20:14:35Z  rm10919
 * CGUIButton updates and changes.
 * Revision 1.2  2004/09/30 17:00:53Z  cf10242
 * Correct for initial make to work
 * Revision 1.1  2004/09/20 18:18:10Z  rm10919
 * Initial revision
 *
 */

#ifndef _CGUI_WINDOW_INCLUDE
#define _CGUI_WINDOW_INCLUDE


#include <vxworks.h>
#include <ugl/ugl.h>
#include <ugl/uglwin.h>
#include <ugl/uglinput.h>

#include "cgui_graphics.h"
#include "cgui_text.h"
#include "datalog_levels.h"

class CGUIText;
class CGUIWindowObject;

//
// The CGUIWindow class provides support for a rectangular window.  This
// window can contain bitmaps, display text, filled graphic shapes
// and/or other (child) windows.
//

class CGUIWindow
{
public:
   unsigned long _guardTop;
   CGUIWindow(CGUIDisplay & display);
   CGUIWindow(CGUIDisplay & display, const CGUIRegion & region);
   virtual ~CGUIWindow();

   WIN_ID winIdGet(CGUIWindow * window) const {return window->_id;}

   void setPosition(short x, short y);
   void setRegion(const CGUIRegion & newRegion);
   void getRegion(CGUIRegion & region) const { region = _region;}

   short top(void) const { return _region.y;}
   short left(void) const { return _region.x;}
   unsigned short width(void) const { return _region.width;}
   unsigned short height(void) const { return _region.height;}

   void attach(CGUIWindow * window, WIN_ATTRIB winAttrib = WIN_ATTRIB_VISIBLE);
   void detach(void);

   void addObjectToFront (CGUIWindowObject * obj);
   void addObjectToBack  (CGUIWindowObject * obj);
   void deleteObject     (CGUIWindowObject * obj);
   void moveObjectToFront(CGUIWindowObject * obj);
   void moveObjectToBack (CGUIWindowObject * obj);
   void setObjectRegion  (CGUIWindowObject * obj, const CGUIRegion & newRegion);
   void setObjectVisible (CGUIWindowObject * obj, bool newVisible);
   void invalidateObjectRegion(CGUIWindowObject * obj);

   void setWindowVisibility(bool newVisible);
   bool getWindowVisibility(void){ return winVisibleGet(_id);}

	void invalidateWindow(void);

   virtual void setDisabled(bool newDisabled) { _disabled = newDisabled;}
   virtual bool disabled(void);

   virtual void draw(void);
   virtual void draw(UGL_GC_ID gc);

   static UGL_STATUS uglDrawCallback (WIN_ID id, WIN_MSG * pMsg, void * pData, void * pParam);
   static UGL_STATUS uglPointerCallback (WIN_ID id, WIN_MSG * pMsg, void * pData, void * pParam);

   struct PointerEvent
   {
      enum EventType
      {
         ButtonPress, ButtonRelease
      };
      EventType   eventType;
      short       x, y;
   };

   virtual void pointerEvent(const PointerEvent & event);

private:
	CGUIWindow();
	CGUIWindow (const CGUIWindow &obj);
	CGUIWindow operator=(CGUIWindow &obj);
   void initializeData(void);
   void addObject(CGUIWindowObject * obj);
   void addChildWindow(CGUIWindow * child);
   void deleteChildWindow(CGUIWindow * child);
   void preDrawObjects(void);
   void drawObjects(UGL_GC_ID gc);

protected:
   UGL_WINDOW_ID  _id;
   CGUIDisplay &  _display;

private:

   CGUIRegion     _region;
   bool           _disabled;
   static bool           _needRelease;

   UGL_REGION_ID  _activeDrawRegion;

   StylingRecord  _defaultTextStylingRecord;

   CGUIWindow *   _parent;
   list<CGUIWindow *>  _childWindows;

   //
   // Window objects for each window are separated into two lists.  The first
   // list contains window objects for which clipSiblings() is true (e.g.
   // bitmaps).  The list is sorted by z-order, with the first element at the
   // top of the z-order.  These objects must be drawn in order, with clipping
   // applied to insure overlapping objects are drawn correctly.
   //
   // The second list contains window objects for which clipSiblings() is false
   // (e.g. text).  These objects are drawn after all objects in the
   // clippedObjects list are drawn, and so effectively stay at the top of
   // the z-order.
   //
   // In addition to z-order, each child window object has a visible flag used
   // to determine if the object should be drawn.
   //
   list<CGUIWindowObject *> _clippedObjects;
   list<CGUIWindowObject *> _transparencyObjects;
   list<CGUIWindowObject *> _nonClippedObjects;
   unsigned long _guardBottom;
};

#endif /* #ifndef _CGUI_WINDOW_INCLUDE */
