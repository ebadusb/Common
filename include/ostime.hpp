/*
 * Copyright (c) 1995-1999 by Cobe BCT, Inc.  All rights reserved.
 *
 * $Header: K:/BCT_Development/vxWorks/Common/include/rcs/ostime.hpp 1.7 2002/11/22 21:07:37 pn02526 Exp jl11312 $
 * $Log: ostime.hpp $
 * Revision 1.7  2002/11/22 21:07:37  pn02526
 * Change the nanosec member of the timeFromTick struct to a long to 1) facilitate computation of negative time deltas, and 2) agree with the timespec struct in <time.h>.
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

#include <unistd.h>

#include "auxclock.h"

class osTime
{
   public:
      // SPECIFICATION:    snapshotRaw.
      //                   osTime method to get raw time from auxclock handler
      //
      // ERROR HANDLING:   none.
      static inline void snapshotRawTime(rawTime & now)
      {
         auxClockTimeGet(&now);
      }

      static int howLongMilliSec(const rawTime & then);   	// return delta in milliseconds between then and now
      static int howLongMicroSec(const rawTime & then);   	// return delta in microseconds between then and now
      static int howLongMilliSecAndUpdate(rawTime & then);  // update then with current time and return delta in milliseconds
      static int howLongMicroSecAndUpdate(rawTime & then); 	// update then with current time and return delta in microseconds
      static void delayMilliSec(int milliSec);              // holds in a tight loop for specified time in milliseconds
};

#endif
