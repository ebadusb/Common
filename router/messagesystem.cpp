/*
 * Copyright (c) 2002 by Gambro BCT, Inc.  All rights reserved.
 *
 * TITLE:      messagesystem.cpp
 *
 */

#include <vxWorks.h>

#include <taskVarLib.h>
#include <stdio.h>

#include "error.h"
#include "messagesystem.h"

MessageSystem *MessageSystem::_TheMessageSystem=0;

MessageSystem::MessageSystem() :
   _Dispatcher( 0 )
{
}

MessageSystem::~MessageSystem()
{
   if ( _TheMessageSystem == this )
   {
      _TheMessageSystem = 0;

      //
      // Remove the task variable
      taskVarDelete( 0, (int *)&_TheMessageSystem );
   }
}

bool MessageSystem::initBlocking( const unsigned int qsize )
{
   return init( taskName( 0 ), qsize, true );
}

bool MessageSystem::initBlocking( const char *qname, const unsigned int qsize )
{
   return init( qname, qsize, true );
}

bool MessageSystem::initNonBlock( const unsigned int qsize )
{
   return init( taskName( 0 ), qsize, false );
}

bool MessageSystem::initNonBlock( const char *qname, const unsigned int qsize )
{
   return init( qname, qsize, false );
}

int MessageSystem::dispatchMessages()
{
   //
   // Give control to the dispatcher ...
   if ( _Dispatcher )
      return _Dispatcher->dispatchMessages();
   return ERROR;
}

void MessageSystem::stopLoop()
{
   //
   // Stop the dispatcher from looping for messages ...
   if ( _Dispatcher )
      _Dispatcher->stopLoop();
}

void MessageSystem::allowLoop()
{
   //
   // Allow the dispatcher to loop for new messages ...
   if ( _Dispatcher )
      _Dispatcher->allowLoop();
}

Dispatcher &MessageSystem::dispatcher() 
{ 
   if ( !_Dispatcher )
      _FATAL_ERROR( __FILE__, __LINE__, "Dispatcher not created" );

   return *_Dispatcher;
}

void MessageSystem::createDispatcher()
{
   if ( _Dispatcher )
      delete _Dispatcher;

   _Dispatcher = new Dispatcher;

   if ( !_Dispatcher )
      _FATAL_ERROR( __FILE__, __LINE__, "Dispatcher create failed" );
}

bool MessageSystem::init( const char *qname, const unsigned int qsize, const bool block )
{
   if ( qname == 0 )
   {
      //
      // no name given ...
      _FATAL_ERROR( __FILE__, __LINE__, "Queue name not given" );
      return false;
   }

   if ( _TheMessageSystem != 0 )
   {
      char buffer[80];
      sprintf( buffer, "Static variable, _TheMessageSystem not = 0 ( value=%lx, currentObj=%lx ).",
               (unsigned long)_TheMessageSystem, (unsigned long)this );
      _FATAL_ERROR( __FILE__, __LINE__, buffer );
      return false;
   }

   //
   // Assign this instance to be the task wide object
   _TheMessageSystem = this;

   //
   // Set up the task variable
   if ( taskVarAdd( 0, (int *)&_TheMessageSystem ) != OK )
   {
      //
      // Notify the system of the error ...
      char buffer[80];
      sprintf( buffer, "taskVarAdd failed.  errno=%d", errno  );
      _FATAL_ERROR( __FILE__, __LINE__, buffer);
      return false;
   }

   //
   // Create the dispatcher ...
   createDispatcher();

   //
   // The dispatcher will call an error function if it fails to initialize ...
   _Dispatcher->init( qname, qsize, block );

   return true;
}

