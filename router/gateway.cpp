/*
 * Copyright (c) 2002 Gambro BCT, Inc.  All rights reserved.
 *
 * TITLE:      gateway.cpp
 *
 */

 
#include <vxWorks.h>

#include <errnoLib.h>
#include <ioLib.h>
#include <stdlib.h>
#include <time.h>
#include <taskHookLib.h>
#include <netinet/tcp.h>

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
:  _ClientSocket( 0 ),
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
   DataLog_Critical criticalLog;

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
      DataLog(criticalLog) << "Gateway::init : router mq_open failed, error->" << strerror( errnoGet() ) << endmsg;
      _FATAL_ERROR( __FILE__, __LINE__, "Router message queue open failed" );
      return false;
   }

	int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
   if ( serverSocket == ERROR )
   {
      //
      // Error ...
      DataLog(criticalLog) << "Gateway::init : socket create failed, error->" << strerror( errnoGet() ) << endmsg;
      _FATAL_ERROR( __FILE__, __LINE__, "Gateway init: socket create failed" );
      return false;
   }

   sockaddr_in	addr;
	memset(&addr , 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(port);

   if ( bind( serverSocket, (sockaddr *)&addr, sizeof(addr) ) == ERROR )
   {
      //
      // Error ...
      DataLog(criticalLog) << "Gateway::init : socket bind failed, error->" << strerror( errnoGet() ) << endmsg;
      _FATAL_ERROR( __FILE__, __LINE__, "Gateway init: socket bind failed" );
      return false;
   }

	struct sockaddr_in	clientAddr;
	int 	clientAddrSize = sizeof(clientAddr);

   if ( listen( serverSocket, 0 ) == ERROR )
   {
      //
      // Error ...
      DataLog(criticalLog) << "Gateway::init : socket listen failed, error->" << strerror( errnoGet() ) << endmsg;
      _FATAL_ERROR( __FILE__, __LINE__, "Gateway init: socket listen failed" );
      return false;
   }

   if ( ( _ClientSocket = accept( serverSocket, (sockaddr*)&clientAddr, &clientAddrSize) ) == ERROR )
   {
      //
      // Error ...
      DataLog(criticalLog) << "Gateway::init : socket accept failed, error->" << strerror( errnoGet() ) << endmsg;
      _FATAL_ERROR( __FILE__, __LINE__, "Gateway init: socket accept failed" );
      return false;
   }

   //
   //
   int optval=16384; // Set the size of the receive buffer to 16
   if ( setsockopt( _ClientSocket, SOL_SOCKET, SO_RCVBUF, (char*)&optval, sizeof(optval) ) == ERROR )
   {
      //
      // Error ...
      DataLog(criticalLog) << "Gateway::init : socket set receive buffer size option SO_RCVBUF failed, error->" << strerror( errnoGet() ) << endmsg;
      _FATAL_ERROR( __FILE__, __LINE__, "Gateway init: socket set option failed" );
      return false;
   }

   optval=1;
   if ( setsockopt( _ClientSocket, SOL_SOCKET, SO_KEEPALIVE, (char*)&optval, sizeof(optval) ) == ERROR )
   {
      //
      // Error ...
      DataLog(criticalLog) << "Gateway::init : socket set option SO_KEEPALIVE failed, error->" << strerror( errnoGet() ) << endmsg;
      _FATAL_ERROR( __FILE__, __LINE__, "Gateway init: socket set option failed" );
      return false;
   }
   if ( setsockopt( _ClientSocket, IPPROTO_TCP, TCP_NODELAY, (char*)&optval, sizeof(optval) ) == ERROR )
   {
      //
      // Error ...
      DataLog(criticalLog) << "Gateway::init : socket set option TCP_NODELAY failed, error->" << strerror( errnoGet() ) << endmsg;
      _FATAL_ERROR( __FILE__, __LINE__, "Gateway init: socket set option failed" );
      return false;
   }

   //
   // Close the server socket ...
   close( serverSocket );

   return true;
}

void Gateway::receiveLoop()
{
   MessagePacket mp;
   do
   {
		char	* mpBuff = (char *)&mp;
		int	  total_byte_count = 0;
		while ( total_byte_count < sizeof(MessagePacket) )
		{
			int byte_count = recv( _ClientSocket, &mpBuff[total_byte_count], sizeof( MessagePacket ) - total_byte_count, 0 );

			if ( byte_count == ERROR )
			{
				DataLog_Critical criticalLog;
				DataLog(criticalLog) << "Gateway::receiveLoop : socket receive failed, error->" << strerror( errnoGet() ) << endmsg;
				_FATAL_ERROR( __FILE__, __LINE__, "socket receive failed" );
				return;
		   }
			else if ( byte_count == 0 )
			{
				DataLog_Critical criticalLog;
				DataLog(criticalLog) << "Gateway::receiveLoop : socket closed" << endmsg;
				_FATAL_ERROR( __FILE__, __LINE__, "socket closed" );
				return;
		   }
		   else
			{
				total_byte_count += byte_count;
			}
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
   // Check the task's queue to see if it is full or not ...
   mq_attr qattributes;
   if (    mq_getattr( _RouterQueue, &qattributes ) == ERROR
        || qattributes.mq_curmsgs >= qattributes.mq_maxmsg )
   {
      //
      // The queue is full!
      //
      // Error ...
      int errorNo = errno;
      DataLog_Critical criticalLog;
      DataLog(criticalLog) << "Sending message=" << hex << mp.msgData().msgId() 
                           << " - Router queue full (" << dec << qattributes.mq_curmsgs << " messages)" 
                           << ", (" << strerror( errorNo ) << ")"
                           << endmsg;
#if !DEBUG_BUILD && CPU != SIMNT
      _FATAL_ERROR( __FILE__, __LINE__, "Message queue full" );
#endif // #if CPU!=SIMNT && BUILD_TYPE!=DEBUG
      return;
   }

   //
   // Send message packet to router ...
   unsigned int retries=0;
   while (    mq_send( _RouterQueue, &mp, sizeof( MessagePacket ), MessageSystemConstant::GATEWAY_MESSAGE_PRIORITY ) == ERROR 
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
   close( _ClientSocket );

   //
   // Close the router's queue ...
   mq_close( _RouterQueue );
   _RouterQueue = (mqd_t)0;

}

void Gateway::cleanup()
{
}
