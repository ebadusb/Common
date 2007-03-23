/*
 *	Copyright (c) 2004 by Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: K:/BCT_Development/vxWorks/Common/cgui/rcs/cgui_time_date.h 1.6 2007/03/22 20:57:49Z pn02526 Exp wms10235 $
 * $Log: cgui_time_date.h $
 * Revision 1.5  2007/03/22 09:19:22  pn02526
 * Add two CGUITextItem pointers for AM  and PM strings (suffixes for 12-hour time strings) to the calling sequence of the constructor, plus functions to get and set them post-construction. The two new constructor arguments are transparent to existing code. If NULL in the constructor or the set function, the class does not attempt to append anything to twelve-hour time strings. To internationalize this, the string handling within the class has been changed to use the wide character type StringChar used internally in the CGUIText and CGUITextItem classes.  The stringCharxxxx functions have been added for this.  
 * Change the date separator character must be changed to hyphen for internationalizing date strings.
 * 
 * 
 * --- Added comments ---  pn02526 [2007/03/22 18:35:30Z]
 * IT 77.
 * Revision 1.4  2005/03/14 18:21:37  rm10919
 * Change CGUIText to not add object to window object list of parent in constructor.
 * Revision 1.3  2005/01/28 23:52:18Z  rm10919
 * CGUITextItem class changed and put into own file.
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
   { DISPLAY_TYPE_ATTRIBUTES_MASK=0x000f, TIME_DATE = 0x0000, DATE_ONLY = 0x0001, TIME_ONLY = 0x0002 };

   enum ClockAttributes
   { CLOCK_ATTRIBUTES_MASK=0x00f0, TWELVE_HOUR = 0x0000, TWENTYFOUR_HOUR = 0x0010 };

   enum DateAttributes
   { DATE_ATTRIBUTES_MASK=0x0f00, MM_DD_YY = 0x0000, DD_MM_YY = 0x0100, YY_MM_DD = 0x0200 };

   enum LineOrderAttributes
   { LINE_ORDER_ATTRIBUTES_MASK=0xf000, ONE_LINE_DATE_TIME = 0x0000, ONE_LINE_TIME_DATE = 0x1000, TWO_LINE_DATE_TIME = 0x2000, TWO_LINE_TIME_DATE = 0x3000 };

   CGUITimeDate(CGUIDisplay & display,
                StylingRecord * stylingRecord, unsigned int displayAttributes = 0,
                CGUITextItem * timeDateTextItem = NULL,
                CGUITextItem * amTextItem = NULL,
                CGUITextItem * pmTextItem = NULL);

   virtual ~CGUITimeDate(void);

   void setupFrame(void);

   void setDisplayAttributes(unsigned int displayAttributes);
   unsigned int getDisplayAttributes(void) {return _displayAttributes;}

   void setAMPMText(CGUITextItem * amTextItem = NULL, CGUITextItem * pmTextItem = NULL) { _amTextItem=amTextItem; _pmTextItem=pmTextItem; };
   void getAMPMText(CGUITextItem ** amTextItem = NULL, CGUITextItem ** pmTextItem = NULL) { *amTextItem=_amTextItem; *pmTextItem=_pmTextItem; };

   void update();
   void update(time_t);

private:
   
   unsigned int    _displayAttributes;

    CGUITextItem * _amTextItem;
    CGUITextItem * _pmTextItem;

    StringChar * stringChar( const char * string );
    size_t stringCharLen( const StringChar * s );
    inline size_t stringCharAllocation(const size_t len);
    inline size_t stringCharAllocation(const StringChar * s);
    void stringCharCpy( StringChar ** ps1, const StringChar * s2 );
    void stringCharCat( StringChar ** ps1, const StringChar * s2 );
};

#endif /* ifndef _CGUI_TIME_DATE_INCLUDE */

