/*
 * Copyright (c) 1995, 1996 by Cobe BCT, Inc.  All rights reserved.
 *
 * $Header: Z:/BCT_Development/Common/ROUTER/rcs/MSG.CPP 1.3 1999/06/02 16:24:42 BS04481 Exp MS10234 $
 * $Log: MSG.CPP $
 * Revision 1.13  1999/04/03 14:53:15  TD10216
 * Revision 1.12  1997/03/24 21:00:58  SS03309
 * Changes message numbering tables to allow for fixed
 * AN2 message numbers and for easier updates
 * Revision 1.11  1996/07/24 19:49:49  SS03309
 * fix MKS 
 * Revision 1.10  1996/07/19 19:17:58  SS03309
 * lint changes
 * Revision 1.9  1996/07/02 23:15:14  SS03309
 * Removed check in interval for zero, zero just disables the timer and is allowed.
 * Revision 1.8  1996/06/28 16:53:59  SS03309
 * Added notes from June 19 design review,
 * 38 - add comment that
 *    time resolution is dependent on ticksize
 * 77 - call interval()
 * 104 - test interval value and fatal error if bad
 * Revision 1.7  1996/06/28 14:24:28  SS03309
 * Comments, defines and constants.  Called interval in constructor
 *
 * TITLE:      Focussed System message router - message classes.
 *
 * ABSTRACT:   These classes support message routing in the Focussed System.
 *             There are several types of messages:
 *             focusTimerMsg - for timers
 *             focusInt32Msg - for int values
 *             focusBufferMsg - for structs and buffers - see buffmsg.hpp
 *
 * DOCUMENTS
 * Requirements:     I:\ieee1498\SSS2.DOC
 * Test:             I:\ieee1498\STD2.DOC
 */

#include <signal.h>
#include <time.h>
#include <sys/proxy.h>
#include <sys/kernel.h>

#include "crc.h"
#include "error.h"
#include "msg.hpp"

// public data

extern dispatcher* dispatch;           // message dispatcher

// constants

static const long MS=1000;             // millisec/sec
static const long US=1000000;          // microsec/sec
//
// timerMsg
//

// SPECIFICATION:    timer message constructor
//                   Parameter:
//                   interval - timer interval in milliseconds, resolution
//                   depends on QNX setting of ticksize.
//
//                   These messages use the QNX timer functions.  Should
//                   the system (QNX) not be able to schedule the timer on
//                   time, QNX will attempt to catch up so that the average rate
//                   of the calls will match the interval chosen.  If accurate
//                   time information is needed, you have to use the QNX clock
//                   functions to determine the real interval between calls.
//
// ERROR HANDLING:   Terminates program.

focusTimerMsg::focusTimerMsg( unsigned long tinterval)
{
   struct sigevent event;           // event structure


   // get proxy for timer
   proxy = qnx_proxy_attach( 0, NULL, 0, -1);
   if (proxy == QNX_ERROR)          // attach failed
   {
      _FATAL_ERROR( __FILE__, __LINE__, TRACE_DISPATCHER, 0, "qnx_proxy_attach()");
   }

   // set event structure and create timer
   event.sigev_signo = -proxy;      // set up proxy
   timerID = timer_create( CLOCK_REALTIME, &event);
   if ( timerID == QNX_ERROR)       // timer create failed
   {
      _FATAL_ERROR( __FILE__, __LINE__, TRACE_DISPATCHER, 0, "timer_create()");
   }

// set dispatcher entry
   dispatch->setTimerEntry( proxy, this);

// set QNX timer, start time and interval
   this->interval( tinterval);
};

// SPECIFICATION:    destructor
//
// ERROR HANDLING:   none.

focusTimerMsg::~focusTimerMsg()
{
   timer_delete( timerID);                // remove QNX timer
   qnx_proxy_detach( proxy);              // remove QNX proxy
   dispatch->clearTimerEntry( proxy);     // clear dispatcher entry
};

// SPECIFICATION:    reset timer interval
//
// ERROR HANDLING:   none.

void
focusTimerMsg::interval( unsigned long tinterval)
{
   struct itimerspec timer;               // timer structure

   // zero interval stops timer

   // set QNX timer, start time and interval
   timer.it_value.tv_sec = tinterval / MS;
   timer.it_interval.tv_sec = tinterval / MS;
   unsigned long fraction = tinterval - MS * timer.it_value.tv_sec;
   fraction *= US;
   timer.it_value.tv_nsec = fraction;
   timer.it_interval.tv_nsec = fraction;
   timer_settime( timerID, 0, &timer, NULL);
};

//
// focusInt32Msg
//

// SPECIFICATION:    constructor
//
// ERROR HANDLING:   none.

// we dont have any messages >65536. Our enums are integers. make the cast.
focusInt32Msg::focusInt32Msg( INT32_MESSAGES mID) :
   routeBuffer( (void**) &message, sizeof( MSG), (unsigned short) mID)
{
   message->data.value = 0;
};

// SPECIFICATION:    destructor
//
// ERROR HANDLING:   none

focusInt32Msg::~focusInt32Msg()
{
   message = NULL;
};

// SPECIFICATION:    get current value
//
// ERROR HANDLING:   none.

long
focusInt32Msg::get() const
{
   return message->data.value;
};

// SPECIFICATION:    set and send new value
//
// ERROR HANDLING:   none.

void
focusInt32Msg::set( long value)
{
   message->data.value = value;
   send();
};

// SPECIFICATION:    send old value
//
// ERROR HANDLING:   none.

void
focusInt32Msg::set()
{
   send();
};


