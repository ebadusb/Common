/*
 * Copyright (c) 2002 by Gambro BCT, Inc.  All rights reserved.
 *
 * TITLE:      msgdefs.h, Common message header.
 *
 * ABSTRACT:   This file defines the message interface for the
 *             Common dispatcher, and router.
 */

#ifndef _MSGDEFS_H_                     
#define _MSGDEFS_H_

#include <iostream.h>
#include <sys/types.h>
#include <string.h>
#include <time.h>

#include "msgcrc.h"


class MessageData
{
public:
   
   //
   //
   //  This size MUST be (divisible by 4) - 1 .  (e.g. only change this number in
   //  +/- increments of 4 bytes).  Choosing non-word-sized messages will cause
   //  the sizeof() operator to return the wrong value for this class, resulting
   //  in a crc mismatch of the message data.
   //
   const int MAX_MESSAGE_SIZE = 127;

   enum OperationType
   {
      TASK_REGISTER,                            // register task with router
      TASK_DEREGISTER,                          // deregister task with router
      MESSAGE_NAME_REGISTER,                    // register message name with router
      MESSAGE_REGISTER,                         // register message with router
      MESSAGE_DEREGISTER,                       // deregister message with router
      GATEWAY_CONNECT,                          // register a gateway task
      GATEWAY_DISCONNECT,                       // deregister a gateway task
      SPOOFER_REGISTER,                         // register the spoofer task
      SPOOFER_DEREGISTER,                       // deregister the spoofer task

      DISTRIBUTE_GLOBALLY,                      // distribute the message to everyone
                                                //  who registered 
      DISTRIBUTE_LOCALLY,                       // distribute the message to everyone
                                                //  who registered on the local node
      TIME_UPDATE,                              // time update from the ISR

      SPOOFED_GLOBALLY,                         // spoofer has modified message for everyone
      SPOOFED_LOCALLY,                          // spoofer has modified message for everyone 
                                                //  on the local node
   };

public:

   MessageData() { memset( this, 0, sizeof( MessageData ) ); }
   MessageData( const MessageData &d ) { operator=( d ); }
   ~MessageData() {}

   MessageData &operator=( const MessageData &d )
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
         msg(          d.msg(), MAX_MESSAGE_SIZE );
      }
      return *this;
   }

   bool operator==( const MessageData &d ) const
   {
      if (    msgId()        == d.msgId() 
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

   //
   // Set/Get for the OS Code instruction
   void osCode( const OperationType v ) { _OSCode = v; } 
   const OperationType osCode() const { return _OSCode; } 

   //
   // Set/Get for the message Id
   void msgId( const unsigned long v ) { _MsgId = v; } 
   const unsigned long msgId() const { return _MsgId; }
   //
   // Set/Get for the message Length
   void msgLength( const unsigned short v ) { _Length = v; } 
   const unsigned short msgLength() const { return _Length; }
   //
   // Set/Get for the node Id
   void nodeId( const unsigned long v ) { _NodeId = v; } 
   const unsigned long nodeId() const { return _NodeId; } 

   //
   // Set/Get for the task Id
   void taskId( const unsigned long v ) { _TaskId = v; } 
   const unsigned long taskId() const { return _TaskId; } 

   //
   // Set/Get for the sent time of the message packet
   void sendTime( const struct timespec v ) { _SendTime = v; } 
   const struct timespec sendTime() const { return _SendTime; } 

   //
   // Set/Get for this packet sequence of the total number
   void seqNum( const unsigned short v ) { _SeqNum = v; } 
   const unsigned short seqNum() const { return _SeqNum; } 

   //
   // Set/Get for the total number of message packets 
   void totalNum( const unsigned short v ) { _TotNum = v; } 
   const unsigned short totalNum() const { return _TotNum; } 
   
   //
   // Set/Get for the packet length
   void packetLength( const unsigned short v ) { _PacketLength = v; } 
   const unsigned short packetLength() const { return _PacketLength; } 

   //
   // Set/Get for the message data ...
   //
   void msg( const unsigned char * v, const int length ) 
   {
      memset( _Msg, 0, MAX_MESSAGE_SIZE + 1 ); 
      if ( length > 0 && v != 0 )
         memmove( (void*) _Msg , (void*) v , ( length > MAX_MESSAGE_SIZE ? MAX_MESSAGE_SIZE : length ) ); 
   }
   const unsigned char *msg() const { return _Msg;} 

   //
   // Dump the contents of the class ...
   void dump( ostream &outs )
   {
      outs << "######################### Message Data ############################" << endl;
      outs << "OSCode: " << _OSCode << " MsgId: " << hex << _MsgId << dec << " ";
      outs << "Length: " << _Length << " Node: " << hex << _NodeId << " ";
      outs << "Tid: " << hex << _TaskId << " Time: " << dec << _SendTime.tv_sec << " " << _SendTime.tv_nsec << " ";
      outs << "Seq: " << _SeqNum << " Tot: " << _TotNum << " ";
      outs << "PcktLngth: " << _PacketLength << " Msg: " << _Msg << endl;
      outs << "Msg: "; for (int i=0;i<MAX_MESSAGE_SIZE+1;i++) outs << hex << (int)((unsigned char)(*(_Msg+i))) << " "; outs << endl;
      outs << "###################################################################" << endl;
   }

protected:

   //  Order from "largest" to "smallest" to avoid OS fillin for boundaries.  The
   //  total header size MUST be a multiple of four bytes or the sizeof operator
   //  will fail for this class, resulting in a crc mismatch of the message data.
   //
   unsigned long   _MsgId;                        // hashed message id
   unsigned long   _NodeId;                       // node ID number
   unsigned long   _TaskId;                       // task PID number
   struct timespec _SendTime;                     // time message sent
   OperationType   _OSCode;                       // os message code
   unsigned short  _SeqNum;                       // For big messages, sequence of total parts of the message
   unsigned short  _TotNum;                       //   ... Total number of parts to the message
   unsigned short  _PacketLength;                 // total length of data in this packet
   unsigned short  _Length;                       // total message data length, bytes
   unsigned char   _Msg[MAX_MESSAGE_SIZE+1];      // Message data 

};


class MessagePacket
{
public:

   MessagePacket() : _MessageData(), _CRC( 0 ), _Unopened( false ) { }
   MessagePacket( const MessagePacket &mp ) { operator=( mp ); }
   ~MessagePacket() { }

   MessagePacket &operator=( const MessagePacket &d )
   {
      if ( &d != this )
      {
         _MessageData = d._MessageData;
         _CRC = d._CRC;
         _Unopened = d._Unopened;
      }
      return *this;
   }

   bool operator==( const MessagePacket &d ) const
   {
      return ( _MessageData == d._MessageData );
   }

   //
   // Get/Set the message data ...
   MessageData &msgData() { return _MessageData; }
   const MessageData &msgData() const { return _MessageData; }
   void msgData( const MessageData &md ) { _MessageData = md; }

   //
   // Generate the crc for the message
   void updateCRC()
   {
      _CRC = msgcrc32( (unsigned char *) &_MessageData, sizeof( MessageData) );
   }

   //
   // Validate the crc for the message
   bool validCRC() const
   {
      if ( _CRC != msgcrc32( (unsigned char *) &_MessageData, sizeof( MessageData) ) )
      {
         //
         // The crc's do not match, return an error ...
         return false;
      }
      return true;
   }

   //
   // Set/Get for the CRC
   void crc( const unsigned long v ) { _CRC = v; } 
   const unsigned long crc() const { return _CRC; } 

   //
   // Set/Get for the unopened flag 
   //   ( meaning the packet hasn't been read since its been received )
   void unopened( const bool v ) { _Unopened = v; } 
   const bool unopened() const { return _Unopened; } 

   //
   // Dump the contents of the class ...
   void dump( ostream &outs )
   {
      outs << "^^^^^^^^^^^^^^^^^^^^^^^^ Message Packet ^^^^^^^^^^^^^^^^^^^^^^^^^^^" << endl;
      _MessageData.dump( outs );
      outs << "CRC: " << hex << _CRC << dec << " Unopened: " << _Unopened << endl;
      outs << "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^" << endl;
   }

protected:

   MessageData   _MessageData;   // Actual message information
   unsigned long _CRC;           // 32 bit CRC of message information
   bool          _Unopened;      // flag to signify the packet is newly received

};

#endif
