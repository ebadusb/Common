/*
 * Copyright (c) 2002 by Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: Q:/BCT_Development/vxWorks/Common/include/rcs/auxclock.h 1.10 2004/05/18 15:58:58Z jl11312 Exp jd11007 $
 * $Log: auxclock.h $
 * Revision 1.9  2004/01/26 18:51:21Z  jl11312
 * - modifications for building under Tornado 2.2
 * Revision 1.8  2003/05/07 20:06:01Z  jl11312
 * - added option to use counting semaphore for aux clock notification
 * Revision 1.7  2003/01/08 23:43:01Z  ms10234
 * Added new function to return auxClock initialization time.
 * Revision 1.6  2002/12/16 18:29:31Z  jl11312
 * - optimized low-level timer related functions
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

#include <vxWorks.h>
#include <mqueue.h>
#include <semLib.h>
#include <time.h>

typedef struct
{
   time_t   sec;
   long     nanosec;
} rawTime;

#ifdef __cplusplus
extern "C" {
#endif /* ifdef __cplusplus */

/* Maximum number of tasks attached to an aux clock driven semaphore */
enum { MaxAuxClockSemaphores = 3 };

/* Get the interrupt rate (in Hertz) for the auxClockTicks counter. */
int auxClockRateGet(void);

/* Initialize the auxClockTicks facility. Call at system initialization time. */
void auxClockInit(void);

/* Get the current value of the raw auxClock time. */
void auxClockTimeGet(rawTime * current);

/* Get the time the auxClock was initialized. */
void auxClockInitTimeGet(struct timespec * initTime);

/* Enable the auxClock Semaphore to toggle every given number of microseconds. */
typedef enum
{
	AuxClockCountingSemaphore,
	AuxClockBinarySemaphore
} AuxClockSemaphoreType;

SEM_ID auxClockSemaphoreAttach(unsigned int microSecInterval, AuxClockSemaphoreType semaphoreType);

/* Enable the auxClock Message Packet queue to send auxClockMuSec every given number of microseconds. */
void auxClockMsgPktEnable(unsigned int microSecInterval, const char * MsgPktQName);

/* additions to system aux clock interface */
int extraAuxClockRateGet(void);
STATUS extraAuxClockConnect(FUNCPTR routine, int arg);

#ifdef __cplusplus
}; // extern "C"
#endif /* ifdef __cplusplus */

#endif /* !defined(AUXCLOCK_H) */
