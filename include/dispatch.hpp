/*
 * Copyright (c) 1995, 1996 by Cobe BCT, Inc.  All rights reserved.
 *
 * $Header: Y:/BCT_Development/Common/INCLUDE/rcs/DISPATCH.HPP 1.7 2000/05/31 17:14:59 BD10648 Exp ms10234 $
 * $Log: dispatch.hpp $
 * Revision 1.4  1999/09/30 04:02:12  BS04481
 * Port fix from Spectra.  Avoids SIGSEV which can occur if a 
 * message object is deleted from inside of a notify for another
 * message.
 * Revision 1.3  1999/09/17 04:21:59  BS04481
 * Changes in support of greater determinism in the drivers.  
 * Revision 1.2  1999/05/31 20:34:54  BS04481
 * Remove unused MSGHEADER structure from messages. 
 * Decrease maximum message size.  Add new version of 
 * focusBufferMsg and focusInt32Msg that do not bounce the message
 * back to the originator.  All changes to increase free memory.
 * Revision 1.1  1999/05/24 23:26:27  TD10216
 * Initial revision
 * Revision 1.7  1999/03/29 17:10:10  TD10216
 * Revision 1.6  1996/12/30 18:59:39  SS03309
 * Added program name to fatal error prints
 * Revision 1.5  1996/12/04 18:32:17  SS03309
 * Added firend for spoofer
 * Revision 1.4  1996/12/04 16:11:47  SS03309
 * Made registerMessage, deregisterMessage, and send virtual functions
 * Revision 1.3  1996/12/02 17:34:37  SM02805
 * Added the send_tcp method for AN2 support.  This method may be used by a task to 
 * place messages into the FS router as though they were sent by another
 * FS task.  The intended use is for a task to be able to place FS messages
 * received via tcp/ip into the FS router. Testing indicates the method 
 * works and does not affect other router operations.
 * Revision 1.2  1996/07/24 19:49:36  SS03309
 * fix MKS 
 * Revision 1.2  1996/07/22 14:42:57  SS03309
 * Revision 1.18  1996/06/28 13:36:30  SS03309
 * Fixed comments, magic numbers
 * Revision 1.17  1996/06/28 13:21:33  SS03309
 * Comments, Fixed magic numbers
 * Revision 1.16  1996/06/27 15:29:10  SS03309
 * Fixed comments and magic numbers
 *
 * TITLE:      dispatch.hpp, Focussed System message dispatcher.
 *
 * ABSTRACT:   These classes support message routing in the Focussed System.
 * The dispatcher runs accepting messages from the Focussed Router,
 * copying the message data to the correct places, and calls the
 * various message notify functions.   For EXAMPLE see msg.hpp.
 *
 * DOCUMENTS
 * Requirements:     I:\ieee1498\SSS2.DOC
 * Test:             I:\ieee1498\STD2.DOC
 *
 * PUBLIC FUNCTIONS:
 * class routeBuffer
 *    routeBuffer()                    // constructor
 *    virtual ~routeBuffer()           // destructor
 *    virtual void notify()            // notify function
 *    void send()                      // send data
 *    unsigned short msgID()           // get message id
 * class dispatcher
 *    class systemError                // exception class
 *    dispatcher()                     // constructor
 *    ~dispatcher()                    // destructor
 *    void registerMessage()           // register message
 *    void deregisterMessage()         // deregister message
 *    void send()                      // send message
 *    void clearTimerEntry()           // clear timer entry
 *    void setTimerEntry()             // set timer entry
 *    virtual void dispatchLoop()      // dispatch loop
 */

#ifndef DISPATCH_HPP                   // prevent multiple includes
#define DISPATCH_HPP

#include <mqueue.h>
#include <string.h>
#include <sys/types.h>
#include "msghdr.h"

// forward references

class dispatcher;
class focusTimerMsg;

// public structures, enums, and data

static const short ARG_COUNT=4;              // command line args
#define COUNT_OF(x) (sizeof(x)/sizeof(*x))   // array size
static const int   DEFAULT_Q_SIZE=22;        // default Q size
static const short PROG_NAME=0;              // command arg for prog name
static const int   MAX_ARG_LENGTH=68;        // 80  - 12
static const int   MAX_QUEUE_NAME_LENGTH=80; // buffer length
static const int   MQ_ERROR = (-1);          // mq_xxx() errors
static const int   QNX_ERROR =(-1);          // qnx functions
static const int   RETRY_COUNT = 3;          // mq retry count

// class definitions

/*
 * The routeBuffer class is the base class used to generate messages.
 * The message array (storage) is maintained by the base class.  The
 * derived classes use a pointer to get/set data.  The notify() function
 * can be used to take action when a message is received.  In a polling
 * situation, this function need not be implemented.  The send function
 * is used to send the data to the Focussed Router.
 */

class routeBuffer
{
   friend class dispatcher;                        // dispatcher copies data
   friend class controlDispatcher;
   friend class spooferDispatcher;
   friend class msglog;                            // tracks msg traffic

   public:
      routeBuffer( void** msg,                     // pointer to msg pointer
                   unsigned short msgLength,       // message length
                   unsigned short id);             // message id
      routeBuffer( void** msg,                     // pointer to msg pointer
                   unsigned short msgLength,       // message length
                   unsigned short id,              // message id
                   bounce_t bounce);               // handling of message back to originator
#if ENUMSAREINTS
      routeBuffer( void** msg,                     // pointer to msg pointer
                   unsigned short msgLength,       // message length
                   int id)             // message id
	  { routeBuffer(msg,msgLength,(unsigned short) id); }
#endif
      virtual ~routeBuffer();

      virtual void notify();
      void send();
      unsigned short msgID();
      void msgHeader( pid_t& pid,                  // sending pid
                      nid_t& nid,                  // sending nid
                      struct timespec& sendTime);  // time msg sent

   private:
      routeBuffer( routeBuffer const &);           // not implemented
      routeBuffer& operator= (routeBuffer const&); // not implemented
      unsigned short _msgID;                       // message id
      void*    message;                            // storage allocated here!
};

/*
 * the dispatcher class is used to register and deregister messages
 * with the system router.   Only one dispatcher should be constructed
 * per task.   The class is also used to send messages.  The
 * dispatchLoop() loops forever (doesn't return) getting messages from
 * the router and sending them to the proper message class(es).
 */

class timerMsg;                                    // forward ref

class dispatcher
{
   public:

// class members

      dispatcher( int argc,                        // command line arg count
                  char** argv,                     // command line arguments
                  int maxMessages=DEFAULT_Q_SIZE); // max size of queue
      virtual ~dispatcher();

      virtual void registerMessage( routeBuffer*);         // used by msg constructor
      virtual void deregisterMessage( routeBuffer*);       // used by msg destructor
      virtual void send( routeBuffer*);                    // send message

      void clearTimerEntry( pid_t proxy);          // used by timer destructor
      void setTimerEntry( pid_t proxy,             // used by timer constructor
                          focusTimerMsg* tm);      // pointer to timer msg

      virtual void dispatchLoop();                 // dispatch loop

      int   signalNumber;                          // signal number causing
                                                   // dispatchLoop to exit
      void trace( unsigned short msgID);           // trace message events

	  // 12/02/96 msm method added for an2 support 
      void send_tcp( void* );                       // tcp/ip send message for AN2 Beta 2.3
 
      char *programName( void);                    // return program name (argv[0])
      void fError(int line, int usercode, char* msg); // fatal error handling
      int   iExist;                                // 1=active, 0=inactive

   protected:
      friend int input_funct( void*, pid_t, void*, size_t);    // photon hook
      virtual void processMessage( pid_t pid);     // used by dispatchers
                                                   // to process msgs
      friend void signalHandler( int signum);      // allow signal handler
      unsigned char taskRunning;                   // task running flag
      char  queueName[MAX_QUEUE_NAME_LENGTH];      // queue name
      mqd_t mq;                                    // input q
      mqd_t routerQueue;                           // router queue id
      unsigned logEnable;                          // =1, to log events

      // task list structure
      typedef struct
      {
         MSGHEADER      h;                         // os message code
         mqd_t          mq;                        // messsage queue
         void*          next;                      // next in chain
      } TASKLIST;

      enum DATA_CODES                              // data log codes
      {
         MSG_CREATE,
         MSG_DELETE,
         MSG_CALLING_NOTIFY,
         MSG_NOTIFY_RETURN,
         MSG_SEND,
         TIMER_CREATE,
         TIMER_DELETE,
         TIMER_CALLING_NOTIFY,
         TIMER_NOTIFY_RETURN
      };
      void logData( long line, long msgID, long code); // internal data logging

   private:
      dispatcher();                                // not implemented
      dispatcher( dispatcher const &);             // not implemented
      dispatcher& operator=(dispatcher const&);    // not implemented
      pid_t qproxy;                                // input q proxy
      struct sigevent qnotify;                     // q notify structure
      char* _programName;                          // program name argv[0]

// define dispatch table here

      struct linked
      {
         unsigned short    osCode;                 // not used
         unsigned short    length;                 // total length
         unsigned long     CRC;                    // CRC
         linked*           next;                   // next link or zero
         routeBuffer*      msg;                    // message base class
      };

      linked* messageTable[MAX_MESSAGES];          // dispatch table

      // pointer to next message to process
      linked* _nextToProcess;

// The timer messages use a linked list to save the proxy and
// timer message pointer.

      struct timerEntry
      {
         unsigned short    osCode;                 // not used
         unsigned short    length;                 // total length
         unsigned long     CRC;                    // CRC
         pid_t             proxy;                  // timer proxy
         class focusTimerMsg* tm;                  // timer message
         timerEntry*       next;                   // next on chain
      };
      timerEntry* timerList;                       // timer list
};

#endif
