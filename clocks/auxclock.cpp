/*
 * Copyright (c) 2002 by Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: //bctquad3/home/BCT_Development/vxWorks/Common/clocks/rcs/auxclock.cpp 1.10 2002/12/16 18:30:25Z jl11312 Exp ms10234 $
 * $Log: auxclock.cpp $
 * Revision 1.7  2002/08/16 16:27:05  pn02526
 * Clear overrun counter when user requests its value.  Per request from Mark Scott to prevent overlogging the event per Scott Butzke.
 * Revision 1.6  2002/07/18 13:18:37  pn02526
 * Change notification interface to use microseconds rather than ticks.
 * Revision 1.5  2002/07/02 10:06:04  ms10234
 * Removed a check to see in the interrupt was enabled inside the function which enabled the interrupt.
 * Revision 1.4  2002/06/19 22:53:29  pn02526
 * Change rawTickString to use sprintf to format the string, rather than its own conversion code.
 * Revision 1.3  2002/06/19 10:01:14  pn02526
 * Simplify the rawTick string formatting code because of the change in typdef from unsigned long long to long long int; completely qualify other type declarations; correct spelling in comments.
 * Revision 1.2  2002/06/18 10:28:09  pn02526
 * Remove embedded comment for compiler.
 * Revision 1.1  2002/06/18 08:55:18  pn02526
 * Initial revision
 *
 * TITLE:      auxclock.cpp, Auxiliary clock ISR and support functions.
 *
 * ABSTRACT:   This module contains the ISR used under vxWorks to access
 *             the auxiliary hardware clock, keep a tick counter, and
 *             notify a timer task when given tick periods elapse.
 *
 * PUBLIC FUNCTIONS:
 *  int auxClockRateGet();                            Get the interrupt rate for the auxClockTicks counter.
 *  void auxClockInit();                              Initialize the auxClockTicks facility.
 *                                                         Call at system initialization time.
 *  int auxClockEnable();                             Enable the auxClockTicks counter.
 *  rawTick auxClockTicksGet();                       Get the current value of the auxClockTicks counter.
 *  char *auxClockTicksString();                      Get the current value of the auxClockTicks counter as a decimal string.
 *  int auxClockMsgPktEnable(                         Enable the auxClock Message Packet queue to send auxClockMuSec every given number of microseconds.
 *        unsigned int number_of_microseconds,
 *        const char * Name_of_Message_Packet_queue,
 *        int *        Flag_for_detecting_overruns );
 *  int auxClockMsgPktReceive( mqd_t, long long *  ); Post a receive for the auxClock Message Packet queue.
 *  unsigned long int auxClockNotificationOverruns(); Return the notification overrun count.
 *  unsigned long int auxClockNotifications();        Return the notification count.
 *  unsigned long int auxClockNotificationFailedAt(); Return the notification count when the first failure occurred.
 *  int auxClockNotificationErrno();                  Return the errno value for the first notification failure that occurred.
 */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <sysLib.h>
#include <ioLib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <iv.h>
#include <extraAuxClock.h>

#include "auxclock.h"

#include "messagepacket.h"

/* Free-running counters */
static volatile rawTick auxClockTicks;
static volatile long long int auxClockMuSec; /* microseconds */

/* Notification microsecond accumulator  */
static volatile long long int auxClockNotifyMuSec;

/* Microsecond counter/accumulator increment */
static long long int auxClockMuSecPerTick;

/* Notification period in microseconds */
static long long int auxClockNotifyMuSecPerPeriod;

static int *auxClockNotifyPointer;
static char auxClockTicksStr[21];

static unsigned int auxClockNotifyExpected;
static mqd_t auxClockMsgPktQDes;
static MessagePacket auxClockMsgPacket;
static volatile unsigned long int auxClockNotifyCount;
static volatile unsigned long int auxClockNotifyOverruns;
static volatile unsigned long int auxClockNotifyFailedAt;
static volatile int auxClockNotifyErrno;

/*                                                            */
/* Interrupt Service Routine for the vanilla (non-notification) auxClockTicks counter. */
/*                                                            */
static void auxClockISR( int arg )
{
    auxClockTicks++;            /* Bump the tick counter. */

    return;
}

/*                                                            */
/* Interrupt Service Routine for the Message Packet (notification) auxClockTicks counter. */
/*                                                            */
static void auxClockMsgPktISR( int arg )
{
    auxClockTicks++;            /* Bump the tick counter. */

    auxClockNotifyMuSec += auxClockMuSecPerTick; /* Increment the notification microsecond accumulator */
    auxClockMuSec += auxClockMuSecPerTick;       /* Increment the free-running microsecond counter */
    if( auxClockNotifyMuSec >= auxClockNotifyMuSecPerPeriod )  /* Is the count completed? */
    {
        auxClockNotifyMuSec -= auxClockNotifyMuSecPerPeriod; /* Decrement the notification accumulator, probably leaving a remainder */
        if( auxClockMsgPktQDes )                        /* Make sure we have a non-NULL Message Queue Descriptor. */
        {
            /* Fill in the static MessagePacket buffer before sending. */
            auxClockMsgPacket.msgData().msg( (const unsigned char *) &auxClockMuSec, (int) sizeof( auxClockMuSec ) );
            auxClockMsgPacket.updateCRC();
            if( ! *auxClockNotifyPointer )     /* Is a task expecting the notification? */
            {                                     /* There is not a task expecting the notification. */
                auxClockNotifyOverruns++;         /* So, bump the overrun counter. */
            }
            /* Post the message on the given Message Packet queue for task notification. */
            auxClockNotifyCount++;            /* Bump the notification counter. */
            if( mq_send (
                      auxClockMsgPktQDes,                    /* message queue descriptor */
                      (const void *) &auxClockMsgPacket,    /* message to send */
                      (size_t) sizeof( auxClockMsgPacket ), /* size of message, in bytes */
                      MQ_PRIORITY_MAX                       /* priority of message */
                    ) == ERROR  && auxClockNotifyFailedAt == 0 )
            {
                auxClockNotifyErrno = errno;
                auxClockNotifyFailedAt = auxClockNotifyCount;
            }
            *auxClockNotifyPointer = FALSE;   /* Clear the notification-expected flag. */  
        }
    }
    return;
}

/* Get the interrupt rate for the auxClockTicks counter. */
int auxClockRateGet()
{
    return extraAuxClockRateGet();
}

/* Initialize the auxClockTicks facility. Call at system initialization time. */
void auxClockInit()
{
    auxClockNotifyMuSecPerPeriod = 0ll;
    auxClockNotifyMuSec = 0ll;
    auxClockNotifyCount = 0;
    auxClockNotifyExpected = FALSE;
    auxClockNotifyPointer = (int *) &auxClockNotifyExpected;
    auxClockNotifyOverruns = 0;
    auxClockNotifyFailedAt = 0;
    auxClockNotifyErrno = 0;
    auxClockTicks=(rawTick)0;
}

/* Enable the auxClockTicks counter */
int auxClockEnable( )
{
    if( auxClockTicks > (rawTick)0 )  /* At least 1 tick will have accumulated if auxClock is already enabled */
    {
        /* Can't enabled an extraAuxClock that's already enabled! */
        return( FALSE );
    }

    if(  extraAuxClockConnect( (FUNCPTR) &auxClockISR, 0xABADDEED ) == ERROR )
    {
        /* Enabling the extraAuxClock failed! */
        return( FALSE );
    }

    /* Wait for at least 1 tick to accumulate (if we hang here, extraAuxClock isn't running!) */
    while( auxClockTicks == (rawTick)0 );

    return( TRUE );
}

/* Get the current value of the auxClockTicks counter. */
rawTick auxClockTicksGet()
{
    rawTick t;

    /* loop to make sure you read out the whole rawTick value on the same tick */
    do
    {
       t = auxClockTicks;
    }
    while ( t != auxClockTicks );

    return( t );
}

/* Convert the given rawTick value to a numeric string. */
char * rawTickString( char * s, rawTick rtValue )
{
    sprintf( s, "%lld", rtValue );
    return s;
}

/* Get the current value of the auxClockTicks counter as a numeric string. */
char *auxClockTicksString()
{
    return( rawTickString( auxClockTicksStr, auxClockTicksGet() ) );
}

/* Enable the auxClock Message Packet queue to send auxClockMuSec every given number of microseconds. */
int auxClockMsgPktEnable( unsigned int periodicity /* number of microseconds */,
                          const char * MsgPktQName  /* Name of Message Packet queue to which to send auxClockMuSec in a Message Packet */,
                          int *MsgPktOverrunFlag /* Flag for detecting overruns by the calling task */ )
{
    if (periodicity < 1)
    {
        /* Can't enable a Message Packet queue with a <= 0 period! */
        return( FALSE );
    }

    if (!MsgPktOverrunFlag)
    {
        /* Can't enable a Message Packet queue without an overrun flag! */
        return( FALSE );
    }

    /* Is a notification already enabled? */
    if( auxClockNotifyMuSecPerPeriod > 0ll ) 
    {
        /* Can't enable a Message Packet queue if notification is already enabled! */
        return( FALSE );
    }

    /* Open the Message Packet Queue */
    auxClockMsgPktQDes = mq_open(
                                 MsgPktQName, /* name of queue to open */
                                 O_WRONLY  /* open flags */
                                );
    if( auxClockMsgPktQDes == (mqd_t)ERROR )
    {
        /* Can't enable a Message Packet queue if the queue can't be opened! */
        return FALSE;
    }

    auxClockMsgPacket.msgData().osCode( MessageData::TIME_UPDATE );
    *MsgPktOverrunFlag = FALSE;
    auxClockNotifyPointer = MsgPktOverrunFlag; 

    /* Clear the free-running microsecond counter */
    auxClockMuSec = 0ll;

    /* Clear the microsecond accumulator */
    auxClockNotifyMuSec = 0ll;

    /* Set up the ISR's microsecond increment value */
    auxClockMuSecPerTick = 1000000ll / ( (long long)auxClockRateGet() );

    /* Give the ISR the average microsecond periodicity of notification */
    auxClockNotifyMuSecPerPeriod = (long long)periodicity;

    /* This tells the ISR to start mq_send-ing the auxClockMuSec value. */
    if( extraAuxClockConnect( (FUNCPTR) &auxClockMsgPktISR, (int)0x3BADDEED ) == ERROR )
    {
        /* Enabling the extraAuxClock failed! */
        return( FALSE );
    }

    return( TRUE );
}

/* Post a receive for the auxClock Message Packet queue. */
int auxClockMsgPktReceive( mqd_t MsgPktQDes, long long *pMuSec /* Pointer to buffer to put current value of the free-running auxClocki microsecond counter  */ )
{
    MessagePacket msgPacket;
    int msgPrio;

    if( MsgPktQDes == (mqd_t)NULL ) 
    {
        /* Can't post to a queue that isn't enabled for the ISR! */
        return( FALSE );
    }
    else
    {
        do
        {
            *auxClockNotifyPointer = TRUE;   /* Let ISR know it is expected */
            /* DO NOT INSERT ANY CODE HERE!!!!! */
            if( mq_receive(
                             MsgPktQDes,     /* Message Packet queue from which to receive */
                             (void *) &msgPacket,    /* buffer to receive message */
                             (size_t) sizeof( msgPacket ), /* length of buffer */
                              &msgPrio /* if not NULL, priority of message */
                           )  == ERROR )
            {
                /* mq_receive request unsuccessful. */
                return( FALSE );
            }
        } while ( msgPacket.msgData().osCode() != MessageData::TIME_UPDATE );
    }
    memmove( (char *) pMuSec , 
             (char *) msgPacket.msgData().msg(), 
             sizeof( *pMuSec ) );
    return( TRUE );
}

/* Return the notification overrun count. */
unsigned long int auxClockNotificationOverruns()
{
    unsigned long int overruns = auxClockNotifyOverruns;
    auxClockNotifyOverruns = 0;
    return overruns;
}

/* Return the notification count. */
unsigned long int auxClockNotifications()
{
    return auxClockNotifyCount;
}

/* Return the notification count when the first failure occurred. */
unsigned long int auxClockNotificationFailedAt()
{
    return auxClockNotifyFailedAt;
}

/* Return the errno value for the first notification failure that occurred. */
int auxClockNotificationErrno()
{
    return auxClockNotifyErrno;
}

