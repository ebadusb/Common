/*
 * Copyright (c) 2002 Gambro BCT, Inc.  All rights reserved.
 *
 * TITLE:      msgsystimer.cpp, Message System Timer 
 *
 */

#include <stdio.h>
#include <vxWorks.h>
#include <taskHookLib.h>

#include "auxclock.h"
#include "datalog.h"
#include "error.h"
#include "messagesystemconstant.h"
#include "msgsystimer.h"


WIND_TCB    *MsgSysTimer::_TheTimerTid=0;
MsgSysTimer *MsgSysTimer::_TheTimer=0;
int MsgSysTimer::_ReadyToReceiveTickMsg=0;


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

MsgSysTimer::MsgSysTimer() 
 : _TickCount( 0 ),
   _TimerMsgMap(),
   _TimerQueue(),
   _TimerMQ( (mqd_t)ERROR ),
   _RouterMQ( (mqd_t)ERROR ),
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
   // I'm now ready to receive a tick count message ...
   _ReadyToReceiveTickMsg=1;

   //
   // Open the router's queue ...
   retries=0;
   do
   {
      _RouterMQ = mq_open( "router", O_WRONLY );
      nanosleep( &MessageSystemConstant::RETRY_DELAY, 0 );
   } 
   while (    _RouterMQ == (mqd_t)ERROR 
           && ++retries < MessageSystemConstant::MAX_NUM_RETRIES );

   if ( _RouterMQ == (mqd_t)ERROR )
   {
      //
      // Error ...
      _FATAL_ERROR( __FILE__, __LINE__, "Router message queue open failed" );
      return false;
   }

   //
   // Initialize the ISR routine ...
   auxClockMsgPktEnable( 10 /* tick rate to notify me every 10ms */, "timertask", &MsgSysTimer::_ReadyToReceiveTickMsg );

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
              && retries++ < MessageSystemConstant::MAX_NUM_RETRIES ) 
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
         char buffer[256];
         unsigned long crc = mp.crc();
         mp.updateCRC();
         sprintf( buffer,"Maintain timers - message CRC validation failed for MsgId=%lx, CRC=%lx and should be %lx",
                  mp.msgData().msgId(), crc, mp.crc() );
         _FATAL_ERROR( __FILE__, __LINE__, buffer );
         return;
      }  

      processMessage( mp );

      //
      // Check the overrun counter to see if I'm behind on tick counts...
      //  ( Log the result if I am behind )
      unsigned long overruns =0; // = auxClockNotificationOverruns();
      if ( overruns > 0 )
      {
         DataLog_Level logError( LOG_ERROR );
         DataLog( logError ) << "TimerOverruns: " << overruns  << endmsg;
      }

   } while ( _StopLoop == false );

}

void MsgSysTimer::dump( ostream &outs )
{
   outs << "??????????????????????? MsgSysTimer DUMP ??????????????????????????" << endl;
   outs << " TickCount: " << dec << ((unsigned long)_TickCount) << endl;
   outs << " TimerMQueue: " << hex << (long)_TimerMQ << endl;
   outs << " RouterMQueue: " << hex << (long)_RouterMQ << endl;
   outs << " StopLoop: " << _StopLoop << endl;

   outs << " Timer Message Map: size " << dec << _TimerMsgMap.size() << endl;
   map< unsigned long, MapEntry* >::iterator miter;
   for ( miter  = _TimerMsgMap.begin() ;
         miter != _TimerMsgMap.end() ;
         miter++ )
   {
      outs << "  Mid " << hex << (*miter).first << " interval: " << dec << ((*miter).second)->_Interval 
                       << hex << " message_pckt: " << ((*miter).second)->_TimerMessage << dec << endl;
      if ( ((*miter).second)->_TimerMessage )
         ((*miter).second)->_TimerMessage->dump( outs );
   }

   outs << " PriorityQueue: size " << dec <<  _TimerQueue.size() << endl;
   if ( !_TimerQueue.empty() )
   {
      outs << "  top: tick_count " <<  dec << ((unsigned long)_TimerQueue.top()._ExpirationTickCount) << endl;
      outs << "       map_entry " << hex << _TimerQueue.top()._MapEntryPtr;
      if ( _TimerQueue.top()._MapEntryPtr )
      {
         outs << " : interval " << dec << _TimerQueue.top()._MapEntryPtr->_Interval << " ";
         outs << " : message_pckt " << hex << _TimerQueue.top()._MapEntryPtr->_TimerMessage <<  dec << endl;
         if ( _TimerQueue.top()._MapEntryPtr->_TimerMessage )
            _TimerQueue.top()._MapEntryPtr->_TimerMessage->dump( outs );
      }
   }

   outs << "???????????????????????????????????????????????????????????????????" << endl;
}

void MsgSysTimer::processMessage( const MessagePacket &mp )
{
   //
   // Determine what type of message this is ...
   //
   long long tc;
   unsigned long interval;
   switch ( mp.msgData().osCode() )
   {
   case MessageData::TASK_DEREGISTER:
      deregisterTimersOfTask( mp.msgData().taskId() );
      break;
   case MessageData::TIME_UPDATE:
      memmove( (char *) &tc , 
               (char *) mp.msgData().msg(), 
               sizeof( long long ) );
      updateTime( tc );
      _ReadyToReceiveTickMsg=1;
      break;
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

void MsgSysTimer::updateTime( const long long tickCount )
{
   //
   // Save the new tick count ...
   _TickCount = tickCount;

   //
   // Check the registered timers ...
   checkTimers();
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

   //
   // Create a new Queue Entry
   QueueEntry qe;
   qe._MapEntryPtr = mePtr;  // the map entry object is owned by the queue entry object
   qe._ExpirationTickCount = _TickCount + interval;

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
         _TimerMsgMap.erase( miter );
      }
   }

}

void MsgSysTimer::checkTimers()
{
   struct timespec ts;
   // 
   // Check the top of the priority queue for entries which
   //  have expired ...
   while (    !_TimerQueue.empty()
           && _TimerQueue.top() <= _TickCount )
   {
      QueueEntry qe = _TimerQueue.top();
      _TimerQueue.pop();

      //
      // If the timer is not disarmed...
      if (    qe._MapEntryPtr 
           && qe._MapEntryPtr->_Interval > 0 )
      {
         //
         // Get the message packet to send off ...
         clock_gettime( CLOCK_REALTIME, &ts );
         MessagePacket *mpPtr = qe._MapEntryPtr->_TimerMessage;
         mpPtr->msgData().sendTime( ts );
         mpPtr->updateCRC();

         //
         // Reinsert the timer for its next interval ...
         qe._ExpirationTickCount += qe._MapEntryPtr->_Interval;
         _TimerQueue.push( qe );

         //
         // Send the message packet ...
         unsigned int retries=0;
         while (    mq_send( _RouterMQ, mpPtr, sizeof( MessagePacket ), 
                             MessageSystemConstant::DEFAULT_TIMER_MESSAGE_PRIORITY ) == ERROR
                 && retries++ < MessageSystemConstant::MAX_NUM_RETRIES );
         if ( retries == MessageSystemConstant::MAX_NUM_RETRIES )
         {
            char buffer[256];
            sprintf( buffer,"Check timers - timer Id=%lx, send failed",
                     mpPtr->msgData().msgId() );
            _FATAL_ERROR( __FILE__, __LINE__, buffer );
         }
         
      }
      else if ( qe._MapEntryPtr )
      {
         //
         // Delete the map entry object that the queue entry points to...
         if ( qe._MapEntryPtr->_TimerMessage )
            delete qe._MapEntryPtr->_TimerMessage;
         delete qe._MapEntryPtr;

      }
   }
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

   if ( _RouterMQ != (mqd_t)0 )
   {
      mq_close( _RouterMQ );
      _RouterMQ = (mqd_t)0;
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
 : _ExpirationTickCount( 0 ), 
   _MapEntryPtr( 0 )
{
}

MsgSysTimer::QueueEntry::QueueEntry( const QueueEntry &qe )
 : _ExpirationTickCount( qe._ExpirationTickCount ), 
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
      _ExpirationTickCount = qe._ExpirationTickCount;
      _MapEntryPtr = qe._MapEntryPtr; 
   }
   return *this; 
}

int MsgSysTimer::QueueEntry::operator==( const MsgSysTimer::QueueEntry &qe ) const 
{
   if ( _ExpirationTickCount == qe._ExpirationTickCount )
      return 1;
   return 0;
}

int MsgSysTimer::QueueEntry::operator==( const long long l ) const 
{
   if ( _ExpirationTickCount == l )
      return 1;
   return 0;
}

int MsgSysTimer::QueueEntry::operator<( const MsgSysTimer::QueueEntry &qe ) const 
{
   return( _ExpirationTickCount < qe._ExpirationTickCount ); 
}

int MsgSysTimer::QueueEntry::operator<( const long long l ) const 
{
   return( _ExpirationTickCount < l );
}

int MsgSysTimer::QueueEntry::operator<=( const long long l ) const
{
   return ( _ExpirationTickCount <= l );
}

int MsgSysTimer::QueueEntry::operator>( const MsgSysTimer::QueueEntry &qe ) const 
{
   return( _ExpirationTickCount > qe._ExpirationTickCount ); 
}

int MsgSysTimer::QueueEntry::operator>( const long long l ) const 
{
   return( _ExpirationTickCount > l );
}

int MsgSysTimer::QueueEntry::operator>=( const long long l ) const
{
   return ( _ExpirationTickCount >= l );
}

void msgsystimerDump()
{
   if ( MsgSysTimer::globalMsgSysTimer() )
      MsgSysTimer::globalMsgSysTimer()->dump( cout );
}
