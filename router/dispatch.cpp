/*
 * Copyright (c) 1995, 1996 by Cobe BCT, Inc.  All rights reserved.
 *
 * $Header: Q:/home1/COMMON_PROJECT/Source/ROUTER/rcs/DISPATCH.CPP 1.2 1999/05/31 20:35:02 BS04481 Exp BS04481 $
 * $Log: dispatch.cpp $
 * Revision 1.35  1999/03/24 21:50:48  BS04481
 * Change hardware version information to provide base version of
 * hardware plus computer type, board rev and FPGA revision byte.
 * Add message support in both drivers to respond to queries for 
 * hardware type.
 * Revision 1.34  1998/10/23 19:38:44  TM02109
 * Modified to allow for inclusion of the buffmsgs.h and intmsgs.h files
 * without getting the entire array of class definitions.
 * Revision 1.33  1998/10/23 17:42:09  TM02109
 * Must use distinct names in the INT32 and BUFFER messages.
 * Revision 1.32  1998/08/10 17:37:08  TD07711
 *   added retries on mq_send() in ~dispatcher() to avoid causing a 
 *   fatal error should ~dispatcher  fails to deregister.
 * Revision 1.31  1998/08/03 20:31:41  TD07711
 *   use mq_check() to log when queue is almost full
 * Revision 1.30  1997/03/24 21:00:56  SS03309
 * Changes message numbering tables to allow for fixed
 * AN2 message numbers and for easier updates
 * Revision 1.29  1996/12/30 18:59:38  SS03309
 * Added program name to fatal error prints
 * Revision 1.28  1996/12/02 17:34:36  SM02805
 * Added the send_tcp method for AN2 support.  This method may be used by a task to
 * place messages into the FS router as though they were sent by another
 * FS task.  The intended use is for a task to be able to place FS messages
 * received via tcp/ip into the FS router. Testing indicates the method
 * works and does not affect other router operations.
 * Revision 1.27  1996/09/09 19:23:25  tm02109
 * AND NOT EQUAL,, opps..
 * Revision 1.26  1996/09/09 19:21:57  tm02109
 * In the re-arming code added the check for EINTR.
 * Revision 1.25  1996/07/24 19:49:35  SS03309
 * fix MKS
 * Revision 1.24  1996/07/19 19:11:13  SS03309
 * lint changes
 * Revision 1.23  1996/07/10 22:06:35  SS03309
 * power fail logic
 * Revision 1.22  1996/07/08 17:37:44  SS03309
 * added SIGPWR to signal handlers
 * Revision 1.21  1996/06/28 13:36:28  SS03309
 * Fixed comments, magic numbers
 * Revision 1.20  1996/06/28 13:21:17  SS03309
 * Added comments, fixed magic numbers, changed 0 to NULL, etc
 *
 * TITLE:      dispatch.cpp, Focussed System message dispatcher.
 *
 * ABSTRACT:   These classes support message routing in the Focussed System.
 *             The dispatcher is used by tasks to send and receive messages
 *             to and from the router.  msg.hpp and buffmsg.hpp contain
 *             message classes used to create messages.
 *             dispatcher is used to actually send and receive data.
 *             Data is received via an input queue, and
 *             is sent to the router's input queue.
 *
 * DOCUMENTS
 * Requirements:     I:\ieee1498\SSS2.DOC
 * Test:             I:\ieee1498\STD2.DOC
 */

// #includes

#include <ctype.h>
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
#include "dispatch.hpp"
#include "error.h"
#include "msg.hpp"
#include "sinver.h"
#include "mq_check.h"



// SPECIFICATION:    signal handler, causes program to stop
//                   Parameter:
//                   signum - signal number, see signal.h
//
// ERROR HANDLING:   Calls _FATAL_ERROR.

void signalHandler( int signum)
{
   dispatch->taskRunning = 0;             // clear running flag
   dispatch->signalNumber = signum;       // save signal
};


// SPECIFICATION:    routeBuffer is a base class used to create messages
//                   base class constructor
//                   Parameter:
//                   msg - pointer to message pointer
//                   msgLength - length of message
//                   id - message id
//
// ERROR HANDLING:   _FATAL_ERROR.

routeBuffer::routeBuffer( void** msg, unsigned short msgLength, unsigned short id) :
   _msgID( id)
{

// range checks

   if (msgLength < sizeof( MSGHEADER))
   {
      dispatch->fError( __LINE__, 0, "message length");
   }
   if (msgLength + sizeof(MSGHEADER) > BSIZE)
   {
      dispatch->fError( __LINE__, msgLength, "message length to big");
   }
   if ((id == FIRST_BUFFER_MESSAGE) || (id >= focusInt32Msg::LAST_INT32_MESSAGE))
   {
      dispatch->fError( __LINE__, id, "message id");
   }

// create message, fill in data

   message = new char[msgLength];
   *msg = message;
   MSGHEADER * hdr = (MSGHEADER*) message;
   hdr->osCode = MESSAGE_REGISTER;        // router code
   hdr->length = msgLength;               // total length, bytes
   hdr->msgID = id;                       // msg id
   hdr->taskPID = getpid();               // PID from OS
   hdr->taskNID = getnid();               // network node
   clock_gettime( CLOCK_REALTIME, &(hdr->sendTime));

   updateFocusMsgCRC( hdr);               // update CRC
   dispatch->registerMessage( this);      // register message
};

// SPECIFICATION:    destructor
//
// ERROR HANDLING:   none.

routeBuffer::~routeBuffer()
{
   dispatch->deregisterMessage( this);    // remove this message
   delete [] message;                     // delete storage
};


// SPECIFICATION:    default notify function, does nothing
//
// ERROR HANDLING:   none.

virtual void routeBuffer::notify()
{
};

// SPECIFICATION:    send function
//
// ERROR HANDLING:   none.

void routeBuffer::send()
{
   dispatch->send( this);                 // send message to router
};

// SPECIFICATION:    get message id
//
// ERROR HANDLING:   none.

unsigned short routeBuffer::msgID()
{
   return _msgID;                         // get message id
};

// SPECIFICATION:    get message header info
//                   parameters:
//                   pid - sending PID
//                   nid - sending NID
//                   sendTime - time message sent (using clock on that NID)
//
// ERROR HANDLING:   none.

void routeBuffer::msgHeader(
                      pid_t& pid,                  // sending pid
                      nid_t& nid,                  // sending nid
                      struct timespec& sendTime)   // time msg sent
{
    MSGHEADER* mhdr = (MSGHEADER*) message;        // ptr to header
    pid = mhdr->taskPID;
    nid = mhdr->taskNID;
    sendTime = mhdr->sendTime;
};


// SPECIFICATION:    The dispatcher is used to distribute messages within
//                   a task.  This is the constructor.
//                   argv[0] = program name
//                      [1] = local queue name
//                      [2] = remote router node (not used)
//                      [3] = remote queue name (not used)
//                      [4] = L to log internal events
//                   maxMessages is used to set Q length
//
// ERROR HANDLING:   _FATAL_ERROR.

dispatcher::dispatcher( int argc, char** argv, int maxMessages)
{
   signalNumber = 0;                   // initialize signal
   _programName = argv[0];             // save program name

// check argument count
   if (argc < ARG_COUNT)               // check argument count
   {
      fError( __LINE__, 0, "argument count");
   }

// message id check

   int testValue=LAST_BUFFER_MESSAGE;  // int used to prevent compile problem
   if (testValue >= focusInt32Msg::FIRST_INT32_MESSAGE)
   {
      fError( __LINE__, 0, "Message ID conflict, check msg.hpp and buffmsg.hpp");
   }

// set up signal handlers
   signal( SIGHUP, signalHandler);     // set up signal handlers
   signal( SIGINT, signalHandler);
   signal( SIGQUIT, signalHandler);
   signal( SIGTERM, signalHandler);
   signal( SIGPWR, signalHandler);     // power fail
   taskRunning = 1;                    // init running flag

// set logging flag

   logEnable = ((argc>=ARG_COUNT+1) && (toupper(*argv[ARG_COUNT]) == 'L')) ? 1 : 0;

// build unique queue name

   if (strlen( basename( argv[PROG_NAME])) > MAX_ARG_LENGTH)
   {
      fError( __LINE__, 0, "argument length");
   }
   sprintf( queueName, "%s%d", basename( argv[PROG_NAME]), getpid());

// open input queue with the basename of the task

   struct mq_attr attr;                // message queue attributes
   attr.mq_maxmsg = maxMessages;       // queue length
   attr.mq_msgsize = BSIZE;            // max msg size, from msghdr.h
   attr.mq_flags = MQ_NONBLOCK;        // set non-block flag
   mq = mq_open( queueName, O_RDWR | O_CREAT, 0666, &attr);
   if (mq == MQ_ERROR)                 // open failed
   {
      fError( __LINE__, 0, "mq_open()");
   }

// create proxy for message queue

   qproxy = qnx_proxy_attach( 0, 0, 0, -1);
   if (qproxy == QNX_ERROR)            // proxy fail
   {
      fError( __LINE__, 0, "qnx_proxy_attach()");
   }

// setup queue notify

   qnotify.sigev_signo = -qproxy;
   if (mq_notify( mq, &qnotify) == MQ_ERROR) // notify fail
   {
      fError( __LINE__, 0, "mq_notify()");
   }
   Trigger( qproxy);                   // avoid race condition, check queue

// open router queue for write access

   routerQueue = mq_open( argv[1], O_WRONLY, 0, 0);
   if (routerQueue == MQ_ERROR)        // open fail
   {
      fError( __LINE__, 0, "mq_open()");
   }

// clear message table

   for (int k=0; k < COUNT_OF(messageTable); k++)
   {
      messageTable[k] = NULL;
   }

   timerList = NULL;

// register this task with local router

   TASKLIST tl;

   tl.h.osCode = TASK_REGISTER;              // os message code
   tl.h.length = sizeof( TASKLIST);          // total message length, bytes
   tl.h.taskPID = getpid();                  // task PID number
   tl.h.taskNID = getnid();                  // task NID
   clock_gettime( CLOCK_REALTIME, &(tl.h.sendTime));
   tl.mq = mq;                               // messsage queue
   tl.next = NULL;                           // next in chain
   updateFocusMsgCRC( &tl);                  // update CRC

// send message to router to register

   k = 0;                                    // counter
   mq_check(routerQueue);
   while((mq_send( routerQueue, &tl, tl.h.length, 0) == MQ_ERROR) &&
         (taskRunning))
   {
      if((errno==EINTR) && (k<RETRY_COUNT))  // signals
      {
         k++;
      }
      else                                   // all other errors
      {
         fError( __LINE__, 0, "mq_send()");
      }
   }
};


// SPECIFICATION:    destructor
//
// ERROR HANDLING:   none.

dispatcher::~dispatcher()
{
// deregister this task with local router

   if (signalNumber != SIGPWR)
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
      mq_check(routerQueue);
      int status;
      for (int i = 0; i < 3; i++) {
          if ((status = mq_send(routerQueue, &tl, tl.h.length,0)) != -1) {
              break;
          }
          if (errno == EINTR) {
              continue;
          }
          else {
              break;
          }
      } // end for

      if (status == -1) {
          char buf[100];
          sprintf(buf, "~dispatcher: mq_send failed, pid=%d errno=%d", getpid(), errno);
          _LOG_ERROR(__FILE__, __LINE__, TRACE_DISPATCHER, errno, buf);
      }

      // close and remove queue
      mq_close( mq);
      mq_unlink( queueName);
   }
};


// SPECIFICATION:    fError()
//                   Parameter:
//                   line - line number
//                   usercode - user code
//                   msg - message string
//
// ERROR HANDLING:   _FATAL_ERROR.

void
dispatcher::fError(int line, int usercode, char* msg)
{
   char* estring = new char[strlen(msg) + strlen( _programName) + 10];
   strcpy( estring, _programName);
   strcat( estring, " ");
   strcat( estring, msg);

   _FATAL_ERROR( __FILE__, line, TRACE_DISPATCHER, usercode, estring);
};

// SPECIFICATION:    register message
//                   Parameter:
//                   m - pointer to message
//
// ERROR HANDLING:   _FATAL_ERROR.

void
dispatcher::registerMessage( routeBuffer* m)
{
// range checks

   if (m == NULL)
   {
      fError( __LINE__, 0, "message NULL");
   }
   MSGHEADER* mhdr = (MSGHEADER*) m->message;
   unsigned short mid = mhdr->msgID;
   if ((mid == FIRST_BUFFER_MESSAGE) || (mid >= focusInt32Msg::LAST_INT32_MESSAGE))
   {
      fError( __LINE__, mid, "message id");
   }

// add to linked list

   linked* n = new linked;                   // create new one
   n->next = messageTable[mid];              // link to front
   n->length = sizeof( linked);              // set message length
   n->msg = m;
   updateFocusMsgCRC( n);                    // update CRC
   messageTable[mid] = n;                    // chain list
   logData( __LINE__, mid, MSG_CREATE);

// send message to router

   mhdr->osCode = MESSAGE_REGISTER;          // tell router to add message
   mhdr->taskPID = getpid();                 // task PID
   mhdr->taskNID = getnid();                 // task NID
   clock_gettime( CLOCK_REALTIME,            // get current time
     &(mhdr->sendTime));
   updateFocusMsgCRC( mhdr);                 // update CRC

   unsigned short k = 0;

// place message in router's input queue
   mq_check(routerQueue);
   while((mq_send( routerQueue, mhdr, mhdr->length, 0) == MQ_ERROR) &&
         (taskRunning))
   {
      if((errno==EINTR) && (k<RETRY_COUNT))  // signals
      {
         k++;                                // try again
      }
      else                                   // fatal error
      {
         fError( __LINE__, 0, "mq_send()");
         break;
      }
   }
};


// SPECIFICATION:    deregister message
//                   Parameter:
//                   m - pointer to message
//
// ERROR HANDLING:   _FATAL_ERROR.

void
dispatcher::deregisterMessage( routeBuffer* m)
{

// range checks

   if (m == NULL)
   {
      fError( __LINE__, 0, "message NULL");
   }
   MSGHEADER* mhdr = (MSGHEADER*) m->message;

   unsigned short mid = mhdr->msgID;
   if ((mid == FIRST_BUFFER_MESSAGE) || (mid >= focusInt32Msg::LAST_INT32_MESSAGE))
   {
      fError( __LINE__, mid, "message id");
   }
   logData( __LINE__, mid, MSG_DELETE);

// send message to router

   mhdr->osCode = MESSAGE_DEREGISTER;        // tell router to remove message
   mhdr->taskPID = getpid();                 // task PID
   mhdr->taskNID = getnid();                 // task NID
   clock_gettime( CLOCK_REALTIME,            // get current time
     &(mhdr->sendTime));
   updateFocusMsgCRC( mhdr);                 // update CRC

   unsigned short k = 0;

// place message in router's input queue
   mq_check(routerQueue);
   while((mq_send( routerQueue, mhdr, mhdr->length, 0) == MQ_ERROR) &&
         (taskRunning))
   {
      if((errno==EINTR) && (k<MQ_ERROR))     // signals
      {
         k++;                                // try again
      }
      else                                   // fatal error
      {
         fError( __LINE__, 0, "mq_send()");
         break;
      }
   }

// remove from local table

   linked* lptr = messageTable[mid];         // get current entry
   if (lptr == NULL)
   {
      fError( __LINE__, 0, "lptr=NULL");
   }
   linked* oneBack;

// remove from linked list, first check head of chain,
// then scan list

   if (lptr->msg == m)
   {
      messageTable[mid] = lptr->next;        // move chain
      delete lptr;                           // remove element
   }
   else                                      // check rest of chain
   {
      oneBack = messageTable[mid];           // save one-back for deletes
      lptr = oneBack->next;                  // get next
      while( (lptr) && (taskRunning))        // process list
      {
         if (lptr->msg == m)                 // match found
         {
            oneBack->next = lptr->next;      // remove entry
            delete lptr;                     // de-allocate entry
            updateFocusMsgCRC( oneBack);     // fix CRC
            lptr = oneBack->next;            // get next link
            if (lptr)                        // if not NULL
            {
               updateFocusMsgCRC( lptr);     // update CRC
            }
            break;
         }
         oneBack = lptr;
         lptr = lptr->next;                  // move down list
      }
   }
};


// SPECIFICATION:    send message to router
//                   Parameter:
//                   m - pointer to message
//
// ERROR HANDLING:   _FATAL_ERROR.

void
dispatcher::send( routeBuffer* m)
{
   if (m == NULL)                            // internal check
   {
      fError( __LINE__, 0, "msg length");
   }

// range checks

   MSGHEADER* mhdr = (MSGHEADER*) m->message;
   unsigned short mid = mhdr->msgID;
   if ((mid == FIRST_BUFFER_MESSAGE) || (mid >= focusInt32Msg::LAST_INT32_MESSAGE))
   {
      fError( __LINE__, mid, "message id");
   }
   logData( __LINE__, mid, MSG_SEND);
   unsigned int mLen = mhdr->length;
   if (mLen > BSIZE)
   {
      fError( __LINE__, mLen, "message length too large");
   }


// send message to router

   mhdr->osCode = MSG_MULTICAST;             // tell router to multicast message
   mhdr->taskPID = getpid();                 // task PID
   mhdr->taskNID = getnid();                 // task NID
   clock_gettime( CLOCK_REALTIME,            // get current time
     &(mhdr->sendTime));
   updateFocusMsgCRC( mhdr);                 // update CRC

   unsigned short k = 0;

// place message in router's input queue
   mq_check(routerQueue);
   while((mq_send( routerQueue, mhdr, mhdr->length, 0) == MQ_ERROR) &&
         (taskRunning))
   {
      if((errno==EINTR) && (k<RETRY_COUNT))  // signals
      {
         k++;                                // try again
      }
      else                                   // fatal error
      {
         fError( __LINE__, 0, "mq_send()");
         break;
      }
   }
};

// SPECIFICATION:    send message received from tcp/ip socket to router
//                   Parameter:
//                   m - pointer to message
//
// ERROR HANDLING:   _FATAL_ERROR.
//
// NOTES:
// 12/02/96
// 1. The send_tcp method is an APOLLOnet II feature added for Beta 2.3.  The method
// allows a task to post FS messages received from an external computing resource via tcp/ip
// to the router running on the FS.  The router will process this posted message in a fashion
// identical to messages that originate from tasks running on the FS.
//

void
dispatcher::send_tcp( void* m)
{
   if (m == NULL)                            // internal check
   {
      fError( __LINE__, 0, "msg length");
   }

// range checks

   MSGHEADER* mhdr = (MSGHEADER*) m;
   unsigned short mid = mhdr->msgID;
   if ((mid == FIRST_BUFFER_MESSAGE) || (mid >= focusInt32Msg::LAST_INT32_MESSAGE))
   {
      fError( __LINE__, mid, "message id");
   }
   logData( __LINE__, mid, MSG_SEND);
   unsigned int mLen = mhdr->length;
   if (mLen > BSIZE)
   {
      fError( __LINE__, mLen, "message length too large");
   }


// send message to router
   mhdr->osCode = MSG_MULTICAST;             // tell router to multicast message
   mhdr->taskPID = getpid();                 // task PID
   mhdr->taskNID = getnid();                 // task NID
    clock_gettime( CLOCK_REALTIME,            // get current time
     &(mhdr->sendTime));
   updateFocusMsgCRC( mhdr);                 // update CRC

   unsigned short k = 0;

// place message in router's input queue
   mq_check(routerQueue);
   while((mq_send( routerQueue, mhdr, mhdr->length, 0) == MQ_ERROR) &&
         (taskRunning))
   {
      if((errno==EINTR) && (k<RETRY_COUNT))  // signals
      {
         k++;                                // try again
      }
      else                                   // fatal error
      {
         fError( __LINE__, 0, "mq_send()");
         break;
      }
   }
};



// SPECIFICATION:    internal data loggger
//                   Parameter:
//                   line - source code line number
//                   msgID - message id/timer proxy pid
//                   code - reason code, see DATA_CODES enum
//
// ERROR HANDLING:   terminates program.

void
dispatcher::logData( long line, long msgID, long code)
{
   if (logEnable)
   {
      Trace3( TRACE_DISPATCHER, _TRACE_SEVERE, line, msgID, code);
   }
};

// SPECIFICATION:    timer functions
//                   clear (destruct) timer entry from table
//                   Parameter:
//                   proxy - proxy to clear
//
// ERROR HANDLING:   _FATAL_ERROR.

void
dispatcher::clearTimerEntry( pid_t proxy)
{
   timerEntry* t;                            // next pointer
   timerEntry* oneBack;                      // one back

   logData( __LINE__, proxy, TIMER_DELETE);  // internal data logging
   t = timerList;                            // start at top
   if (t->proxy == proxy)                    // head of chain?
   {
      timerList = t->next;
      delete t;
   }
   else                                      // run down linked list
   {
      oneBack = timerList;                   // start at second entry
      t = timerList->next;
      while((t) && (taskRunning))
      {
         if (t->proxy == proxy)              // match?
         {
            oneBack->next = t->next;         // remove entry
            delete t;                        // de-allocate memory
            updateFocusMsgCRC( oneBack);     // update CRC
            t = oneBack->next;               // get next link
            if (t)                           // if not NULL
            {
               updateFocusMsgCRC( t);        // update CRC
            }
            break;
         }
         oneBack = t;                        // move down list
         t = t->next;
      }
   }
};


// SPECIFICATION:    set timer entry on list
//                   Parameter:
//                   proxy - proxy to attach
//                   tm -  pointer to timer message
//
// ERROR HANDLING:   _FATAL_ERROR.

void
dispatcher::setTimerEntry( pid_t proxy, class focusTimerMsg* tmsg)
{
   timerEntry* t = new timerEntry;
   if (t == NULL)
   {
      fError( __LINE__, 0, "failed memory allocation");
   }
   if (tmsg == NULL)
   {
      fError( __LINE__, 0, "tmsg==NULL");
   }
   logData( __LINE__, proxy, TIMER_CREATE);  // internal data logging
   t->osCode = MESSAGE_REGISTER;             // build timer entry
   t->length = sizeof( timerEntry);          // set length
   t->proxy = proxy;                         // set proxy PID
   t->tm = tmsg;                               // set message pointer
   t->next = timerList;                      // insert on front
   updateFocusMsgCRC( t);                    // update CRC
   timerList = t;                            // add to front
};


// SPECIFICATION:    Loop here processing messages
//
// ERROR HANDLING:   _FATAL_ERROR.

void
dispatcher::dispatchLoop()
{
   pid_t       pid;                          // msg received from pid
   char        msg[BSIZE];                   // buffer for messages

   sinVerInitialize();                       // init sin ver processing

// loop processing messages and timer clicks

   while( taskRunning)                       // loop until signal
   {
      pid = Receive( 0, msg, sizeof( msg));  // get message
      if (pid == QNX_ERROR)
      {
         if (errno == EINTR)                 // signal
         {
            continue;                        // continue
         }
         fError( __LINE__, 0, "Receive()");
      }

// first check system messages, then process message

      if (!sinVerMessage( pid, (SINVERMSG*) &msg[0]))
      {
         processMessage( pid);
      }
   }

   delete this;
}

// SPECIFICATION:    trace message events, sends a message to router
//                   Parameter:
//                   msgID - id to trace
//
// ERROR HANDLING:   _FATAL_ERROR.

void
dispatcher::trace( unsigned short msgID)
{
   MSGHEADER msg;

   msg.osCode = MESSAGE_TRACE;              // os message code
   msg.length = sizeof( msg);               // total message length, bytes
   msg.msgID = msgID;                       // enum message id
   msg.taskNID = getnid();                  // task network node id
   msg.taskPID = getpid();                  // task PID number
   clock_gettime( CLOCK_REALTIME, &(msg.sendTime));

   updateFocusMsgCRC( &msg);                // update CRC
   mq_check(routerQueue);
   if (mq_send( routerQueue, &msg, msg.length, 0) == -1) {
       char buf[100];
       sprintf(buf, "trace: mq_send failed, pid=%d errno=%d", getpid(), errno);
       _LOG_ERROR(__FILE__, __LINE__, TRACE_DISPATCHER, errno, buf);
   }
   // FIXME - should mq_send retry on EINTR?
};


// SPECIFICATION:    process messages from router
//                   Parameter:
//                   pid - PID of message sender
//
// ERROR HANDLING:   _FATAL_ERROR.

virtual void
dispatcher::processMessage( pid_t pid)
{
   timerEntry* t;                               // timer pointer
   char msg[BSIZE];                             // message buffer

// check for timers

   if (pid != qproxy)                           // if not queue proxy, try timers
   {
      t = timerList;
      while((t) && (taskRunning))               // scan list
      {
         if (!validFocusMsgCRC( t))             // internal CRC check
         {
            fError( __LINE__, 0, "CRC fail in timer processing");
         }

         if (t->proxy == pid)                   // match found
         {
            logData( __LINE__, pid, TIMER_CALLING_NOTIFY);
            t->tm->notify();                    // call notify function
            logData( __LINE__, pid, TIMER_NOTIFY_RETURN);
            break;
         }
         t = t->next;                           // next in chain
      }
   }
   else                                         // message queue
   {
// reset notify for next time

      if (mq_notify( mq, &qnotify) == MQ_ERROR)
      {
         if ( (errno != EBUSY) && (errno != EINTR) )
         {
            fError(__LINE__, 0, "mq_notify()");
         }
      }

// pull all data from queue

      while( taskRunning)                       // process all data from queue
      {
         if (mq_receive( mq, msg, BSIZE, 0) != MQ_ERROR)
         {
            if (validFocusMsgCRC( msg))         // message CRC check
            {
               // get message header info
               unsigned int mlen = ((MSGHEADER*) msg)->length;
               unsigned short mid = ((MSGHEADER*) msg)->msgID;
               linked* lptr = messageTable[mid];      // get table entry

               // find matching message(s)
               while ((lptr) && (taskRunning))        // run down chain
               {
                  if (!validFocusMsgCRC( lptr)) // internal CRC check
                  {
                     fError( __LINE__, 0, "CRC error\n");
                  }

                  // verify message lengths
                  unsigned int tlen = ((MSGHEADER*)lptr->msg->message)->length;
                  if (tlen != mlen)
                  {
                     fError( __LINE__, 0, "msg length");
                  }

                  // copy message and verify
                  memcpy( lptr->msg->message, msg, mlen);
                  if (memcmp( lptr->msg->message, msg, mlen) != 0)
                  {
                     fError( __LINE__, 0, "copy error");
                  }

                  // call notify function
                  logData( __LINE__, mid, MSG_CALLING_NOTIFY);
                  linked* next = lptr->next;      // get next pointer
                  lptr->msg->notify();
                  logData( __LINE__, mid, MSG_NOTIFY_RETURN);

                  // move to next entry
                  lptr = next;
               }
            }
            else
            {
               fError( __LINE__, 0, "bad message CRC");
            }                                   // end CRC test
         }
         else                                   // queue error
         {
            if (errno == EAGAIN) break;         // no more data
            if (errno == EINTR) continue;       // signal
            fError( __LINE__, 0, "mq_receive()");
         }                                      // end if mq_receive
      }                                         // end while taskRunning
   }                                            // end pid test
};

