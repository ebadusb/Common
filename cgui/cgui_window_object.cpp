/*
 *	Copyright (c) 2004 by Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: K:/BCT_Development/vxWorks/Common/cgui/rcs/cgui_window_object.cpp 1.5 2005/03/02 01:37:51Z cf10242 Exp jl11312 $
 * $Log: cgui_window_object.cpp $
 * Revision 1.1  2004/09/20 18:18:11Z  rm10919
 * Initial revision
 * Initial revision
 *
 */

#include <vxWorks.h>
#include "cgui_graphics.h"
#include "cgui_window_object.h"
#include "cgui_window.h"

CGUIWindowObject::CGUIWindowObject(CGUIDisplay & display)
: _display(display)
{
   initializeData();
}

CGUIWindowObject::CGUIWindowObject(CGUIDisplay & display, const CGUIRegion & region)
: _display(display)
{
   initializeData();
   _region = region;
}

void CGUIWindowObject::initializeData(void)
{
   _owner = NULL;
   _visible = true;
}

CGUIWindowObject::~CGUIWindowObject()
{
   if (_owner)
   {
      _owner->deleteObject(this);
      _owner = NULL;
   }
}

void CGUIWindowObject::setPosition(short x, short y)
{
   if (x != _region.x ||
       y != _region.y)
   {
      CGUIRegion newRegion(x, y, _region.width, _region.height);
      CGUIWindowObject::setRegion(newRegion);
   }
}

void CGUIWindowObject::setRegion(const CGUIRegion & newRegion)
{
   if (newRegion.x != _region.x ||
       newRegion.y != _region.y ||
       newRegion.width != _region.width ||
       newRegion.height != _region.height)
   {
      if (_owner)
      {
         _owner->setObjectRegion(this, newRegion);
      } else
      {
         _region = newRegion;
      }
   }
}

void CGUIWindowObject::setVisible(bool newVisible)
{
   if (_visible != newVisible)
   {
      if (_owner)
      {
         _owner->setObjectVisible(this, newVisible);
      } else
      {
         _visible = newVisible;
      }
   }
}

void CGUIWindowObject::moveToBack(void)
{
   if (_owner)
   {
      _owner->moveObjectToBack(this);
   }
}

void CGUIWindowObject::moveToFront(void)
{
   if (_owner)
   {
      _owner->moveObjectToFront(this);
   }
}