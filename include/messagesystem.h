/*
 * Copyright (c) 2002 by Gambro BCT, Inc.  All rights reserved.
 *
 * TITLE:      messagesystem.h
 *
 * ABSTRACT:   This file initializes the message interface for the
 *              calling task
 */

#ifndef _MESSAGE_SYSTEM_H_
#define _MESSAGE_SYSTEM_H_

#include "dispatcher.h"

class MessageSystem
{
public:

   //
   // Static method to gain access to the message system
   static MessageSystem *MsgSystem() { return _TheMessageSystem; }

private:

   //
   // Task wide message system pointer ...
   static MessageSystem *_TheMessageSystem;

public:

   //
   // Default constructor
   MessageSystem();

   //
   // Default destructor
   virtual ~MessageSystem();

   //
   // Initialize the message system
   bool init();                                     // uses default task name
   bool init( const char *qname );

   //
   // Go into the message loop ...
   void dispatchMessages();

   //
   // Stop receiving messages ...
   void shutdown();

   //
   // Get the dispatcher
   Dispatcher &dispatcher() { return _Dispatcher; }

protected:

   //
   // The tasks interface to the system's message router
   Dispatcher _Dispatcher;

private:
   MessageSystem( const MessageSystem & ); // not implemented
   MessageSystem &operator=( const MessageSystem & ); // not implemented

};

#endif
