/*
 * Copyright (c) 1995-1999 by Cobe BCT, Inc.  All rights reserved.
 *
 * $Header: K:/BCT_Development/vxWorks/Common/include/rcs/ostime.hpp 1.7 2002/11/22 21:07:37 pn02526 Exp jl11312 $
 * $Log: ostime.hpp $
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
#include <sys/kernel.h>
#include <sys/osinfo.h>
#include <unistd.h>


typedef struct
{
   time_t   sec;
   time_t   nanosec;
}timeFromTick;

class osTime
{
   public:
      osTime();
      ~osTime();
      inline void snapshotTime(timeFromTick* now);        // get new time from kernel
      void whatTimeIsIt(timeFromTick* now);        // get new time from object
      int howLong(timeFromTick then);              // return delta between then and now
      int howLongMicro(timeFromTick then);         // return delta in usecond between then and now
      int howLongAndUpdate(timeFromTick* then);    // update then with time from object and return delta
      int howLongMicroAndUpdate(timeFromTick* then); // update then with time from object and return delta in usec
      void delayTime(int deltaTime);               // holds in a tight loop for specified time
      int getTimeCounter(void);                    // timeCounter != 0 => time reset in progress
      void countDown(void);                        // decrement timeCounter
      int* _timeCounter;                           
   private:
      struct _timesel far *_timeptr;
      int    _fd_timecounter;
};

#endif




