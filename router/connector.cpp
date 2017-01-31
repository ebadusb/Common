/*
 * Copyright (c) 2002 Gambro BCT, Inc.  All rights reserved.
 *
 * TITLE:      connector.cpp
 *
 */

 
#include <vxWorks.h>

#include <map>
#include <errnoLib.h>
#include <ioLib.h>
#include <mqueue.h>
#include <stdlib.h>
#include <time.h>
#include <netinet/tcp.h>

#include "datalog.h"
#include "datalog_levels.h"
#include "error.h"
#include "connector.h"
#include "messagepacket.h"
#include "messagesystemconstant.h"
#include "systemoverrides.h"


int Connector_main( unsigned long netAddress, int sock )
{
   //
   // Open the Router's queue
   //
   mqd_t RouterQueue;
   unsigned int retries=0;
   while (    ( RouterQueue = mq_open( "router", O_WRONLY ) ) == (mqd_t)ERROR 
           && ++retries < MessageSystemConstant::MAX_NUM_RETRIES ) 
      nanosleep( &MessageSystemConstant::RETRY_DELAY, 0 );
   if ( RouterQueue == (mqd_t)ERROR )
   {
      //
      // Error ...
      DataLog( log_level_critical ) << "Connector::init : router mq_open failed, error->" << errnoMsg << endmsg;
      _FATAL_ERROR( __FILE__, __LINE__, "Router message queue open failed" );
      return ERROR;
   }

   sockaddr_in	addr;
	memset(&addr , 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = netAddress;
   addr.sin_port = htons( getGatewayPort() );

   DataLog( log_level_router_info ) << "Connector (" << hex << netAddress
                    << ") TCP port " << dec << getGatewayPort() << " - attempt connection..."
                    << endmsg;

   struct timeval timeOut = {10 /* seconds */, 0 /* microsecs */};

   while (    sock == ERROR 
           || connectWithTimeout( sock, (sockaddr *)&addr, sizeof(addr), &timeOut ) == ERROR )
   {
      if ( sock != ERROR )
         close( sock );  // start over again
     
      //
      // Set up my socket ...
      sock = socket(AF_INET, SOCK_STREAM, 0);
      if ( sock != ERROR )
      {
         //
         //
         int optval=16384; // Set the size of the send buffer to 16k
         if ( setsockopt( sock, SOL_SOCKET, SO_SNDBUF, (char*)&optval, sizeof(optval) ) == ERROR )
         {
            //
            // Error ...
            DataLog( log_level_critical ) << "Router init : socket set send buffer size option SO_SNDBUF failed, error->" << errnoMsg << endmsg;
            _FATAL_ERROR( __FILE__, __LINE__, "Router init: socket set option failed" );
            return false;
         }
      }
   
      nanosleep( &MessageSystemConstant::CONNECT_RETRY_DELAY, 0 );
   }

   DataLog( log_level_router_info ) << "Connector (" << hex << netAddress
                    << ") TCP socket fd " << dec << sock << ": port " << getGatewayPort() << " - connection established" 
                    << endmsg;

   //
   // Send the message packet to finish connection with the gateway ...
   MessagePacket mp;
   mp.msgData().osCode( MessageData::GATEWAY_CONNECT );
   mp.msgData().msgId( 0 );
   mp.msgData().nodeId( netAddress );
   mp.msgData().taskId( taskIdSelf() );
   mp.msgData().totalNum( 1 );
   mp.msgData().seqNum( 1 );
   mp.msgData().msg( (unsigned char*)&sock, sizeof( sock ) );
   mp.updateCRC();

   //
   // Check the task's queue to see if it is full or not ...
   mq_attr qattributes;
   if (    mq_getattr( RouterQueue, &qattributes ) == ERROR
        || qattributes.mq_curmsgs >= qattributes.mq_maxmsg )
   {
      //
      // The queue is full!
      //
      // Error ...
      int errorNo = errno;
      DataLog( log_level_critical ) << "Sending message=" << hex << mp.msgData().msgId() 
                           << " - Router queue full (" << dec << qattributes.mq_curmsgs << " messages)" 
                           << ", (" << errnoMsg << ")"
                           << endmsg;
#if !DEBUG_BUILD && CPU != SIMNT
      _FATAL_ERROR( __FILE__, __LINE__, "Message queue full" );
#endif // #if CPU!=SIMNT && BUILD_TYPE!=DEBUG
      return ERROR;
   }

   //
   // Send message packet to router ...
   retries=0;
   while (    mq_send( RouterQueue, &mp, sizeof( MessagePacket ), MessageSystemConstant::GATEWAY_CONNECT_PRIORITY ) == ERROR 
           && ++retries < MessageSystemConstant::MAX_NUM_RETRIES )
   {
      DataLog( log_level_router_info ) << "Connector : retrying mqueue send" << endmsg;
      nanosleep( &MessageSystemConstant::RETRY_DELAY, 0 );
   }
   if ( retries >= MessageSystemConstant::MAX_NUM_RETRIES )
   {
      //
      // Error ...
      _FATAL_ERROR( __FILE__, __LINE__, "Router message queue send failed" );
      return ERROR;
   }

   //
   // Close the router's queue ...
   mq_close( RouterQueue );

   return OK;
}

short getGatewayPort()
{
   map< short, unsigned long > portAddressMap;
   map< short, unsigned long >::iterator paiter;

   //
   // Get the map of network connections
   getNetworkedNodes( portAddressMap );

   //
   // Determine my connection port ...
   for ( paiter = portAddressMap.begin() ;
         paiter != portAddressMap.end() ;
         ++paiter )
   {
      if ( (*paiter).second == getNetworkAddress() )
         return (*paiter).first;
   }
   return 0;
}

