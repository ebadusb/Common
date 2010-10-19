/*
 * Copyright (c) 2002 by Gambro BCT, Inc.  All rights reserved.
 *
 * $Header$
 * $Log: auxclock.cpp $
 * Revision 1.14  2004/01/26 18:56:15Z  jl11312
 * - modifications for building under Tornado 2.2
 * Revision 1.13  2003/05/07 20:05:44Z  jl11312
 * - added option to use counting semaphore for aux clock notification
 * Revision 1.12  2003/04/24 19:04:09Z  jl11312
 * - corrected routine to capture current aux clock time (IT 6009)
 * Revision 1.11  2003/01/08 23:41:46Z  ms10234
 * Added function to get the time when the auxClockInit function was called
 * Revision 1.10  2002/12/16 18:30:25Z  jl11312
 * - optimized low-level timer related functions
 * Revision 1.9  2002/12/13 20:47:45  pn02526
 * Reinstate iand revise Semaphore handling in order to facillitate optimizing hardware driver tasks.
 * Revision 1.8  2002/09/25 11:11:18  jl11312
 * - added volatile modifier to variables used in ISR
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
#include <vxWorks.h>
#include <errno.h>
#include <ioLib.h>
#include <semLib.h>

#include "auxclock.h"
#include "messagepacket.h"

/* Free-running counters */
static volatile rawTime auxClockTime;

/* Initialization time */
static struct timespec auxClockInitTime;

/* Nanosecond counter/accumulator increment */
static unsigned long auxClockNanoSecPerTick;

/* Notification periods in nanoseconds */
static unsigned long auxClockQueueNanoSecInterval;
static long long int auxClockSemaphoreNanoSecInterval;

/* Queue notification related data */
static mqd_t auxClockMsgPktQDes;
static MessagePacket auxClockMsgPacket;
static volatile unsigned long auxClockQueueNotifyCount;

/* Semaphore notification related data */
static SEM_ID auxClockSemaphoreID[MaxAuxClockSemaphores];
static SEM_ID isrSemaphoreID[MaxISRSemaphores];
static bool   auxClockInitDone = false;

static void auxClockISR( int arg )
{
	static unsigned long	queueNanoSecIntervalTime = 0;
	static unsigned long semaphoreNanoSecIntervalTime = 0;

	auxClockTime.nanosec += auxClockNanoSecPerTick;
	if ( auxClockTime.nanosec >= 1000000000 )
	{
		auxClockTime.nanosec -= 1000000000;
		auxClockTime.sec += 1;
	}

	/* call any ISR pending semaphores */
	for ( int idx = 0; idx < MaxISRSemaphores; idx++)
	{
		if (isrSemaphoreID[idx]) 
			semGive(isrSemaphoreID[idx]);
	}

	if ( auxClockSemaphoreNanoSecInterval > 0 )
	{
		semaphoreNanoSecIntervalTime += auxClockNanoSecPerTick;
		if ( semaphoreNanoSecIntervalTime >= auxClockSemaphoreNanoSecInterval )
		{
			/* Notify all tasks attached to an aux clock semaphore */
			for ( int idx=0; idx<MaxAuxClockSemaphores; idx++ )
			{
				if ( auxClockSemaphoreID[idx] ) semGive(auxClockSemaphoreID[idx]);
			}

			semaphoreNanoSecIntervalTime -= auxClockSemaphoreNanoSecInterval;
	   }
	}

	if ( auxClockQueueNanoSecInterval > 0 )
	{
		queueNanoSecIntervalTime += auxClockNanoSecPerTick;
		if ( queueNanoSecIntervalTime >= auxClockQueueNanoSecInterval )  /* Is the count completed? */
		{
			queueNanoSecIntervalTime -= auxClockQueueNanoSecInterval;  /* Decrement the notification accumulator, probably leaving a remainder */
	
			/* Fill in the static MessagePacket buffer before sending. */
			auxClockQueueNotifyCount += 1;
			auxClockMsgPacket.msgData().msg( (const unsigned char *) &auxClockQueueNotifyCount, sizeof( auxClockQueueNotifyCount ) );
			auxClockMsgPacket.updateCRC();
	
			/* Post the message on the given Message Packet queue for task notification. */
			mq_send(auxClockMsgPktQDes, &auxClockMsgPacket, sizeof(auxClockMsgPacket), MQ_PRIORITY_MAX);
		}
   }

   return;
}

/* Get the interrupt rate (in Hertz) for the auxClockTicks counter. */
int auxClockRateGet()
{
   return extraAuxClockRateGet();
}

/* Initialize the auxClockTicks facility. Call at system initialization time. */
void auxClockInit()
{
	if ( !auxClockInitDone )
	{
		memset(auxClockSemaphoreID, 0, sizeof(auxClockSemaphoreID));
		memset(isrSemaphoreID, 0, sizeof(isrSemaphoreID));
		auxClockSemaphoreNanoSecInterval = 0;

		auxClockMsgPktQDes = NULL;
		auxClockQueueNanoSecInterval = 0;
		auxClockQueueNotifyCount = 0;

		memset((void *)&auxClockTime, 0, sizeof(auxClockTime));
		auxClockNanoSecPerTick = 1000000000/auxClockRateGet();

		extraAuxClockConnect( (FUNCPTR) &auxClockISR, 0x2BADDEED );
      clock_gettime( CLOCK_REALTIME, &auxClockInitTime );

		auxClockInitDone = true;
   }
}

/* Get the current value of the raw auxClock time. */
void auxClockTimeGet(rawTime * current)
{
   /* loop to make sure you read out the whole rawTime value on the same tick */
   do
   {
      memcpy((void *)current, (void *)&auxClockTime, sizeof(rawTime));
   }
   while ( current->sec != auxClockTime.sec ||
			  current->nanosec != auxClockTime.nanosec );
}

/* Get the current value of the time when the auxClockInit function was called. */
void auxClockInitTimeGet(struct timespec * initTime)
{
   memcpy((void *)initTime, (void *)&auxClockInitTime, sizeof(timespec));
}

/* Enable the auxClock semaphore to toggle every given number of microseconds. */
SEM_ID auxClockSemaphoreAttach(unsigned int microSecInterval, AuxClockSemaphoreType semaphoreType)
{
	SEM_ID	result = NULL;

	if ( !auxClockInitDone )
	{
		auxClockInit();
	}

	/*
	 *	All clock semaphores must be at the same rate (this limitation
	 * is necessary to avoid excess computation time in the timer
	 * interrupt handler).
	 */
	if ( !auxClockSemaphoreNanoSecInterval ||
		  auxClockSemaphoreNanoSecInterval == microSecInterval*1000 )
	{
		auxClockSemaphoreNanoSecInterval = microSecInterval * 1000;
		for ( int idx=0; idx<MaxAuxClockSemaphores && !result; idx++ )
		{
			if ( !auxClockSemaphoreID[idx] )
			{
				switch ( semaphoreType )
				{
				case AuxClockCountingSemaphore:
					auxClockSemaphoreID[idx] = semCCreate(SEM_Q_PRIORITY, SEM_EMPTY);
					break;

				case AuxClockBinarySemaphore:
					auxClockSemaphoreID[idx] = semBCreate(SEM_Q_PRIORITY, SEM_EMPTY);
					break;					
				}
				
				result = auxClockSemaphoreID[idx];
			}
		}
	}

	return result;
}

SEM_ID isrSemaphoreAttach()
{
	SEM_ID result = NULL;

	for (int idx = 0; idx < MaxISRSemaphores && !result; idx++)
	{
		if (!isrSemaphoreID[idx])
		{
			isrSemaphoreID[idx] = semCCreate(SEM_Q_PRIORITY, SEM_EMPTY);
			result = isrSemaphoreID[idx];
		}
	}

	return result;
}

void isrSemaphoreDetach(SEM_ID semaphoreID)
{
	for (int idx = 0; idx < MaxISRSemaphores; idx++)
	{
		if (isrSemaphoreID[idx] == semaphoreID)
		{
			semDelete(semaphoreID);
			isrSemaphoreID[idx] = NULL;
			break;
		}
	}
}

/* Enable the auxClock Message Packet queue to send auxClockMuSec every given number of microseconds. */
void auxClockMsgPktEnable( unsigned int microSecInterval /* number of microseconds */,
                           const char * MsgPktQName      /* Name of Message Packet queue to which to send auxClockMuSec in a Message Packet */ )
{
	if ( !auxClockInitDone )
	{
		auxClockInit();
	}

	if ( !auxClockQueueNanoSecInterval )
	{
		/* Open the Message Packet Queue */
		auxClockMsgPktQDes = mq_open(
                                   MsgPktQName, /* name of queue to open */
                                   O_WRONLY  /* open flags */
                                  );

		if( auxClockMsgPktQDes != (mqd_t)ERROR )
		{
			auxClockMsgPacket.msgData().osCode( MessageData::TIME_UPDATE );
			auxClockQueueNanoSecInterval = microSecInterval * 1000;
	   }
   }
}

