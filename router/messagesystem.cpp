/*
 * Copyright (c) 2002 by Gambro BCT, Inc.  All rights reserved.
 *
 * TITLE:      messagesystem.cpp
 *
 */

#include <vxWorks.h>
#include <taskVarLib.h>

#include "messagesystem.h"

MessageSystem *MessageSystem::_TheMessageSystem=0;

MessageSystem::MessageSystem() :
   _Dispatcher()
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

void MessageSystem::dispatchMessages()
{
   //
   // Give control to the dispatcher ...
   _Dispatcher.dispatchMessages();
}

void MessageSystem::stopLoop()
{
   //
   // Stop the dispatcher from looping for messages ...
   _Dispatcher.stopLoop();
}

void MessageSystem::allowLoop()
{
   //
   // Allow the dispatcher to loop for new messages ...
   _Dispatcher.allowLoop();
}

bool MessageSystem::init( const char *qname, const unsigned int qsize, const bool block )
{
   if ( _TheMessageSystem == 0 )
   {
      //
      // Assign this instance to be the task wide object
      _TheMessageSystem = this;

      //
      // Set up the task variable
      cout << endl << " MsgSys-> " << (int*)&_TheMessageSystem << endl << endl;
      if ( taskVarAdd( 0, (int *)&_TheMessageSystem ) != OK )
      {
         //
         // Notify the system of the error ...
         return false;
      }
   }
   if ( qname == 0 )
   {
      //
      // no name given ...
      return false;
   }

   //
   // The dispatcher will call an error function if it fails to initialize ...
   _Dispatcher.init( qname, qsize, block );

   return true;
}

