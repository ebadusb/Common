/*
 *	Copyright (c) 2004 by Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: K:/BCT_Development/vxWorks/Common/cgui/rcs/cgui_window.cpp 1.9 2005/03/02 01:37:51Z cf10242 Exp psanusb $
 * $Log: cgui_window.cpp $
 * Revision 1.1  2004/09/20 18:18:09Z  rm10919
 * Initial revision
 *
 */

#include <vxWorks.h>
#include "cgui_graphics.h"
#include "cgui_window.h"
#include "cgui_window_object.h"


CGUIWindow::CGUIWindow(CGUIDisplay & display)
: _display(display)
{
   initializeData();
}

CGUIWindow::CGUIWindow(CGUIDisplay & display, const CGUIRegion & region)
: _display(display)
{
   initializeData();
   _region = region;
}

void CGUIWindow::initializeData(void)
{
   _id = UGL_NULL_ID;
   _parent = NULL;
   _disabled = false;
   _activeDrawRegion = UGL_NULL_ID;

   //
   // Default styling information for text in the window
   //
   _defaultTextStylingRecord.color = UGL_NULL;   // If a set of standard colors is defined, set to an actual color????
   _defaultTextStylingRecord.attributes = 0x0000;
   _defaultTextStylingRecord.region = CGUIRegion(0, 0, 10, 10);     
   _defaultTextStylingRecord.fontSize = 14;   
   _defaultTextStylingRecord.fontId = UGL_NULL_ID;     
}

CGUIWindow::~CGUIWindow()
{
   //
   // Clear _owner reference for any objects contained in this window, since
   // this reference will no longer be valid.
   //
   list<CGUIWindowObject *>::iterator objIter;
   for (objIter = _clippedObjects.begin(); objIter != _clippedObjects.end(); ++objIter)
   {
      (*objIter)->_owner = NULL;
   }

   for (objIter = _nonClippedObjects.begin(); objIter != _nonClippedObjects.end(); ++objIter)
   {
      (*objIter)->_owner = NULL;
   }

   //
   // Delete the underlying UGL resources for this window.
   //
   detach();
}

void CGUIWindow::setPosition(short x, short y)
{
   if (x != _region.x ||
       y != _region.y)
   {
      CGUIRegion newRegion(x, y, _region.width, _region.height);
      CGUIWindow::setRegion(newRegion);
   }
}

void CGUIWindow::setRegion(const CGUIRegion & newRegion)
{
   _region = newRegion;
   if (_id != UGL_NULL_ID)
   {
      UGL_RECT rect;
      _region.convertToUGLRect(rect);
      winRectSet(_id, &rect);
   }
}


void CGUIWindow::attach(CGUIWindow * window, WIN_ATTRIB winAttrib /* = WIN_ATTRIB_VISIBLE*/)
{
   if (_parent != window ||
       _id == UGL_NULL_ID)
   {
      //
      // Make sure this window has detached from any previous parent
      //
      detach();

      //
      // Create the underlying UGL window
      //
      void * data = (void *) this;
      _id = winCreate(_display.app(), UGL_NULL, winAttrib, 
                      _region.x, _region.y, _region.width, 
                      _region.height, (void *)&data, sizeof(void *), UGL_NULL);

      winCbAdd(_id, MSG_DRAW, 0, CGUIWindow::uglDrawCallback, UGL_NULL);

      //
      // Attach all of this window's children
      //
      if (!_childWindows.empty())
      {
         list<CGUIWindow *>::iterator childIter = _childWindows.end();
         do
         {
            CGUIWindow * child = *--childIter;
            if (child->_id != UGL_NULL_ID)
            {
               winAttach(child->_id, _id, UGL_NULL_ID);
            }
         } while (childIter != _childWindows.begin());
      }

      //
      // Notify parent that this window has been attached
      //

      UGL_WINDOW_ID rootWindow = _display.rootWindow();

      _parent = window;

      if (_parent)
      {
         _parent->addChildWindow(this);
         if (_parent->_id != UGL_NULL_ID)
         {
            winAttach(_id, _parent->_id, UGL_NULL_ID);
         }
      }
      else
      {
         winAttach(_id, _display.rootWindow(), UGL_NULL_ID);
      }

      //
      // Notify display of new window
      //
      _display.addWindow(this);
   }
}


void CGUIWindow::detach(void)
{
   if (_id != UGL_NULL_ID)
   {
      //
      // Detach all of this window's children
      //
      list<CGUIWindow *>::iterator childIter;
      for (childIter=_childWindows.begin(); childIter!=_childWindows.end(); ++childIter)
      {
         CGUIWindow * child = *childIter;
         if (child->_id != UGL_NULL_ID)
         {
            winDetach(child->_id);
         }
      }

      //
      // Notify display that window is no longer displayed
      //
      _display.removeWindow(this);

      //
      // Notify parent that this window is being detached
      //
      if (_parent)
      {
         _parent->deleteChildWindow(this);
         _parent = NULL;
      }

      //
      // Detach the underlying UGL window from its parent and then delete
      // the UGL window
      //
      if (winParent(_id) != UGL_NULL_ID)
      {
         winDetach(_id);
      }

      winDestroy(_id);
      _id = UGL_NULL_ID;
   }
}


void CGUIWindow::addObject(CGUIWindowObject * obj)
{
   obj->_owner = this;
   invalidateObjectRegion(obj);
}


void CGUIWindow::addObjectToFront(CGUIWindowObject * obj)
{
   if (obj->clipSiblings())
   {
      _clippedObjects.push_front(obj);
   }
   else
   {
      _nonClippedObjects.push_front(obj);
   }

   addObject(obj);
}


void CGUIWindow::addObjectToBack(CGUIWindowObject * obj)
{
   if (obj->clipSiblings())
   {
      _clippedObjects.push_back(obj);
   }
   else
   {
      _nonClippedObjects.push_back(obj);
   }

   addObject(obj);
}

void CGUIWindow::deleteObject(CGUIWindowObject * obj)
{
   if (obj->_owner == this)
   {
      //
      // Can't check clipSiblings method since deleteObject is
      // called from CGUIWindowObject destructor, and clipSiblings()
      // is defined only by classes derived from CGUIWindowObject.  So
      // here we just attempt to remove the object from both lists.
      //
      _clippedObjects.remove(obj);
      _nonClippedObjects.remove(obj);

      invalidateObjectRegion(obj);
      obj->_owner = NULL;
   }
}

void CGUIWindow::moveObjectToFront(CGUIWindowObject * obj)
{
   //
   // Order is important only for clipped objects
   //
   if (obj->clipSiblings() &&
       _clippedObjects.front() != obj)
   {
      _clippedObjects.remove(obj);
      _clippedObjects.push_front(obj);
      invalidateObjectRegion(obj);
   }
}

void CGUIWindow::moveObjectToBack(CGUIWindowObject * obj)
{
   //
   // Order is important only for clipped objects
   //
   if (obj->clipSiblings() &&
       _clippedObjects.back() != obj)
   {
      _clippedObjects.remove(obj);
      _clippedObjects.push_back(obj);
      invalidateObjectRegion(obj);
   }
}


void CGUIWindow::setObjectRegion(CGUIWindowObject * obj, const CGUIRegion & newRegion)
{
   //
   // Since we may be resizing/moving the object, we need to invalidate
   // both the old object position and the new position.
   //
   invalidateObjectRegion(obj);
   obj->_region = newRegion;
   invalidateObjectRegion(obj);
}


void CGUIWindow::setObjectVisible(CGUIWindowObject * obj, bool newVisible)
{
   if (obj->_visible != newVisible)
   {
      //
      // Force _visible to true for call to invalidateObjectRegion to insure
      // we really invalidate the region.
      //
      obj->_visible = true;
      invalidateObjectRegion(obj);

      obj->_visible = newVisible;
   }
}


void CGUIWindow::invalidateObjectRegion(CGUIWindowObject * obj)
{
   if (_id != UGL_NULL_ID &&
       obj->isVisible())
   {
      UGL_RECT rect;
      obj->_region.convertToUGLRect(rect);

      //
      // If we are not currently drawing the window, we can simply
      // invalidate the region.  Otherwise, we must include the region
      // within the drawing area, since simply invalidating the region
      // will not automatically add it to the active clip region.
      //
      if (_activeDrawRegion == UGL_NULL_ID)
      {
         //
         // Make sure that we invalidate at least one pixel.  Some
         // objects are not sized until first rendered, and initial
         // have a zero size region.  Expanding the region if
         // necessary insures that the parent window will be redrawn
         // and the object will get a chance to compute its region.
         //
         if (rect.left >= rect.right) rect.right = rect.left+1;
         if (rect.top >= rect.bottom) rect.bottom = rect.top+1;
         winRectInvalidate(_id, &rect);
      }else
      {
         uglRegionRectInclude(_activeDrawRegion, &rect);
      }
   }
}


bool CGUIWindow::disabled(void)
{
   //
   // The window is considered disabled if the window itself
   // or any of its parents are disabled.
   //
   bool  disabledFlag = _disabled;
   CGUIWindow * parent = _parent;

   while (disabledFlag == false &&
          parent)
   {
      disabledFlag = parent->_disabled;
      parent = parent->_parent;
   }

   return disabledFlag;
}


void CGUIWindow::addChildWindow(CGUIWindow * child)
{
   _childWindows.push_front(child);
}


void CGUIWindow::deleteChildWindow(CGUIWindow * child)
{
   _childWindows.remove(child);
}


UGL_STATUS CGUIWindow::uglDrawCallback (WIN_ID id, WIN_MSG * pMsg, void * pData, void * pParam)
{
   CGUIWindow * window = *(CGUIWindow **)pData;
   window->draw(pMsg->data.draw.gcId);

   return UGL_STATUS_FINISHED;
}


UGL_STATUS CGUIWindow::uglPointerCallback (WIN_ID id, WIN_MSG * pMsg, void * pData, void * pParam)
{
   UGL_WINDOW_ID  windowId = id;
   CGUIWindow * window = *(CGUIWindow **)pData;

   if (window)
   {
      if ((pMsg->data.ptr.buttonChange & 0x01) == 0)
      {
         // We are only interested in left button press or release events
         return UGL_STATUS_FINISHED;
      }

      CGUIWindow::PointerEvent ptEvent;
      if
          ((pMsg->data.ptr.buttonState & 0x01) != 0)
      {
         // Left button is pressed.  Grab the pointer to insure
         // we get the release event as well.
         winPointerGrab (windowId);
         ptEvent.eventType = CGUIWindow::PointerEvent::ButtonPress;
      }
      else
      {
         winPointerUngrab (windowId);
         ptEvent.eventType = CGUIWindow::PointerEvent::ButtonRelease;  
      }

      ptEvent.x = pMsg->data.ptr.position.x;
      ptEvent.y = pMsg->data.ptr.position.y;

      if (!window->disabled())
      {
         window->pointerEvent(ptEvent);
      }
   }

   return UGL_STATUS_FINISHED;

}


void CGUIWindow::draw(void)
{
   draw(_display.gc());
}


void CGUIWindow::draw(UGL_GC_ID gc)
{

   if (_id != UGL_NULL_ID)
   {
      _activeDrawRegion = uglRegionCreate();

#if CPU==SIMNT
      uglRegionUnion(_activeDrawRegion, winVisibleRegionGet(_id), _activeDrawRegion);
#else
      uglRegionUnion(_activeDrawRegion, winDirtyRegionGet(_id), _activeDrawRegion);
#endif  /* if CPU=SIMNT */

      preDrawObjects();

      if (!uglRegionIsEmpty(_activeDrawRegion))
      {
         drawObjects(gc);
      }

      uglRegionDestroy(_activeDrawRegion);
      _activeDrawRegion = UGL_NULL_ID;
   }
}


void CGUIWindow::preDrawObjects(void)
{
   list<CGUIWindowObject *>::iterator objIter;
   for (objIter = _clippedObjects.begin(); objIter != _clippedObjects.end(); ++objIter)
   {
      CGUIWindowObject * obj = *objIter;
      if (obj->_visible)
      {
         obj->preDraw();
      }
   }

   for (objIter = _nonClippedObjects.begin(); objIter != _nonClippedObjects.end(); ++objIter)
   {
      CGUIWindowObject * obj = *objIter;
      if (obj->_visible)
      {
         obj->preDraw();
      }
   }
}


void CGUIWindow::drawObjects(UGL_GC_ID gc)
{
   UGL_REGION_ID  clippedDrawRegion = uglRegionCreate();
   uglRegionCopy(_activeDrawRegion, clippedDrawRegion);

   winDrawStart(_id, gc, false);
   uglClipRegionSet(gc, clippedDrawRegion);

#if CPU == SIMNT
   uglRectangle(_display.gc(), 0, 0, 800, 600);
#endif  //  if CPU=SIMNT  */

   list<CGUIWindowObject *>::iterator objIter;
   for (objIter = _clippedObjects.begin(); objIter != _clippedObjects.end(); ++objIter)
   {
      CGUIWindowObject * obj = *objIter;
      if (obj->_visible)
      {
         obj->draw(gc);

         UGL_RECT objRect;
         obj->_region.convertToUGLRect(objRect);
         uglRegionRectExclude(clippedDrawRegion, &objRect);
         if (uglRegionIsEmpty(clippedDrawRegion))
         {
            break;
         }

         uglClipRegionSet(gc, clippedDrawRegion);
      }
   }
   uglClipRegionSet(gc, _activeDrawRegion);
   for (objIter = _nonClippedObjects.begin(); objIter != _nonClippedObjects.end(); ++objIter)
   {
      CGUIWindowObject * obj = *objIter;
      if (obj->_visible)
      {
         UGL_RECT objRect;
         obj->_region.convertToUGLRect(objRect);

         uglRegionCopy(_activeDrawRegion, clippedDrawRegion);
         uglRegionClipToRect(clippedDrawRegion, &objRect);
         if (!uglRegionIsEmpty(clippedDrawRegion))
         {
            obj->draw(gc);
         }
      }
   }
   winDrawEnd(_id, gc, true);
   uglRegionDestroy(clippedDrawRegion);
}


void CGUIWindow::pointerEvent(const PointerEvent & event)
{
}