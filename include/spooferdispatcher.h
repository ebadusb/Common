/*
 * Copyright (c) 2002 Gambro BCT, Inc.  All rights reserved.
 *
 * TITLE:      spooferdispatcher.h, Spoofing Message Dispatcher.
 *
 * ABSTRACT:   The spoofer dispatcher class is used to send messages and
 *             spoof messages in the message system.
 */

#ifndef _SPOOFER_DISPATCHER_H_
#define _SPOOFER_DISPATCHER_H_

#include <map>
#include "callback.h"
#include "dispatcher.h"
#include "messagebase.h"
#include "messagepacket.h"

class SpooferDispatcher : public Dispatcher
{
public:

   SpooferDispatcher( );
   virtual ~SpooferDispatcher();

   //
   // Deregister the message from my lists first, then call the base class
   //  behavior.
   virtual void deregisterMessage( const MessageBase &mb, MessagePacket &mp );

   //
   // Make the message base object a spoofer message.  Use
   //  the function without a callback function to send spoof
   //  messages without receiving anything first.  Use the 
   //  function with a callback function to receive the message
   //  type first to trigger spoofing.
   void spoofMessage( MessageBase &mb );
   void spoofMessage( MessageBase &mb, const CallbackBase &cb );

   //
   // Change the message base object back to a normal, non-spoofed
   //  message.
   void despoofMessage( MessageBase &mb );

   //
   // Crash the message crc.
   static void sendCorruptMessage( MessageBase &mb );

protected:

   //
   // Distribute the message received from the router to the
   //  spoofer callback or the normal message callback depending
   //  on whether the message is in the list or not.
   virtual void processMessage( MessagePacket &mp );

   //
   // Cleanup the map.
   void cleanup();

private:
   SpooferDispatcher( SpooferDispatcher const &);             // not implemented
   SpooferDispatcher& operator=(SpooferDispatcher const&);    // not implemented

protected:

   //
   // This map contains the list of messages currently being
   //  spoofed along with the callback to be called to perform 
   //  the spoof.
   map< const MessageBase*, CallbackBase > _SpoofedMsgMap;
};


#endif
