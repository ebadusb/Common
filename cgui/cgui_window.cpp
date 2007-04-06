/*
 *	Copyright (c) 2004 by Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: Z:/BCT_Development/vxWorks/Common/cgui/rcs/cgui_window.cpp 1.21 2010/04/02 16:26:25Z agauusb Exp agauusb $
 * $Log: cgui_window.cpp $
 * Revision 1.15  2006/06/17 17:42:32Z  cf10242
 * IT 52: insure button release event is sent to window even if disabled
 * Revision 1.14  2005/11/22 00:36:01Z  rm10919
 * Invalidate window region to redraw when setting window visibility.
 * Revision 1.13  2005/08/15 18:47:43Z  cf10242
 * IT 674 - log failed UGl cleanup
 * Revision 1.12  2005/08/11 16:26:12Z  cf10242
 * TAOS IT 674 - ungrab when deleting to prevent page fault
 * Revision 1.11  2005/04/12 18:14:24Z  cf10242
 * remove changes from 1.10
 * Revision 1.9  2005/03/02 01:37:51Z  cf10242
 * deleting objects that are already deleted causing issues on TAOS
 * Revision 1.8  2005/02/21 17:17:13Z  cf10242
 * IT 133 - delete all allocated memory to avoid unrecovered memory
 * Revision 1.7  2005/02/07 18:53:55Z  rm10919
 * Fix screen size references to be generic.
 * Revision 1.6  2005/01/03 20:40:55Z  cf10242
 * add defensive coding to catch gui crashes
 * Revision 1.5  2004/12/09 13:16:03Z  jl11312
 * - needed to move fix to ugl source for now, complete fix would have required accessing private (to UGL) data structures from the application
 * Revision 1.4  2004/12/09 00:19:37Z  cf10242
 * GUI crash test
 * Revision 1.3  2004/11/04 20:19:09Z  rm10919
 * Common updates and changes.
 * Revision 1.2  2004/09/30 17:00:52Z  cf10242
 * Correct for initial make to work
 * Revision 1.1  2004/09/20 18:18:09Z  rm10919
 * Initial revision
 *
 */

#include <vxWorks.h>
#include "cgui_graphics.h"
#include "cgui_window.h"
#include "cgui_window_object.h"

extern UGL_DDB_ID offscreenBitMap;

bool CGUIWindow::_needRelease = false;

CGUIWindow::CGUIWindow(CGUIDisplay & display)
: _display(display)
{
   _guardTop = 0xcafef00d;
   _guardBottom = 0xcafef00d;
   initializeData();
}

CGUIWindow::CGUIWindow(CGUIDisplay & display, const CGUIRegion & region)
: _display(display)
{
   _guardTop = 0xcafef00d;
   _guardBottom = 0xcafef00d;
   initializeData();
   _region = region;
}

void CGUIWindow::initializeData(void)
{
   if(_guardTop != 0xcafef00d || _guardBottom != 0xcafef00d)
      DataLog( log_level_cgui_info ) << "Guard variable overwrite detected in " << __FILE__ << "  at" << __LINE__ << endmsg;
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
    UGL_STATUS status;
    if(_guardTop != 0xcafef00d || _guardBottom != 0xcafef00d)
       DataLog( log_level_cgui_info ) << "Guard variable overwrite detected in " << __FILE__ << "  at" << __LINE__ << endmsg;

	status = winCbRemove(_id, CGUIWindow::uglDrawCallback);
    if(status != UGL_STATUS_OK)
        DataLog( log_level_cgui_info ) << "winCbRemove failed = " << status << endmsg;

    // NULL out the window
    winDataSet(_id, UGL_NULL, UGL_NULL, 0);
    if(winPointerGrabGet(NULL) == _id)
    {
        status = winPointerUngrab (_id);
        if(status != UGL_STATUS_OK)
            DataLog( log_level_cgui_info ) << "winPointerUngrab failed = " << status << endmsg;
    }

   detach();
}

void CGUIWindow::setPosition(short x, short y)
{
    if(_guardTop != 0xcafef00d || _guardBottom != 0xcafef00d)
       DataLog( log_level_cgui_info ) << "Guard variable overwrite detected in " << __FILE__ << "  at" << __LINE__ << endmsg;
   if (x != _region.x ||
       y != _region.y)
   {
      CGUIRegion newRegion(x, y, _region.width, _region.height);
      CGUIWindow::setRegion(newRegion);
   }
}

void CGUIWindow::setRegion(const CGUIRegion & newRegion)
{
    if(_guardTop != 0xcafef00d || _guardBottom != 0xcafef00d)
       DataLog( log_level_cgui_info ) << "Guard variable overwrite detected in " << __FILE__ << "  at" << __LINE__ << endmsg;
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
    if(_guardTop != 0xcafef00d || _guardBottom != 0xcafef00d)
       DataLog( log_level_cgui_info ) << "Guard variable overwrite detected in " << __FILE__ << "  at" << __LINE__ << endmsg;
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
            if (child && (child->_id != UGL_NULL_ID) )
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
    if(_guardTop != 0xcafef00d || _guardBottom != 0xcafef00d)
       DataLog( log_level_cgui_info ) << "Guard variable overwrite detected in " << __FILE__ << "  at" << __LINE__ << endmsg;
   if (_id != UGL_NULL_ID)
   {
      //
      // Detach all of this window's children
      //
      list<CGUIWindow *>::iterator childIter;
      for (childIter=_childWindows.begin(); childIter!=_childWindows.end(); ++childIter)
      {
         CGUIWindow * child = *childIter;
         if (child && (child->_id != UGL_NULL_ID) )
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
    if(_guardTop != 0xcafef00d || _guardBottom != 0xcafef00d)
       DataLog( log_level_cgui_info ) << "Guard variable overwrite detected in " << __FILE__ << "  at" << __LINE__ << endmsg;
	if(obj)
	{
		obj->_owner = this;
		invalidateObjectRegion(obj);
	}
}


void CGUIWindow::addObjectToFront(CGUIWindowObject * obj)
{
    if(_guardTop != 0xcafef00d || _guardBottom != 0xcafef00d)
       DataLog( log_level_cgui_info ) << "Guard variable overwrite detected in " << __FILE__ << "  at" << __LINE__ << endmsg;
	if(obj)
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
}


void CGUIWindow::addObjectToBack(CGUIWindowObject * obj)
{
    if(_guardTop != 0xcafef00d || _guardBottom != 0xcafef00d)
       DataLog( log_level_cgui_info ) << "Guard variable overwrite detected in " << __FILE__ << "  at" << __LINE__ << endmsg;
	if(obj)
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
}

void CGUIWindow::deleteObject(CGUIWindowObject * obj)
{
    if(_guardTop != 0xcafef00d || _guardBottom != 0xcafef00d)
       DataLog( log_level_cgui_info ) << "Guard variable overwrite detected in " << __FILE__ << "  at" << __LINE__ << endmsg;
	if(obj)
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
}

void CGUIWindow::moveObjectToFront(CGUIWindowObject * obj)
{
    if(_guardTop != 0xcafef00d || _guardBottom != 0xcafef00d)
       DataLog( log_level_cgui_info ) << "Guard variable overwrite detected in " << __FILE__ << "  at" << __LINE__ << endmsg;
   //
   // Order is important only for clipped objects
   //
	if(obj)
	{
		if (obj->clipSiblings() &&
			_clippedObjects.front() != obj)
		{
			_clippedObjects.remove(obj);
			_clippedObjects.push_front(obj);
			invalidateObjectRegion(obj);
		}
	}
}

void CGUIWindow::moveObjectToBack(CGUIWindowObject * obj)
{
    if(_guardTop != 0xcafef00d || _guardBottom != 0xcafef00d)
       DataLog( log_level_cgui_info ) << "Guard variable overwrite detected in " << __FILE__ << "  at" << __LINE__ << endmsg;
   //
   // Order is important only for clipped objects
   //
	if(obj)
	{
		if (obj->clipSiblings() &&
			_clippedObjects.back() != obj)
		{
			_clippedObjects.remove(obj);
			_clippedObjects.push_back(obj);
			invalidateObjectRegion(obj);
		}
	}
}


void CGUIWindow::setObjectRegion(CGUIWindowObject * obj, const CGUIRegion & newRegion)
{
    if(_guardTop != 0xcafef00d || _guardBottom != 0xcafef00d)
       DataLog( log_level_cgui_info ) << "Guard variable overwrite detected in " << __FILE__ << "  at" << __LINE__ << endmsg;
   //
   // Since we may be resizing/moving the object, we need to invalidate
   // both the old object position and the new position.
   //
	if(obj)
	{
		invalidateObjectRegion(obj);
		obj->_region = newRegion;
		invalidateObjectRegion(obj);
	}
}


void CGUIWindow::setObjectVisible(CGUIWindowObject * obj, bool newVisible)
{
    if(_guardTop != 0xcafef00d || _guardBottom != 0xcafef00d)
       DataLog( log_level_cgui_info ) << "Guard variable overwrite detected in " << __FILE__ << "  at" << __LINE__ << endmsg;
   if (obj && (obj->_visible != newVisible) )
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
    if(_guardTop != 0xcafef00d || _guardBottom != 0xcafef00d)
       DataLog( log_level_cgui_info ) << "Guard variable overwrite detected in " << __FILE__ << "  at" << __LINE__ << endmsg;
   if (_id != UGL_NULL_ID && obj &&
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

void CGUIWindow::setWindowVisibility (bool newVisible)
{
    if(_guardTop != 0xcafef00d || _guardBottom != 0xcafef00d)
       DataLog( log_level_cgui_info ) << "Guard variable overwrite detected in " << __FILE__ << "  at" << __LINE__ << endmsg;
   if (newVisible != winVisibleGet(_id))
   {
      UGL_RECT rect;
      _region.convertToUGLRect(rect);

      winVisibleSet(_id, newVisible);
      winRectInvalidate(_id, &rect);
   }
}

bool CGUIWindow::disabled(void)
{
    if(_guardTop != 0xcafef00d || _guardBottom != 0xcafef00d)
       DataLog( log_level_cgui_info ) << "Guard variable overwrite detected in " << __FILE__ << "  at" << __LINE__ << endmsg;
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
    if(_guardTop != 0xcafef00d || _guardBottom != 0xcafef00d)
       DataLog( log_level_cgui_info ) << "Guard variable overwrite detected in " << __FILE__ << "  at" << __LINE__ << endmsg;
   _childWindows.push_front(child);
}


void CGUIWindow::deleteChildWindow(CGUIWindow * child)
{
    if(_guardTop != 0xcafef00d || _guardBottom != 0xcafef00d)
       DataLog( log_level_cgui_info ) << "Guard variable overwrite detected in " << __FILE__ << "  at" << __LINE__ << endmsg;
   _childWindows.remove(child);
}


UGL_STATUS CGUIWindow::uglDrawCallback (WIN_ID id, WIN_MSG * pMsg, void * pData, void * pParam)
{
//    if(_guardTop != 0xcafef00d || _guardBottom != 0xcafef00d)
//       DataLog( log_level_cgui_info ) << "Guard variable overwrite detected in " << __FILE__ << "  at" << __LINE__ << endmsg;
   CGUIWindow * window = *(CGUIWindow **)pData;
	if(window && pMsg)
		window->draw(pMsg->data.draw.gcId);

   return UGL_STATUS_FINISHED;
}


UGL_STATUS CGUIWindow::uglPointerCallback (WIN_ID id, WIN_MSG * pMsg, void * pData, void * pParam)
{
//    if(_guardTop != 0xcafef00d || _guardBottom != 0xcafef00d)
//       DataLog( log_level_cgui_info ) << "Guard variable overwrite detected in " << __FILE__ << "  at" << __LINE__ << endmsg;
   UGL_WINDOW_ID  windowId = id;
   CGUIWindow * window = *(CGUIWindow **)pData;

   if ((windowId != UGL_NULL) && window && pMsg)
   {
      if ((pMsg->data.ptr.buttonChange & 0x01) == 0)
      {
         // We are only interested in left button press or release events
         return UGL_STATUS_FINISHED;
      }

      CGUIWindow::PointerEvent ptEvent;
      if ((pMsg->data.ptr.buttonState & 0x01) != 0)
      {
         // Left button is pressed.  Grab the pointer to insure
         // we get the release event as well.
         //DataLog( log_level_cgui_info ) << "Grabbing pointer " << endmsg;
         winPointerGrab (windowId);
         ptEvent.eventType = CGUIWindow::PointerEvent::ButtonPress;
		 if(!window->disabled())
			 _needRelease = true;
      }
      else
      {
        // DataLog( log_level_cgui_info ) << "Ungrabbing pointer " << endmsg;
         winPointerUngrab (windowId);
         ptEvent.eventType = CGUIWindow::PointerEvent::ButtonRelease;
      }

      ptEvent.x = pMsg->data.ptr.position.x;
      ptEvent.y = pMsg->data.ptr.position.y;

      if (!window->disabled() || (ptEvent.eventType == CGUIWindow::PointerEvent::ButtonRelease &&
								  _needRelease) )
      {
        // DataLog( log_level_cgui_info ) << "Processing pointer event " << endmsg;
         window->pointerEvent(ptEvent);
      }

	  if (ptEvent.eventType == CGUIWindow::PointerEvent::ButtonRelease)
		  _needRelease = false;
     // DataLog( log_level_cgui_info ) << "Completed pointer event processing " << endmsg;
   }

   return UGL_STATUS_FINISHED;

}


void CGUIWindow::draw(void)
{
    if(_guardTop != 0xcafef00d || _guardBottom != 0xcafef00d)
       DataLog( log_level_cgui_info ) << "Guard variable overwrite detected in " << __FILE__ << "  at" << __LINE__ << endmsg;
   draw(_display.gc());
}


void CGUIWindow::draw(UGL_GC_ID gc)
{

    if(_guardTop != 0xcafef00d || _guardBottom != 0xcafef00d)
       DataLog( log_level_cgui_info ) << "Guard variable overwrite detected in " << __FILE__ << "  at" << __LINE__ << endmsg;
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
    if(_guardTop != 0xcafef00d || _guardBottom != 0xcafef00d)
       DataLog( log_level_cgui_info ) << "Guard variable overwrite detected in " << __FILE__ << "  at" << __LINE__ << endmsg;
   list<CGUIWindowObject *>::iterator objIter;
   for (objIter = _clippedObjects.begin(); objIter != _clippedObjects.end(); ++objIter)
   {
      CGUIWindowObject * obj = *objIter;
      if (obj && obj->_visible)
      {
         obj->preDraw();
      }
   }

   for (objIter = _nonClippedObjects.begin(); objIter != _nonClippedObjects.end(); ++objIter)
   {
      CGUIWindowObject * obj = *objIter;
      if (obj && obj->_visible)
      {
         obj->preDraw();
      }
   }
}


void CGUIWindow::drawObjects(UGL_GC_ID gc)
{
    if(_guardTop != 0xcafef00d || _guardBottom != 0xcafef00d)
       DataLog( log_level_cgui_info ) << "Guard variable overwrite detected in " << __FILE__ << "  at" << __LINE__ << endmsg;
   UGL_REGION_ID  clippedDrawRegion = uglRegionCreate();
   uglRegionCopy(_activeDrawRegion, clippedDrawRegion);

   winDrawStart(_id, gc, false);

	// This allows a screen to be drawn to an offscreen bitmap.
	if( offscreenBitMap != UGL_NULL_ID )
	{
		UGL_POS top, bottom, left, right;
		uglViewPortGet( gc, &left, &top, &right, &bottom );
		uglDefaultBitmapSet( gc, offscreenBitMap );
		uglViewPortSet( gc, left, top, right, bottom );
	}

	uglClipRegionSet(gc, clippedDrawRegion);

#if CPU == SIMNT
   uglRectangle(_display.gc(), 0, 0, _display.width(), _display.height());
#endif  //  if CPU=SIMNT  */

   list<CGUIWindowObject *>::iterator objIter;
   for (objIter = _clippedObjects.begin(); objIter != _clippedObjects.end(); ++objIter)
   {
      CGUIWindowObject * obj = *objIter;
      if (obj && obj->_visible && obj->_owner)
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
      if (obj && obj->_visible)
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
    if(_guardTop != 0xcafef00d || _guardBottom != 0xcafef00d)
       DataLog( log_level_cgui_info ) << "Guard variable overwrite detected in " << __FILE__ << "  at" << __LINE__ << endmsg;
}
