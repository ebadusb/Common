/*
 *	Copyright (c) 2004 by Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: //BCTquad3/home/BCT_Development/vxWorks/Common/cgui/rcs/cgui_time_date.cpp 1.7 2007/01/29 09:08:39 rm10919 Exp pn02526 $
 * $Log: cgui_time_date.cpp $
 * Revision 1.3  2005/01/17 17:59:18Z  cf10242
 * some code clean-up issues
 * Revision 1.2  2005/01/03 20:40:54Z  cf10242
 * add defensive coding to catch gui crashes
 * Revision 1.1  2004/11/03 17:44:11Z  rm10919
 * Initial revision
 *
 */

#include <vxWorks.h>
#include "cgui_time_date.h"


CGUITimeDate::CGUITimeDate(CGUIDisplay & display, CGUIWindow * parent,
                           StylingRecord * stylingRecord, unsigned int displayAttributes = 0,
                           CGUITextItem * timeDateTextItem = NULL)
:CGUIText(display, parent),
_displayAttributes(displayAttributes)
{
   if (timeDateTextItem)
   {
      setText(timeDateTextItem);
   }
   setStylingRecord(stylingRecord);
   update();
}

CGUITimeDate::~CGUITimeDate()
{
}

void CGUITimeDate::setupFrame(void)
{
}

void CGUITimeDate::setDisplayAttributes(unsigned int displayAttributes)
{
   _displayAttributes = displayAttributes;
   update();
}

void CGUITimeDate::update()
{
   unsigned int attribute;

   char dateString[20];
   char  timeString[20];
   
   time_t currentdate = time(NULL);
   struct tm   local_tm;

   localtime_r(&currentdate, &local_tm);
   
   //
   // Date String using DateAttributes
   //
   
   attribute = _displayAttributes & 0x0100;

   switch (attribute)
   {
      case DD_MM_YY:
         strftime(dateString, 10, "%d/%m/%Y", &local_tm);
         break;

      case YY_MM_DD:
         strftime(dateString, 10, "%Y/%m/%d", &local_tm);
         break;

      default : //MM_DD_YY
         strftime(dateString, 10, "%m/%d/%Y", &local_tm);
         break;

   }

   // replace default first zero char if present
   if (dateString[0] == '0') dateString[0] = ' ';
   
   //
   // Time String using ClockAttributes
   //

   attribute = _displayAttributes & 0x0010;

   switch (attribute)
   {
      case TWENTYFOUR_HOUR:
         strftime(timeString, 5, "%H:%M", &local_tm);
         break;

      default : //TWELVE_HOUR
         strftime(timeString, 5, "%I:%M", &local_tm);
         break;
      }

   // replace default first zero char if present
   if (timeString[0] == '0') timeString[0] = ' ';

      
   //
   // Now put them together or not!
   // Display and Order Attributes
   //
      
   attribute = _displayAttributes & 0x0001;

   switch(attribute)
   {
      case DATE_ONLY:
         setText(dateString);
         break;
      
      case TIME_ONLY:
         setText(timeString);
         break;
      
      case TIME_DATE:
      default:
         unsigned int lineAttribute;
         lineAttribute = _displayAttributes & 0x1000;
         
         switch(lineAttribute)
         {
            case ONE_LINE_DATE_TIME:
               strcat(dateString, " ");
               strcat(dateString, timeString);
               setText(dateString);
               break;
            
            case ONE_LINE_TIME_DATE:
               strcat(timeString, " ");
               strcat(timeString, dateString);
               setText(timeString);
               break;
            
            case TWO_LINE_DATE_TIME:
               strcat(dateString, "\n");
               strcat(dateString, timeString);
               setText(dateString);
               break;
            
            case TWO_LINE_TIME_DATE:
            default:
               strcat(timeString, "\n");
               strcat(timeString, dateString);
               setText(timeString);
               break;
         }
         break;         
   }
}
