/*
 *	Copyright (c) 2004 by Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: K:/BCT_Development/vxWorks/Common/cgui/rcs/cgui_window_object.h 1.6 2008/11/06 22:24:17Z rm10919 Exp wms10235 $
 * $Log: cgui_window_object.h $
 *
 */

#ifndef _CGUI_WINDOW_OBJECT_INCLUDE
#define _CGUI_WINDOW_OBJECT_INCLUDE


#include <vxworks.h>
#include <ugl/ugl.h>
#include <ugl/uglwin.h>
#include <ugl/uglinput.h>

#include "cgui_graphics.h"

class CGUIWindow;
class CGUIWindowObject
{

   friend class CGUIWindow;
public:
   CGUIWindowObject(CGUIDisplay & display);
   CGUIWindowObject(CGUIDisplay & display, const CGUIRegion & region);
   virtual ~CGUIWindowObject();

   virtual void setPosition(short x, short y);
   virtual void setRegion(const CGUIRegion & newRegion);
   const CGUIRegion & getRegion(void) const { return _region;}

   short top(void) const { return _region.y;}
   short left(void) const { return _region.x;}
   unsigned short width(void) const { return _region.width;}
   unsigned short height(void) const { return _region.height;}

   virtual void setVisible(bool newVisible);
   bool isVisible(void) const { return _visible;}

   virtual void moveToBack(void);
   virtual void moveRoFront(void);

protected:
   virtual void preDraw(void) {}
   virtual void draw(UGL_GC_ID gc) = 0;

private:
   void initializeData(void);
   virtual bool clipSiblings(void) const = 0;

protected:
   CGUIRegion _region;
   bool _visible;

   CGUIDisplay & _display;
   CGUIWindow  * _owner;
};


#endif /* #ifndef _CGUI_WINDOW_OBJECT_INCLUDE */
