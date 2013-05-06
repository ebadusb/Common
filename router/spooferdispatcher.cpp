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
#include "datalog_levels.h"
#include "spooferdispatcher.h"
#include "messagesystem.h"
#include "auxclock.h"


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
   DataLog( log_level_message_spoof_info ) << "Spoofing message " << hex << mb.msgId() << dec
                                           << " (" << mb.messageName() << ")" << endmsg;

   //
   // Change the message packet list for the MessageBase object ...
   list< MessagePacket* >::iterator pckt;
   for ( pckt  = mb._PacketList.begin();
         pckt != mb._PacketList.end() ;
         ++pckt ) 
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
   int len = mb.messageName().length();
   MessagePacket spoofmp;
   spoofmp.msgData().osCode( MessageData::SPOOF_MSG_REGISTER );
   spoofmp.msgData().msgId( mb.msgId() );
   spoofmp.msgData().msgLength( len );
   spoofmp.msgData().nodeId( 0 );
   spoofmp.msgData().taskId( mb.originTask() );
   spoofmp.msgData().seqNum( 1 );
   spoofmp.msgData().totalNum( 1 );
   spoofmp.msgData().packetLength( len );
   spoofmp.msgData().msg( (const unsigned char *)mb.messageName().c_str(), len );
   spoofmp.updateTime();
   spoofmp.updateCRC();
   send( spoofmp );
}

void SpooferDispatcher :: despoofMessage( MessageBase &mb )
{
   DataLog( log_level_message_spoof_info ) << "Despoofing message " << hex << mb.msgId() << dec
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
         ++pckt ) 
   {
      if ( (*pckt)->msgData().osCode() == MessageData::SPOOFED_GLOBALLY )
         (*pckt)->msgData().osCode( MessageData::DISTRIBUTE_GLOBALLY );
      else if ( (*pckt)->msgData().osCode() == MessageData::SPOOFED_LOCALLY )
         (*pckt)->msgData().osCode( MessageData::DISTRIBUTE_LOCALLY );
   }

   //
   // Deregister this message with the router task ...
   int len = mb.messageName().length();
   MessagePacket spoofmp;
   spoofmp.msgData().osCode( MessageData::SPOOF_MSG_DEREGISTER );
   spoofmp.msgData().msgId( mb.msgId() );
   spoofmp.msgData().msgLength( len );
   spoofmp.msgData().nodeId( 0 );
   spoofmp.msgData().taskId( mb.originTask() );
   spoofmp.msgData().seqNum( 1 );
   spoofmp.msgData().totalNum( 1 );
   spoofmp.msgData().packetLength( len );
   spoofmp.msgData().msg( (const unsigned char *)mb.messageName().c_str(), len );
   spoofmp.updateTime();
   spoofmp.updateCRC();
   send( spoofmp );
}

void SpooferDispatcher :: sendCorruptMessage( MessageBase &mb )
{
    //
   // Change the message packet list for the MessageBase object ...
    //
   DataLog( log_level_message_spoof_info ) << "sendCorruptMessage:crc error spoofer " << endmsg;
   list< MessagePacket* >::iterator pckt;
   for ( pckt  = mb._PacketList.begin();
         pckt != mb._PacketList.end() ;
         ++pckt ) 
   {                  
      (*pckt)->msgData().msgLength((*pckt)->msgData().msgLength() - 1);
      
      MessageSystem::MsgSystem()->dispatcher().send( *(*pckt) );

      DataLog( log_level_message_spoof_info ) << "sendCorruptMessage:msg send " << endmsg;
   }
}

void SpooferDispatcher::sendLatentMessage( MessageBase& mb,
                                           unsigned long msecsDelay,
                                           bool useSentTime )
{
   const long E3( 1000 );       // 1x10^3
   const long E6( 1000000 );    // 1x10^6
   const long E9( 1000000000 ); // 1x10^9

   struct timespec fakeTime = {0};
   if ( useSentTime )
   {
      fakeTime = mb.sentTime();
   }
   else // Base the latency on current clock time.
   {
      // Get the current tick count and the initialization time
      rawTime  currentTickCount;
      auxClockTimeGet( &currentTickCount );
      auxClockInitTimeGet( &fakeTime );

      // Calculate the current time
      fakeTime.tv_sec  += currentTickCount.sec;
      fakeTime.tv_nsec += currentTickCount.nanosec;
      if ( fakeTime.tv_nsec >= E9 )
      {
         fakeTime.tv_nsec -= E9;
         ++fakeTime.tv_sec;
      }
   }

   // fake the latency by subtracting the delay from the original sent time
   fakeTime.tv_sec  -= (msecsDelay / E3);
   fakeTime.tv_nsec -= (msecsDelay % E3) * E6;
   if ( fakeTime.tv_nsec < 0 )
   {
      fakeTime.tv_nsec += E9;
      --fakeTime.tv_sec;
   }

#if 0
   // Check our work for debug using latency()
   struct timespec origTime = mb._SentTime;
   mb._SentTime = fakeTime;
   DataLog( log_level_message_spoof_info ) << __FUNCTION__ << "(): spoofing latency="
         << mb.latency() << " (delay=" << msecsDelay << " msecs) on node=" << mb.originNode()
         << " mid=0x" << hex << mb.msgId() << dec << " : " << mb.messageName()
         << endmsg;
   mb._SentTime = origTime;
#endif

   //
   // Change the message packet list send times and send them along
   //
   list< MessagePacket* >::iterator pckt;
   for ( pckt  = mb._PacketList.begin();
         pckt != mb._PacketList.end() ;
         ++pckt )
   {
      (*pckt)->msgData().sendTime( fakeTime );
      (*pckt)->updateCRC();
      MessageSystem::MsgSystem()->dispatcher().send( *(*pckt) );
   }
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
            ++spiter )
      {
         //
         // if the message id of the message packet contains an
         //  id that we have in our spoofed messages list ...
         if ( ((*spiter).first)->msgId() == mp.msgData().msgId() )
         {
            // 
            // Call the appropriate callback function ...
            MessageBase::notify( (*(MessageBase*)((*spiter).first)), mp, ((*spiter).second) );
   
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

