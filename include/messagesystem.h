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
   // default Q size
   const unsigned int DEFAULT_Q_SIZE=30;

public:
   //
   // Static method to gain access to the message system pointer
   static void MsgSystem( MessageSystem *msgSys ) { _TheMessageSystem = msgSys; }
   static MessageSystem *MsgSystem() { return _TheMessageSystem; }

   static void taskCleanup( unsigned long tId );

private:

   //
   // Task wide message system pointer ...
   static MessageSystem *_TheMessageSystem;
   
   //
   // System wide task id to message system map
   static map< unsigned long, unsigned long > _TheMessageSystemTaskMap;

public:

   //
   // Default constructor
   MessageSystem();

   //
   // Default destructor
   virtual ~MessageSystem();

   //
   // Initialize the message system
   bool initBlocking( const unsigned int qSize=DEFAULT_Q_SIZE ); // uses default task name
   bool initBlocking( const char *qname, const unsigned int qSize=DEFAULT_Q_SIZE );
   bool initNonBlock( const unsigned int qSize=DEFAULT_Q_SIZE ); // uses default task name
   bool initNonBlock( const char *qname, const unsigned int qSize=DEFAULT_Q_SIZE );

   //
   // Go into the message loop, or try to receive a message in the message
   //  loop is disabled ...
   int dispatchMessages();

   //
   // Stop the receive messages loop ...
   void stopLoop();
   //
   // Allow the receive messages loop ...
   void allowLoop();

   //
   // Get the dispatcher
   Dispatcher &dispatcher() { return _Dispatcher; }

protected:

   //
   // Actual function which does the work ...
   bool init( const char *qName, const unsigned int qSize, const bool block );

protected:

   //
   // The tasks interface to the system's message router
   Dispatcher _Dispatcher;

private:
   MessageSystem( const MessageSystem & ); // not implemented
   MessageSystem &operator=( const MessageSystem & ); // not implemented

};

#endif
