/*
 * Copyright (c) 2002 by Gambro BCT, Inc.  All rights reserved.
 *
 * TITLE:      messagebase.h, Common message header.
 *
 * ABSTRACT:   This file defines the message interface for the
 *              base class for messages
 */

#include <vxWorks.h>

#include "auxclock.h"
#include "datalog.h"
#include "datalog_levels.h"
#include "error.h"
#include "messagebase.h"
#include "messagesystem.h"
#include "messagesystemconstant.h"

MessageBase::MessageBase( ) :
   _VirtualNotify(),
   _DistributionType( SNDRCV_RECEIVE_ONLY ),
   _RegisteredFlag( false ),
   _MsgId( 0 ),
   _NodeId( 0 ),
   _TaskId( 0 ),
   _MessageName(),
   _PacketList()
{
}

MessageBase::~MessageBase()
{
   if ( _RegisteredFlag == true )
      deregisterMsg();
   cleanup();
}

bool MessageBase::init( const SendType st )
{
   if ( MessageSystem::MsgSystem() )
   {
      postConstructInit();
      deregisterMsg();
   
      if ( st == SEND_LOCAL )
         _DistributionType = SNDRCV_LOCAL;
      else if ( st == SEND_GLOBAL )
         _DistributionType = SNDRCV_GLOBAL;
   
      cleanup();
      return init();
   }
   else
      _FATAL_ERROR( __FILE__, __LINE__, "Message system not initialized" );
   return false;
}

bool MessageBase::init( const CallbackBase &cb, const SendReceiveType st )
{
   if ( MessageSystem::MsgSystem() )
   {
      postConstructInit();
      registerMsg( cb );
   
      _DistributionType = st;
   
      cleanup();
      return init();
   }
   else
      _FATAL_ERROR( __FILE__, __LINE__, "Message system not initialized" );
   return false;
}

void MessageBase::registerMsg( const CallbackBase &cb )
{
   _VirtualNotify = cb;
   if ( _RegisteredFlag == false )
   {
      int len = _MessageName.length();

      //
      // Reset my data because I am sending the message ...
      _TaskId = taskIdSelf();
      _NodeId = 0;

      //
      // Register this message with the dispatcher...
      MessagePacket regPckt;
      regPckt.msgData().osCode( MessageData::MESSAGE_REGISTER );
      regPckt.msgData().msgId( _MsgId );
      regPckt.msgData().msgLength( len );
      regPckt.msgData().nodeId( _NodeId );
      regPckt.msgData().taskId( _TaskId );
      regPckt.msgData().seqNum( 1 );
      regPckt.msgData().totalNum( 1 );
      regPckt.msgData().packetLength( len );
      regPckt.msgData().msg( (const unsigned char *)messageName().c_str(), len );
      regPckt.updateTime();
      regPckt.updateCRC();

      //
      // Send the message ...
      if ( MessageSystem::MsgSystem() )
      {
         MessageSystem::MsgSystem()->dispatcher().registerMessage( *this, regPckt );
         _RegisteredFlag = true;
      }
      else
         _FATAL_ERROR( __FILE__, __LINE__, "Message system not initialized" );
   }
}

void MessageBase::deregisterMsg( )
{
   _VirtualNotify = CallbackBase();

   int len = _MessageName.length();

   //
   // Reset my data because I am sending the message ...
   _TaskId = taskIdSelf();
   _NodeId = 0;

   //
   // Deregister this message with the dispatcher...
   MessagePacket regPckt;
   regPckt.msgData().osCode( MessageData::MESSAGE_NAME_REGISTER );
   regPckt.msgData().msgId( _MsgId );
   regPckt.msgData().msgLength( len );
   regPckt.msgData().nodeId( 0 );
   regPckt.msgData().taskId( _TaskId );
   regPckt.msgData().seqNum( 1 );
   regPckt.msgData().totalNum( 1 );
   regPckt.msgData().packetLength( len );
   regPckt.msgData().msg( (const unsigned char *)messageName().c_str(), len );

   if ( _RegisteredFlag == true )
      regPckt.msgData().osCode( MessageData::MESSAGE_DEREGISTER );

   regPckt.updateTime();
   regPckt.updateCRC();

   //
   // Send the message ...
   if ( MessageSystem::MsgSystem() )
   {
      MessageSystem::MsgSystem()->dispatcher().deregisterMessage( *this, regPckt );
      _RegisteredFlag = false;
   }
   else
      _FATAL_ERROR( __FILE__, __LINE__, "Message system not initialized" );

}

void MessageBase::send()
{  
   if ( _DistributionType == SNDRCV_RECEIVE_ONLY )
   {
      postConstructInit();
      DataLog_Critical criticalLog;
      DataLog(criticalLog) << "Message Id " << hex << _MsgId << " ( " << _MessageName.c_str() << " ) not initialized for sending" << endmsg;
      _FATAL_ERROR( __FILE__, __LINE__, "message usage error" );
      return;
   }

   if ( MessageSystem::MsgSystem() )
   {
      //
      // reset my task and node Ids to my own because I am the last sender ...
      _TaskId = taskIdSelf();
      _NodeId = 0;

      list< MessagePacket* >::iterator pckt;
      for ( pckt  = _PacketList.begin();
            pckt != _PacketList.end() ;
            pckt++ ) 
      {
         //
         // Set the sent time, task, and node in the message packets ...
         (*pckt)->updateTime();
         (*pckt)->msgData().taskId( _TaskId );
         (*pckt)->msgData().nodeId( _NodeId );
         (*pckt)->updateCRC();
   
         //
         // Request the dispatcher to distribute this message packet...
         //
         MessageSystem::MsgSystem()->dispatcher().send( *(*pckt) );
      }
   }
   else
      _FATAL_ERROR( __FILE__, __LINE__, "Message system not initialized" );
}

unsigned long MessageBase::msgId() const
{
   return _MsgId;
}

unsigned long MessageBase::originNode() const
{
   return _NodeId;
}

unsigned long MessageBase::originTask() const
{
   return _TaskId;
}

struct timespec MessageBase::sentTime() const
{
   return _SentTime;
}

long MessageBase::latency() const
{
   long msecs;
   //
   // Get the time when the tick counter started ...
   struct rawTime  currentTickCount;
   struct timespec diffTime;
   struct timespec startTime;

   //
   // Get the current tick count and the initialization time ...
   auxClockTimeGet( &currentTickCount );
   auxClockInitTimeGet( &startTime );

   //
   // Calculate the current time ...
   //
   diffTime.tv_sec  = startTime.tv_sec;
   diffTime.tv_nsec = startTime.tv_nsec;
   //
   //   add on the current tick count ...
   diffTime.tv_sec  += currentTickCount.sec;
   diffTime.tv_nsec += currentTickCount.nanosec;
   if ( diffTime.tv_nsec >= 1000000000 )
   {
      diffTime.tv_nsec -= 1000000000;
      diffTime.tv_sec++;
   }

   //
   // Calculate the difference ...
   diffTime.tv_sec -= _SentTime.tv_sec; 
   diffTime.tv_nsec -= _SentTime.tv_nsec;
   if ( diffTime.tv_nsec < 0 )
   {
      diffTime.tv_nsec += 1000000000;
      diffTime.tv_sec--;
   }
   msecs = diffTime.tv_sec*1000;
   msecs += diffTime.tv_nsec/1000000; 
   
   return msecs;
}

void MessageBase::dump( DataLog_Stream &outs ) 
{
   outs << "========================== Message Base ===========================" << endmsg;
   outs << "Name: " << _MessageName << " ";
   outs << "DisType: " << _DistributionType << " RegFlag: " << (bool)_RegisteredFlag 
        << " MsgId: " << hex << _MsgId << " NodeId: " << hex << _NodeId  << " Tid: " << hex << _TaskId 
        << " Time: " << dec << _SentTime.tv_sec << " " << _SentTime.tv_nsec << " ";

   outs << endmsg;
   int i = 0;
   list< MessagePacket* >::iterator pckt;
   for ( pckt  = _PacketList.begin() ;
         pckt != _PacketList.end() ;
         pckt++ ) 
   {
      outs << "pckt#" << i++ << endmsg;
      (*pckt)->dump( outs );
   }
   outs << "===================================================================" << endmsg;
}

void MessageBase::postConstructInit()
{
   genMsgName();
   genMsgId();
   _TaskId = taskIdSelf();
}

bool MessageBase::init( )
{
   //
   // Break up the message data into the message packets ...
   //
   unsigned long dataSize = sizeOfData();
   unsigned long leftOver = dataSize % MessageSystemConstant::MAX_MESSAGE_SIZE;
   unsigned long numPackets = ( dataSize / MessageSystemConstant::MAX_MESSAGE_SIZE ) + ( leftOver > 0 ? 1 : 0 );

   //
   // Set my data to the initial state 
   _TaskId = taskIdSelf();
   _NodeId = 0;

   for ( unsigned long i=0 ; i<numPackets ; i++ )
   {
      //
      // Only one packet needed ...
      //
      MessagePacket *mp = new MessagePacket();

      if ( _DistributionType == SNDRCV_LOCAL )
         mp->msgData().osCode( MessageData::DISTRIBUTE_LOCALLY );
      else if ( _DistributionType == SNDRCV_GLOBAL )
         mp->msgData().osCode( MessageData::DISTRIBUTE_GLOBALLY );
      mp->msgData().msgId( _MsgId );
      mp->msgData().msgLength( dataSize );
      mp->msgData().nodeId( _NodeId );
      mp->msgData().taskId( _TaskId );
      mp->msgData().seqNum( i+1 );
      mp->msgData().totalNum( numPackets );
      mp->msgData().packetLength( ( i+1<numPackets ? MessageSystemConstant::MAX_MESSAGE_SIZE : 
                                    ( leftOver > 0 ? leftOver : MessageSystemConstant::MAX_MESSAGE_SIZE ) ) );
      _PacketList.push_back( mp );
   }

   setMsgData();

   return true;
}

unsigned long MessageBase::genMsgId()
{
   unsigned long initcrc = 0;
   if ( crcgen32( &initcrc,  (unsigned char *)_MessageName.c_str(), _MessageName.length() ) == 0 )
      return ( _MsgId = initcrc );

   //
   // Default Id ...
   return 0;
}

bool MessageBase::notify( const MessagePacket &mp )
{
   return notify( mp, _VirtualNotify );
}

bool MessageBase::notify( const MessagePacket &mp, const CallbackBase &cb )
{
   //
   // Copy the message packet into the packet list ...
   if ( findAndCopy( mp ) == false )
   {
      //
      // Application error 
      return false;
   }

   bool allMsgsIn = true;
   list< MessagePacket* >::iterator pckt;
   for ( pckt  = _PacketList.begin();
         pckt != _PacketList.end() ;
         pckt++ ) 
   {
      if ( (*pckt)->unopened() == false ) // message packet has not came in yet 
      {
         allMsgsIn = false;
      }
      else
      {
          if ( !(    (*pckt)->msgData().taskId() == mp.msgData().taskId()     // or, the message packet
                  && (*pckt)->msgData().nodeId() == mp.msgData().nodeId() ) ) // wasn't from the same sender
          {
             allMsgsIn = false;
             DataLog(log_level_message_system_error) << "Message clash for Id=" << hex << (*pckt)->msgData().msgId()
                                    << ", sender1=" << (*pckt)->msgData().taskId() << " node1=" << mp.msgData().nodeId() 
                                    << " and sender2=" <<  mp.msgData().taskId() << " node2=" << mp.msgData().nodeId() << endmsg;
          }
      }
   }

   //
   // After all data has been received ... 
   if ( allMsgsIn == true )
   {
      //
      // Put the data into T ...
      retrieveMsgData();

      //
      // Put the header info into myself ...
      _NodeId = mp.msgData().nodeId();
      _TaskId = mp.msgData().taskId();
      _SentTime = mp.msgData().sendTime();

      //
      // and notify the application ...
      cb();
   }

   return true;
}

bool MessageBase :: findAndCopy( const MessagePacket &mp )
{
   list< MessagePacket* >::iterator pckt;
   for ( pckt  = _PacketList.begin();
         pckt != _PacketList.end() ;
         pckt++ ) 
   {
      if ( (**pckt) == mp )
      {
         MessageData::OperationType ot = (*pckt)->msgData().osCode(); 
         (**pckt) = mp;
         if ( (*pckt)->validCRC() == false )
         {
            //
            // Error ...
            unsigned long crc = (*pckt)->crc();
            (*pckt)->updateCRC();
            DataLog_Critical criticalLog;
            DataLog(criticalLog) << "Message CRC validation failed for MsgId=" << hex << (*pckt)->msgData().msgId() 
                                 << ", CRC=" << crc << " and should be " <<  (*pckt)->crc() << endmsg;
            _FATAL_ERROR( __FILE__, __LINE__, "CRC check failed" );
            (*pckt)->dump( DataLog( log_level_message_system_error ) );
            return false;
         }

         //
         // Preserve the distribution type ...
         (*pckt)->msgData().osCode( ot );

         //
         // Set the message packet to unopened so that
         //  we can determine the data has changed...
         (*pckt)->unopened( true );
         break;
      }
   }
   return true;
}

void MessageBase::cleanup()
{
   list< MessagePacket* >::iterator pckt;
   for ( pckt  = _PacketList.begin();
         pckt != _PacketList.end() ;
         pckt++ ) 
   {
      delete (*pckt);
   }
   _PacketList.clear();
}

