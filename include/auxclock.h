/*
 * Copyright (c) 2002 by Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: //bctquad3/home/BCT_Development/vxWorks/Common/include/rcs/auxclock.h 1.6 2002/12/16 18:29:31Z jl11312 Exp ms10234 $
 * $Log: auxclock.h $
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
int auxClockRateGet();

/* Initialize the auxClockTicks facility. Call at system initialization time. */
void auxClockInit();

/* Enable the auxClockTicks counter. */
int auxClockEnable();

/* Get the current value of the auxClockTicks counter. */
rawTick auxClockTicksGet();

/* Workaround for the lack of a %ll format in vxWorks' [sf]printf() */
char * rawTickString( char * /* String buffer pointer */, rawTick /* tick value to convert to string */ );

/* Get the current value of the auxClockTicks counter as a decimal string. */
char *auxClockTicksString();

/* Enable the auxClock Message Packet queue to send auxClockTicks every given number of ticks. */
int auxClockMsgPktEnable( unsigned int /* number of ticks */,
                          const char * /* Name of Message Packet queue to which to send auxClockTicks in a Message Packet */,
                          int *        /* Flag for detecting overruns by the calling task */ );
/* Post a receive for the auxClock Message Packet queue. */
int auxClockMsgPktReceive( mqd_t, rawTick * /* Pointer to buffer to put current value of the auxClock tick counter */ );

/* Return the notification overrun count. */
unsigned long int auxClockNotificationOverruns();

/* Return the notification count. */
unsigned long int auxClockNotifications();

/* Return the notification count when the first failure occurred. */
unsigned long int auxClockNotificationFailedAt();

/* Return the errno value for the first notification failure that occurred. */
int auxClockNotificationErrno();

#endif /* !defined(AUXCLOCK_H) */
