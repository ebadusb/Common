/*
 * Copyright (c) 2002 Gambro BCT, Inc.  All rights reserved.
 *
 * TITLE:      gateway.cpp
 *
 */

 
#include <stdlib.h>
#include <errnoLib.h>
#include <time.h>
#include <vxWorks.h>
#include <taskHookLib.h>

#include "error.h"
#include "gateway.h"
#include "systemoverrides.h"

const unsigned int Gateway::MAX_NUM_RETRIES=1;
const struct timespec Gateway::RETRY_DELAY={ 1 /* seconds */, 0 /*nanoseconds*/ };
const unsigned int Gateway::DEFAULT_Q_SIZE=100;

int Gateway::Gateway_main( short port )
{
   Gateway msgGateway;
   if ( !msgGateway.init( port ) )
      _FATAL_ERROR( __FILE__, __LINE__, "Gateway : init failed" );

   msgGateway.receiveLoop();

   return OK;
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
   // Open the Router's queue
   //
   unsigned int retries=0;
   while (    ( _RouterQueue = mq_open( "router", O_WRONLY ) ) == (mqd_t)ERROR 
           && retries++ < MAX_NUM_RETRIES ) nanosleep( &Gateway::RETRY_DELAY, 0 );
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
         char buffer[256];
         sprintf( buffer,"Gateway::receiveLoop : socket receive failed, error no -> %d",
                  errnoGet() );
         _FATAL_ERROR( __FILE__, __LINE__, buffer );
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
           && retries++ < Gateway::MAX_NUM_RETRIES )
      nanosleep( &Gateway::RETRY_DELAY, 0 );
   if ( retries == Gateway::MAX_NUM_RETRIES )
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