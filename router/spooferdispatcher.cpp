/*
 * Copyright (c) 2002 Gambro BCT, Inc.  All rights reserved.
 *
 * TITLE:      spooferdispatcher.cpp
 *
 */

#include <vxWorks.h>
#include <list>
#include <time.h>

#include "datalog.h"
#include "spooferdispatcher.h"


SpooferDispatcher :: SpooferDispatcher( ) : Dispatcher()
{
}

SpooferDispatcher :: ~SpooferDispatcher()
{
   cleanup();
}

void SpooferDispatcher :: deregisterMessage( const MessageBase &mb, MessagePacket &mp )
{
   despoofMessage( (MessageBase&)mb );
   Dispatcher::deregisterMessage( mb, mp );
}

void SpooferDispatcher :: spoofMessage( MessageBase &mb )
{
   CallbackBase cb;
   spoofMessage( mb, cb );
}

void SpooferDispatcher :: spoofMessage( MessageBase &mb, const CallbackBase &cb )
{
   DataLog_Level slog( "Spoofer" );
   slog( __FILE__, __LINE__ ) << "Spoofing message " << hex << mb.msgId() 
                              << " (" << mb.messageName() << ")" << endmsg;

   //
   // Change the message packet list for the MessageBase object ...
   list< MessagePacket* >::iterator pckt;
   for ( pckt  = mb._PacketList.begin();
         pckt != mb._PacketList.end() ;
         pckt++ ) 
   {
      if ( (*pckt)->msgData().osCode() == MessageData::DISTRIBUTE_GLOBALLY )
         (*pckt)->msgData().osCode( MessageData::SPOOFED_GLOBALLY );
      else if ( (*pckt)->msgData().osCode() == MessageData::DISTRIBUTE_LOCALLY )
         (*pckt)->msgData().osCode( MessageData::SPOOFED_LOCALLY );
   }

   //
   // Save the message pointer and the callback function in the
   //  spoofed messages list...
   _SpoofedMsgMap[ &mb ] = cb;

   //
   // Register this message as a spoofer message with the router task ...
   struct timespec ts;
   clock_gettime( CLOCK_REALTIME, &ts );
   int len = mb.messageName().length();
   MessagePacket spoofmp;
   spoofmp.msgData().osCode( MessageData::SPOOF_MSG_REGISTER );
   spoofmp.msgData().msgId( mb.msgId() );
   spoofmp.msgData().msgLength( len );
   spoofmp.msgData().nodeId( 0 );
   spoofmp.msgData().taskId( mb.originTask() );
   spoofmp.msgData().sendTime( ts );
   spoofmp.msgData().seqNum( 1 );
   spoofmp.msgData().totalNum( 1 );
   spoofmp.msgData().packetLength( len );
   spoofmp.msgData().msg( (const unsigned char *)mb.messageName().data(), len );
   spoofmp.updateCRC();
   send( spoofmp );
}

void SpooferDispatcher :: despoofMessage( MessageBase &mb )
{
   DataLog_Level slog( "Spoofer" );
   slog( __FILE__, __LINE__ ) << "Despoofing message " << hex << mb.msgId() 
                              << " (" << mb.messageName() << ")" << endmsg;

   //
   // Remove the message entry from the spoofed messages list ...
   map< const MessageBase*, CallbackBase >::iterator spiter;
   spiter = _SpoofedMsgMap.find( (const MessageBase*)&mb );
   if ( spiter != _SpoofedMsgMap.end() )
      _SpoofedMsgMap.erase( spiter );

   //
   // Change the message packet list back for the MessageBase object ...
   list< MessagePacket* >::iterator pckt;
   for ( pckt  = mb._PacketList.begin();
         pckt != mb._PacketList.end() ;
         pckt++ ) 
   {
      if ( (*pckt)->msgData().osCode() == MessageData::SPOOFED_GLOBALLY )
         (*pckt)->msgData().osCode( MessageData::DISTRIBUTE_GLOBALLY );
      else if ( (*pckt)->msgData().osCode() == MessageData::SPOOFED_LOCALLY )
         (*pckt)->msgData().osCode( MessageData::DISTRIBUTE_LOCALLY );
   }

   //
   // Deregister this message with the router task ...
   struct timespec ts;
   clock_gettime( CLOCK_REALTIME, &ts );
   int len = mb.messageName().length();
   MessagePacket spoofmp;
   spoofmp.msgData().osCode( MessageData::SPOOF_MSG_DEREGISTER );
   spoofmp.msgData().msgId( mb.msgId() );
   spoofmp.msgData().msgLength( len );
   spoofmp.msgData().nodeId( 0 );
   spoofmp.msgData().taskId( mb.originTask() );
   spoofmp.msgData().sendTime( ts );
   spoofmp.msgData().seqNum( 1 );
   spoofmp.msgData().totalNum( 1 );
   spoofmp.msgData().packetLength( len );
   spoofmp.msgData().msg( (const unsigned char *)mb.messageName().data(), len );
   spoofmp.updateCRC();
   send( spoofmp );
}

void SpooferDispatcher :: processMessage( MessagePacket &mp )
{
   bool weAreSpoofingThisOne=false;

   //
   // If we haven't already spoofed this message packet ...
   if (    mp.msgData().osCode() != MessageData::SPOOFED_GLOBALLY 
        && mp.msgData().osCode() != MessageData::SPOOFED_LOCALLY )
   {
      //
      // Search the spoofed message map for a message
      //  which matches this message packet id ...
      map< const MessageBase*, CallbackBase >::iterator spiter;
      for ( spiter = _SpoofedMsgMap.begin() ;
            spiter != _SpoofedMsgMap.end() ;
            spiter++ )
      {
         //
         // if the message id of the message packet contains an
         //  id that we have in our spoofed messages list ...
         if ( ((*spiter).first)->msgId() == mp.msgData().msgId() )
         {
            // 
            // Call the appropriate callback function ...
            ((MessageBase*)((*spiter).first))->notify( mp, ((*spiter).second) );
   
            weAreSpoofingThisOne = true;
         }
      }
   }

   //
   // else we will treat this message packet like any
   //  normal application messages ...
   if ( weAreSpoofingThisOne == false )
   {
      Dispatcher::processMessage( mp );
   }
}

void SpooferDispatcher :: cleanup()
{
   //
   // Clean up my map ...
   _SpoofedMsgMap.empty();
}

