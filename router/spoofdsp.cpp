/*
 * Copyright (c) 1995, 1996 by Cobe BCT, Inc.  All rights reserved.
 *
 * $Header: Y:/BCT_Development/Common/ROUTER/rcs/SPOOFDSP.CPP 1.3 1999/06/14 23:10:32 BS04481 Exp MS10234 $
 * $Log: SPOOFDSP.CPP $
 * Revision 1.2  1999/05/31 20:35:15  BS04481
 * Remove unused MSGHEADER structure from messages. 
 * Decrease maximum message size.  Add new version of 
 * focusBufferMsg and focusInt32Msg that do not bounce the message
 * back to the originator.  All changes to increase free memory.
 * Revision 1.1  1999/05/24 23:29:56  TD10216
 * Initial revision
 * Revision 1.6  1998/10/23 19:38:47  TM02109
 * Modified to allow for inclusion of the buffmsgs.h and intmsgs.h files
 * without getting the entire array of class definitions.
 * Revision 1.5  1998/10/23 17:42:13  TM02109
 * Must use distinct names in the INT32 and BUFFER messages.
 * Revision 1.4  1997/05/23 15:38:22  SS03309
 * Fixed comments, error condition test
 * Revision 1.3  1997/03/24 21:00:54  SS03309
 * Changes message numbering tables to allow for fixed
 * AN2 message numbers and for easier updates
 * Revision 1.2  1996/12/06 15:47:57  SS03309
 * tcp gateway and spoofer
 * Revision 1.1  1996/12/04 18:32:56  SS03309
 * Initial revision
 *
 * TITLE:      Focussed System message - spoofer dispatcher.
 *
 * ABSTRACT:   These classes support message routing in the Focussed System.
 *             The spoofer dispatcher is used by the spoofer to intercept 
 *             message trafic from tasks before it gets distributed.  This 
 *             allows the spoofer to modify the data.
 *
 * DOCUMENTS
 * Requirements:     I:\ieee1498\SSS2.DOC
 * Test:             I:\ieee1498\STD2.DOC
 */

// #includes

#include <ctype.h>
#include <env.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/irqinfo.h>
#include <sys/kernel.h>
#include <sys/name.h>
#include <sys/proxy.h>
#include <sys/psinfo.h>
#include <sys/seginfo.h>
#include <sys/trace.h>
#include <sys/tracecod.h>
#include <sys/types.h>
#include <unistd.h>

#include "buffmsg.hpp"
#include "crc.h"
#include "spoofdsp.hpp"
#include "error.h"
#include "msg.hpp"
#include "sinver.h"



// SPECIFICATION:    The dispatcher is used to distribute messages within
//                   a task.  This is the constructor.
//                   argv[0] = program name
//                      [1] = local queue name
//                      [2] = remote router node
//                      [3] = remote queue name 
//                      [4] = L to log internal events
//                   maxMessages is used to set Q length
//
// ERROR HANDLING:   _FATAL_ERROR.

spooferDispatcher::spooferDispatcher( int argc, char** argv, int maxMessages) :
dispatcher( argc, argv, maxMessages)
{

   // open other nodes router queue for write access
   // set environment variable to other node

   if(atoi( argv[2]) == getnid())
   {
      printf(__FILE__ " no second router\n");
      routerQueue2 = QNX_ERROR;
   }
   else
   {
      if(setenv( "MQ_NODE", argv[2], 1) != 0)  // set remote node
      {
         _FATAL_ERROR( __FILE__, __LINE__, TRACE_DISPATCHER, 0, "setenv()");
      }
      routerQueue2 = mq_open( argv[3], O_WRONLY, 0, 0);
      if(routerQueue2 == MQ_ERROR)        // open fail
      {
         _FATAL_ERROR( __FILE__, __LINE__, TRACE_DISPATCHER, 0, "mq_open()");
      }
   }

   // register this task as the spoofer with routers

   TASKLIST tl;

   tl.h.osCode = SPOOFER_REGISTER;           // os message code
   tl.h.length = sizeof( TASKLIST);          // total message length, bytes
   tl.h.taskPID = getpid();                  // task PID number
   tl.h.taskNID = getnid();                  // task NID
   clock_gettime( CLOCK_REALTIME, &(tl.h.sendTime));
   tl.mq = mq;                               // messsage queue
   tl.next = NULL;                           // next in chain
   updateFocusMsgCRC( &tl);                  // update CRC

   // send message to local router to register

   int k = 0;                                // counter
   while((mq_send( routerQueue, &tl, tl.h.length, 0) == MQ_ERROR) &&
         (taskRunning))
   {
      if((errno==EINTR) && (k<RETRY_COUNT))  // signals
      {
         k++;
      }
      else                                   // all other errors
      {
         _FATAL_ERROR( __FILE__, __LINE__, TRACE_DISPATCHER, 0, "mq_send()");
      }
   }

   // send message to other router to register

   k = 0;                                    // counter
   if(routerQueue2 != QNX_ERROR)
   {

      while((mq_send( routerQueue2, &tl, tl.h.length, 0) == MQ_ERROR) &&
            (taskRunning))
      {
         if((errno==EINTR) && (k<RETRY_COUNT))  // signals
         {
            k++;
         }
         else                                   // all other errors
         {
            _FATAL_ERROR( __FILE__, __LINE__, TRACE_DISPATCHER, 0, "mq_send()");
         }
      }
   }
};


// SPECIFICATION:    destructor
//
// ERROR HANDLING:   none.

spooferDispatcher::~spooferDispatcher()
{
   // deregister this task with remote router

   if((signalNumber != SIGPWR) && (routerQueue2 != QNX_ERROR))
   {
      TASKLIST tl;

      tl.h.osCode = TASK_DEREGISTER;            // os message code
      tl.h.length = sizeof( TASKLIST);          // total message length, bytes
      tl.h.taskPID = getpid();                  // task PID number
      tl.h.taskNID = getnid();                  // task NID
      clock_gettime( CLOCK_REALTIME, &(tl.h.sendTime));
      tl.mq = mq;                               // messsage queue
      tl.next = NULL;                           // next in chain
      updateFocusMsgCRC( &tl);                  // update CRC
      mq_send(routerQueue2, &tl, tl.h.length,0); // send to router
   }
};

// SPECIFICATION:    register message
//                   Parameter:
//                   m - pointer to message
//
// ERROR HANDLING:   _FATAL_ERROR.

void
spooferDispatcher::registerMessage( routeBuffer* m)
{
   dispatcher::registerMessage( m);          // call base class

   if(routerQueue2 != QNX_ERROR)
   {

      MSGHEADER* mhdr = (MSGHEADER*) m->message;
      unsigned short mid = mhdr->msgID;

      // send message to router

//      mhdr->osCode = MESSAGE_REGISTER;          // tell router to add message
      mhdr->taskPID = getpid();                 // task PID
      mhdr->taskNID = getnid();                 // task NID
      clock_gettime( CLOCK_REALTIME,            // get current time
                     &(mhdr->sendTime));
      updateFocusMsgCRC( mhdr);                 // update CRC

      unsigned short k = 0;

      // place message in other router's input queue

      while((mq_send( routerQueue2, mhdr, mhdr->length, 0) == MQ_ERROR) &&
            (taskRunning))
      {
         if((errno==EINTR) && (k<RETRY_COUNT))  // signals
         {
            k++;                                // try again
         }
         else                                   // fatal error
         {
            _FATAL_ERROR( __FILE__, __LINE__, TRACE_DISPATCHER, 0, "mq_send()");
            break;
         }
      }
   }
};


// SPECIFICATION:    deregister message
//                   Parameter:
//                   m - pointer to message
//
// ERROR HANDLING:   _FATAL_ERROR.

void
spooferDispatcher::deregisterMessage( routeBuffer* m)
{
   dispatcher::deregisterMessage( m);        // call base class

   if(routerQueue2 != QNX_ERROR)
   {

      MSGHEADER* mhdr = (MSGHEADER*) m->message;

      unsigned short mid = mhdr->msgID;

      // send message to other router

      mhdr->osCode = MESSAGE_DEREGISTER;        // tell router to remove message
      mhdr->taskPID = getpid();                 // task PID
      mhdr->taskNID = getnid();                 // task NID
      clock_gettime( CLOCK_REALTIME,            // get current time
                     &(mhdr->sendTime));
      updateFocusMsgCRC( mhdr);                 // update CRC

      unsigned short k = 0;

      // place message in router's input queue

      while((mq_send( routerQueue2, mhdr, mhdr->length, 0) == MQ_ERROR) &&
            (taskRunning))
      {
         if((errno==EINTR) && (k<MQ_ERROR))     // signals
         {
            k++;                                // try again
         }
         else                                   // fatal error
         {
            _FATAL_ERROR( __FILE__, __LINE__, TRACE_DISPATCHER, 0, "mq_send()");
            break;
         }
      }
   }
};


// SPECIFICATION:    send message to router
//                   Parameter:
//                   m - pointer to message
//
// ERROR HANDLING:   _FATAL_ERROR.

void
spooferDispatcher::send( routeBuffer* m)
{
   if(m == NULL)                            // internal check
   {
      _FATAL_ERROR( __FILE__, __LINE__, TRACE_DISPATCHER, 0, "msg length");
   }

   // range checks

   MSGHEADER* mhdr = (MSGHEADER*) m->message;
   unsigned short mid = mhdr->msgID;
   if ((mid == FIRST_BUFFER_MESSAGE) || (mid >= focusInt32Msg::LAST_INT32_MESSAGE))
   {
      _FATAL_ERROR( __FILE__, __LINE__, TRACE_DISPATCHER, mid, "message id");
   }
   logData( __LINE__, mid, MSG_SEND);
   unsigned int mLen = mhdr->length;
   if(mLen > BSIZE)
   {
      _FATAL_ERROR( __FILE__, __LINE__, TRACE_DISPATCHER, mLen, "message length too large");
   }


   // send message to router

   mhdr->osCode = SPOOFED_MESSAGE;           // tell router to multicast message
   updateFocusMsgCRC( mhdr);                 // update CRC

   unsigned short k = 0;

   // place message in router's input queue

   while((mq_send( routerQueue, mhdr, mhdr->length, 0) == MQ_ERROR) &&
         (taskRunning))
   {
      if((errno==EINTR) && (k<RETRY_COUNT))  // signals
      {
         k++;                                // try again
      }
      else                                   // fatal error
      {
         _FATAL_ERROR( __FILE__, __LINE__, TRACE_DISPATCHER, 0, "mq_send()");
         break;
      }
   }

   // place message in other router's input queue

   if(routerQueue2 != QNX_ERROR)
   {

      k = 0;
      while((mq_send( routerQueue2, mhdr, mhdr->length, 0) == MQ_ERROR) &&
            (taskRunning))
      {
         if((errno==EINTR) && (k<RETRY_COUNT))  // signals
         {
            k++;                                // try again
         }
         else                                   // fatal error
         {
            _FATAL_ERROR( __FILE__, __LINE__, TRACE_DISPATCHER, 0, "mq_send()");
            break;
         }
      }
   }
};


