/*
 * Copyright (c) 2002 by Gambro BCT, Inc.  All rights reserved.
 *
 * TITLE:      message.h, Common message header.
 *
 * ABSTRACT:   This file defines the interface for templated messages
 */

#ifndef _MESSAGE_H_
#define _MESSAGE_H_

#include <typeinfo>

#include "messagebase.h"

template < class T >
class Message : public MessageBase
{
public:

   //
   // Create message without initializing ...
   Message( );

   //
   // Create this message for sending only
   //   ( THESE CONSTRUCTORS SHOULD NOT BE CALLED FROM DERIVED CLASSES.  THE DERIVED
   //     CLASSES SHOULD CALL THE DEFAULT AND CALL THE 'init' FUNCTION FOR THEMSELVES )
   //
   //  This constructor will create a send-only message
   //   st - SEND_LOCAL, or SEND_GLOBAL
   Message( const SendType st ); 
   //
   // Create this message for sending and receiving
   //   ( THESE CONSTRUCTORS SHOULD NOT BE CALLED FROM DERIVED CLASSES.  THE DERIVED
   //     CLASSES SHOULD CALL THE DEFAULT AND CALL THE 'init' FUNCTION FOR THEMSELVES )
   //
   //  This constructor will create a send/receive message or a receive only message
   //   cb - Function to be called when the message is received
   //   st - SNDRCV_LOCAL, SNDRCV_GLOBAL, or SNDRCV_RECEIVE_ONLY
   Message( const CallbackBase &cb, const SendReceiveType srt=SNDRCV_RECEIVE_ONLY ); 

   //
   // Destructor
   virtual ~Message();

   //
   // Send the message out with this data
   void send( const T &buffer);

   //
   // Get/Set the message data
   const T &getData( ) const;
   // set the data
   void setData( const T &buffer);

protected:

   //
   // Get the size of the data for this message ...
   virtual unsigned long sizeOfData() const;

   //
   // Get the message name ...
   //  (used for creating message Ids )
   virtual const char *genMsgName();

   //
   // Put the 'T' data into the message packets ...
   virtual void setMsgData();

   //
   // Put the packet's data into the 'T' data format ...
   virtual bool retrieveMsgData();

private:
   Message( Message const &);           // not implemented
   Message& operator= (Message const&); // not implemented

protected:

   //
   // The message data 
   T                _MsgData;

};

template < class T >
Message<T>::Message() :
   MessageBase( )
{
}

template < class T >
Message<T>::Message( const SendType st ) :
   MessageBase( )
{
   init( st );
}

template < class T >
Message<T>::Message( const CallbackBase &cb, const SendReceiveType st ) :
   MessageBase( )
{
   init( cb, st );
}

template < class T >
Message<T>::~Message()
{
}

template < class T >
void Message<T>::send( const T &buffer )
{
   setData( buffer );
   MessageBase::send();
}

template < class T >
const T &Message<T>::getData() const
{
   return _MsgData;
}

template < class T >
void Message<T>::setData( const T &buffer)
{
   _MsgData = buffer; 

   setMsgData();
}

template < class T >
unsigned long Message<T>::sizeOfData() const
{
   return sizeof( T );
}

template < class T >
const char *Message<T>::genMsgName() 
{
   _MessageName = typeid( *this ).name();
   return (const char*)_MessageName.data();
}

template < class T >
void Message<T>::setMsgData()
{
   //
   // Break the message up into packets ...
   int offset = 0;
   list< MessagePacket* >::iterator pckt;
   for ( pckt  = _PacketList.begin();
         pckt != _PacketList.end() ;
         pckt++ ) 
   {
      ((MessagePacket*)(*pckt))->msgData().msg( ( ( (const unsigned char *) &_MsgData) + offset ), 
                                               ((MessagePacket*)(*pckt))->msgData().packetLength() );
      offset += ((MessagePacket*)(*pckt))->msgData().packetLength();


      //
      // Generate the CRC's
      ((MessagePacket*)(*pckt))->updateCRC();

   }

}

template < class T >
bool Message<T>::retrieveMsgData()
{
   //
   // Get the message data out of the packets ...
   int offset = 0;
   list< MessagePacket* >::iterator pckt;
   for ( pckt  = _PacketList.begin();
         pckt != _PacketList.end() ;
         pckt++ ) 
   {
      memmove( ( (char *) &_MsgData ) + offset , 
               (char *) ((MessagePacket*)(*pckt))->msgData().msg(), 
               ((MessagePacket*)(*pckt))->msgData().packetLength() );
      offset += ((MessagePacket*)(*pckt))->msgData().packetLength();

      (*pckt)->unopened( false );

      if ( (*pckt)->validCRC() == false )
         return false;
   }

   return true;
}

#endif
