/*
 * Copyright (c) 2002 Gambro BCT, Inc.  All rights reserved.
 *
 * TITLE:      dispatcher.h, Message Dispatcher.
 *
 * ABSTRACT:   These classes support message routing. The dispatcher class 
 *             is used to send messages to the system router.   Only one 
 *             dispatcher should be constructed per task.   
 */

#ifndef _DISPATCHER_H_
#define _DISPATCHER_H_

#include <map>
#include <set>
#include <stdio.h>
#include <fcntl.h>
#include <mqueue.h>
#include <string.h>

#include "messagebase.h"

class Dispatcher
{
public:        
   
   //
   // Number of retries before error for messaging operations ...
   const unsigned int MAX_NUM_RETRIES=1;
   //
   // default Q size
   const unsigned int DEFAULT_Q_SIZE=30;

public:

   Dispatcher( );
   virtual ~Dispatcher();

   //
   // Initialize the task's mqueue ...
   void init( const char *qname,
              unsigned int maxMessages=DEFAULT_Q_SIZE);

   //
   // Send a packet to the router ...
   void send( const MessagePacket &mp );

   //
   // receive messages from the router
   void dispatchMessages();          
   void stopLoop() { _StopLoop=true; }

   //
   // register the message
   void registerMessage( const MessageBase &mb, MessagePacket &mp );
   void registerMessage( const unsigned long messageId, const MessageBase &mb );
   void deregisterMessage( const MessageBase &mb, MessagePacket &mp );
   void deregisterMessage( const unsigned long messageId, const MessageBase &mb );

   void dump();

protected:


   //
   // Distribute the message received from the router ...
   virtual void processMessage( MessagePacket &mp );

   // 
   // Shutdown message queues ...
   void shutdown();

   //
   // Cleanup the message lists ...
   void cleanup();

private:
   Dispatcher( Dispatcher const &);             // not implemented
   Dispatcher& operator=(Dispatcher const&);    // not implemented

protected:

   //
   // Message Queues
   mqd_t _MyQueue;
   mqd_t _RQueue;
   //
   // List of messages and timers used by this application...
   map< unsigned long, set< MessageBase* > >  _MessageMap;

   //
   // Flag to free the dispatcher from the loop...
   bool _StopLoop;
};


#endif
