/*
 *	Copyright (c) 2004 by Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: K:/BCT_Development/vxWorks/Common/cgui/rcs/cgui_window.h 1.6 2005/02/21 17:17:13Z cf10242 Exp psanusb $
 * $Log: cgui_window.h $
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
   void initializeData(void);
   void addObject(CGUIWindowObject * obj);
   void addChildWindow(CGUIWindow * child); 
   void deleteChildWindow(CGUIWindow * child);
   void preDrawObjects(void);
   void drawObjects(UGL_GC_ID gc);

protected:
   UGL_WINDOW_ID  _id;

private:
   CGUIDisplay &  _display;

   CGUIRegion     _region;
   bool           _disabled;

   UGL_REGION_ID  _activeDrawRegion;

   CGUIText::StylingRecord  _defaultTextStylingRecord;

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
   list<CGUIWindowObject *> _nonClippedObjects;
};

#endif /* #ifndef _CGUI_WINDOW_INCLUDE */
