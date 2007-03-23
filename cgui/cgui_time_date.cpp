/*
 *	Copyright (c) 2004 by Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: //BCTquad3/home/BCT_Development/vxWorks/Common/cgui/rcs/cgui_time_date.cpp 1.8 2007/03/21 12:59:54 pn02526 Exp pn02526 $
 * $Log: cgui_time_date.cpp $
 * Revision 1.8  2007/03/21 12:59:54  pn02526
 * Add two CGUITextItem pointers for AM  and PM strings (suffixes for 12-hour time strings) to the calling sequence of the constructor, plus functions to get and set them post-construction. The two new constructor arguments are transparent to existing code. If NULL in the constructor or the set function, the class does not attempt to append anything to twelve-hour time strings. To internationalize this, the string handling within the class has been changed to use the wide character type StringChar used internally in the CGUIText and CGUITextItem classes.  The stringCharxxxx functions have been added for this.  
 * Change the date separator character must be changed to hyphen for internationalizing date strings.
 * 
 * 
 * --- Added comments ---  pn02526 [2007/03/22 18:34:57Z]
 * IT 77.
 * Revision 1.7  2007/01/29 09:08:39  rm10919
 * Fix getting single atrribute to look at the full range.
 * Revision 1.6  2006/01/28 20:49:42Z  cf10242
 * IT 49 - round time to nearest minute
 * Revision 1.5  2005/03/15 00:21:36Z  rm10919
 * Change CGUIText to not add object to window object list of parent in constructor.
 * Revision 1.4  2005/01/28 23:52:18Z  rm10919
 * CGUITextItem class changed and put into own file.
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

const size_t textBlockSize = 16;  // minimum StringChar string allocation in bytes.

const StringChar null_char = '\0';
const StringChar zero_char = '0';
const StringChar space_char = ' ';
const StringChar space_string[2] = { space_char , null_char }; 
const StringChar newline_string[2] = { '\n', null_char }; 

// Return the length of a StringChar string.
size_t CGUITimeDate::stringCharLen( const StringChar * s )
{
    size_t l=0;
    if( s != NULL ) while( *s++!=null_char ) l++;
    return l;
} 

// Compute allocations needed for StringChar strings.
size_t CGUITimeDate::stringCharAllocation(const size_t len) { return ((len+textBlockSize)/textBlockSize)*textBlockSize; };
size_t CGUITimeDate::stringCharAllocation(const StringChar * s) { return (s == NULL) ? 0 : stringCharAllocation(stringCharLen(s)); };

// Form a StringChar string from a const char string.
StringChar * CGUITimeDate::stringChar( const char * string )
{
   StringChar * s =  NULL;
   if (string != NULL)
   {
      int stringLength = strlen(string);
      s = new StringChar[stringCharAllocation(stringLength)];  
      for (int i=0; i<=stringLength; i++) s[i] = (unsigned char)(StringChar)string[i];
   }
   return s;
}

// Copy a StringChar string into another, reallocating the destination if necessary.
void CGUITimeDate::stringCharCpy( StringChar ** ps1, const StringChar * s2 )
{
    if( ps1 != NULL )  // Do nothing if we have no pointer to an s1.
    {
        StringChar * s1 = *ps1;
        size_t s1size = stringCharAllocation( s1 );
        size_t s2size = stringCharAllocation( s2 );
        if( s1 != NULL && s1size == s2size )
        {
            // Don't need to (re)allocate, just copy the string.
            if( s1 != NULL && s2 != NULL )
            {
                while( *s2 != null_char && --s1size > 0) { *s1++ = *s2++; }
                *s1 = null_char;
            }
        }
        else
        {
            //(Re)allocate *ps1.  Degenerative case is s1==NULL;
            if( s1 != NULL ) delete[] s1;
            *ps1 = new StringChar[s2size];
            stringCharCpy( ps1, s2 );
        }
    }
}

// Destructively Cat s2 to the end of *ps1, reallocating *ps1 if necessary.
void CGUITimeDate::stringCharCat( StringChar ** ps1, const StringChar * s2 )
{
    if( ps1 != NULL )  // Do nothing if we have no pointer to an s1.
    {
        StringChar * s1 = *ps1;
        if( s1 != NULL )
        {
            size_t s1len = stringCharLen(s1);
            size_t s1size = stringCharAllocation( s1len );
            size_t newSize = stringCharAllocation( s1len + stringCharLen(s2));
            if( newSize != s1size )
            {
                // Reallocate to fit cat'ed string
                StringChar * sNew = new StringChar[newSize]; // This does the new allocation
                stringCharCpy( &sNew, s1 );  // Copy s1 into the new allocation.
                delete[] s1;  // Get rid of old allocation.
                s1 = sNew;    // Point to new.
                *ps1 = sNew;  //   "    "  "
            }
            s1 += s1len;
            stringCharCpy( &s1, s2);
        }
        else
            // Degenerate to a newly allocated string copy.
            stringCharCpy( ps1, s2 );
    }
}


CGUITimeDate::CGUITimeDate(CGUIDisplay & display,
                           StylingRecord * stylingRecord, unsigned int displayAttributes = 0,
                           CGUITextItem * timeDateTextItem = NULL,
                           CGUITextItem * amTextItem = NULL,
                           CGUITextItem * pmTextItem = NULL )
:CGUIText(display),
_displayAttributes(displayAttributes),
_amTextItem(amTextItem),
_pmTextItem(pmTextItem)
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
   const time_t current_time = time(NULL);
   update(current_time);
}

void CGUITimeDate::update(const time_t the_timeval)
{
   unsigned int displayType = _displayAttributes & DISPLAY_TYPE_ATTRIBUTES_MASK;
   unsigned int attribute;

   StringChar * timeStringChar=NULL;
   StringChar * dateStringChar=NULL;
   
   struct tm   local_tm;

   time_t my_timeval = (time_t)the_timeval;
   localtime_r(&my_timeval, &local_tm);  // give it a timeval variable to trash, if that is its pleasure
   
   // round up if 30 seconds past the minute or more
   if(local_tm.tm_sec >= 30)
   {
       if(++local_tm.tm_min > 59)
       {
           local_tm.tm_min = 0;
           if(++local_tm.tm_hour > 23)
               local_tm.tm_hour = 0;
       }
   }

   //
   // Date String using DateAttributes
   //
   if( displayType != TIME_ONLY )
   {
       attribute = _displayAttributes & DATE_ATTRIBUTES_MASK;
       char dateString[20];
       switch (attribute)
       {
          case DD_MM_YY:
             strftime(dateString, 10, "%d-%m-%Y", &local_tm);
             break;
          case YY_MM_DD:
             strftime(dateString, 10, "%Y-%m-%d", &local_tm);
             break;
          default : //MM_DD_YY
             strftime(dateString, 10, "%m-%d-%Y", &local_tm);
             break;
       }
       // replace default first zero char if present
       if (dateString[0] == '0') dateString[0] = ' ';
       dateStringChar = stringChar(dateString);
   }

   
   //
   // Time String using ClockAttributes
   //
   if( displayType != DATE_ONLY )
   {
       attribute = _displayAttributes & CLOCK_ATTRIBUTES_MASK;
       char  timeString[20];
       switch (attribute)
       {
          case TWENTYFOUR_HOUR:
             strftime(timeString, 5, "%H:%M", &local_tm);
             timeStringChar = stringChar(timeString);
             break;

          default : //TWELVE_HOUR
             const StringChar * suffix=NULL;
             strftime(timeString, 5, "%I:%M", &local_tm);
             timeStringChar = stringChar(timeString);
             if(local_tm.tm_hour < 12)
             {
                if( _amTextItem != NULL ) suffix = _amTextItem->getText();
             }
             else
             {
                if( _pmTextItem != NULL ) suffix = _pmTextItem->getText();
             }
             if( suffix != NULL ) stringCharCat( &timeStringChar, suffix );
             break;
       }
       // replace default first zero char if present
       if (*timeStringChar == zero_char) *timeStringChar = space_char;
   }

      
   //
   // Now put them together or not!
   // Display and Order Attributes
   //
   switch(displayType)
   {
      case DATE_ONLY:
         setText(dateStringChar);
         break;
      
      case TIME_ONLY:
         setText(timeStringChar);
         break;
      
      case TIME_DATE:
      default:
         unsigned int lineAttribute;
         lineAttribute = _displayAttributes & LINE_ORDER_ATTRIBUTES_MASK;
         
         switch(lineAttribute)
         {
            case ONE_LINE_DATE_TIME:
               stringCharCat(&dateStringChar, space_string);
               stringCharCat(&dateStringChar, timeStringChar);
               setText(dateStringChar);
               break;
            
            case ONE_LINE_TIME_DATE:
               stringCharCat(&timeStringChar, space_string);
               stringCharCat(&timeStringChar, dateStringChar);
               setText(timeStringChar);
               break;
            
            case TWO_LINE_DATE_TIME:
               stringCharCat(&dateStringChar, newline_string);
               stringCharCat(&dateStringChar, timeStringChar);
               setText(dateStringChar);
               break;
            
            case TWO_LINE_TIME_DATE:
            default:
               stringCharCat(&timeStringChar, newline_string);
               stringCharCat(&timeStringChar, dateStringChar);
               setText(timeStringChar);
               break;
         }
         break;         
   }
   delete[] dateStringChar;
   delete[] timeStringChar;
}
