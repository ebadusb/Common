/*
 *	Copyright (c) 2004 by Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: //BCTquad3/home/BCT_Development/vxWorks/Common/cgui/rcs/cgui_time_date.h 1.4 2005/03/14 18:21:37 rm10919 Exp pn02526 $
 * $Log: cgui_time_date.h $
 *
 */

#ifndef _CGUI_TIME_DATE_INCLUDE
#define _CGUI_TIME_DATE_INCLUDE

#include "cgui_graphics.h"
#include "cgui_window.h"
#include "cgui_text.h"

class CGUITimeDate : public CGUIText
{
public:
   enum DisplayTypeAttributes
   { TIME_ONLY = 0x0000, DATE_ONLY = 0x0001, TIME_DATE = 0x0002 };

   enum ClockAttributes
   { TWELVE_HOUR = 0x0000, TWENTYFOUR_HOUR = 0x0010 };

   enum DateAttributes
   { MM_DD_YY = 0x0000, DD_MM_YY = 0x0100, YY_MM_DD = 0x0200 };

   CGUITimeDate(CGUIDisplay & display, CGUIWindow * parent, 
                TextItem * timeDateText, StylingRecord * stylingRecord = NULL, 
                unsigned int displayAttributes = 0, int timeDate);

   virtual ~CGUITimeDate();

   void setupFrame(list of CGUIFrame parameters);

   void setDisplayAttributes(unsigned int displayAttributes);
   unsigned int getDisplayAttributes(void) {return _displayAttributes};

   void update(unsigned int timeDate);

private:
   
   unsigned int    _displayAttributes;
   
   unsigned int    _timeDate;
};

#endif /* ifndef _CGUI_TIME_DATE_INCLUDE */

