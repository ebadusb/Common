/*
 * Copyright (c) 1995-1999 by Cobe BCT, Inc.  All rights reserved.
 *
 * $Header: K:/BCT_Development/vxWorks/Common/include/rcs/ostime.hpp 1.7 2002/11/22 21:07:37 pn02526 Exp jl11312 $
 * $Log: ostime.hpp $
 * Revision 1.6  2002/06/19 17:01:09  pn02526
 * Updates for VxWorks: remove/replace system level includes; remove the sensitivity to time-of-day changes; modify to use auxClock as a time base.
 * Revision 1.5  1999/10/28 14:29:19  BS04481
 * Code review change.  Previous design disabled soft watchdogs
 * while the clock was being set.  This was unacceptable.  This code
 * changes the soft watchdogs to run off of an element of the 
 * kernel's ticks space which is not sensitive to changes in the 
 * real-time clock.    All code with disables the soft watchdogs is
 * disabled.  The soft watchdog will go off if not petted within 2
 * seconds under all conditions.  The machine will also safe-state
 * if the soft watchdogs ever go backward.   In addition, time set
 * changes are rejected if received while air-to-donor monitoring is
 * in effect.  This is done because the time jumps in the real-time
 * clocks effect the control loops.
 * 
 * Revision 1.4  1999/10/08 18:35:01  BS04481
 * Reference the variable that controls the watchdogs during time 
 * setting via a function instead of directly.
 * Revision 1.3  1999/09/30 04:08:02  BS04481
 * Remove message send and receive functions from the driver 
 * service loop. 
 * Revision 1.2  1999/09/23 00:30:19  BS04481
 * Change snapshot to an inline function and call from the howLong
 * functions.
 * Revision 1.1  1999/09/17 15:04:34  BS04481
 * Initial revision
 * TITLE:      osTime.hpp, Time measurement
 *
 * ABSTRACT:   Computes time intervals by using the kernel's tick-time
 *             space.
 *
 * DOCUMENTS
 * Requirements:     I:\ieee1498\SSS3.DOC
 * Test:             I:\ieee1498\STD3.DOC
 *
 * PUBLIC FUNCTIONS:
 *
 *               
 */

#ifndef OSTIME_HPP
#define OSTIME_HPP

#include <time.h>
#include <unistd.h>

#include "auxclock.h"


typedef struct
{
   time_t   sec;
   long     nanosec;
}timeFromTick;

class osTime
{
   public:
      osTime();
      ~osTime();

      // SPECIFICATION:    TimeFromRawTicks.
      //                   osTime method to convert the rawTick type to a timeFromTick struct.
      //
      // ERROR HANDLING:   none.
      inline void TimeFromRawTicks(timeFromTick *tftptr, rawTick rt )
      {
          rawTick rtSec = (rt) / (rawTick)_TicksPerSecond;
          tftptr->sec  = (time_t) rtSec;
      	  tftptr->nanosec = (long) ( (rt - rtSec*(rawTick)_TicksPerSecond ) *  (rawTick)_NanoSecondsPerTick );
          return;
      }
      
      // SPECIFICATION:    snapshotTime.
      //                   osTime method to get time from the kernel.
      //
      // ERROR HANDLING:   none.
      inline void snapshotTime(timeFromTick* now)
      {
         TimeFromRawTicks( now, auxClockTicksGet() );
      };
      
      
      // SPECIFICATION:    snapshotRaw.
      //                   osTime method to get raw ticks from the kernel.
      //                   (assumes typedef TICK rawTick)
      //
      // ERROR HANDLING:   none.
      inline void snapshotRaw(rawTick* now)
      {
         *now = auxClockTicksGet();
      };

      void whatTimeIsIt(timeFromTick* now);        // get new time from object
      int howLong(timeFromTick then);              // return delta between then and now
      int howLongMicro(timeFromTick then);         // return delta in usecond between then and now
      int howLongAndUpdate(timeFromTick* then);    // update then with time from object and return delta
      int howLongMicroAndUpdate(timeFromTick* then); // update then with time from object and return delta in usec
      int howLongRaw(rawTick then);                // return delta between now and then based on raw clock ticks
      void delayTime(int deltaTime);               // holds in a tight loop for specified time
      static const char compileDateTime[];
   private:
      int _TicksPerSecond, _NanoSecondsPerTick;
};

#endif
