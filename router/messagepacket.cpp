/*
 * Copyright (c) 2002 by Gambro BCT, Inc.  All rights reserved.
 *
 * TITLE:      msgdefs.h, Common message header.
 *
 * ABSTRACT:   This file defines the message interface for the
 *             Common dispatcher, and router.
 */

#include "messagepacket.h"

#include "auxclock.h"
#include "datalog.h"
#include "messagesystemconstant.h"
#include "msgcrc.h"


MessageData::MessageData()
{
   memset( this, 0, sizeof( MessageData ) );
}

MessageData::MessageData( const MessageData &d )
{
   operator=( d );
}

MessageData::~MessageData()
{
}

MessageData &MessageData::operator=( const MessageData &d )
{
   if ( &d != this )
   {
      osCode(       d.osCode() );
      msgId(        d.msgId() );
      msgLength(    d.msgLength() );
      nodeId(       d.nodeId() );
      taskId(       d.taskId() );
      sendTime(     d.sendTime() );
      seqNum(       d.seqNum() );
      totalNum(     d.totalNum() );
      packetLength( d.packetLength() );
      msg(          d.msg(), d.packetLength() );
   }
   return *this;
}

bool MessageData::operator==( const MessageData &d ) const
{
   if ( msgId()        == d.msgId() 
        && msgLength()    == d.msgLength() 
        && seqNum()       == d.seqNum()
        && totalNum()     == d.totalNum()
        && packetLength() == d.packetLength()
      )
   {
      return true;
   }
   return false;
}

void MessageData::msg( const unsigned char * v, const int length ) 
{
   memset( _Msg, 0, MessageSystemConstant::MAX_MESSAGE_SIZE + 1 ); 
   if ( length > 0 && v != 0 )
      memmove( (void*) _Msg , (void*) v , 
               ( length > MessageSystemConstant::MAX_MESSAGE_SIZE ? 
                 MessageSystemConstant::MAX_MESSAGE_SIZE : 
                 length ) 
             );
   _PacketLength = length;
}
const unsigned char *MessageData::msg() const
{
   return _Msg;
} 

unsigned short MessageData::sizeOfData() const
{
   unsigned short size = sizeof(MessageData);

   size -= MessageSystemConstant::MAX_MESSAGE_SIZE+1;
   size += _PacketLength;

   return size;
}

void MessageData::dump( ostream &outs )
{
   outs << "OSCode: " << _OSCode << " MsgId: " << hex << _MsgId << dec << " ";
   outs << "Length: " << _Length << " Node: " << hex << _NodeId << " ";
   outs << "Tid: " << hex << _TaskId << " Time: " << dec << _SendTime.tv_sec << " " << _SendTime.tv_nsec << " ";
   outs << "Seq: " << _SeqNum << " Tot: " << _TotNum << " ";
   outs << "PcktLngth: " << _PacketLength << endmsg;
   outs << " MsgId: " << hex << _MsgId << dec << " (cont.) Msg: " << _Msg << endmsg;
   outs << " MsgId: " << hex << _MsgId << dec << " (cont.) Msg: "; 
   for ( int i=0;i<MessageSystemConstant::MAX_MESSAGE_SIZE+1;i++ )
      outs << hex << (int)((unsigned char)(*(_Msg+i))) << " "; outs << endmsg;
}

MessagePacket::MessagePacket() : _MessageData(), _CRC( 0 ), _Unopened( false )
{
}

MessagePacket::MessagePacket( const MessagePacket &mp )
{
   operator=( mp );
}

MessagePacket::~MessagePacket()
{
}

MessagePacket &MessagePacket::operator=( const MessagePacket &d )
{
   if ( &d != this )
   {
      _MessageData = d._MessageData;
      _CRC = d._CRC;
      _Unopened = d._Unopened;
   }
   return *this;
}

bool MessagePacket::operator==( const MessagePacket &d ) const
{
   return( _MessageData == d._MessageData );
}

void MessagePacket::updateTime()
{
   //
   // Get the time when the tick counter started ...
   struct rawTime  currentTickCount;
   struct timespec currentTime;
   struct timespec startTime;

   //
   // Get the current tick count and the initialization time ...
   auxClockTimeGet( &currentTickCount );
   auxClockInitTimeGet( &startTime );

   //
   // Calculate the current time ...
   //
   currentTime.tv_sec  = startTime.tv_sec;
   currentTime.tv_nsec = startTime.tv_nsec;
   //
   //   add on the current tick count ...
   currentTime.tv_sec  += currentTickCount.sec;
   currentTime.tv_nsec += currentTickCount.nanosec;
   if ( currentTime.tv_nsec >= 1000000000 )
   {
      currentTime.tv_nsec -= 1000000000;
      currentTime.tv_sec++;
   }

   _MessageData.sendTime( currentTime );
}

void MessagePacket::updateCRC()
{
   _CRC = msgcrc32( (unsigned char *) &_MessageData, _MessageData.sizeOfData() );
}

bool MessagePacket::validCRC() const
{
   if ( _CRC != msgcrc32( (unsigned char *) &_MessageData, _MessageData.sizeOfData() ) )
   {
      //
      // The crc's do not match, return an error ...
      return false;
   }
   return true;
}

void MessagePacket::dump( ostream &outs )
{
   _MessageData.dump( outs );
   outs << "MessagePacket:: CRC: " << hex << _CRC << dec << " Unopened: " << _Unopened << endmsg;
}

