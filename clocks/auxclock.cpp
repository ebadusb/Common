/*
 * Copyright (c) 2002 by Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: //bctquad3/home/BCT_Development/vxWorks/Common/clocks/rcs/auxclock.cpp 1.4 2002/06/19 22:53:29 pn02526 Exp ms10234 $
 * $Log: auxclock.cpp $
 *
 * TITLE:      auxclock.cpp, Auxilliary clock ISR and support functions.
 *
 * ABSTRACT:   This module contains the ISR used under vxWorks to access
 *             the auxilliary hardware clock, keep a tick counter, and
 *             notify a timer task when given tick periods elapse.
 *
 * PUBLIC FUNCTIONS:
 *  int auxClockRateGet();                           Get the interrupt rate for the auxClockTicks counter.
 *  void auxClockInit();                             Initialize the auxClockTicks facility.
 *                                                        Call at system initialization time.
 *  int auxClockEnable();                            Enable the auxClockTicks counter.
 *  rawTick auxClockTicksGet();                      Get the current value of the auxClockTicks counter.
 *  char *auxClockTicksString();                     Get the current value of the auxClockTicks counter as a decimal string.
 *  int auxClockMsgPktEnable(                        Enable the auxClock Message Packet queue to send auxClockTicks every given number of ticks.
 *        unsigned int number_of_ticks,
 *        const char * Name_of_Message_Packet_queue,
 *        int *        Flag_for_detecting_overruns );
 *  int auxClockMsgPktReceive( mqd_t, rawTick *  );  Post a receive for the auxClock Message Packet queue.
 *  unsigned long auxClockNotificationOverruns();    Return the notification overrun count.
 *  unsigned long auxClockNotifications();           Return the notification count.
 *  unsigned long auxClockNotificationFailedAt();    Return the notification count when the first failure occurred.
 *  int auxClockNotificationErrno();                 Return the errno value for the first notification failure that occurred.
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
#include <intLib.h>
#include <extraAuxClock.h>

#include "auxclock.h"

#include "messagepacket.h"

/* Define shift counts for the unsigned long long division used in rawTickString(). */
#define    SHIFTMID  (((int)(sizeof(unsigned long long)*CHAR_BIT))/3)
#define    SHIFTHI  ((2*((int)(sizeof(unsigned long long)*CHAR_BIT)))/3)

static rawTick auxClockTicks;
static int *auxClockNotifyPointer;
static char auxClockTicksStr[21];

static unsigned int auxClockNotifyDivisor;
static unsigned int auxClockNotifyCountDown;
static unsigned int auxClockNotifyExpected;
static mqd_t auxClockMsgPktQDes;
static MessagePacket auxClockMsgPacket;
static unsigned long auxClockNotifyCount;
static unsigned long auxClockNotifyOverruns;
static unsigned long auxClockNotifyFailedAt;
static int auxClockNotifyErrno;

/*                                                            */
/* Interrupt Service Routine for the vanilla auxClockTicks counter. */
/*                                                            */
static void auxClockISR( int arg )
{
    auxClockTicks++;            /* Bump the tick counter. */

    return;
}

/*                                                            */
/* Interrupt Service Routine for the Message Packet auxClockTicks counter. */
/*                                                            */
static void auxClockMsgPktISR( int arg )
{
    auxClockTicks++;            /* Bump the tick counter. */

    if( auxClockNotifyCountDown )  /* A non-zero Count Down Counter indicates that notification is enabled. */
    {
       if( (--auxClockNotifyCountDown) == 0 )         /* Is the count down completed? */
       {
           auxClockNotifyCountDown = auxClockNotifyDivisor;  /* Reset the count down divisor for the next period. */
           if( auxClockMsgPktQDes )                        /* Make sure we have a non-NULL Message Queue Descriptor. */
           {
               /* Fill in the static MessagePacket buffer before sending. */
               auxClockMsgPacket.msgData().msg( (const unsigned char *) &auxClockTicks, (int) sizeof( auxClockTicks ) );
               auxClockMsgPacket.updateCRC();
               if( ! *auxClockNotifyPointer )     /* Is a task expecting the notification? */
               {                                     /* There is not a task expecting the notification. */
                   auxClockNotifyOverruns++;         /* So, bump the overrun counter. */
               }
               /* Post the message on the given Message Packet queue. */
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
    }
    return;
}

/* Get the interrupt rate for the auxClockTicks counter. */
int auxClockRateGet()
{
    return extraAuxClockRateGet();
}

/* Initialize the auxClockTicks facility. Call at system initialiazation time. */
void auxClockInit()
{
    auxClockNotifyDivisor = 0;
    auxClockNotifyCountDown = 0;
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

unsigned long long rawTickDivideByUlong( rawTick dividend, unsigned long divisor )
{
    rawTick rTmidlo, quotient;
    unsigned long ulHi, ulMid, ulLow;

    /*  The following code implements a "divide-by-n" for rawTick's.
        (Apparently, the vxWorks-supplied ull divide operator doesn't work) */
    ulHi = (unsigned long) ( dividend >> SHIFTHI ) ;                           /* Get the High-order 22 bits */
    quotient = dividend - ( ((rawTick)( ulHi )) << SHIFTHI );
    ulMid = (unsigned long) ( ( quotient ) >> SHIFTMID );                      /* Get the Mid-order 21 bits */
    ulLow = (unsigned long) ( quotient - ((rawTick)( ulMid ) << SHIFTMID)  );  /* Get the Low-order 21 bits */

    ulMid += (ulHi % divisor) << (SHIFTHI - SHIFTMID); /* Add the High-order n's remainder to the Mid-order dividend */
    ulLow += (ulMid % divisor) << SHIFTMID;            /* Add the Mid-order n's remainder to the Low-order dividend */

    /* Complete the divide-by-n */
    quotient  = ( ( (rawTick)( ulHi / divisor ) ) << SHIFTHI )
              + (((rawTick)( ulMid / divisor )) << SHIFTMID )
              + (rawTick)( ulLow / divisor );

    return( quotient );
}

/* Workaround for the lack of a %ull format in vxWorks' [sf]printf() */
char * rawTickString( char * s, rawTick rTvalue )
{
    rawTick rTmidlo;
    unsigned long ulHi, ulMid, ulLow;
    char *cp, *cp1;
    char c;

    /* Build the string in the buffer backwards, since that is easier.  It will be reversed below. */
    cp = s;
    do {
        /*  The following code implements a "divide-by-10" for rawTick's.
            (Apparently, the vxWorks-supplied ull divide operator doesn't work) */
        ulHi = (unsigned long) ( rTvalue >> SHIFTHI ) ;                           /* Get the High-order 22 bits */
        rTmidlo = rTvalue - ( ((rawTick)( ulHi )) << SHIFTHI );
        ulMid = (unsigned long) ( ( rTmidlo ) >> SHIFTMID );                      /* Get the Mid-order 21 bits */
        ulLow = (unsigned long) ( rTmidlo - ((rawTick)( ulMid ) << SHIFTMID)  );  /* Get the Low-order 21 bits */

        ulMid += (ulHi % 10) << (SHIFTHI - SHIFTMID); /* Add the High-order 10s remainder to the Mid-order dividend */
        ulLow += (ulMid % 10) << SHIFTMID;            /* Add the Mid-order 10s remainder to the Low-order dividend */

        /* Complete the divide-by-10 */
        rTvalue = ( ( (rawTick)( ulHi / 10 ) ) << SHIFTHI )
                  + (((rawTick)( ulMid / 10 )) << SHIFTMID )
                  + (rawTick)( ulLow / 10 );

        *cp++ = (char)( ulLow%10 + '0' );  /* Convert the Low-order digit to a char and put it in the string */

    }
    while( rTvalue > 0 && cp - s < 20 );
    /* Null-terminate the string */
    *cp='\0';

    /* Since the string was built low->high order in a left->right orientation,
       reverse the order of the characters so that it reads high->low. */
    for( cp1 = s; cp1 < (--cp); cp1++)
    {
        c = *cp;
        *cp = *cp1;
        *cp1 = c;
    }

    return s;
}

/* Get the current value of the auxClockTicks counter as a decimal string. */
char *auxClockTicksString()
{
    return( rawTickString( auxClockTicksStr, auxClockTicksGet() ) );
}

/* Enable the auxClock Message Packet queue to send auxClockTicks every given number of ticks. */
int auxClockMsgPktEnable( unsigned int periodicity /* number of ticks */,
                          const char * MsgPktQName  /* Name of Message Packet queue to which to send auxClockTicks in a Message Packet */,
                          int *MsgPktOverrunFlag /* Flag for detecting overruns by the calling task */ )
{
    int lockKey;

    if( auxClockTicks == (rawTick)0 )  /* At least 1 tick will have accumulated if auxClock is enabled */
    {
        /* Can't enable a Message Packet queue without an ISR enabled! */
        return( FALSE );
    }

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

    /* Open the Message Packet Queue if necessary */
    if( auxClockNotifyDivisor == 0 ) 
    {
        auxClockMsgPktQDes = mq_open(
                                    MsgPktQName, /* name of queue to open */
/*                                    O_WRONLY | O_NONBLOCK  /* open flags */
                                    O_WRONLY  /* open flags */
                                   );
    }
    if( auxClockMsgPktQDes == (mqd_t)ERROR )
    {
        /* Can't enable a Message Packet queue if the queue can't be opened! */
        return FALSE;
    }


    lockKey = intLock();

    /* Is a notification already enabled? */
    if( auxClockNotifyDivisor == 0 ) 
    {
        /* No, set it up */
        auxClockMsgPacket.msgData().osCode( MessageData::TIME_UPDATE );
        auxClockNotifyCountDown = 0;
        auxClockNotifyDivisor = periodicity;
        *MsgPktOverrunFlag = FALSE;
        auxClockNotifyPointer = MsgPktOverrunFlag; 
    }
    else
    {
        intUnlock( lockKey );
        /* Can't enable a Message Packet queue if notification is already enabled! */
        return( FALSE );
    }

    intUnlock( lockKey );


    if( extraAuxClockConnect( (FUNCPTR) &auxClockMsgPktISR, (int)0x3BADDEED ) == ERROR )
    {
        /* Enabling the extraAuxClock failed! */
        return( FALSE );
    }

    /* This tells the ISR to start mq_send-ing the auxClockTicks value. */
    auxClockNotifyCountDown = auxClockNotifyDivisor;

    return( TRUE );
}

/* Post a receive for the auxClock Message Packet queue. */
int auxClockMsgPktReceive( mqd_t MsgPktQDes, rawTick *ticks /* Pointer to buffer to put current value of the auxClock tick counter */ )
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
    memmove( (char *) ticks , 
             (char *) msgPacket.msgData().msg(), 
             sizeof( *ticks ) );
    return( TRUE );
}

/* Return the notification overrun count. */
unsigned long auxClockNotificationOverruns()
{
    return auxClockNotifyOverruns;
}

/* Return the notification count. */
unsigned long auxClockNotifications()
{
    return auxClockNotifyCount;
}

/* Return the notification count when the first failure occurred. */
unsigned long auxClockNotificationFailedAt()
{
    return auxClockNotifyFailedAt;
}

/* Return the errno value for the first notification failure that occurred. */
int auxClockNotificationErrno()
{
    return auxClockNotifyErrno;
}

