/*
 * Copyright (c) 1995-1999 by Cobe BCT, Inc.  All rights reserved.
 *
 * $Header: //bctquad3/HOME/BCT_Development/vxWorks/Common/clocks/rcs/ostime.cpp 1.11 2002/09/25 11:11:36 jl11312 Exp pn02526 $
 * $Log: ostime.cpp $
 * Revision 1.8  2001/04/05 14:16:14  jl11312
 * - internal timer handling changes required for versa logic CPU board
 * Revision 1.7  2000/03/17 16:41:25  BS04481
 * Non-essential issues from 3.3 code review
 * Revision 1.6  1999/10/28 20:29:16  BS04481
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
 * Revision 1.5  1999/10/13 04:14:08  BS04481
 * Correct delta time calculation to avoid overflow.  Overflow 
 * resulted in truncation of left 12 bits of an intermediate result and
 * would not effect the final result.
 * Revision 1.4  1999/10/08 18:34:59  BS04481
 * Reference the variable that controls the watchdogs during time 
 * setting via a function instead of directly.
 * Revision 1.3  1999/09/30 04:08:00  BS04481
 * Remove message send and receive functions from the driver 
 * service loop. 
 * Revision 1.2  1999/09/23 00:30:16  BS04481
 * Change snapshot to an inline function and call from the howLong
 * functions.
 * Revision 1.1  1999/09/17 15:05:15  BS04481
 * Initial revision
 * TITLE:      osTime.cpp, Time measurement
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
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "error.h"
#include "ostime.hpp"

const unsigned long BILLION=1000000000ul;
const unsigned long MILLION=1000000ul;

// SPECIFICATION:    osTime constructor.
//                   Gets rate of aux clock and converts it to nsec/rawTick for use by other class methods.
//
// ERROR HANDLING:   none.

osTime::osTime(void)
{
   _TicksPerSecond = auxClockRateGet( ); /* Get the number of ticks per second of the aux clock */
   _NanoSecondsPerTick = (int) ( BILLION / (unsigned long)_TicksPerSecond );
};

// SPECIFICATION:    osTime destructor.
//
// ERROR HANDLING:   none.

osTime::~osTime()
{
};



// SPECIFICATION:    whatTimeIsIt.
//                   Just like snapshotTime except not inline
//
// ERROR HANDLING:   none.

void 
osTime::whatTimeIsIt(timeFromTick* now)
{
    TimeFromRawTicks( now, auxClockTicksGet() );
};


// SPECIFICATION:    howLongAndUpdate.
//                   Sets time in the then structure to time 
//                   Returns delta between previous then value
//                   and now.
//
// ERROR HANDLING:   none.

int
osTime::howLongAndUpdate(timeFromTick* then)
{
   timeFromTick   now;
   int delta;        // milliseconds

   snapshotTime(&now);
   
   delta = ( (now.sec - then->sec) * 1000)
         + ( (now.nanosec - then->nanosec) / 1000000);

   then->sec = now.sec;
   then->nanosec = now.nanosec;

   return(delta);
};

// SPECIFICATION:    howLongMicroAndUpdate.
//                   Sets time in the then structure to time 
//                   Returns delta in microseconds between previous 
//                   then and now.
//
// ERROR HANDLING:   none.

int
osTime::howLongMicroAndUpdate(timeFromTick* then)
{
   timeFromTick   now;
   int delta;        // microseconds

   snapshotTime(&now);
   
   delta = ( (now.sec - then->sec) * 1000000)
         + ( (now.nanosec - then->nanosec) / 1000);

   then->sec = now.sec;
   then->nanosec = now.nanosec;

   return(delta);
};


// SPECIFICATION:    howLong.
//                   Returns delta between then value
//                   and now.
//
// ERROR HANDLING:   none.

int
osTime::howLong(timeFromTick then)
{
   timeFromTick   now;
   int delta;        // milliseconds

   snapshotTime(&now);
   
   delta = ( (now.sec - then.sec) * 1000)
         + ( (now.nanosec - then.nanosec) / 1000000);

   return(delta);
};

// SPECIFICATION:    howLongMicro.
//                   Returns delta in microseconds between then value
//                   and now.
//
// ERROR HANDLING:   none.

int
osTime::howLongMicro(timeFromTick then)
{
   timeFromTick   now;
   int delta;        // microseconds

   snapshotTime(&now);
   
   delta = ( (now.sec - then.sec) * 1000000)
         + ( (now.nanosec - then.nanosec) / 1000);

   return(delta);
};

      
// SPECIFICATION:    howLongRaw.
//                   Returns delta in msec between previous 
//                   then and now based on the raw clock.
// ERROR HANDLING:   none

int
osTime::howLongRaw(rawTick then)
{
   rawTick   now;
   unsigned long deltaRaw;             // ticks
   int deltaMsec;             // milliseconds    

   snapshotRaw(&now);

   deltaRaw = now - then;

   // convert to msec
   deltaMsec = (int) ( deltaRaw *  (unsigned long)_NanoSecondsPerTick / MILLION ); 
   
   return(deltaMsec);
};


// SPECIFICATION:    delayTime.
//                   osTime method to delay without using a kernel call.
//                   Use sparingly as this holds the processor for the
//                   requested time.
//                   Will not function if deltaTime is greater than 99ms
//
// ERROR HANDLING:   none.

void
osTime::delayTime(int deltaTime)
{
//   trima_assert(deltaTime<100);
   if( deltaTime >= 100 ) _FATAL_ERROR( __FILE__, __LINE__, "osTime::delayTime called with deltaTime > 99ms.");

   timeFromTick now, start;
   int delta=0;

   snapshotTime(&start);

   while(delta < deltaTime)
   {
      snapshotTime(&now);
      
      delta = ( (now.sec - start.sec) * 1000)
            + ( (now.nanosec - start.nanosec) / 1000000);
      if ( (delta < 0) || (delta > 100) )
         snapshotTime(&start);
   }

};
