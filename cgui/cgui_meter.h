/*
 *	Copyright (c) 2004 by Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: J:/BCT_Development/vxWorks/Common/cgui/rcs/cgui_meter.h 1.1 2004/09/24 21:59:49Z rm10919 Exp rm10919 $
 * This file defines the CGUIMeter class, which provides the ability for 
 * showing a sliding rectanglar meter with a background color and a fill 
 * color.  The parent window will have to create and manipulate any labeling 
 * needed for the meter.  If a scale is not provided, a default scale of
 * zero to one (0-1) is assumed.
 * $Log: cgui_meter.h $
 *
 */
#ifndef _CGUI_METER_INCLUDE
#define _CGUI_METER_INCLUDE


#include <vxworks.h>
#include <ugl/ugl.h>
#include <ugl/uglwin.h>
#include <ugl/uglinput.h>

#include "cgui_graphics.h"
#include "cgui_rectangle.h"

class CGUIMeter : public CGUIWindow 
{
public:
   enum  FillType
   {
      FILL_EDGE_LEFT   = 0x0001, // fill meter from left edge to right edge
      FILL_EDGE_RIGHT  = 0x0010, // fill meter from right edge to left edge
      FILL_EDGE_BOTTOM = 0x0100, // fill meter from bottom edge to top edge
      FILL_EDGE_TOP    = 0x1000  // fill meter from top edge to bottom edge
      };

    struct Scale
    {
       float initialEdge = 0;
       float fillEdge = 0;
       float finishEdge = 1;
    }

   CGUIMeter(CGUIDisplay & display, 
             CGUIWindow * window,
             CGUIRegion & region, 
             FillType fillType = FILL_EDGE_LEFT, 
             Scale * scale = NULL,
             CGUIColor backgroundColor = 0, 
             CGUIColor fillColor = 0);

   virtual ~CGUIMeter();

   // Meter Colors
   
   void setBackgroundColor(CGUIColor backgroundColor);
   CGUIColor getBackgroundColor(void) { return _backgroundColor;}

   void setFillColor(CGUIColor fillColor);
   CGUIColor getFillColor(void) { return _fillColor;}

   // Update Meters

   void setFillType (FillType fillType);
   FillType getFillType (void){ return _fillType;}

   void update(float fillEdge);
   void updateByPercent(int fillPercent);

   // Add a frame around the meter
   
   void setupFrame( list of CGUIFrame options - frame constructed with region from constructor )

   void setMeterRegion (CGUIRegion * region);
   CGUIRegion * getMeterRegion (void) { return _meterRegion;}

private:

   CGUIColor   _backgroundColor;             // Background CGUIRectangle color.
   CGUIColor   _fillColor;                   // Fill CGUIRectangle color.

   Scale    *  _scale;                       // Hold the scale values for meter.

   FillType    _fillType;                    // Which region edge does meter fill from?

   bool _isFramed;                           //Does the meter have a frame? Default is false.

   CGUIRegion  *  _meterRegion;              // Define the entire meter region.

   CGUIRectangle  *   _fillRectangle;        // Filled area of meter.
   CGUIRectangle  *   _backgroundRectangle;  // Background area of meter.

   //
   // Determine the pecentage the of the fill area based on a scale.
   //
   int getFillAreaPercent (float startPoint, float fillEdge, float finishPoint);
};
#endif /* #ifndef _CGUI_METER_INCLUDE */
