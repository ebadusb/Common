/*
 * Copyright (c) 2002 by Gambro BCT, Inc.  All rights reserved.
 *
 * TITLE:      messagesystem.cpp
 *
 */

#include <vxWorks.h>

#include <taskVarLib.h>
#include <signal.h>
#include <stdio.h>

#include "datalog.h"
#include "datalog_levels.h"
#include "error.h"
#include "messagesystem.h"

MessageSystem *MessageSystem::_TheMessageSystem=0;


void MessageSystem::datalogErrorHandler( const char * file, int line, 
                                         DataLog_ErrorType error, 
                                         const char * msg, 
                                         int continuable )
{
   if ( !continuable )
   {
      _FATAL_ERROR( __FILE__, __LINE__, "Data log error" );
   }
   cerr << "Data log error - " << error << " : " << msg << endl;
}

void MessageSystem::signalHandler( int sig )
{
   if (    _TheMessageSystem 
        && _TheMessageSystem->_Dispatcher )
   {
      _TheMessageSystem->_Dispatcher->stopLoop();
      _TheMessageSystem->_Dispatcher->receivedSignal( sig );
   }
}

MessageSystem::MessageSystem() :
   _Dispatcher( 0 )
{
}

MessageSystem::~MessageSystem()
{
   if ( _TheMessageSystem == this )
   {
      _TheMessageSystem = 0;
      if ( _Dispatcher )
      {
         delete _Dispatcher;
         _Dispatcher = 0;
      }

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
   //
   // Install the datalog error handler ...
   datalog_SetTaskErrorHandler( taskIdSelf(), &MessageSystem::datalogErrorHandler );

   if ( _Dispatcher != 0 )
   {
      DataLog( log_level_critical ) << "MessageSystem object already initialized." << endmsg;
      _FATAL_ERROR( __FILE__, __LINE__, "MessageSystem initialization failed" );
      return false;
   }

   if ( qname == 0 )
   {
      //
      // no name given ...
      _FATAL_ERROR( __FILE__, __LINE__, "Queue name not given" );
      return false;
   }

	if ( exists() )
	{
      DataLog( log_level_critical ) << "Task variable _TheMessageSystem has already been added for this task ("
                           << hex << (unsigned long)_TheMessageSystem << " currentObj=" 
                           << (unsigned long)this << " )" << endmsg;
      _FATAL_ERROR( __FILE__, __LINE__, "MessageSystem initialization failed" );
   }

   //
   // Set up the task variable
   if ( taskVarAdd( 0, (int *)&_TheMessageSystem ) != OK )
   {
      //
      // Notify the system of the error ...
      DataLog( log_level_critical ) << "taskVarAdd failed.  errno=" << errno << endmsg;
      _FATAL_ERROR( __FILE__, __LINE__, "MessageSystem initialization failed" );
   }

   //
   // Assign this instance to be the task wide object
   _TheMessageSystem = this;

   //
   // Create the dispatcher ...
   createDispatcher();

   //
   // The dispatcher will call an error function if it fails to initialize ...
   _Dispatcher->init( qname, qsize, block );

   //
   // Set up the task's signal handler for the appropriate signals ...
   signal( SIGHUP,  &MessageSystem::signalHandler );
   signal( SIGQUIT, &MessageSystem::signalHandler );

   return true;
}

bool MessageSystem::exists()
{
	return taskVarGet(taskIdSelf(), (int *)&_TheMessageSystem) != ERROR;
}


