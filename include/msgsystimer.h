/*
 * Copyright (c) 2002 Gambro BCT, Inc.  All rights reserved.
 *
 * TITLE:      msgsystimer.h, Message System Timer 
 *
 * ABSTRACT:   These classes support message system timer messages. The message
 *             system timer class interfaces with the message system and the 
 *             hardware clock to manage the timers.   Only one timer task should 
 *             created per processor.   
 */

#ifndef _MSGSYSTIMER_H_
#define _MSGSYSTIMER_H_

#include <fcntl.h>
#include <map>
#include <mqueue.h>
#include <queue>

#include "datalog.h"
#include "messagepacket.h"

class MsgSysTimer
{
public:

   //
   // This function will act as the main entry point for the message system timer task.  
   //  This function will create the message system timer object.  When this function 
   //  exits, the message system timer task will also be considered finished.
   static int MsgSysTimer_main();

   //
   // Function to get the global message system timer task ...
   static WIND_TCB *globalMsgSysTimerTid();
   static MsgSysTimer *globalMsgSysTimer();

   //
   // Function called whenever any task gets created ...
   static int taskCreateHook( WIND_TCB *pTcb );
   //
   // Function called whenever any task gets deleted ...
   static int taskDeleteHook( WIND_TCB *pTcb );

   //
   // Function to handle task level datalog errors ...
   static void datalogErrorHandler( const char * file, int line, 
                                    DataLog_ErrorType error, 
                                    const char * msg, 
                                    int continuable );

private:

   static WIND_TCB    *_TheTimerTid;
   static MsgSysTimer *_TheTimer;

   static int _ReadyToReceiveTimeMsg;

public:

   MsgSysTimer();
   virtual ~MsgSysTimer();

   //
   // Function which will create the message queue, and set up all the 
   //  necessary structures to start the processing.  
   bool init();

   //
   // This function will continue processing indefinitely while it is blocked on the
   //  timer semaphore.
   void maintainTimers();
   void stopLoop() { _StopLoop=true; }
  
   //
   // Dump the contents of this class
   void dump( ostream &outs );

protected:

   //
   // Receive a message packet and determine its function 
   void processMessage( const MessagePacket &mp );
   
   //
   // Function to update the current time 
   void updateTime( const long long usecs );

   //
   // This function will be called to register a task in the task maps
   void registerTask( unsigned long tId, const char *qName );
   //
   // This function will remove the task from the task map, and the message maps.  No
   //  references to the passed in taskId will remain in any of the structures.  All
   //  messages registered for by this task will be deregistered.
   void deregisterTask( unsigned long tId );

   //
   // Function to register a new timer
   void registerTimer( const MessagePacket &mp, const unsigned long interval );

   //
   // Function to deregister the requested timer message
   void deregisterTimer( const unsigned long mId );
   void deregisterTimersOfTask( const unsigned long tId );

   //
   // Function to check all timers to see if they have expired
   void checkTimers();

   //
   // Shutdown the message queues ...
   void shutdown();

   //
   // Cleanup the priority queue and the map ... 
   void cleanup();

private:

   long long _Time;

   struct MapEntry;
   class QueueEntry
   {
   public:
      QueueEntry();
      QueueEntry( const QueueEntry &qe );

      virtual ~QueueEntry();

      QueueEntry &operator=( const QueueEntry &qe );

      int operator==( const QueueEntry &qe ) const;
      int operator==( const long long l ) const;
      int operator<( const QueueEntry &qe ) const;
      int operator<( const long long l ) const;
      int operator<=( const long long l ) const;
      int operator>( const QueueEntry &qe ) const;
      int operator>( const long long l ) const;
      int operator>=( const long long l ) const;

   public:

      long long _ExpirationTime;
      MapEntry           *_MapEntryPtr;

   };

   struct MapEntry
   {
      MessagePacket *_TimerMessage;
      unsigned long _Interval;
   };

   map< unsigned long, MapEntry* > _TimerMsgMap;
   priority_queue< QueueEntry, vector< QueueEntry >, greater< QueueEntry > > _TimerQueue;

   //
   // This structure will map the task Id with it's associated Posix message queue. The 
   //  router will open a queue for each task as a write only queue.  The task must create
   //  the queue before registering itself with the router.
   map< unsigned long, mqd_t >                                  _TaskQueueMap;

   mqd_t _TimerMQ;

   bool _StopLoop;

};

#ifdef __cplusplus 
extern "C" void msgsystimerInit();
extern "C" void msgsystimerDump();
#else 
void msgsystimerInit();
void msgsystimerDump();
#endif

#endif
