/*
 * Copyright (c) 2002 by Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: //bctquad3/home/BCT_Development/vxWorks/Common/include/rcs/auxclock.h 1.6 2002/12/16 18:29:31Z jl11312 Exp ms10234 $
 * $Log: auxclock.h $
 * Revision 1.5  2002/12/13 19:03:06  pn02526
 * Reinstate Semaphore handling in order to facillitate optimizing hardware driver tasks.
 * Revision 1.4  2002/07/18 13:18:22  pn02526
 * Change notification interface to use microseconds rather than ticks.
 * Revision 1.3  2002/06/19 16:59:28  pn02526
 * Make usable by either C or C++ programs.  Modify some comments.
 * Revision 1.2  2002/06/19 09:57:01  pn02526
 * Change the typedef for the tick counter to long long; completely qualify other type declarations; correct spelling in comments.
 * Revision 1.1  2002/06/18 09:07:27  pn02526
 * Initial revision
 *
 * TITLE:      auxclock.h, Header file for the auxiliary clock ISR and support functions.
 *
 * ABSTRACT:   This module contains the header file info for the ISR
 *             used under vxWorks to access the auxiliary hardware clock,
 *             keep a tick counter, and notify a timer task when given tick
 *             periods elapse.
 */
#if !defined(AUXCLOCK_H)
#define AUXCLOCK_H

#include <mqueue.h>
#include <time.h>

typedef struct
{
   time_t   sec;
   long     nanosec;
} rawTime;

#ifdef __cplusplus
extern "C" {
#endif /* ifdef __cplusplus */

/* Get the interrupt rate (in Hertz) for the auxClockTicks counter. */
int auxClockRateGet();

/* Initialize the auxClockTicks facility. Call at system initialization time. */
void auxClockInit();

/* Get the current value of the raw auxClock time. */
void auxClockTimeGet(rawTime *);

/* Enable the auxClock Semaphore to toggle every given number of microseconds. */
void auxClockSemaphoreEnable( unsigned int /* number of microseconds */ );

/* Wait for the auxClock Semaphore to toggle. */
STATUS auxClockBlockOnSemaphore();

/* Enable the auxClock Message Packet queue to send auxClockMuSec every given number of microseconds. */
void auxClockMsgPktEnable( unsigned int /* number of microseconds */,
                           const char * /* Name of Message Packet queue to which to send auxClockMuSec in a Message Packet */ );

/* Return the MsgQ Send count. */
unsigned long int auxClockQueueNotifications();

/* Return the notification count when the first failure occurred. */
unsigned long int auxClockQueueNotificationFailedAt();

/* Return the errno value for the first notification failure that occurred. */
int auxClockQueueNotificationErrno();

#ifdef __cplusplus
}; // extern "C"
#endif /* ifdef __cplusplus */

#endif /* !defined(AUXCLOCK_H) */
