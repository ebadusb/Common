/*
 * Copyright (c) 2002 Gambro BCT, Inc.  All rights reserved.
 *
 * TITLE:      dispatcher.cpp
 *
 */

#include <vxWorks.h>

#include "error.h"
#include "dispatcher.h"
#include "messagesystemconstant.h"


Dispatcher :: Dispatcher( ) :
_Blocking( true ),
_StopLoop( false )
{
}

Dispatcher :: ~Dispatcher()
{
   shutdown();
   cleanup();
}

void Dispatcher :: init( const char *qname, unsigned int maxMessages, const bool block )
{
   if ( !qname )
   {
      //
      // Error ...
      _FATAL_ERROR( __FILE__, __LINE__, "Message queue name not provided" );
      return;
   }

   struct timespec ts;
   clock_gettime( CLOCK_REALTIME, &ts );

   struct mq_attr attr;                                                             // message queue attributes 
   attr.mq_maxmsg =  ( maxMessages>MessageSystemConstant::MAX_MESSAGE_QUEUE_SIZE ?  // set max number of messages 
                                MessageSystemConstant::MAX_MESSAGE_QUEUE_SIZE :     //            ... maximum queue size
                                ( maxMessages==0 ? 1 : maxMessages ) );             // ... minimun queue size = 1              
   attr.mq_msgsize = sizeof( MessagePacket );     // set message size 
   if ( block == false )
   {
      attr.mq_flags = O_NONBLOCK;                 // set non-block 
      _Blocking = false;
      _StopLoop = true;
   }
   
   //
   // Open the message queue for read-only ...
   unsigned int retries=0;
   do
   {
      _MyQueue = mq_open( qname, O_RDONLY | O_CREAT, 0666, &attr);
      nanosleep( &MessageSystemConstant::RETRY_DELAY, 0 );
   }
   while (    _MyQueue == (mqd_t)ERROR 
           && retries++ < MessageSystemConstant::MAX_NUM_RETRIES );

   if ( _MyQueue == (mqd_t)ERROR )
   {
      //
      // Error ...
      _FATAL_ERROR( __FILE__, __LINE__, "Message queue open failed" );
      return;
   }

   //
   // Open the router's queue ...
   retries=0;
   do
   {
      _RQueue = mq_open( "router", O_WRONLY );
      nanosleep( &MessageSystemConstant::RETRY_DELAY, 0 );
   } 
   while (    _RQueue == (mqd_t)ERROR 
           && retries++ < MessageSystemConstant::MAX_NUM_RETRIES );

   if ( _RQueue == (mqd_t)ERROR )
   {
      //
      // Error ...
      _FATAL_ERROR( __FILE__, __LINE__, "Router message queue open failed" );
      return;
   }

   //
   // open the timer task's queue ...
   retries=0;
   do
   {
      _TimerQueue = mq_open( "timertask", O_WRONLY );
      nanosleep( &MessageSystemConstant::RETRY_DELAY, 0 );
   } 
   while (    _TimerQueue == (mqd_t)ERROR 
           && retries++ < MessageSystemConstant::MAX_NUM_RETRIES );

   if ( _TimerQueue == (mqd_t)ERROR )
   {
      //
      // Error ...
      _FATAL_ERROR( __FILE__, __LINE__, "Timer task message queue open failed" );
      return;
   }

   //
   // Register this task ...
   MessagePacket mp;
   char *tname = taskName( 0 );
   int length = strlen( tname );
   mp.msgData().osCode( MessageData::TASK_REGISTER );
   mp.msgData().msgId( 0 );
   mp.msgData().nodeId( 0 );
   mp.msgData().taskId( taskIdSelf() );
   mp.msgData().msgLength( length );
   mp.msgData().sendTime( ts );
   mp.msgData().totalNum( 1 );
   mp.msgData().seqNum( 1 );
   mp.msgData().packetLength( length );
   mp.msgData().msg( (const unsigned char*)tname, length );
   mp.updateCRC();

   //
   // Send register message to router ...
   send( mp );

}

void Dispatcher :: send( const MessagePacket &mp )
{
   //
   // Send message packet to router ...
   unsigned int retries=0;
   while (    mq_send( _RQueue, &mp, sizeof( MessagePacket ), 
                       MessageSystemConstant::DEFAULT_MESSAGE_PRIORITY ) == ERROR 
           && retries++ < MessageSystemConstant::MAX_NUM_RETRIES )
      nanosleep( &MessageSystemConstant::RETRY_DELAY, 0 );
   if ( retries == MessageSystemConstant::MAX_NUM_RETRIES )
   {
      //
      // Error ...
      _FATAL_ERROR( __FILE__, __LINE__, "Router message queue send failed" );
      return;
   }

}

void Dispatcher :: sendTimerMessage( const MessagePacket &mp )
{
   //
   // Send message packet to timer task ...
   unsigned int retries=0;
   while (    mq_send( _TimerQueue, &mp, sizeof( MessagePacket ), 
                       MessageSystemConstant::DEFAULT_MESSAGE_PRIORITY ) == ERROR 
           && retries++ < MessageSystemConstant::MAX_NUM_RETRIES )
      nanosleep( &MessageSystemConstant::RETRY_DELAY, 0 );
   if ( retries == MessageSystemConstant::MAX_NUM_RETRIES )
   {
      //
      // Error ...
      _FATAL_ERROR( __FILE__, __LINE__, "Timer message queue send failed" );
      return;
   }

}

int Dispatcher :: dispatchMessages()
{

   //
   // mq_receive loop ...
   int size;
   do
   {
      MessagePacket mp;
      unsigned int retries=0;
      while (    ( size = mq_receive( _MyQueue, &mp, sizeof( MessagePacket ), 0 ) ) == ERROR 
              && retries++ < MessageSystemConstant::MAX_NUM_RETRIES );
      if ( size != ERROR )
      {
         processMessage( mp );
      }
      else
      {
         if ( errno == EAGAIN )
         {
            continue;
         }
         else
         {
            //
            // Error ...
            _FATAL_ERROR( __FILE__, __LINE__, "Message queue receive failed" );
            return ERROR;
         }
      }

   } while ( _StopLoop == false );

   //
   // Check the task's queue to see if it is full or not ...
   mq_attr qattributes;
   mq_getattr( _MyQueue, &qattributes );

   return (int)qattributes.mq_curmsgs;
}

void Dispatcher :: registerMessage( const MessageBase &mb, MessagePacket &mp )
{
   //
   // Register this message with the router ...
   send( mp );

   //
   // Add this message to the local list ...
   registerMessage( mp.msgData().msgId(), mb );
}

void Dispatcher :: registerMessage( const unsigned long mId, const MessageBase &mb )
{
   map< unsigned long, set< MessageBase* > >::iterator miter;

   miter = _MessageMap.find( mId ); // find the message in the reg. messages list ...

   //
   // If anyone registered for this message ...
   if ( miter != _MessageMap.end() )
   {
      //
      // Make sure this message is not already in the set ...
      set< MessageBase* > &rSet = (*miter).second;
      if ( rSet.find( (MessageBase*) &mb ) == rSet.end() ) // Message not found ...
      {
         rSet.insert( (MessageBase*) &mb );
      }
   }
   else // no one has registered for the message yet ...
   {
      set< MessageBase* > newSet;
      newSet.insert( (MessageBase*) &mb );

      //
      // Give it to the map ...
      _MessageMap[ mId ] = newSet;
   }

}

void Dispatcher :: deregisterMessage( const MessageBase &mb, MessagePacket &mp )
{
   //
   // Deregister this message with the router ...
   send( mp );

   //
   // Remove this message from the local list
   deregisterMessage( mp.msgData().msgId(), mb );
}

void Dispatcher :: deregisterMessage( const unsigned long mId, const MessageBase &mb )
{
   map< unsigned long, set< MessageBase* > >::iterator miter;

   miter = _MessageMap.find( mId ); // find the message in the reg. messages list ...

   //
   // If anyone registered for this message ...
   if ( miter != _MessageMap.end() )
   {
      //
      // Remove this message from the set ...
      set< MessageBase* > &rSet = (*miter).second;
      rSet.erase( (MessageBase*) &mb );

      if ( rSet.empty() == true )
         _MessageMap.erase( miter );
   }

}

void Dispatcher :: dump( ostream &outs )
{                  
   outs << "************************* Dispatcher DUMP *************************" << endl;
   set< MessageBase* >::iterator siter;
   map< unsigned long, set< MessageBase* > >::iterator miter;

   outs << "Map size = " << dec << _MessageMap.size() << " " << (int)(_MessageMap.begin() == _MessageMap.end()) << endl;
   for ( miter = _MessageMap.begin() ; 
         miter != _MessageMap.end() ;
         miter++ )
   {
      for ( siter = (*miter).second.begin() ;
            siter != (*miter).second.end() ;
            siter++ )
      {
         ( (MessageBase*)(*siter) )->dump( outs );
      }
    }
   outs << "************************** DUMP finished **************************" << endl;
}

void Dispatcher :: processMessage( MessagePacket &mp )
{
   //
   // Validate the CRC ...
   if ( mp.validCRC() == true )
   {
      //
      // Distribute the message to all who registered ...
      //
      set< MessageBase* >::iterator siter;
      map< unsigned long, set< MessageBase* > >::iterator miter;

      miter = _MessageMap.find( mp.msgData().msgId() ); // find the message in the reg. messages list ...

      //
      // If anyone registered for this message ...
      if ( miter != _MessageMap.end() )
         //
         // Distribute the message ...
         for ( siter = (*miter).second.begin() ; siter != (*miter).second.end() ; siter++ )
         {
            if ( ((MessageBase*)(*siter))->notify( mp ) == false )
            {
               char buffer[256];
               sprintf( buffer, "Message notification failed for MsgId=%lx", mp.msgData().msgId() );
               _FATAL_ERROR( __FILE__, __LINE__, buffer );
            }
         }
   }
   else
   {
      //
      // Error ...
      char buffer[256];
      unsigned long crc = mp.crc();
      mp.updateCRC();
      sprintf( buffer, "Message CRC validation failed for MsgId=%lx, CRC=%lx and should be %lx",
               mp.msgData().msgId(), crc, mp.crc() );
      _FATAL_ERROR( __FILE__, __LINE__, buffer );
      return;
   }
}

void Dispatcher :: shutdown()
{
   //
   // Close my queue ...
   mq_close( _MyQueue );
   _MyQueue = (mqd_t)0;

   //
   // Deregister me with the router ...

   struct timespec ts;
   clock_gettime( CLOCK_REALTIME, &ts );

   MessagePacket mp;
   char *tname = taskName( 0 );
   int length = strlen( tname );
   mp.msgData().osCode( MessageData::TASK_DEREGISTER );
   mp.msgData().msgId( 0 );
   mp.msgData().nodeId( 0 );
   mp.msgData().taskId( taskIdSelf() );
   mp.msgData().msgLength( length );
   mp.msgData().sendTime( ts );
   mp.msgData().totalNum( 1 );
   mp.msgData().seqNum( 1 );
   mp.msgData().packetLength( length );
   mp.msgData().msg( (const unsigned char*)tname, length );
   mp.updateCRC();

   //
   // Send deregister message to router ...
   send( mp );
   sendTimerMessage( mp );

   //
   // Close the router's queue ...
   mq_close( _RQueue );
   _RQueue = (mqd_t)0;

   //
   // Close the timer task's queue ...
   mq_close( _TimerQueue );
   _TimerQueue = (mqd_t)0;

}

void Dispatcher :: cleanup()
{
   //
   // Cleanup the map ...
   //  ( no dynamic memory was created, so doing
   //    a clear will clean the map fine )
   _MessageMap.clear();

}

