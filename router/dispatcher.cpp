/*
 * Copyright (c) 2002 Gambro BCT, Inc.  All rights reserved.
 *
 * TITLE:      dispatcher.cpp
 *
 */


#include "dispatcher.h"

const unsigned int Dispatcher::MAX_NUM_RETRIES=1;
const struct timespec Dispatcher::RETRY_DELAY={ 1 /* seconds */, 0 /*nanoseconds*/ };
const unsigned int Dispatcher::MAX_MESSAGE_QUEUE_SIZE=400;

Dispatcher :: Dispatcher( ) :
_Blocking( true ),
_StopLoop( false )
{
   cleanup();
}

Dispatcher :: ~Dispatcher()
{
   shutdown();
   cleanup();
}

void Dispatcher :: init( const char *qname, unsigned int maxMessages, const bool block )
{
   if ( !qname )
      //
      // Error ...
      return;

   struct timespec ts;
   clock_gettime( CLOCK_REALTIME, &ts );

   struct mq_attr attr;                                      // message queue attributes 
   attr.mq_maxmsg =  ( maxMessages>MAX_MESSAGE_QUEUE_SIZE ?  // set max number of messages 
                                MAX_MESSAGE_QUEUE_SIZE :     //            ... maximum queue size
                                ( maxMessages==0 ? 1 : maxMessages ) ); // ... minimun queue size = 1              
   attr.mq_msgsize = sizeof( MessagePacket );                // set message size 
   if ( block == false )
   {
      attr.mq_flags = O_NONBLOCK;                            // set non-block 
      _Blocking = false;
      _StopLoop = true;
   }
   
   //
   // Open the message queue for read-only ...
   unsigned int retries=0;
   _MyQueue = (mqd_t)ERROR;
   while (    _MyQueue == (mqd_t)ERROR 
           && retries++ < MAX_NUM_RETRIES )
   {
      _MyQueue = mq_open( qname, O_RDONLY | O_CREAT, 0666, &attr);
      nanosleep( &Dispatcher::RETRY_DELAY, 0 );
   }
   if ( _MyQueue == (mqd_t)ERROR )
      //
      // Error ...
      return;

   //
   // Open the router's queue ...
   retries=0;
   _RQueue = (mqd_t)ERROR;
   while (    _RQueue == (mqd_t)ERROR 
           && retries++ < 15 )
   {
      _RQueue = mq_open( "router", O_WRONLY );
      nanosleep( &Dispatcher::RETRY_DELAY, 0 );
   }
   if ( _RQueue == (mqd_t)ERROR )
      //
      // Error ...
      return;

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
   retries=0;
   while (    mq_send( _RQueue, &mp, sizeof( MessagePacket ), 0 ) == ERROR 
           && retries++ < MAX_NUM_RETRIES ) 
      nanosleep( &Dispatcher::RETRY_DELAY, 0 );
   if ( retries == MAX_NUM_RETRIES )
      //
      // Error ...
      return;

}

void Dispatcher :: send( const MessagePacket &mp )
{
   //
   // Send message packet to router ...
   unsigned int retries=0;
   while (    mq_send( _RQueue, &mp, sizeof( MessagePacket ), 0 ) == ERROR 
           && retries++ < MAX_NUM_RETRIES )
      nanosleep( &Dispatcher::RETRY_DELAY, 0 );
   if ( retries == MAX_NUM_RETRIES )
      //
      // Error ...
      return;

}

unsigned int Dispatcher :: dispatchMessages()
{

   //
   // mq_receive loop ...
   int size;
   do
   {
      MessagePacket mp;
      unsigned int retries=0;
      while (    ( size = mq_receive( _MyQueue, &mp, sizeof( MessagePacket ), 0 ) ) != ERROR 
              && retries++ < MAX_NUM_RETRIES );
      if ( size != ERROR )
      {
         processMessage( mp );
      }
      else
      {
         if ( errno == EAGAIN )
            continue;
         else
            //
            // Error ...
            return;
      }

   } while ( _StopLoop == false );

   //
   // Check the task's queue to see if it is full or not ...
   mq_attr qattributes;
   mq_getattr( (*tqiter).second, &qattributes );

   return (unsigned int)qattributes.mq_curmsgs;
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

   outs << "Map size = " << _MessageMap.size() << " " << (int)(_MessageMap.begin() == _MessageMap.end()) << endl;
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
            ((MessageBase*)(*siter))->notify( mp );
   }
   else
      //
      // Error ...
      return;
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
   // Send register message to router ...
   unsigned int retries=0;
   while (    mq_send( _RQueue, &mp, sizeof( MessagePacket ), 0 ) == ERROR 
           && retries++ < MAX_NUM_RETRIES )
      nanosleep( &Dispatcher::RETRY_DELAY, 0 );
   if ( retries == MAX_NUM_RETRIES )
      //
      // Error ...
      return;

   //
   // Close the router's queue ...
   mq_close( _RQueue );
   _RQueue = (mqd_t)0;
}

void Dispatcher :: cleanup()
{
   //
   // Cleanup the map ...
   //  ( no dynamic memory was created, so doing
   //    a clear will clean the map fine )
   _MessageMap.clear();

}

