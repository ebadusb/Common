/*
 * Copyright (c) 2002 Gambro BCT, Inc.  All rights reserved.
 *
 * TITLE:      gateway.cpp
 *
 */

 
#include <vxWorks.h>

#include <ioLib.h>
#include <stdlib.h>
#include <time.h>
#include <taskHookLib.h>
#include <netinet/tcp.h>

#include "datalog.h"
#include "datalog_levels.h"
#include "error.h"
#include "failure_debug.h"
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
   //
   // Install the datalog error handler ...
   datalog_SetTaskErrorHandler( taskIdSelf(), &Gateway::datalogErrorHandler );

   //
   // Open the Router's queue
   //
   unsigned int retries=0;
   while (    ( _RouterQueue = mq_open( "router", O_WRONLY ) ) == (mqd_t)ERROR 
           && ++retries < MessageSystemConstant::MAX_NUM_RETRIES ) 
      nanosleep( &MessageSystemConstant::RETRY_DELAY, 0 );
   if ( _RouterQueue == (mqd_t)ERROR )
   {
      //
      // Error ...
      DataLog( log_level_critical ) << "Gateway::init : router mq_open failed, error->" << errnoMsg << endmsg;
      _FATAL_ERROR( __FILE__, __LINE__, "Router message queue open failed" );
      return false;
   }

	int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
   if ( serverSocket == ERROR )
   {
      //
      // Error ...
      DataLog( log_level_critical ) << "Gateway::init : socket create failed, error->" << errnoMsg << endmsg;
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
      DataLog( log_level_critical ) << "Gateway::init : socket bind failed, error->" << errnoMsg << endmsg;
      _FATAL_ERROR( __FILE__, __LINE__, "Gateway init: socket bind failed" );
      return false;
   }

	struct sockaddr_in	clientAddr;
	int 	clientAddrSize = sizeof(clientAddr);

   DataLog( log_level_gateway_info ) << "Gateway TCP socket fd " << dec << serverSocket << ": port " << port << " - listen for connection" 
                    << endmsg;

   if ( listen( serverSocket, 0 ) == ERROR )
   {
      //
      // Error ...
      DataLog( log_level_critical ) << "Gateway::init : socket listen failed, error->" << errnoMsg << endmsg;
      _FATAL_ERROR( __FILE__, __LINE__, "Gateway init: socket listen failed" );
      return false;
   }

   if ( ( _ClientSocket = accept( serverSocket, (sockaddr*)&clientAddr, &clientAddrSize) ) == ERROR )
   {
      //
      // Error ...
      DataLog( log_level_critical ) << "Gateway::init : socket accept failed, error->" << errnoMsg << endmsg;
      _FATAL_ERROR( __FILE__, __LINE__, "Gateway init: socket accept failed" );
      return false;
   }

   DataLog( log_level_gateway_info ) << "Gateway TCP socket fd " << dec << serverSocket << ": port " << port << " - accept connection" 
                    << endmsg;

   //
   //
   int optval=1;
   if ( setsockopt( _ClientSocket, SOL_SOCKET, SO_KEEPALIVE, (char*)&optval, sizeof(optval) ) == ERROR )
   {
      //
      // Error ...
      DataLog( log_level_critical ) << "Gateway::init : socket set option SO_KEEPALIVE failed, error->" << errnoMsg << endmsg;
      _FATAL_ERROR( __FILE__, __LINE__, "Gateway init: socket set option failed" );
      return false;
   }
   if ( setsockopt( _ClientSocket, IPPROTO_TCP, TCP_NODELAY, (char*)&optval, sizeof(optval) ) == ERROR )
   {
      //
      // Error ...
      DataLog( log_level_critical ) << "Gateway::init : socket set option TCP_NODELAY failed, error->" << errnoMsg << endmsg;
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
   unsigned char msgBuf[ sizeof( MessagePacket ) +( 4*sizeof( unsigned long ) ) ];
   do
   {
      //
      // Reset my data structures ...
      memset( &mp,    0, sizeof( MessagePacket ) );
      memset( msgBuf, 0, sizeof( MessagePacket ) + ( 4*sizeof( unsigned long ) ) );

      int msgSize;
		int totalByteCount = 0;
		while ( totalByteCount < sizeof( int ) )
      {
         int byteCount = recv( _ClientSocket, (char*)((&msgSize)+totalByteCount), sizeof( int )-totalByteCount, 0 );
   
         if ( byteCount == ERROR )
         {
            DataLog( log_level_critical ) << "Gateway::receiveLoop : socket receive failed, error->" << errnoMsg << endmsg;
            _FATAL_ERROR( __FILE__, __LINE__, "socket receive failed" );
            return;
         }
		   totalByteCount += byteCount;
      }
      if ( msgSize <= 0 )
      {
         DataLog( log_level_critical ) << "Gateway::receiveLoop : invalid message size" << endmsg;
         _FATAL_ERROR( __FILE__, __LINE__, "invalid message size" );
         return;
      }

		totalByteCount = 0;
		while ( totalByteCount < msgSize )
		{
			int byteCount = recv( _ClientSocket, (char*)&(msgBuf[totalByteCount]), msgSize-totalByteCount, 0 );

			if ( byteCount == ERROR )
			{
				DataLog( log_level_critical ) << "Gateway::receiveLoop : socket receive failed, error->" << errnoMsg << endmsg;
				_FATAL_ERROR( __FILE__, __LINE__, "socket receive failed" );
				return;
		   }
		   totalByteCount += byteCount;
		}

      unsigned long crc;
      unsigned long msgBegin, msgEnd;
      memmove( &msgBegin , (void*)   msgBuf                                          ,             sizeof( unsigned long ) );
      memmove( &msgEnd   , (void*)&( msgBuf[ msgSize -   sizeof( unsigned long )   ]),             sizeof( unsigned long ) );
      memmove( &mp       , (void*)&( msgBuf[             sizeof( unsigned long )   ]), msgSize-( 3*sizeof( unsigned long ) ) );
      memmove( &crc      , (void*)&( msgBuf[ msgSize-( 2*sizeof( unsigned long ) ) ]),             sizeof( unsigned long ) );
      mp.crc( crc );

      if ( msgBegin != 0xeeeeeeee || msgEnd != 0xeeeeeeee )
      {
         DataLog( log_level_critical ) << "Gateway::receiveLoop : receive failure (" << hex 
                              << msgBegin << "-" << msgEnd << ")" << endmsg;
         _FATAL_ERROR( __FILE__, __LINE__, "receive failure" );
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
   // Check the task's queue to see if it is full or not ...
   mq_attr qattributes;
   if (    mq_getattr( _RouterQueue, &qattributes ) == ERROR
        || qattributes.mq_curmsgs >= qattributes.mq_maxmsg )
   {
      //
      // The queue is full!
      //
      // Error ...
      DataLog( log_level_critical ) << "Sending message=" << hex << mp.msgData().msgId() 
                           << " - Router queue full (" << dec << qattributes.mq_curmsgs << " messages)" 
                           << ", (" << errnoMsg << ")"
                           << endmsg;
      DBG_DumpData();
      dumpQueue( _RouterQueue, DataLog( log_level_gateway_error ) );

#if !DEBUG_BUILD && CPU != SIMNT
      _FATAL_ERROR( __FILE__, __LINE__, "Message queue full" );
#endif // #if CPU!=SIMNT && BUILD_TYPE!=DEBUG
      return;
   }
#if MESSAGE_SYSTEM_STATISTICS
   else if ( qattributes.mq_curmsgs >= qattributes.mq_maxmsg/10 )
   {
      DataLog( log_level_gateway_info ) << "Sending message=" << hex << mp.msgData().msgId() 
                           << " Router queue contains " << dec << qattributes.mq_curmsgs << " messages" 
                           << endmsg;
   }
#endif


   //
   // Send message packet to router ...
   unsigned int retries=0;
   while (    mq_send( _RouterQueue, &mp, sizeof( MessagePacket ), MessageSystemConstant::GATEWAY_MESSAGE_PRIORITY ) == ERROR 
           && ++retries < MessageSystemConstant::MAX_NUM_RETRIES )
      nanosleep( &MessageSystemConstant::RETRY_DELAY, 0 );
   if ( retries >= MessageSystemConstant::MAX_NUM_RETRIES )
   {
      //
      // Error ...
      _FATAL_ERROR( __FILE__, __LINE__, "Router message queue send failed" );
      return;
   }
}


void Gateway::dumpQueue( mqd_t mqueue, DataLog_Stream &out )
{
   struct mq_attr old_attr;                                    // message queue attributes 
   struct mq_attr attr;                                    // message queue attributes 
   attr.mq_flags = O_NONBLOCK;
   mq_setattr( mqueue, &attr, &old_attr );

   char buffer[ sizeof(MessagePacket) ];
   int count=0;
   int priority;

   //
   // Read the queue entry ...
   while ( mq_receive( mqueue, &buffer, sizeof( MessagePacket ), &priority ) != ERROR )
   {

#if !DEBUG_BUILD && CPU != SIMNT
      //
      // Format the data ...
      MessagePacket mp;
      memmove( &mp, &buffer, sizeof( MessagePacket ) );

      out << " Message# " << dec << count << " priority " << priority << " msgId " << hex << mp.msgData().msgId()
                                                                      << " p# "    << hex << mp.msgData().seqNum()
                                                                      << " tot# "  << hex << mp.msgData().totalNum()  
                                                                      << " msg -> ";
      for (int i=0;i<30;++i) 
      {
         out << hex << (int)(unsigned char)buffer[i] << " "; 
      }
      out << endmsg;
#endif // #if BUILD_TYPE!=DEBUG && CPU!=SIMNT

   }
   mq_setattr( mqueue, &old_attr, 0 );
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
