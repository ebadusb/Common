/*
 * Copyright (c) 2002 Gambro BCT, Inc.  All rights reserved.
 *
 * TITLE:      gateway.cpp
 *
 */

 
#include <vxWorks.h>

#include <stdlib.h>
#include <errnoLib.h>
#include <time.h>
#include <taskHookLib.h>

#include "datalog.h"
#include "error.h"
#include "gateway.h"
#include "messagesystemconstant.h"
#include "systemoverrides.h"


int Gateway::Gateway_main( short port )
{
   Gateway msgGateway;
   if ( !msgGateway.init( port ) )
      _FATAL_ERROR( __FILE__, __LINE__, "Gateway : init failed" );

   msgGateway.receiveLoop();

   return OK;
}

void Gateway::datalogErrorHandler( const char * file, int line, 
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

Gateway::Gateway()
:  _ServerSocket( sockbuf::sock_stream ),
   _ClientSocket( ),
   _RouterQueue( 0 )
{
}

Gateway::~Gateway()
{
   shutdown();
   cleanup();
}

bool Gateway::init( short port )
{
   //
   // Install the datalog error handler ...
   datalog_SetTaskErrorHandler( taskIdSelf(), &Gateway::datalogErrorHandler );

   //
   // Open the Router's queue
   //
   unsigned int retries=0;
   while (    ( _RouterQueue = mq_open( "router", O_WRONLY ) ) == (mqd_t)ERROR 
           && retries++ < MessageSystemConstant::MAX_NUM_RETRIES ) 
      nanosleep( &MessageSystemConstant::RETRY_DELAY, 0 );
   if ( _RouterQueue == (mqd_t)ERROR )
   {
      //
      // Error ...
      _FATAL_ERROR( __FILE__, __LINE__, "Router message queue open failed" );
      return false;
   }


   if ( _ServerSocket.bind( INADDR_ANY, port ) != 0 )
   {
      //
      // Error ...
      _FATAL_ERROR( __FILE__, __LINE__, "Gateway init: socket bind failed" );
      return false;
   }

   _ServerSocket.listen();
   if ( ( _ClientSocket = _ServerSocket.accept() ) == ERROR )
   {
      //
      // Error ...
      _FATAL_ERROR( __FILE__, __LINE__, "Gateway init: socket accept failed" );
      return false;
   }
   _ClientSocket.keepalive( 1 );
   _ClientSocket.nodelay( 1 );

   return true;
}

void Gateway::receiveLoop()
{
   MessagePacket mp;
   do
   {
      int byte_count = _ClientSocket.recv( &mp, sizeof( MessagePacket ), 0 );

      if ( byte_count == ERROR )
      {
         DataLog_Critical criticalLog;
         DataLog(criticalLog) << "Gateway::receiveLoop : socket receive failed, error->" << strerror( errnoGet() ) << endmsg;
         _FATAL_ERROR( __FILE__, __LINE__, "socket receive failed" );
         return;
      }

      //
      // Send the message packet to the router ...
      //
      sendMsgToRouter( mp );
      
   }
   while ( true );
}

void Gateway::sendMsgToRouter( const MessagePacket &mp )
{
   //
   // Send message packet to router ...
   unsigned int retries=0;
   while (    mq_send( _RouterQueue, &mp, sizeof( MessagePacket ), 0 ) == ERROR 
           && retries++ < MessageSystemConstant::MAX_NUM_RETRIES )
      nanosleep( &MessageSystemConstant::RETRY_DELAY, 0 );
   if ( retries == MessageSystemConstant::MAX_NUM_RETRIES )
   {
      //
      // Error ...
      _FATAL_ERROR( __FILE__, __LINE__, "Router message queue send failed" );
      return;
   }
}

void Gateway::shutdown()
{
   //
   // Close the socket ...
   _ServerSocket.shutdown(sockbuf::shut_readwrite);

   //
   // Close the router's queue ...
   mq_close( _RouterQueue );
   _RouterQueue = (mqd_t)0;

}

void Gateway::cleanup()
{
}
