/*
 * Copyright (c) 1995-1999 by Cobe BCT, Inc.  All rights reserved.
 *
 * $Header: //Bctquad3/HOME/BCT_Development/vxWorks/Common/clocks/rcs/ostime.cpp 1.8 2001/04/05 14:16:14 jl11312 Exp pn02526 $
 * $Log: ostime.cpp $
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
#include <i86.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <signal.h>
#include <time.h>
#include <fcntl.h>
#include <sys/sched.h>
#include <sys/proxy.h>
#include <sys/kernel.h>
#include <unistd.h>
#include <sys/osinfo.h>
#include <sys/mman.h>

#include "error.h"
#include "common.h"
#include "ostime.hpp"

const int MS_EQUIVALENT=1193;          //equivalent of 1ms in raw clock ticks for out clock

// SPECIFICATION:    osTime constructor.
//                   Sets up pointer to kernel's time-tick space and
//                   initializes time counter to indicate no time set
//                   in progress
//
// ERROR HANDLING:   none.

osTime::osTime(void)
{
   struct _osinfo osdata;

   // get pointer to kernel's tick memory
   qnx_osinfo(0, &osdata);
   _timeptr = (struct _timesel far *)MK_FP(osdata.timesel,0);
   if (_timeptr == NULL)
      FATAL_ERROR( __LINE__, __FILE__ "No ticktime pointer!");

   // open shared memory area for time reset indicator
   _fd_timecounter = shm_open("timecounter", O_RDWR, 0777);
   if (_fd_timecounter == -1)
      FATAL_ERROR( __LINE__, __FILE__ "timecounter share create failed");

   // map the share memory objects
   _timeCounter = (int *)mmap(0,sizeof(int), PROT_READ|PROT_WRITE, MAP_SHARED, 
                                    _fd_timecounter, 0);
   if (_timeCounter == (void *) -1)
      FATAL_ERROR( __LINE__, __FILE__ "timecounter share map failed");

   // initialize counter to indicate no time set is in progress
   *_timeCounter = 0;
};

// SPECIFICATION:    osTime destructor.
//
// ERROR HANDLING:   none.

osTime::~osTime()
{
   _timeptr = NULL;
};



// SPECIFICATION:    snapshotTime.
//                   osTime method to get time from the kernel.
//
// ERROR HANDLING:   none.

inline void
osTime::snapshotTime(timeFromTick* now)
{
   // loop to make sure you take both readings on the same tick
   do
   {
      now->sec = _timeptr->seconds;
      now->nanosec = _timeptr->nsec;

   } while ( now->sec != _timeptr->seconds || now->nanosec != _timeptr->nsec  );
};


// SPECIFICATION:    snapshotRaw.
//                   osTime method to get raw ticks from the kernel.
//
// ERROR HANDLING:   none.

inline void
osTime::snapshotRaw(rawTick* now)
{
   // loop to make sure you take both readings on the same tick
   do
   {
      now->sec = _timeptr->seconds;
      now->nanosec = _timeptr->nsec;
      now->cnt8254 = _timeptr->cnt8254;
      now->cycles_per_sec = _timeptr->cycles_per_sec;
      now->cycle = _timeptr->cycle_lo;

   } while ( now->sec != _timeptr->seconds || now->nanosec != _timeptr->nsec  );
};


// SPECIFICATION:    whatTimeIsIt.
//                   Just like snapshotTime except not inline
//
// ERROR HANDLING:   none.

void 
osTime::whatTimeIsIt(timeFromTick* now)
{
   // loop to make sure you take both readings on the same tick
   do
   {
      now->sec = _timeptr->seconds;
      now->nanosec = _timeptr->nsec;

   } while ( now->sec != _timeptr->seconds || now->nanosec != _timeptr->nsec  );
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
//
// ERROR HANDLING:   FATAL_ERROR if the ticksize is not 2.
//                   FATAL_ERROR if the clock has changed.

int
osTime::howLongRaw(rawTick then)
{
   rawTick   now;
   long deltaRaw;             // ticks
   int deltaMsec;             // milliseconds    

   snapshotRaw(&now);

   if (now.cnt8254 != 2386) // magic number that represents 2ms tick
      FATAL_ERROR( __LINE__, __FILE__ "Ticksize is not 2!");
   if (now.cycles_per_sec != 1193181) // magic number that represents the clock rate of our clock
      FATAL_ERROR( __LINE__, __FILE__ "Clock chip is wrong!");

   // this element of the timesel structure is not sensitive 
   // changes to the realtime clock.  This value should increment by
   // 23860 (decimal) each tick which is 1193000 each second.  
   // The register will wrap in just over 3600 seconds but QNX 
   // handles the wrap.
   deltaRaw = now.cycle - then.cycle;

   // convert to msec
   deltaMsec = deltaRaw / MS_EQUIVALENT; 
   
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
   ASSERT(deltaTime<100);

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


// SPECIFICATION:    getTimeCounter.
//                   returns current value of timeCounter which is 
//                   non-zero if a time set operation is in progress
//
// ERROR HANDLING:   none.

int
osTime::getTimeCounter(void)
{
   return(*_timeCounter);
};

// SPECIFICATION:    countDown.
//                   counts down timeCounter.  When timeCounter hits
//                   zero (or is ordered to zero), the soft watchdogs
//                   are re-enabled.
//
// ERROR HANDLING:   none.

void
osTime::countDown(void)
{
   *_timeCounter -= 1;
}


