/*
 * Copyright (c) 1995-1999 by Cobe BCT, Inc.  All rights reserved.
 *
 * $Header: //Bctquad3/HOME/BCT_Development/vxWorks/Common/clocks/rcs/ostime.cpp 1.8 2001/04/05 14:16:14 jl11312 Exp pn02526 $
 * $Log: ostime.cpp $
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

const int TICKTIME=2;                  // our ticksize is set to 2 in the sysinit

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
   
   delta = ( ( (now.sec * 1000) + (now.nanosec / 1000000) )
           - ( (then->sec * 1000) + (then->nanosec / 1000000) ) );

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
   
   delta = ( ( (now.sec * 1000000) + (now.nanosec / 1000) )
           - ( (then->sec * 1000000) + (then->nanosec / 1000) ) );

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
   
   delta = ( ( (now.sec * 1000) + (now.nanosec / 1000000) )
           - ( (then.sec * 1000) + (then.nanosec / 1000000) ) );

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
   
   delta = ( ( (now.sec * 1000000) + (now.nanosec / 1000) )
           - ( (then.sec * 1000000) + (then.nanosec / 1000) ) );

   return(delta);
};


// SPECIFICATION:    delayTime.
//                   osTime method to delay without using a kernel call.
//                   Use sparingly as this holds the processor for the
//                   requested time.
//
// ERROR HANDLING:   none.

void
osTime::delayTime(int deltaTime)
{
   timeFromTick now, start;
   int delta=0;

   snapshotTime(&start);
   
   while(delta < deltaTime)
   {
      snapshotTime(&now);
      
      delta = ( ( (now.sec * 1000) + (now.nanosec / 1000000) )
              - ( (start.sec * 1000) + (start.nanosec / 1000000) ) );
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


