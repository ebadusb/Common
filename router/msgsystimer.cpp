/*
 * Copyright (c) 2002 Gambro BCT, Inc.  All rights reserved.
 *
 * TITLE:      msgsystimer.cpp, Message System Timer 
 *
 */

#include <vxWorks.h>

#include <stdio.h>
#include <taskHookLib.h>

#include "auxclock.h"
#include "datalog.h"
#include "datalog_levels.h"
#include "error.h"
#include "failure_debug.h"
#include "messagesystemconstant.h"
#include "msgsystimer.h"


WIND_TCB    *MsgSysTimer::_TheTimerTid=0;
MsgSysTimer *MsgSysTimer::_TheTimer=0;
int MsgSysTimer::_ReadyToReceiveTimeMsg=0;
int MilliSecPerTick = 50;

int MsgSysTimer::MsgSysTimer_main()
{
   if ( _TheTimer )
      return !OK;

   MsgSysTimer msgSysTimer;
   msgSysTimer.init();

   msgSysTimer.maintainTimers();

   return OK;
}

WIND_TCB *MsgSysTimer::globalMsgSysTimerTid()
{
   return _TheTimerTid;
}

MsgSysTimer *MsgSysTimer::globalMsgSysTimer()
{
   return _TheTimer;
}

int MsgSysTimer::taskCreateHook( WIND_TCB *pTcb )
{
   return 1;
}

int MsgSysTimer::taskDeleteHook( WIND_TCB *pTcb )
{
   if ( _TheTimerTid == pTcb )
   {
      delete _TheTimer; 
      _TheTimer = 0;
      _TheTimerTid = 0;
   }

   if ( _TheTimer )
   {
      _TheTimer->deregisterTimersOfTask( (unsigned long) pTcb );
   }
   
   return 1;
}

void MsgSysTimer::datalogErrorHandler( const char * file, int line, 
                                       DataLog_ErrorType error, 
                                       const char * msg, 
                                       int continuable )
{
   if ( !continuable )
   {
      _FATAL_ERROR( __FILE__, __LINE__, "Data log error" );
   }
   cerr << "Data log error - " << error << " : " << msg << endl;
}

MsgSysTimer::MsgSysTimer() 
 : _Ticks( 0 ),
   _TimerMsgMap(),
   _TimerQueue(),
   _TaskQueueMap(),
   _TaskQueueActiveMap(),
   _TimerMQ( (mqd_t)ERROR ),
   _MessageHighWaterMark( 0 ),
   _NumMessages( 0 ),
   _MessageHighWaterMarkPerPeriod( 0 ),
   _PrevMessageHighWaterMarkPerPeriod( 0 ),
   _StopLoop( false )
{
}

MsgSysTimer::~MsgSysTimer()
{
   shutdown();
   cleanup();
}


bool MsgSysTimer::init()
{
   if ( _TheTimer )
      return false;

   //
   // Add the task delete hooks to catch all task deletion and 
   //  keep the timer lists up to date ...
   taskDeleteHookAdd( (FUNCPTR) &MsgSysTimer::taskDeleteHook );

   //
   // Install the datalog error handler ...
   datalog_SetTaskErrorHandler( taskIdSelf(), &MsgSysTimer::datalogErrorHandler );

   //
   // Set up my queue ...
   struct mq_attr attr;                                      // message queue attributes 
   attr.mq_maxmsg = 15; 
   attr.mq_msgsize = sizeof( MessagePacket );                // set message size 
   attr.mq_flags = 0;
   
   //
   // Open the message queue for read-only ...
   unsigned int retries=0;
   do
   {
      _TimerMQ = mq_open( "timertask", O_RDONLY | O_CREAT, 0666, &attr);
      nanosleep( &MessageSystemConstant::RETRY_DELAY, 0 );
   }
   while (    _TimerMQ == (mqd_t)ERROR 
           && ++retries < MessageSystemConstant::MAX_NUM_RETRIES );

   if ( _TimerMQ == (mqd_t)ERROR )
   {
      //
      // Error ...
      _FATAL_ERROR( __FILE__, __LINE__, "Timer message queue open failed" );
      return false;
   }
   //
   // I'm now ready to receive a time message ...
   _ReadyToReceiveTimeMsg=1;

   //
   // Initialize the ISR routine ...
   unsigned long	minMilliSecPerTick = 1000/auxClockRateGet();
   if ( MilliSecPerTick < minMilliSecPerTick)
	{
      DataLog( log_level_critical ) << "MsgSysTimer increased tick time from " <<
									MilliSecPerTick << " to " << minMilliSecPerTick << " msec" << endmsg;
		
		MilliSecPerTick = minMilliSecPerTick;
	}

   auxClockMsgPktEnable( 1000 * MilliSecPerTick, "timertask" );

   //
   // Set the static pointer to ensure we only init once ...
   _TheTimer = this;
   _TheTimerTid = taskTcb( 0 );

   return true;
}

void MsgSysTimer::maintainTimers()
{
   if ( _TimerMQ == 0 )
      return;

   int size=0;
   MessagePacket mp;
   do
   {
      //
      // Read the queue entry ...
      unsigned int retries=0;
      while (    ( size = mq_receive( _TimerMQ, &mp, sizeof( MessagePacket ), 0 ) ) == ERROR 
              && ++retries < MessageSystemConstant::MAX_NUM_RETRIES ) 
         nanosleep( &MessageSystemConstant::RETRY_DELAY, 0 );
      if ( size == ERROR )
      {
         //
         // Error ...
         _FATAL_ERROR( __FILE__, __LINE__, "Maintain timers - message queue receive failed" );
         return;
      }

      if ( mp.validCRC() == false )
      {
         //
         // Error ...
         unsigned long crc = mp.crc();
         mp.updateCRC();
         DataLog( log_level_critical ) << "Maintain timers - message CRC validation failed for MsgId=" << hex << mp.msgData().msgId() 
                              << ", CRC=" << crc << " and should be " << mp.crc() << endmsg;
         _FATAL_ERROR( __FILE__, __LINE__, "CRC check failed" );
         return;
      }  

      processMessage( mp );

      //
      // Check the task's queue to see if we went over the high-water mark ...
      mq_attr qattributes;
      mq_getattr( _TimerMQ, &qattributes );
      if ( qattributes.mq_curmsgs > _MessageHighWaterMark )
      {
         _MessageHighWaterMark = _MessageHighWaterMarkPerPeriod = qattributes.mq_curmsgs;
         DataLog( log_level_message_system_info ) << "mqueue max: " << _MessageHighWaterMark << "/" << qattributes.mq_maxmsg << endmsg;
      }
      else 
      {
         if ( qattributes.mq_curmsgs > _MessageHighWaterMarkPerPeriod )
            _MessageHighWaterMarkPerPeriod = qattributes.mq_curmsgs;

         if ( ( _NumMessages % ( MessageSystemConstant::MESSAGES_BETWEEN_LOG * qattributes.mq_maxmsg ) ) == 0 )
         {
            if ( _MessageHighWaterMarkPerPeriod != _PrevMessageHighWaterMarkPerPeriod )
               DataLog( log_level_message_system_info ) << "mqueue max per period: " << _MessageHighWaterMarkPerPeriod << "/" << qattributes.mq_maxmsg << endmsg;
            _PrevMessageHighWaterMarkPerPeriod = _MessageHighWaterMarkPerPeriod;
            _MessageHighWaterMarkPerPeriod = 0;
         }
      }

      _NumMessages++;

   } while ( _StopLoop == false );

}

void MsgSysTimer::dump( DataLog_Stream &outs )
{
   outs << "??????????????????????? MsgSysTimer DUMP ??????????????????????????" << endmsg;
   // mq_attr qattributes;
   // if ( _TimerMQ != (mqd_t)0 ) mq_getattr( _TimerMQ, &qattributes );
   outs << " MsgSysTimerQueue: " << hex << (long)_TimerMQ 
        // << "  flags " << qattributes.mq_flags
        // << "  size " << qattributes.mq_curmsgs
        // << "  maxsize " << qattributes.mq_maxmsg 
        << endmsg;

   outs << "   Task Queue Map: size " << dec << _TaskQueueMap.size() << endmsg;
   map< unsigned long, mqd_t >::iterator tqiter;                                  // _TaskQueueMap;
   for ( tqiter  = _TaskQueueMap.begin() ;
         tqiter != _TaskQueueMap.end() ;
         tqiter++ )
   {
      // if ( (*tqiter).second != (mqd_t)0 ) mq_getattr( (*tqiter).second, &qattributes );
      outs << "    Tid " << hex << (*tqiter).first << " " << hex << (long)(*tqiter).second << " " << (bool)_TaskQueueActiveMap[ (*tqiter).first ]
           // << "  flags " << qattributes.mq_flags
           // << "  size " << qattributes.mq_curmsgs
           // << "  maxsize " << qattributes.mq_maxmsg 
           << endmsg;
   }
   outs << endmsg 
        << " Ticks: " << dec << _Ticks << endmsg;
   outs << " StopLoop: " << _StopLoop << endmsg;

   outs << " Timer Message Map: size " << dec << _TimerMsgMap.size() << endmsg;
   map< unsigned long, MapEntry* >::iterator miter;
   for ( miter  = _TimerMsgMap.begin() ;
         miter != _TimerMsgMap.end() ;
         miter++ )
   {
      outs << "  Mid " << hex << (*miter).first << " interval: " << dec << ((*miter).second)->_Interval << "msecs" 
                       << hex << " message_pckt: " << ((*miter).second)->_TimerMessage << dec << endmsg;
      if ( ((*miter).second)->_TimerMessage )
         ((*miter).second)->_TimerMessage->dump( outs );
   }

   outs << " PriorityQueue: size " << dec <<  _TimerQueue.size() << endmsg;
   if ( !_TimerQueue.empty() )
   {
      outs << "  top: exp. time " <<  dec << ((unsigned long)_TimerQueue.top()._ExpirationTick) << " ticks"<< endmsg;
      outs << "       map_entry " << hex << _TimerQueue.top()._MapEntryPtr;
      if ( _TimerQueue.top()._MapEntryPtr )
      {
         outs << " : interval " << dec << _TimerQueue.top()._MapEntryPtr->_Interval << " msecs ";
         outs << " : message_pckt " << hex << _TimerQueue.top()._MapEntryPtr->_TimerMessage <<  dec << endmsg;
         if ( _TimerQueue.top()._MapEntryPtr->_TimerMessage )
            _TimerQueue.top()._MapEntryPtr->_TimerMessage->dump( outs );
      }
   }

   outs << "???????????????????????????????????????????????????????????????????" << endmsg;
}

void MsgSysTimer::processMessage( const MessagePacket &mp )
{
   //
   // Determine what type of message this is ...
   //
   unsigned long ticks;
   unsigned long interval;
   switch ( mp.msgData().osCode() )
   {
   case MessageData::TASK_REGISTER:
      registerTask( mp.msgData().taskId(), (const char *)(mp.msgData().msg() ) );
      break;
   case MessageData::TASK_DEREGISTER:
      deregisterTask( mp.msgData().taskId() );
      deregisterTimersOfTask( mp.msgData().taskId() );
      break;
   case MessageData::ENABLE_MESSAGE_QUEUE:
      if ( _TaskQueueMap.find( mp.msgData().taskId() ) != _TaskQueueMap.end() )
         _TaskQueueActiveMap[ mp.msgData().taskId() ] = true;
      break;
   case MessageData::DISABLE_MESSAGE_QUEUE:
      if ( _TaskQueueMap.find( mp.msgData().taskId() ) != _TaskQueueMap.end() )
         _TaskQueueActiveMap[ mp.msgData().taskId() ] = false;
      break;
   case MessageData::TIME_UPDATE:
      memmove( (char *) &ticks , 
               (char *) mp.msgData().msg(), 
               sizeof( ticks ) );
      updateTicks( ticks );
      break;
   case MessageData::GATEWAY_CONNECT:
   case MessageData::DISTRIBUTE_LOCALLY:
   case MessageData::DISTRIBUTE_GLOBALLY:
   case MessageData::SPOOFED_LOCALLY:
   case MessageData::SPOOFED_GLOBALLY:
      memmove( (char *) &interval , 
               (char *) mp.msgData().msg(), 
               sizeof( unsigned long ) );
      if ( interval == 0 )
         deregisterTimer( mp.msgData().msgId() );
      else
         registerTimer( mp, interval );

      break;
   default:
      _FATAL_ERROR( __FILE__, __LINE__, "Unknown OSCode in message packet" );
      break;
   }

}

void MsgSysTimer::updateTicks( unsigned long ticks )
{
   //
   // Save the new time ...
   _Ticks = ticks;

   //
   // Check the registered timers ...
   checkTimers();
}

void MsgSysTimer::registerTask( unsigned long tId, const char *qName )
{
   //
   // Try to find the task in the list ...
   if ( _TaskQueueMap.find( tId ) == _TaskQueueMap.end() )
   {
      //
      // The task is not registered, 
      //  so try to open the given queue ...
      mqd_t tQueue = (mqd_t)ERROR;
      unsigned int retries=0;
      while ( ( tQueue = mq_open( qName, O_RDWR ) ) == (mqd_t)ERROR 
              && ++retries < MessageSystemConstant::MAX_NUM_RETRIES ) nanosleep( &MessageSystemConstant::RETRY_DELAY, 0 );

      //
      // If opened ...
      if ( tQueue != (mqd_t)ERROR )
      {
         //
         // ... add task and queue to map ...
         _TaskQueueMap[ tId ] = tQueue;
         _TaskQueueActiveMap[ tId ] = false;

      }
      //
      // if not opened ...
      else
      {
         //
         // ... error, the task should already have opened the queue
         //  before connecting to the router.
         //
         // Error ...
         DataLog( log_level_critical ) << "Register task=" << hex << tId << "(" << taskName( tId ) << ") - message queue open failed" 
                              << endmsg;
         _FATAL_ERROR( __FILE__, __LINE__, "mq_open failed" );
      }
   }

}

void MsgSysTimer::deregisterTask( unsigned long tId )
{
   //
   // Find the task in the list ...
   map< unsigned long, mqd_t >::iterator titer;
   titer = _TaskQueueMap.find( tId );
   if ( titer != _TaskQueueMap.end() )
   {
      //
      // If found ...  
      //    remove the task and close the queue
      // 
      mq_close( (mqd_t)(*titer).second );
      _TaskQueueMap.erase( titer );
      _TaskQueueActiveMap.erase( tId );
   }
}

void MsgSysTimer::registerTimer( const MessagePacket &mp, const unsigned long interval )
{
   //
   // Deregister the timer if it already existed ...
   deregisterTimer( mp.msgData().msgId() );

   //
   // Create a new Map Entry 
   MapEntry *mePtr = new MapEntry;
   MessagePacket *mpPtr = new MessagePacket( mp );
   mePtr->_TimerMessage = mpPtr;
   mePtr->_Interval = interval;
   mePtr->_IntervalInTicks = interval/MilliSecPerTick;
   if ( mePtr->_IntervalInTicks < 1 )
	{
		mePtr->_IntervalInTicks = 1;
	}

   //
   // Create a new Queue Entry
   QueueEntry qe;
   qe._MapEntryPtr = mePtr;  // the map entry object is owned by the queue entry object
   qe._ExpirationTick = _Ticks + mePtr->_IntervalInTicks;
   if ( qe._ExpirationTick == _Ticks )
	{
		qe._ExpirationTick = _Ticks+1;
	}

   //
   // Add the queue entry to the priority queue ...
   _TimerQueue.push( qe );

   //
   // Create a new entry in the timer message map ...
   _TimerMsgMap[ mp.msgData().msgId() ] = mePtr;

}

void MsgSysTimer::deregisterTimer( const unsigned long mId )
{
   //
   // Find the timer in the timer message map ...
   map< unsigned long, MapEntry* >::iterator miter =
                                               _TimerMsgMap.find( mId );

   //
   // If found ...
   if ( miter != _TimerMsgMap.end() )
   {
      //
      // Set the map entry object to have an interval
      //  of zero, which will disable the map and queue entry 
      MapEntry *meOldPtr = (*miter).second;
      meOldPtr->_Interval = 0;
      meOldPtr->_IntervalInTicks = 0;

      //
      // remove the map entry from the timer msg. map ...
      _TimerMsgMap.erase( miter );
   }

}

void MsgSysTimer::deregisterTimersOfTask( const unsigned long tId )
{
   //
   // Iterate through the TimerMsgMap and delete and obsolete all timers
   //  which belong the the given tId ...
   MapEntry *mePtr;
   map< unsigned long, MapEntry* >::iterator miter;
   for ( miter = _TimerMsgMap.begin() ; 
         miter != _TimerMsgMap.end() ; 
         miter++ )
   {
      mePtr = (*miter).second;
      if (    mePtr->_TimerMessage 
           && mePtr->_TimerMessage->msgData().taskId() == tId )
      {
         //
         // deregister this timer ...
         mePtr->_Interval = 0;
         mePtr->_IntervalInTicks = 0;
         _TimerMsgMap.erase( miter );
      }
   }

}

void MsgSysTimer::checkTimers()
{
   // 
   // Check the top of the priority queue for entries which
   //  have expired ...
   QueueEntry qe;
   while (    !_TimerQueue.empty()
           && ( qe = _TimerQueue.top() ) <= _Ticks )
   {
      _TimerQueue.pop();

      //
      // If the timer is not disarmed...
      if ( qe._MapEntryPtr->_Interval > 0 )
      {
         //
         // Get the message packet to send off ...
         MessagePacket *mpPtr = qe._MapEntryPtr->_TimerMessage;
         mpPtr->updateTime();
         mpPtr->updateCRC();

         //
         // Reinsert the timer for its next interval ...
         qe._ExpirationTick += ( qe._MapEntryPtr->_IntervalInTicks );
         _TimerQueue.push( qe );

         //
         // Get the task out of the registered task/queue map ...
         map< unsigned long, mqd_t >::iterator tqiter;
         tqiter = _TaskQueueMap.find( mpPtr->msgData().taskId() );

         if ( tqiter == _TaskQueueMap.end() )
         {
            //
            // Error ...
            DataLog( log_level_critical ) << "Sending message=" << hex << mpPtr->msgData().msgId() 
                                 << "- Task Id=" << mpPtr->msgData().taskId() 
                                 << "(" << taskName( mpPtr->msgData().taskId() ) << ")"
                                 << " not found in task list" << endmsg;
            _FATAL_ERROR( __FILE__, __LINE__, "Task lookup failed" );
         }
         //
         // If the task was found to be registered ...
         else
         {
            if ( _TaskQueueActiveMap[ mpPtr->msgData().taskId() ] == false )
               return; // Don't send if the task hasn't activated its mqueue

            //
            // Check the task's queue to see if it is full or not ...
            mq_attr qattributes;
            if (    mq_getattr( (*tqiter).second, &qattributes ) == ERROR
                 || qattributes.mq_curmsgs >= qattributes.mq_maxmsg )
            {
               //
               // The task's queue is full!
               //
               // Error ...
               DataLog( log_level_critical ) << "Sending message=" << hex << mpPtr->msgData().msgId() 
                                    << "- Task Id=" << mpPtr->msgData().taskId() 
                                    << "(" << taskName( mpPtr->msgData().taskId() ) << ")"
                                    << " queue full (" << dec << qattributes.mq_curmsgs << " messages)" 
                                    << ", (" << errnoMsg << ")"
                                    << endmsg;
               DBG_DumpData();
               dumpQueue( (*tqiter).second, DataLog( log_level_message_system_timer_error ) );

#if !DEBUG_BUILD && CPU != SIMNT
               _FATAL_ERROR( __FILE__, __LINE__, "Message queue full" );
#endif // #if CPU!=SIMNT && BUILD_TYPE!=DEBUG
               return;

            }

            //
            // Send the message packet ...
            unsigned int retries=0;
            while (    mq_send( (*tqiter).second, mpPtr, sizeof( MessagePacket ), 
                                MessageSystemConstant::DEFAULT_TIMER_MESSAGE_PRIORITY ) == ERROR
                    && ++retries < MessageSystemConstant::MAX_NUM_RETRIES );
            if ( retries >= MessageSystemConstant::MAX_NUM_RETRIES )
            {
               DataLog( log_level_critical ) << "Check timers - timer Id=" << hex << mpPtr->msgData().msgId() 
                                    << ", send failed for error-" << errnoMsg << endmsg;
               _FATAL_ERROR( __FILE__, __LINE__, "mq_send failed" );
            }
         }
      }
      else 
      {
         //
         // Delete the map entry object that the queue entry points to...
         if ( qe._MapEntryPtr->_TimerMessage )
            delete qe._MapEntryPtr->_TimerMessage;
         delete qe._MapEntryPtr;

      }
   }
}


void MsgSysTimer::dumpQueue( mqd_t mqueue, DataLog_Stream &out )
{
   struct mq_attr old_attr;                                    // message queue attributes 
   struct mq_attr attr;                                    // message queue attributes 
   attr.mq_flags = O_NONBLOCK;
   mq_setattr( mqueue, &attr, &old_attr );

   char buffer[ sizeof(MessagePacket) ];
   int count=0;
   int priority;

   //
   // Read the queue entry ...
   while ( mq_receive( mqueue, &buffer, sizeof( MessagePacket ), &priority ) != ERROR )
   {

#if !DEBUG_BUILD && CPU != SIMNT
      //
      // Format the data ...
      MessagePacket mp;
      memmove( &mp, &buffer, sizeof( MessagePacket ) );

      out << " Message# " << dec << count << " priority " << priority << " msgId " << hex << mp.msgData().msgId()
                                                                      << " p# "    << hex << mp.msgData().seqNum()
                                                                      << " tot# "  << hex << mp.msgData().totalNum()  
                                                                      << " msg -> ";
      for (int i=0;i<30;i++) 
      {
         out << hex << (int)(unsigned char)buffer[i] << " "; 
      }
      out << endmsg;
#endif // #if BUILD_TYPE!=DEBUG && CPU!=SIMNT

   }
   mq_setattr( mqueue, &old_attr, 0 );
}

void MsgSysTimer::shutdown()
{
   //
   // Close the queues ...
   if ( _TimerMQ != (mqd_t)0 )
   {
      mq_close( _TimerMQ );
      _TimerMQ = (mqd_t)0;
   }

   //
   // Close the other queues ...
   map< unsigned long, mqd_t >::iterator qiter;
   for ( qiter = _TaskQueueMap.begin() ;
         qiter != _TaskQueueMap.end() ;
         qiter++ )
   {
      mq_close( (*qiter).second );
      (*qiter).second = (mqd_t)0;
      _TaskQueueActiveMap[ (*qiter).first ] = false;
   }

}

void MsgSysTimer::cleanup()
{
   //
   // Remove the delete hook ...
   taskDeleteHookDelete( (FUNCPTR) &MsgSysTimer::taskDeleteHook );

   //
   // Clean the list structures ...
   _TimerMsgMap.empty();
   _TaskQueueMap.clear();
   _TaskQueueActiveMap.clear();

   MapEntry *mePtr;
   while ( _TimerQueue.size() > 0 )
   {
      QueueEntry qe = _TimerQueue.top();
      _TimerQueue.pop();

      mePtr = qe._MapEntryPtr;
      if ( mePtr)
      {
         if ( mePtr->_TimerMessage )
            delete mePtr->_TimerMessage;
         delete mePtr;
      }
   }

   _TheTimer=0;
   _TheTimerTid=0;
}

MsgSysTimer::QueueEntry::QueueEntry()
 : _ExpirationTick( 0 ), 
   _MapEntryPtr( 0 )
{
}

MsgSysTimer::QueueEntry::QueueEntry( const QueueEntry &qe )
 : _ExpirationTick( qe._ExpirationTick ), 
   _MapEntryPtr( qe._MapEntryPtr )
{
}

MsgSysTimer::QueueEntry::~QueueEntry()
{
}

MsgSysTimer::QueueEntry &MsgSysTimer::QueueEntry::operator=( const MsgSysTimer::QueueEntry &qe ) 
{
   if ( &qe != this )
   {
      _ExpirationTick = qe._ExpirationTick;
      _MapEntryPtr = qe._MapEntryPtr; 
   }
   return *this; 
}

int MsgSysTimer::QueueEntry::operator==( const MsgSysTimer::QueueEntry &qe ) const 
{
   if ( _ExpirationTick == qe._ExpirationTick )
      return 1;
   return 0;
}

int MsgSysTimer::QueueEntry::operator==( unsigned long l ) const 
{
   if ( _ExpirationTick == l )
      return 1;
   return 0;
}

int MsgSysTimer::QueueEntry::operator<( const MsgSysTimer::QueueEntry &qe ) const 
{
   return( _ExpirationTick < qe._ExpirationTick ); 
}

int MsgSysTimer::QueueEntry::operator<( unsigned long l ) const 
{
   return( _ExpirationTick < l );
}

int MsgSysTimer::QueueEntry::operator<=( unsigned long l ) const
{
   return ( _ExpirationTick <= l );
}

int MsgSysTimer::QueueEntry::operator>( const MsgSysTimer::QueueEntry &qe ) const 
{
   return( _ExpirationTick > qe._ExpirationTick ); 
}

int MsgSysTimer::QueueEntry::operator>( unsigned long l ) const 
{
   return( _ExpirationTick > l );
}

int MsgSysTimer::QueueEntry::operator>=( unsigned long l ) const
{
   return ( _ExpirationTick >= l );
}

void msgsystimerInit()
{
   MsgSysTimer::MsgSysTimer_main();
}

void msgsystimerDump()
{
   if ( MsgSysTimer::globalMsgSysTimer() )
      MsgSysTimer::globalMsgSysTimer()->dump( DataLog( log_level_message_system_timer_info ) );
}
