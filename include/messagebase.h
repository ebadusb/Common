/*
 * Copyright (c) 2002 by Gambro BCT, Inc.  All rights reserved.
 *
 * TITLE:      messagebase.h, Common message header.
 *
 * ABSTRACT:   This file defines the interface for the base class for messages
 */

#ifndef _MESSAGE_BASE_H_
#define _MESSAGE_BASE_H_

#include <list>
#include <string>

#include "callback.h"
#include "crcgen.h"
#include "messagepacket.h"

class MessageBase 
{
public:
   friend class Dispatcher;
   friend class SpooferDispatcher;
   friend class TaosSpooferDispatcher;
public:

   //
   // Enumeration for sending messages
   //
   enum SendType
   {
      SEND_LOCAL, // Send to processes on the local node
      SEND_GLOBAL // Send to processes on all nodes
   };
   //
   // Enumeration for sending and/or receiving messages
   //
   enum SendReceiveType
   {
      SNDRCV_LOCAL=SEND_LOCAL,   // Send to processes on the local node
      SNDRCV_GLOBAL=SEND_GLOBAL, // Send to processes on all nodes
      SNDRCV_RECEIVE_ONLY,       // No sending this message, only receiving
      SNDRCV_SPOOF_MSG          // Sending and receiving of spoofed messages
   };

public:

   //
   // Default constructor
   MessageBase( );      

   //
   // Default destructor
   virtual ~MessageBase();

   //
   // Initialize the message for sending only
   bool init( const SendType dt );
   //
   // Initialize the message for sending and receiving
   bool init( const CallbackBase &cb, const SendReceiveType dt=SNDRCV_RECEIVE_ONLY );

   //
   // Register the message for reception 
   void registerMsg( const CallbackBase &cb );
   //
   // Deregister the message to stop reception
   void deregisterMsg();

   //
   // send the message
   void send( );

   // Get the message's originating node
   unsigned long msgId( ) const;
   // Get the message's originating node
   unsigned long originNode( ) const;
   // Get the message's originating task
   unsigned long originTask( ) const;
   // Get the message's last sent time
   struct timespec sentTime( ) const;
   //
   // Return the latency between sentTime and currentTime in milliseconds
   long latency() const;

   //
   // Dump the contents of the class ...
   void dump( DataLog_Stream &outs );

protected:

   //
   // Get the size of the data for this message ...
   virtual unsigned long sizeOfData() const = 0;

   //
   // Generate the message name ...
   //  (used for creating message Ids )
   virtual const char *genMsgName() = 0;

   //
   // Get the message name from the member data
   const string &messageName() const { return _MessageName; }

   //
   // Initialization that needs to occur after construction
   void postConstructInit();

   //
   // Initialize the message 
   bool init();

   //
   // Put the data into the message packets ...
   //  This function breaks up the message data into
   //  message packets. The type of message data is not
   //  known in this abstract class, so derived classes
   //  must furnish the implementation.
   virtual void setMsgData() = 0;

   //
   // Put the packet's data into the data format ...
   //  This function puts the message packet data back
   //  into the message data format.  The type of the 
   //  message data is not known, so derived classes must
   //  supply the functionality.
   virtual void retrieveMsgData() = 0;

   //
   // Function to produce a unique message Id
   unsigned long genMsgId();

   //
   // Function called by the dispatcher to notify this message
   //  that it has been received.  If the message data was
   //  corrupt, the function returns 'false'
   static bool notify( MessageBase &me, const MessagePacket &mp );
   static bool notify( MessageBase &me, const MessagePacket &mp, const CallbackBase &cb );

   //
   // This function finds the given message packet in the
   //  message packet list and copies it, preserving the distribution
   //  type.
   bool findAndCopy( const MessagePacket &mp );

   //
   // Clean-up any dynamic memory
   void cleanup();

private:
   MessageBase( MessageBase const &);           // not implemented
   MessageBase& operator= (MessageBase const&); // not implemented

protected:

   // This will call a user specified member function of any
   //  type of class.
   CallbackBase     _VirtualNotify;

   // Flag to signify we only want this message to be distributed 
   //  on the local node
   SendReceiveType  _DistributionType;

   // Flag to hold whether or not this message has registered 
   //  with the dispatcher
   char             _RegisteredFlag;

   unsigned long    _MsgId;           // message Id
   unsigned long    _NodeId;          // sender's node id
   unsigned long    _TaskId;          // sender's task id
   struct timespec  _SentTime;        // last sent time

   string           _MessageName;     // Name of the message used for Id generation

   list< MessagePacket* > _PacketList; // List of packets that make up the data 
                                       //  for this message
  
};

#endif
