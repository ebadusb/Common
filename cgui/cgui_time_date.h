/*
 *	Copyright (c) 2004 by Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: //BCTquad3/home/BCT_Development/vxWorks/Common/cgui/rcs/cgui_time_date.h 1.4 2005/03/14 18:21:37 rm10919 Exp pn02526 $
 * $Log: cgui_time_date.h $
 * Revision 1.2  2004/11/03 17:43:55Z  rm10919
 * Finally Working!
 * Revision 1.1  2004/09/24 21:30:56Z  rm10919
 * Initial revision
 *
 */

#ifndef _CGUI_TIME_DATE_INCLUDE
#define _CGUI_TIME_DATE_INCLUDE

#include <time.h>
#include "cgui_graphics.h"
#include "cgui_window.h"
#include "cgui_text.h"

class CGUITimeDate : public CGUIText
{
public:
   enum DisplayTypeAttributes
   { TIME_DATE = 0x0000, DATE_ONLY = 0x0001, TIME_ONLY = 0x0002 };

   enum ClockAttributes
   { TWELVE_HOUR = 0x0000, TWENTYFOUR_HOUR = 0x0010 };

   enum DateAttributes
   { MM_DD_YY = 0x0000, DD_MM_YY = 0x0100, YY_MM_DD = 0x0200 };

   enum LineOrderAttributes
   { ONE_LINE_DATE_TIME = 0x0000, ONE_LINE_TIME_DATE = 0x1000, TWO_LINE_DATE_TIME = 0x2000, TWO_LINE_TIME_DATE = 0x3000 };

   CGUITimeDate(CGUIDisplay & display, CGUIWindow * parent,
                StylingRecord * stylingRecord, unsigned int displayAttributes = 0,
                CGUITextItem * timeDateTextItem = NULL);

   virtual ~CGUITimeDate(void);

   void setupFrame(void);

   void setDisplayAttributes(unsigned int displayAttributes);
   unsigned int getDisplayAttributes(void) {return _displayAttributes;}

   void update();

private:
   
   unsigned int    _displayAttributes;
};

#endif /* ifndef _CGUI_TIME_DATE_INCLUDE */

