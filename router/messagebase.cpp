/*
 * Copyright (c) 2002 by Gambro BCT, Inc.  All rights reserved.
 *
 * TITLE:      messagebase.h, Common message header.
 *
 * ABSTRACT:   This file defines the message interface for the
 *              base class for messages
 */

#include <vxWorks.h>

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
      struct timespec ts;
      clock_gettime( CLOCK_REALTIME, &ts );

      int len = _MessageName.length();
      //
      // Register this message with the dispatcher...
      MessagePacket regPckt;
      regPckt.msgData().osCode( MessageData::MESSAGE_REGISTER );
      regPckt.msgData().msgId( _MsgId );
      regPckt.msgData().msgLength( len );
      regPckt.msgData().nodeId( _NodeId );
      regPckt.msgData().taskId( _TaskId );
      regPckt.msgData().sendTime( ts );
      regPckt.msgData().seqNum( 1 );
      regPckt.msgData().totalNum( 1 );
      regPckt.msgData().packetLength( len );
      regPckt.msgData().msg( (const unsigned char *)messageName().data(), len );
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

   struct timespec ts;
   clock_gettime( CLOCK_REALTIME, &ts );

   int len = _MessageName.length();

   //
   // Deregister this message with the dispatcher...
   MessagePacket regPckt;
   regPckt.msgData().osCode( MessageData::MESSAGE_NAME_REGISTER );
   regPckt.msgData().msgId( _MsgId );
   regPckt.msgData().msgLength( len );
   regPckt.msgData().nodeId( 0 );
   regPckt.msgData().taskId( _TaskId );
   regPckt.msgData().sendTime( ts );
   regPckt.msgData().seqNum( 1 );
   regPckt.msgData().totalNum( 1 );
   regPckt.msgData().packetLength( len );
   regPckt.msgData().msg( (const unsigned char *)messageName().data(), len );

   if ( _RegisteredFlag == true )
      regPckt.msgData().osCode( MessageData::MESSAGE_DEREGISTER );

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
      char buffer[80];
      sprintf( buffer, "Message Id %lx ( %s ) not initialized for sending", _MsgId, _MessageName.data() );
      _FATAL_ERROR( __FILE__, __LINE__, buffer );
      return;
   }

   if ( MessageSystem::MsgSystem() )
   {
      //
      // Set the sent time in the message packets ...
      clock_gettime( CLOCK_REALTIME, &_SentTime );
      list< MessagePacket* >::iterator pckt;
      for ( pckt  = _PacketList.begin();
            pckt != _PacketList.end() ;
            pckt++ ) 
      {
         (*pckt)->msgData().sendTime( _SentTime );
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

void MessageBase::dump( ostream &outs ) 
{
   outs << "========================== Message Base ===========================" << endl;
   outs << "Name: " << _MessageName << " ";
   outs << "DisType: " << _DistributionType << " RegFlag: " << (bool)_RegisteredFlag 
        << " MsgId: " << hex << _MsgId << " NodeId: " << hex << _NodeId  << " Tid: " << hex << _TaskId 
        << " Time: " << dec << _SentTime.tv_sec << " " << _SentTime.tv_nsec << " ";

   outs << endl;
   int i = 0;
   list< MessagePacket* >::iterator pckt;
   for ( pckt  = _PacketList.begin() ;
         pckt != _PacketList.end() ;
         pckt++ ) 
   {
      outs << "pckt#" << i++ << endl;
      (*pckt)->dump( outs );
   }
   outs << "===================================================================" << endl;
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
   struct timespec ts = { 0,0 };
   unsigned long dataSize = sizeOfData();
   unsigned long leftOver = dataSize % MessageSystemConstant::MAX_MESSAGE_SIZE;
   unsigned long numPackets = ( dataSize / MessageSystemConstant::MAX_MESSAGE_SIZE ) + ( leftOver > 0 ? 1 : 0 );

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
      mp->msgData().sendTime( ts );
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
   if ( crcgen32( &initcrc,  (unsigned char *)messageName().data(), _MessageName.length() ) == 0 )
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
   list< MessagePacket* >::iterator pckt;
   for ( pckt  = _PacketList.begin();
         pckt != _PacketList.end() ;
         pckt++ ) 
   {
      if ( (**pckt) == mp )
      {
         (**pckt) = mp;
         (*pckt)->unopened( true );
      }
   }

   bool allMsgsIn = true;
   for ( pckt  = _PacketList.begin();
         pckt != _PacketList.end() ;
         pckt++ ) 
   {
      if (    (*pckt)->unopened() == false  // message packet has not came in yet 
           || !(    (*pckt)->msgData().taskId() == mp.msgData().taskId()     // or, the message packet
                 && (*pckt)->msgData().nodeId() == mp.msgData().nodeId() ) ) // wasn't from the same sender
         allMsgsIn = false;
   }

   //
   // After all data has been received ... 
   if ( allMsgsIn == true )
   {
      //
      // Put the data into T ...
      if ( retrieveMsgData() == false )
      {
         //
         // Application error ...
         return false;
      }
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

