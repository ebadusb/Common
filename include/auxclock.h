/*
 * Copyright (c) 2002 by Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: //bctquad3/home/BCT_Development/vxWorks/Common/include/rcs/auxclock.h 1.6 2002/12/16 18:29:31Z jl11312 Exp ms10234 $
 * $Log: auxclock.h $
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

typedef long long int rawTick;  /* Type of the auxClockTicks counter */

/* Get the interrupt rate for the auxClockTicks counter. */
#ifdef __cplusplus 
extern "C" int auxClockRateGet();
#else 
int auxClockRateGet();
#endif

/* Initialize the auxClockTicks facility. Call at system initialization time. */
#ifdef __cplusplus 
extern "C" void auxClockInit();
#else 
void auxClockInit();
#endif

/* Enable the auxClockTicks counter. */
#ifdef __cplusplus 
extern "C" int auxClockEnable();
#else 
int auxClockEnable();
#endif

/* Get the current value of the auxClockTicks counter. */
#ifdef __cplusplus 
extern "C" rawTick auxClockTicksGet();
#else 
rawTick auxClockTicksGet();
#endif

/* Convert the given rawTick value to a numeric string. */
#ifdef __cplusplus 
extern "C" char * rawTickString( char * /* String buffer pointer */, rawTick /* tick value to convert to string */ );
#else 
char * rawTickString( char * /* String buffer pointer */, rawTick /* tick value to convert to string */ );
#endif

/* Get the current value of the auxClockTicks counter as a numeric string. */
#ifdef __cplusplus 
extern "C" char *auxClockTicksString();
#else 
char *auxClockTicksString();
#endif

/* Enable the auxClock Semaphore to toggle every given number of microseconds. */
#ifdef __cplusplus 
extern "C" int auxClockSemaphoreEnable( unsigned int /* number of microseconds */ );
#else 
int auxClockSemaphoreEnable( unsigned int /* number of microseconds */ );
#endif

/* Wait for the auxClock Semaphore to toggle. */
#ifdef __cplusplus 
extern "C" int auxClockBlockOnSemaphore();
#else 
int auxClockBlockOnSemaphore();
#endif


/* Enable the auxClock Message Packet queue to send auxClockMuSec every given number of microseconds. */
#ifdef __cplusplus 
extern "C" int auxClockMsgPktEnable( unsigned int /* number of microseconds */,
                          const char * /* Name of Message Packet queue to which to send auxClockMuSec in a Message Packet */,
                          int *        /* Flag for detecting overruns by the calling task */ );
#else 
int auxClockMsgPktEnable( unsigned int /* number of microseconds */,
                          const char * /* Name of Message Packet queue to which to send auxClockMuSec in a Message Packet */,
                          int *        /* Flag for detecting overruns by the calling task */ );
#endif

/* Post a receive for the auxClock Message Packet queue. */
#ifdef __cplusplus 
extern "C" int auxClockMsgPktReceive( mqd_t, long long * /* Pointer to buffer to put current value of the free-running auxClocki microsecond counter  */ );
#else 
int auxClockMsgPktReceive( mqd_t, long long * /* Pointer to buffer to put current value of the free-running auxClocki microsecond counter  */ );
#endif

/* Return the MsgQ overrun count. */
#ifdef __cplusplus 
extern "C" unsigned long int auxClockNotificationOverruns();
#else 
unsigned long int auxClockNotificationOverruns();
#endif

/* Return the MsgQ Send count. */
#ifdef __cplusplus 
extern "C" unsigned long int auxClockNotifications();
#else 
unsigned long int auxClockNotifications();
#endif

/* Return the notification count when the first failure occurred. */
#ifdef __cplusplus 
extern "C" unsigned long int auxClockNotificationFailedAt();
#else 
unsigned long int auxClockNotificationFailedAt();
#endif

/* Return the errno value for the first notification failure that occurred. */
#ifdef __cplusplus 
extern "C" int auxClockNotificationErrno();
#else 
int auxClockNotificationErrno();
#endif

#endif /* !defined(AUXCLOCK_H) */
