/*
 * Copyright (c) 2002 Gambro BCT, Inc.  All rights reserved.
 *
 * TITLE:      router.cpp, Message Router.
 *
 */

#include <vxWorks.h>

#include <ioLib.h>
#include <stdio.h>
#include <taskHookLib.h>
#include <netinet/tcp.h>

#include "common_datalog.h"
#include "connector.h"
#include "failure_debug.h"
#include "gateway.h"
#include "messagesystem.h"
#include "messagesystemconstant.h"
#include "router.h"
#include "systemoverrides.h"


WIND_TCB *Router::_TheRouterTid=0;
Router *Router::_TheRouter=0;

int Router::Router_main()
{
   if ( _TheRouter )
      return !OK;

   Router msgRouter;
   msgRouter.init();

   msgRouter.dispatchMessages();

   return OK;
}

WIND_TCB *Router::globalRouterTid()
{
   return _TheRouterTid;
}

Router *Router::globalRouter()
{
   return _TheRouter;
}

int Router::taskDeleteHook( WIND_TCB *pTcb )
{
   if ( _TheRouterTid == pTcb )
   {
      delete _TheRouter; 
      _TheRouter = 0;
      _TheRouterTid = 0;
   }

   if ( _TheRouter )
   {
      _TheRouter->deregisterTask( (unsigned long) pTcb );
   }
   
   return 1;
}

void Router::datalogErrorHandler( const char * file, int line, 
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

Router::Router()
:  _RouterQueue( 0 ),
   _MessageHighWaterMark( 0 ),
   _NumMessages( 0 ),
   _MessageHighWaterMarkPerPeriod( 0 ),
   _PrevMessageHighWaterMarkPerPeriod( 0 ),
   _TimerQueue( 0 ),
   _MsgIntegrityMap(),
   _MsgToGatewaySynchMap(),
   _MessageTaskMap(),
   _TaskQueueMap(),
   _InetGatewayMap(),
   _GatewayConnSynchedMap(),
   _SpooferMsgMap(),
   _StopLoop( false ),
   _NetSequenceNum( 0 )
{
}

Router::~Router()
{
   shutdown();
   cleanup();
}

bool Router::init()
{
   if ( _TheRouter )
      return false;

   //
   // Add the task delete hook to catch all task deletion and
   // keep the task lists up to date ...
   taskDeleteHookAdd( (FUNCPTR) &Router::taskDeleteHook );

   //
   // Install the datalog error handler ...
   datalog_SetTaskErrorHandler( taskIdSelf(), &Router::datalogErrorHandler );

   struct mq_attr attr;                                    // message queue attributes 
   attr.mq_maxmsg =  MessageSystemConstant::DEFAULT_ROUTER_Q_SIZE; // set max number of messages 
   attr.mq_msgsize = sizeof( MessagePacket );              // set message size 
   attr.mq_flags = 0;

   //
   // open queue
   //
   unsigned int retries=0;
   while (    ( _RouterQueue = mq_open( "router", O_RDWR | O_CREAT , 0666, &attr) ) == (mqd_t)ERROR 
           && ++retries < MessageSystemConstant::MAX_NUM_RETRIES ) 
      nanosleep( &MessageSystemConstant::RETRY_DELAY, 0 );
   if ( _RouterQueue == (mqd_t)ERROR )
   {
      //
      // Error ...
      DataLog( log_level_critical ) << "Router message queue open failed" << endmsg;
      _FATAL_ERROR( __FILE__, __LINE__, "Router message queue open failed" );
      return false;
   }

   //
   // Connect up with our timer task ...
   //  don't do anything else until we connnect.
   while ( ( _TimerQueue = mq_open( "timertask", O_RDWR ) ) == (mqd_t)ERROR ) 
      nanosleep( &MessageSystemConstant::RETRY_DELAY, 0 );

   //
   // If not opened ...
   if ( _TimerQueue == (mqd_t)ERROR )
   {
      //
      // Error ...
      DataLog( log_level_critical ) << "Timer message queue open failed" << endmsg;
      _FATAL_ERROR( __FILE__, __LINE__, "Timer message queue open failed" );
      return false;
   }

   MessagePacket mp;
   mp.msgData().osCode( MessageData::TASK_REGISTER );
   mp.msgData().taskId( taskIdSelf() );
   mp.msgData().msg( (const unsigned char*)"router", strlen( "router" ) );
   mp.updateCRC();
   sendMessage( mp, _TimerQueue, taskIdSelf(), MessageSystemConstant::DEFAULT_REGISTER_PRIORITY );

   if ( initGateways() == false )
   {
      //
      // Error ...
      _FATAL_ERROR( __FILE__, __LINE__, "Router init gateways failed" );
      return false;
   }

   //
   // Set the static pointer to ensure we only init once ...
   _TheRouter = this;
   _TheRouterTid = taskTcb( 0 );
   return true;
}

void Router::dispatchMessages()
{
   if ( _RouterQueue == 0 )
      return;

   int size=0;
   int priority=0;
   char buffer[ sizeof( MessagePacket ) ];
   do
   {
      //
      // Read the queue entry ...
      unsigned int retries=0;
      while (    ( size = mq_receive( _RouterQueue, &buffer, sizeof( MessagePacket ), &priority ) ) == ERROR 
              && ++retries < MessageSystemConstant::MAX_NUM_RETRIES ) 
      {
         nanosleep( &MessageSystemConstant::RETRY_DELAY, 0 );
      }
      if ( size == ERROR || size == 0 )
      {
         //
         // Error ...
         DataLog( log_level_critical ) << "Dispatching message - mq_receive return size=" << size
                              << " and (" << errnoMsg << ")" << endmsg;
         _FATAL_ERROR( __FILE__, __LINE__, "Dispatching message - message queue receive failed" );
         return;
      }

      //
      // Format the data ...
      MessagePacket mp;
      memmove( &mp, &buffer, sizeof( MessagePacket ) );

      if ( mp.validCRC() == false )
      {
         //
         // Error ...
         unsigned long crc = mp.crc();
         mp.updateCRC();
         DataLog( log_level_critical ) << "Dispatching message - message CRC validation failed for MsgId=" 
                              << hex << mp.msgData().msgId() 
                              << "(" << _MsgIntegrityMap[ mp.msgData().msgId() ].c_str() << ")" 
                              << ", CRC=" << crc << " and should be " << mp.crc() << endmsg;
         mp.dump( DataLog( log_level_router_error ) );
         _FATAL_ERROR( __FILE__, __LINE__, "CRC check failed" );
      }  

      processMessage( mp, priority );

      //
      // Check the task's queue to see if we went over the high-water mark ...
      mq_attr qattributes;
      mq_getattr( _RouterQueue, &qattributes );
      if ( qattributes.mq_curmsgs > _MessageHighWaterMark )
      {
         _MessageHighWaterMark = _MessageHighWaterMarkPerPeriod = qattributes.mq_curmsgs;
         DataLog( log_level_message_system_info ) << "mqueue max: " << _MessageHighWaterMark << "/" << qattributes.mq_maxmsg << endmsg;
      }
      else 
      {
         if ( qattributes.mq_curmsgs > _MessageHighWaterMarkPerPeriod )
            _MessageHighWaterMarkPerPeriod = qattributes.mq_curmsgs;

         if ( ( _NumMessages % ( MessageSystemConstant::MESSAGES_BETWEEN_LOG * qattributes.mq_maxmsg ) ) == 0 )
         {
            if ( _MessageHighWaterMarkPerPeriod != _PrevMessageHighWaterMarkPerPeriod )
               DataLog( log_level_message_system_info ) << "mqueue max per period: " << _MessageHighWaterMarkPerPeriod << "/" << qattributes.mq_maxmsg << endmsg;
            _PrevMessageHighWaterMarkPerPeriod = _MessageHighWaterMarkPerPeriod;
            _MessageHighWaterMarkPerPeriod = 0;
         }
      }

      _NumMessages++;

   } while ( _StopLoop == false );

}

void Router::dump( DataLog_Stream &outs )
{
   outs << "------------------------- Router DUMP -----------------------------";

   // mq_attr qattributes;
   // if ( _RouterQueue != (mqd_t)0 ) mq_getattr( _RouterQueue, &qattributes );
   outs << " RouterQueue: " << hex << (long)_RouterQueue 
        // << "  flags " << qattributes.mq_flags
        // << "  size " << qattributes.mq_curmsgs
        // << "  maxsize " << qattributes.mq_maxmsg 
        << endmsg;
   // if ( _TimerQueue != (mqd_t)0 ) mq_getattr( _TimerQueue, &qattributes );
   outs << " MsgSysTimerQueue: " << hex << (long)_TimerQueue 
        // << "  flags " << qattributes.mq_flags
        // << "  size " << qattributes.mq_curmsgs
        // << "  maxsize " << qattributes.mq_maxmsg 
        << endmsg;

   outs << " Message Integrity Map: size " << dec << _MsgIntegrityMap.size() << endmsg;
   map< unsigned long, string >::iterator miiter;                                 // _MsgIntegrityMap;
   for ( miiter  = _MsgIntegrityMap.begin() ;
         miiter != _MsgIntegrityMap.end() ;
         miiter++ )
   {
      outs << "  Mid " << hex << (*miiter).first << " " << (*miiter).second;
   }
   outs << endmsg;
   outs << " Message to Gateway Synch Map: size " << dec << _MsgToGatewaySynchMap.size();
   outs << endmsg;
   map< unsigned long, set< unsigned long > >::iterator mtogiter;                                 // _MsgToGatewaySynchMap;
   for ( mtogiter  = _MsgToGatewaySynchMap.begin() ;
         mtogiter != _MsgToGatewaySynchMap.end() ;
         mtogiter++ )
   {
      outs << "  Mid " << hex << (*mtogiter).first;
      set< unsigned long >::iterator gateiter;
      for ( gateiter  = ((*mtogiter).second).begin() ;
            gateiter != ((*mtogiter).second).end() ;
            gateiter++ )
         outs << " " << (*gateiter);
      outs << endmsg;
   }
   outs << " Message Task Map: size " << _MessageTaskMap.size();
   outs << endmsg;
   map< unsigned long, map< unsigned long, unsigned char > >::iterator mtiter;    // _MessageTaskMap;
   map< unsigned long, unsigned char >::iterator triter;                          
   for ( mtiter  = _MessageTaskMap.begin() ;
         mtiter != _MessageTaskMap.end() ;
         mtiter++ )
   {
      outs << "  Mid " << hex << (*mtiter).first;
      for ( triter  = ((*mtiter).second).begin() ;
            triter != ((*mtiter).second).end() ;
            triter++ )
      {
         outs << "    Tid " << hex << (*triter).first << " #regs " << (int)(*triter).second;
      }
      outs << endmsg;
   }
   outs << " Task Queue Map: size " << dec << _TaskQueueMap.size();
   outs << endmsg;
   map< unsigned long, mqd_t >::iterator tqiter;                                  // _TaskQueueMap;
   for ( tqiter  = _TaskQueueMap.begin() ;
         tqiter != _TaskQueueMap.end() ;
         tqiter++ )
   {
      // if ( (*tqiter).second != (mqd_t)0 ) mq_getattr( (*tqiter).second, &qattributes );
      outs << "  Tid " << hex << (*tqiter).first << " " << hex << (long)(*tqiter).second
           // << "  flags " << qattributes.mq_flags
           // << "  size " << qattributes.mq_curmsgs
           // << "  maxsize " << qattributes.mq_maxmsg 
           << endmsg;
   }
   outs << " Message Gateway Map: size " << dec << _MessageGatewayMap.size();
   outs << endmsg;
   map< unsigned long, set< unsigned long > >::iterator mgiter;                   // _MessageGatewayMap;
   for ( mgiter  = _MessageGatewayMap.begin() ;
         mgiter != _MessageGatewayMap.end() ;
         mgiter++ )
   {
      outs << "  Mid " << hex << (*mgiter).first << " Gateway Set: size " << dec << (long)(*mgiter).second.size();
      set< unsigned long >::iterator giter;                                      // GatewaySet;
      for ( giter  = (*mgiter).second.begin() ;
            giter != (*mgiter).second.end() ;
            giter++ )
      {
         outs << "   Gateway address " << hex << (*giter);
      }
      outs << endmsg;
   }
   outs << " Inet Gateway Map: size " << dec << _InetGatewayMap.size();
   outs << endmsg;
   map< unsigned long, int >::iterator igiter;                            // _InetGatewayMap;
   for ( igiter  = _InetGatewayMap.begin() ;
         igiter != _InetGatewayMap.end() ;
         igiter++ )
   {
      outs << "  Address " << hex << (*igiter).first << " sock " << (*igiter).second << endmsg;
   }
   outs << " Gateway Connection Attempts Map: size " << dec << _GatewayConnSynchedMap.size();
   outs << endmsg;
   map< unsigned long, Router::GatewaySynched >::iterator gcaiter;        // _GatewayConnSynchedMap;
   for ( gcaiter  = _GatewayConnSynchedMap.begin() ;
         gcaiter != _GatewayConnSynchedMap.end() ;
         gcaiter++ )
   {
      outs << "  Address " << hex << (*gcaiter).first << " conn attempts: " << (*gcaiter).second << endmsg;
   }
   outs << " Spoofer Message Map: size " << dec << _SpooferMsgMap.size();
   outs << endmsg;
   map< unsigned long, unsigned long >::iterator smiter;                            // _SpooferMsgMap;
   for ( smiter  = _SpooferMsgMap.begin() ;
         smiter != _SpooferMsgMap.end() ;
         smiter++ )
   {
      outs << "  Mid " << hex << (*smiter).first << " Spoofer Tid " << hex << (*smiter).second << endmsg;
   }
   outs << " StopLoop " << dec << _StopLoop << endmsg;
   outs << "-------------------------------------------------------------------" << endmsg;

}

void Router::msgDump()
{
   DataLog( log_level_router_info ) << " Message Integrity Map: size " << dec << _MsgIntegrityMap.size() << endmsg;
   map< unsigned long, string >::iterator miiter;                                 // _MsgIntegrityMap;
   for ( miiter  = _MsgIntegrityMap.begin() ;
         miiter != _MsgIntegrityMap.end() ;
         miiter++ )
   {
      DataLog( log_level_router_info ) << "  Mid " << hex << (*miiter).first << " " << (*miiter).second;
   }
   DataLog( log_level_router_info ) << endmsg;
}

void Router::msgTaskDump()
{
   DataLog( log_level_router_info ) << " Message Task Map: size " << _MessageTaskMap.size();
   DataLog( log_level_router_info ) << endmsg;
   map< unsigned long, map< unsigned long, unsigned char > >::iterator mtiter;    // _MessageTaskMap;
   map< unsigned long, unsigned char >::iterator triter;                          
   for ( mtiter  = _MessageTaskMap.begin() ;
         mtiter != _MessageTaskMap.end() ;
         mtiter++ )
   {
      DataLog( log_level_router_info ) << "  Mid " << hex << (*mtiter).first;
      for ( triter  = ((*mtiter).second).begin() ;
            triter != ((*mtiter).second).end() ;
            triter++ )
      {
         DataLog( log_level_router_info ) << "    Tid " << hex << (*triter).first << " #regs " << (int)(*triter).second;
      }
      DataLog( log_level_router_info ) << endmsg;
   }
}

void Router::gatewayDump()
{
   DataLog( log_level_router_info ) << " Inet Gateway Map: size " << dec << _InetGatewayMap.size();
   DataLog( log_level_router_info ) << endmsg;
   map< unsigned long, int >::iterator igiter;                            // _InetGatewayMap;
   for ( igiter  = _InetGatewayMap.begin() ;
         igiter != _InetGatewayMap.end() ;
         igiter++ )
   {
      DataLog( log_level_router_info ) << "  Address " << hex << (*igiter).first << " sock " << (*igiter).second << endmsg;
   }
   DataLog( log_level_router_info ) << " Gateway Connection Attempts Map: size " << dec << _GatewayConnSynchedMap.size();
   DataLog( log_level_router_info ) << endmsg;
   map< unsigned long, Router::GatewaySynched >::iterator gcaiter;        // _GatewayConnSynchedMap;
   for ( gcaiter  = _GatewayConnSynchedMap.begin() ;
         gcaiter != _GatewayConnSynchedMap.end() ;
         gcaiter++ )
   {
      DataLog( log_level_router_info ) << "  Address " << hex << (*gcaiter).first << " conn attempts: " << (*gcaiter).second << endmsg;
   }
}

void Router::remoteMsgDump()
{
   DataLog( log_level_router_info ) << " Message to Gateway Synch Map: size " << dec << _MsgToGatewaySynchMap.size();
   DataLog( log_level_router_info ) << endmsg;
   map< unsigned long, set< unsigned long > >::iterator mtogiter;                                 // _MsgToGatewaySynchMap;
   for ( mtogiter  = _MsgToGatewaySynchMap.begin() ;
         mtogiter != _MsgToGatewaySynchMap.end() ;
         mtogiter++ )
   {
      DataLog( log_level_router_info ) << "  Mid " << hex << (*mtogiter).first;
      set< unsigned long >::iterator gateiter;
      for ( gateiter  = ((*mtogiter).second).begin() ;
            gateiter != ((*mtogiter).second).end() ;
            gateiter++ )
         DataLog( log_level_router_info ) << " " << (*gateiter);
      DataLog( log_level_router_info ) << endmsg;
   }
   DataLog( log_level_router_info ) << " Message Gateway Map: size " << dec << _MessageGatewayMap.size();
   DataLog( log_level_router_info ) << endmsg;
   map< unsigned long, set< unsigned long > >::iterator mgiter;                   // _MessageGatewayMap;
   for ( mgiter  = _MessageGatewayMap.begin() ;
         mgiter != _MessageGatewayMap.end() ;
         mgiter++ )
   {
      DataLog( log_level_router_info ) << "      Mid " << hex << (*mgiter).first 
                       << "(" << _MsgIntegrityMap[ (*mgiter).first ].c_str() << ") " 
                       << " Gateway Set: size " << dec << (long)(*mgiter).second.size();
      set< unsigned long >::iterator giter;                                      // GatewaySet;
      for ( giter  = (*mgiter).second.begin() ;
            giter != (*mgiter).second.end() ;
            giter++ )
      {
         DataLog( log_level_router_info ) << "   Gateway address " << hex << (*giter);
      }
      DataLog( log_level_router_info ) << endmsg;
   }
}

bool Router::initGateways()
{
   //
   // Spawn all gateways and give myself messages informing me to connect
   //  to the gateways ...
   short remoteport=0;
   unsigned long netAddress=0;
   char gateName[17];
   char connectorName[19];

   map< short, unsigned long > portAddressMap;
   map< short, unsigned long >::iterator paiter;

   //
   // Get the map of network connections
   getNetworkedNodes( portAddressMap );

   //
   // Connect to the other nodes ...
   for ( paiter = portAddressMap.begin() ;
         paiter != portAddressMap.end() ;
         paiter++ )
   {
      netAddress = (*paiter).second;
      if ( netAddress != getNetworkAddress() )
      {
         remoteport = (*paiter).first;
         sprintf( gateName, "tGateway%lx", netAddress );
         if ( spawnGateway( gateName, (FUNCPTR)Gateway::Gateway_main, remoteport ) == ERROR )
         {
            //
            // Error ...
            DataLog( log_level_critical ) << "Router init - could not spawn gateway for address -> " << hex << netAddress 
                                 << ", (" << errnoMsg << ")"
                                 << endmsg;
            _FATAL_ERROR( __FILE__, __LINE__, "Gateway spawn error" );
            return false;
         }
   
         int sock = ERROR;
         sprintf( connectorName, "tConnector%lx", netAddress );
         if ( spawnConnector( connectorName, (FUNCPTR)Connector_main, netAddress, sock ) == ERROR )
         {
            //
            // Error ...
            DataLog( log_level_critical ) << "Router init - could not spawn connector for address -> " << hex << netAddress 
                                 << ", (" << errnoMsg << ")"
                                 << endmsg;
            _FATAL_ERROR( __FILE__, __LINE__, "Connector spawn error" );
            return false;
         }
   
         _InetGatewayMap[ netAddress ] = sock;
         _GatewayConnSynchedMap[ netAddress ] = Router::NoConn;

      }
   }

   return true;
}

void Router::processMessage( MessagePacket &mp, int priority )
{
   //
   // Determine what type of message this is ...
   //
   switch ( mp.msgData().osCode() )
   {
   case MessageData::TASK_REGISTER:
      registerTask( mp.msgData().taskId(), (const char *)(mp.msgData().msg() ) );
      break;
   case MessageData::TASK_DEREGISTER:
      deregisterTask( mp.msgData().taskId() );
      break;
   case MessageData::MESSAGE_NAME_REGISTER:
      checkMessageId( mp.msgData().msgId(), (const char *)( mp.msgData().msg() ) );
      sendMessageToGateways( mp );
      break;
   case MessageData::MESSAGE_REGISTER:
      checkMessageId( mp.msgData().msgId(), (const char *)( mp.msgData().msg() ) );
      if ( mp.msgData().nodeId() == 0 ) 
         registerMessage( mp.msgData().msgId(), mp.msgData().taskId() );
      else
         registerMessageWithGateway( mp.msgData().msgId(), mp.msgData().nodeId() );
      sendMessageToGateways( mp );
      break;
   case MessageData::MESSAGE_DEREGISTER:
      checkMessageId( mp.msgData().msgId(), (const char *)( mp.msgData().msg() ) );
      if ( mp.msgData().nodeId() == 0 ) 
         deregisterMessage( mp.msgData().msgId(), mp.msgData().taskId() );
      else
         deregisterMessageWithGateway( mp.msgData().msgId(), mp.msgData().nodeId() );
      break;
   case MessageData::GATEWAY_CONNECT:
      connectWithGateway( mp );
      break;
   case MessageData::GATEWAY_DISCONNECT:
      disconnectWithGateway( mp.msgData().nodeId() );
      break;
   case MessageData::GATEWAY_MESSAGE_SYNCH_BEGIN:
      if ( _GatewayConnSynchedMap[ mp.msgData().nodeId() ] == Router::NoConn )
         _GatewayConnSynchedMap[ mp.msgData().nodeId() ] = Router::Connected;
      else if ( _GatewayConnSynchedMap[ mp.msgData().nodeId() ] == Router::Connected )
      {
         _GatewayConnSynchedMap[ mp.msgData().nodeId() ] = Router::Inprogress;
         synchUpRemoteNode( mp.msgData().nodeId() );
      }
      DataLog( log_level_router_info ) << "Gateway (" << hex << mp.msgData().nodeId()
                    << ") connection status : " << dec << gatewayConnStatus( mp.msgData().nodeId() )
                    << endmsg;
      break;
   case MessageData::GATEWAY_MESSAGE_SYNCH:
      synchUpRemoteNode( mp.msgData().nodeId() );
      break;
   case MessageData::GATEWAY_MESSAGE_SYNCH_COMPLETE:
      if ( _GatewayConnSynchedMap[ mp.msgData().nodeId() ] == Router::LocalComplete )
         _GatewayConnSynchedMap[ mp.msgData().nodeId() ] = Router::Synched;
      else if ( _GatewayConnSynchedMap[ mp.msgData().nodeId() ] == Router::Inprogress )
         _GatewayConnSynchedMap[ mp.msgData().nodeId() ] = Router::RemoteComplete;
      DataLog( log_level_router_info ) << "Gateway (" << hex << mp.msgData().nodeId()
                    << ") synchronization status : " << dec << gatewayConnStatus( mp.msgData().nodeId() ) 
                    << endmsg;
      break;
   case MessageData::SPOOF_MSG_REGISTER:
      checkMessageId( mp.msgData().msgId(), (const char *)( mp.msgData().msg() ) );
      registerSpooferMessage( mp.msgData().msgId(), mp.msgData().taskId() );
      break;
   case MessageData::SPOOF_MSG_DEREGISTER:
      checkMessageId( mp.msgData().msgId(), (const char *)( mp.msgData().msg() ) );
      deregisterSpooferMessage( mp.msgData().msgId() );
      break;
   case MessageData::DISTRIBUTE_GLOBALLY:
   case MessageData::DISTRIBUTE_LOCALLY:
   case MessageData::SPOOFED_GLOBALLY:
   case MessageData::SPOOFED_LOCALLY:
      checkMessageId( mp.msgData().msgId() );
      sendMessage( mp, priority );
      break;
   default:
      _FATAL_ERROR( __FILE__, __LINE__, "Unknown OSCode in message packet" );
      break;
   }

}

void Router::connectWithGateway( const MessagePacket &mp )
{
   //
   // Find gateway in list ...
   map< unsigned long, int >::iterator sockiter;
   sockiter = _InetGatewayMap.find( mp.msgData().nodeId() );

   //
   // If found ...
   //
   if ( sockiter != _InetGatewayMap.end() )
   {
      close( (*sockiter).second );

      int sock;
      memmove( (char *) &sock , 
               (char *) mp.msgData().msg(), 
               sizeof( sock ) );
      _InetGatewayMap[ mp.msgData().nodeId() ] = sock;

      int optval=1;
      setsockopt( sock, SOL_SOCKET, SO_KEEPALIVE, (char*)&optval, sizeof(optval) );
      setsockopt( sock, IPPROTO_TCP, TCP_NODELAY, (char*)&optval, sizeof(optval) );

      //
      // Synch up with the remote node ...
      if ( _GatewayConnSynchedMap[ mp.msgData().nodeId() ] == Router::NoConn )
         _GatewayConnSynchedMap[ mp.msgData().nodeId() ] = Router::Connected;
      else if ( _GatewayConnSynchedMap[ mp.msgData().nodeId() ] == Router::Connected )
      {
         _GatewayConnSynchedMap[ mp.msgData().nodeId() ] = Router::Inprogress;
         synchUpRemoteNode( mp.msgData().nodeId() );
      }

      //
      // Notify the other node that we are connected ...
      MessagePacket newMP( mp );
      newMP.msgData().osCode( MessageData::GATEWAY_MESSAGE_SYNCH_BEGIN );
      sendMessageToGateway( mp.msgData().nodeId(), newMP );

      DataLog( log_level_router_info ) << "Gateway (" << hex << mp.msgData().nodeId()
                       << ") connection status : " << dec << gatewayConnStatus( mp.msgData().nodeId() )
                       << endmsg;

   }
   else
   {
      //
      // ... error, the gateway has not already established a socket
      //
      // Error ...
      DataLog( log_level_critical ) << "Connect with gateway=" << hex << mp.msgData().nodeId() << " - no socket found" << endmsg;
      _FATAL_ERROR( __FILE__, __LINE__, "Gateway connect failed" );
   }
}

void Router::disconnectWithGateway( unsigned long address )
{
   //
   // Find gateway in list ...
   map< unsigned long, int >::iterator sockiter;
   sockiter = _InetGatewayMap.find( address );

   //
   // If found ...
   //
   if ( sockiter != _InetGatewayMap.end() )
   {
      //
      // disconnect the socket interface
      close((*sockiter).second);

      //
      // remove the entry from the list
      _InetGatewayMap.erase( sockiter );
   }
}

void Router::registerTask( unsigned long tId, const char *qName )
{
   //
   // Try to find the task in the list ...
   if ( _TaskQueueMap.find( tId ) == _TaskQueueMap.end() )
   {
      //
      // The task is not registered, 
      //  so try to open the given queue ...
      mqd_t tQueue = (mqd_t)ERROR;
      unsigned int retries=0;
      while ( ( tQueue = mq_open( qName, O_RDWR ) ) == (mqd_t)ERROR 
              && ++retries < MessageSystemConstant::MAX_NUM_RETRIES ) nanosleep( &MessageSystemConstant::RETRY_DELAY, 0 );

      //
      // If opened ...
      if ( tQueue != (mqd_t)ERROR )
      {
         //
         // ... add task and queue to map ...
         _TaskQueueMap[ tId ] = tQueue;

      }
      //
      // if not opened ...
      else
      {
         //
         // ... error, the task should already have opened the queue
         //  before connecting to the router.
         //
         // Error ...
         DataLog( log_level_critical ) << "Register task=" << hex << tId << "(" << taskName( tId ) << ") - message queue open failed" 
                              << endmsg;
         _FATAL_ERROR( __FILE__, __LINE__, "mq_open failed" );
      }
   }

}

void Router::deregisterTask( unsigned long tId )
{
   //
   // Find the task in the list ...
   map< unsigned long, mqd_t >::iterator titer;
   titer = _TaskQueueMap.find( tId );
   if ( titer != _TaskQueueMap.end() )
   {
      //
      // If found ...  
      //    remove the task and close the queue
      // 
      mq_close( (mqd_t)(*titer).second );
      _TaskQueueMap.erase( titer );

      //
      // Remove the entry from the Message-Task map ...
      //
      //  iterate over the message-task map ...
      map< unsigned long, map< unsigned long, unsigned char > >::iterator mtiter;
      for ( mtiter = _MessageTaskMap.begin() ; 
            mtiter != _MessageTaskMap.end() ;
            mtiter++ )
      {
         //
         //    find the task in the message's task list
         map< unsigned long, unsigned char>::iterator titer;
         titer = (*mtiter).second.find( tId );

         //
         // If task found ...
         if ( titer != (*mtiter).second.end() )
         {
            //
            // decrement the task's number of registrations
            (*titer).second = 1;
            
            //
            // deregister the message formerly ...
            deregisterMessage( (*mtiter).first, tId );
         }
      }

      //
      // Remove any entry from the Spoofer-Message map ...
      //
      //  iterate over the spoofer-message map ...
      map< unsigned long, unsigned long >::iterator smiter;
      for ( smiter = _SpooferMsgMap.begin() ; 
            smiter != _SpooferMsgMap.end() ;
            smiter++ )
      {
         //
         // erase the entry if it relates to this task ...
         if ( (*smiter).second == tId )
            _SpooferMsgMap.erase( smiter );
      }
   }
}

void Router::checkMessageId( unsigned long msgId, const char *mname )
{
   //
   // Find the message Id in the list ...
   map< unsigned long, string >::iterator miter;
   miter = _MsgIntegrityMap.find( msgId );

   //
   // If message found ...
   if ( miter != _MsgIntegrityMap.end() )
   {
      //
      //    verify that the message Id and the message name are the
      //    same as the passed in data ...
      //
      if ( (*miter).second != mname )
      {
         //
         // Error ...
         DataLog( log_level_critical ) << "Check message Id - Message name=" << mname << " to Id=" << hex << msgId << " integrity check failed" << endmsg;
         _FATAL_ERROR( __FILE__, __LINE__, "Message integrity check failed" );
      }
   }
   //
   // else the message was not found ...
   else
   {
      //
      //    add the message Id and the message name to the list...
      //
      _MsgIntegrityMap[ msgId ]      = mname;

      //
      // Create an entry to synchronize with the other gateways ...
      set< unsigned long > gSet;
      _MsgToGatewaySynchMap[ msgId ] = gSet;
   } 

}

void Router::checkMessageId( unsigned long msgId )
{
   //
   // Find the message Id in the list ...
   map< unsigned long, string >::iterator miter;
   miter = _MsgIntegrityMap.find( msgId );

   //
   // If message found ...
   if ( miter == _MsgIntegrityMap.end() )
   {
      //
      // Error ...
      DataLog( log_level_critical ) << "Check message Id - Message Id=" << hex << msgId << " unregistered use in message system" << endmsg;
      _FATAL_ERROR( __FILE__, __LINE__, "Message integrity check failed" );
   }
}

void Router::registerMessage( unsigned long msgId, unsigned long tId )
{
   //
   // Find the message Id in the list ...
   map< unsigned long, map< unsigned long, unsigned char > >::iterator miter;
   miter = _MessageTaskMap.find( msgId );

   //
   // If message found ...
   if ( miter != _MessageTaskMap.end() )
   {
      //
      //    find the task in the message's task list
      map< unsigned long, unsigned char >::iterator titer;
      titer = (*miter).second.find( tId );

      //
      // If task found ...
      if ( titer != (*miter).second.end() )
      {
         //
         // increment the task's number of registrations
         (*titer).second += 1;
      }
      //
      // else task not found
      else
      {
         //
         // add the task to the message's task list with the 
         //  number of registrations set to 1.
         ( (*miter).second )[ tId ] = 1;
      }
   }
   //
   // else message not found ...
   else
   {
      //
      // add the task to a new task map with the
      //  number of registrations set to 1 ...
      map< unsigned long, unsigned char > tMap;
      tMap[ tId ] = 1;

      //
      // ... then add the new task map to the message map.
      _MessageTaskMap[ msgId ] = tMap;
   }
}

void Router::registerMessageWithGateway( unsigned long msgId, unsigned long nodeId )
{
   //
   // Find the message Id in the list ...
   map< unsigned long, set< unsigned long > >::iterator miter;
   miter = _MessageGatewayMap.find( msgId );

   //
   // If message found ...
   if ( miter != _MessageGatewayMap.end() )
   {
      //
      //    find the gateway in the message's gateway list
      set< unsigned long >::iterator giter;
      giter = (*miter).second.find( nodeId );

      //
      // If gateway not found ...
      if ( giter == (*miter).second.end() )
      {
         //
         // add the gateway to the message's gateway list.
         ( (*miter).second ).insert( nodeId );
      }
   }
   //
   // else message not found ...
   else
   {
      //
      // add the gateway to a new gateway set ...
      set< unsigned long > gSet;
      gSet.insert( nodeId );

      //
      // ... then add the new task map to the message map.
      _MessageGatewayMap[ msgId ] = gSet;
   }

}

void Router::registerSpooferMessage( unsigned long msgId, unsigned long tId )
{
   DataLog( log_level_router_info ) << "Spoofer task " << hex << tId 
                                    << " registered for message " << hex << msgId << endmsg;

   //
   // Add the message Id to the map ...
   _SpooferMsgMap[ msgId ] = tId;      
   
}

void Router::deregisterMessage( unsigned long msgId, unsigned long tId )
{

   //
   // Decrement the task registrations from the message's task list ...
   //  remove the task if the number of registrations for the task goes
   //  to 0.

   //
   // Find the message Id in the list ...
   map< unsigned long, map< unsigned long, unsigned char > >::iterator miter;
   miter = _MessageTaskMap.find( msgId );

   //
   // If message found ...
   if ( miter != _MessageTaskMap.end() )
   {
      //
      //    find the task in the message's task list
      map< unsigned long, unsigned char>::iterator titer;
      titer = (*miter).second.find( tId );

      //
      // If task found ...
      if ( titer != (*miter).second.end() )
      {
         //
         // decrement the task's number of registrations
         (*titer).second -= 1;

         if ( (*titer).second <= 0 )
         {
            //
            // remove the task from the message list ...
            (*miter).second.erase( titer );
         }
      }

      if ( (*miter).second.size() == 0 )
      {
         //
         // Since the message no longer has any tasks listening for it, 
         //  deregister this task with the remote gateways.
         MessagePacket mp;
         mp.msgData().osCode( MessageData::MESSAGE_DEREGISTER );
         mp.msgData().msgId( msgId );
         string messageName( _MsgIntegrityMap[ msgId ] );
         mp.msgData().msg( (const unsigned char *)messageName.c_str(), messageName.length() );

         sendMessageToGateways( mp ); 
      }
   }

}

void Router::deregisterMessageWithGateway( unsigned long msgId, unsigned long nodeId )
{
   //
   // Find the message Id in the list ...
   map< unsigned long, set< unsigned long > >::iterator miter;
   miter = _MessageGatewayMap.find( msgId );

   //
   // If message found ...
   if ( miter != _MessageGatewayMap.end() )
   {
      //
      //  find the gateway in the message's gateway list
      set< unsigned long >::iterator giter;
      giter = (*miter).second.find( nodeId );

      //
      // If gateway found ...
      if ( giter != (*miter).second.end() )
      {
         //
         // remove the gateway from the message's list ...
         (*miter).second.erase( giter );
      }

      //
      // If no gateways are registered for this message ...
      if ( (*miter).second.size() == 0 )
      {
         //
         // remove the message from the list ...
         _MessageGatewayMap.erase( miter );
      }
   }
   
}

void Router::deregisterSpooferMessage( unsigned long msgId)
{
   DataLog( log_level_router_info ) << "Deregistering Spoofer for message " << hex << msgId << endmsg;

   //
   // Find the message Id in the list ...
   map< unsigned long, unsigned long >::iterator mtiter;
   mtiter = _SpooferMsgMap.find( msgId );

   //
   // If message found ...
   if ( mtiter != _SpooferMsgMap.end() )
   {
      //
      // Delete the entry from the map ...
      _SpooferMsgMap.erase( mtiter );
   }
}

void Router::sendMessage( const MessagePacket &mp, int priority )
{
	//
	// Send message info to debug handler
	//
	DBG_LogSentMessage(taskIdSelf(), (int)mp.msgData().osCode(), mp.msgData().msgId());

   //
   // Check for spoofer ...
   if ( sendMessageToSpoofer( mp, priority ) == true )
      //
      // The spoofer is spoofing this message, so don't send
      //  it to the normal subscribers ...
      return;

   //
   // Distribute the message to the local tasks
   unsigned long msgId = mp.msgData().msgId();

   //
   // Find the message Id in the list ...
   map< unsigned long, map< unsigned long, unsigned char > >::iterator miter;
   miter = _MessageTaskMap.find( msgId );

   //
   // If message found ...
   if ( miter != _MessageTaskMap.end() )
   {
      //
      // Send to all tasks in the message's task list
      map< unsigned long, unsigned char >::iterator titer;
      map< unsigned long, mqd_t >::iterator tqiter;
      for ( titer  = (*miter).second.begin() ;
            titer != (*miter).second.end() ;
            titer++ )
      {
         //
         // Get the task out of the registered task/queue map ...
         tqiter = _TaskQueueMap.find( (*titer).first );

         if ( tqiter == _TaskQueueMap.end() )
         {
            //
            // Error ...
            DataLog( log_level_critical ) << "Sending message=" << hex << mp.msgData().msgId() 
                                 << "(" << _MsgIntegrityMap[ mp.msgData().msgId() ].c_str() << ") " 
                                 << "- Task Id=" << (*titer).first 
                                 << "(" << taskName( (*titer).first ) << ")"
                                 << " not found in task list" << endmsg;
            _FATAL_ERROR( __FILE__, __LINE__, "Task lookup failed" );
         }
         //
         // If the task was found to be registered ...
         else
         {
            sendMessage( mp, (*tqiter).second, (*tqiter).first, priority );
         }

      }
   }

   //
   // ... and if the message isn't supposed to be just local, distribute
   //  the message over the socket connections to the gateways...
   //
   sendMessageToGateways( mp );
}

void Router::sendMessage( const MessagePacket &mp, mqd_t mqueue, const unsigned long tId, int priority )
{
   //
   // Check the task's queue to see if it is full or not ...
   mq_attr qattributes;
   if (    mq_getattr( mqueue, &qattributes ) == ERROR
        || qattributes.mq_curmsgs >= qattributes.mq_maxmsg )
   {
      //
      // The task's queue is full!
      //
      // Error ...
      DataLog( log_level_critical ) << "Sending message=" << hex << mp.msgData().msgId() 
                           << "(" << _MsgIntegrityMap[ mp.msgData().msgId() ].c_str() << ") " 
                           << " - Task Id=" << tId 
                           << "(" << taskName( tId ) << ")"
                           << " queue full (" << dec << qattributes.mq_curmsgs << " messages)" 
                           << ", (" << errnoMsg << ")"
                           << endmsg;
      dumpQueue( tId, mqueue, DataLog( log_level_router_error ) );

#if !DEBUG_BUILD && CPU != SIMNT
      _FATAL_ERROR( __FILE__, __LINE__, "Message queue full" );
#endif // #if BUILD_TYPE!=DEBUG && CPU!=SIMNT

   }

   //
   // Send message to the task ...
   unsigned int retries=0;
   while (    mq_send( mqueue , &mp, sizeof( MessagePacket ), priority ) == ERROR 
           && ++retries < MessageSystemConstant::MAX_NUM_RETRIES ) 
      nanosleep( &MessageSystemConstant::RETRY_DELAY, 0 );
   if ( retries == MessageSystemConstant::MAX_NUM_RETRIES )
   {
      //
      // Error ...
      DataLog( log_level_critical ) << "Sending message=" << hex << mp.msgData().msgId() 
                           << "(" << _MsgIntegrityMap[ mp.msgData().msgId() ].c_str() << ") " 
                           << " - Task Id=" << tId 
                           << "(" << taskName( tId ) << ")"
                           << " send failed" 
                           << ", (" << errnoMsg << ")"
                           << endmsg;
      _FATAL_ERROR( __FILE__, __LINE__, "mq_send failed" );
   }
}

void Router::sendMessageToGateways( const MessagePacket &mpConst )
{    
   //
   // Only send message packets to the other nodes if they contain
   //  these osCodes ...
   if (    (    mpConst.msgData().osCode() == MessageData::DISTRIBUTE_GLOBALLY
             || mpConst.msgData().osCode() == MessageData::SPOOFED_GLOBALLY
             || mpConst.msgData().osCode() == MessageData::MESSAGE_NAME_REGISTER
             || mpConst.msgData().osCode() == MessageData::MESSAGE_REGISTER
             || mpConst.msgData().osCode() == MessageData::MESSAGE_DEREGISTER 
             || mpConst.msgData().osCode() == MessageData::GATEWAY_MESSAGE_SYNCH_COMPLETE )
        && mpConst.msgData().nodeId() == 0 )
   {
      map< unsigned long, int >::iterator sockiter;
      //
      // Send message packets with these osCodes to the nodes which have
      //  subscribers for this message packet's message Id ...
      if (    mpConst.msgData().osCode() == MessageData::DISTRIBUTE_GLOBALLY
           || mpConst.msgData().osCode() == MessageData::SPOOFED_GLOBALLY )
      {
         map< unsigned long, set< unsigned long > >::iterator mgiter;
         mgiter = _MessageGatewayMap.find( mpConst.msgData().msgId() );
         if ( mgiter != _MessageGatewayMap.end() )
         {
            set< unsigned long >::iterator gatewayiter;
            for ( gatewayiter = (*mgiter).second.begin() ;
                  gatewayiter != (*mgiter).second.end() ;
                  gatewayiter++ )
            {
               sockiter = _InetGatewayMap.find( (*gatewayiter) );
               if ( sockiter != _InetGatewayMap.end() )
               {
                  //
                  // Send the message to the gateway ...
                  sendMessageToGateway( ((*sockiter).first), mpConst );
               }
            }
         }
      }
      //
      // ... send remaining types of osCodes to all nodes ...
      else
      {
         for ( sockiter = _InetGatewayMap.begin() ;
               sockiter != _InetGatewayMap.end() ;
               sockiter++ )
         {
            //
            // Check for socket connection status.
            if (    _GatewayConnSynchedMap[ (*sockiter).first ] != Router::NoConn 
                 && (*sockiter).second != ERROR )
            {
               //
               // Save the fact that we registered this message with this node ...
               if (    mpConst.msgData().osCode() == MessageData::MESSAGE_NAME_REGISTER
                    || mpConst.msgData().osCode() == MessageData::MESSAGE_REGISTER )
               {
                  _MsgToGatewaySynchMap[ mpConst.msgData().msgId() ].insert( (*sockiter).first );
               }
   
               //
               // Send the message to the gateway ...
               sendMessageToGateway( ((*sockiter).first), mpConst );
            }
         }
      }
   } 
   else if (    (    mpConst.msgData().osCode() == MessageData::MESSAGE_NAME_REGISTER
                  || mpConst.msgData().osCode() == MessageData::MESSAGE_REGISTER )
             && mpConst.msgData().nodeId() != 0 )
   {
      //
      // Try to find the message Id in the message/task map ...
      map< unsigned long, map< unsigned long, unsigned char > >::iterator mtiter;
      mtiter = _MessageTaskMap.find( mpConst.msgData().msgId() );
      
      //
      // If message found , but not tasks ...
      if ( mtiter == _MessageTaskMap.end() )
         _MsgToGatewaySynchMap[ mpConst.msgData().msgId() ].insert( mpConst.msgData().nodeId() );
   }
}

void Router::sendMessageToGateway( unsigned long nodeId, const MessagePacket &mpConst )
{
   if (    mpConst.msgData().osCode() != MessageData::MESSAGE_REGISTER
        && mpConst.msgData().osCode() != MessageData::MESSAGE_NAME_REGISTER 
        && mpConst.msgData().osCode() != MessageData::GATEWAY_MESSAGE_SYNCH_BEGIN 
        && mpConst.msgData().osCode() != MessageData::GATEWAY_MESSAGE_SYNCH_COMPLETE )
   {
      if ( _GatewayConnSynchedMap[ nodeId ] != Router::Synched )
         return;
   }

   int sock = _InetGatewayMap[ nodeId ];
   if ( sock != ERROR )
   {
      MessagePacket mp( mpConst );
      //
      // Assign the message packet this nodes network address
      mp.msgData().netSequenceNum( _NetSequenceNum++ );
      mp.msgData().nodeId( getNetworkAddress() );
      mp.updateCRC();
                            
      //
      // Form the message
      unsigned long tmp      = 0xeeeeeeee;
      unsigned long crc      = mp.crc();
      unsigned long msgSize  = mp.msgData().sizeOfData();
      int           pcktSize = msgSize + 3*sizeof( unsigned long );
      unsigned char tcpMsg[ sizeof( MessagePacket ) + ( 4*sizeof( unsigned long ) ) ];
      memmove( (void*)   tcpMsg                                        , &pcktSize, sizeof( int ) );
      memmove( (void*)&( tcpMsg[             sizeof( unsigned long ) ]), &tmp     , sizeof( unsigned long ) );
      memmove( (void*)&( tcpMsg[           2*sizeof( unsigned long ) ]), &mp      , msgSize );
      memmove( (void*)&( tcpMsg[ msgSize + 2*sizeof( unsigned long ) ]), &crc     , sizeof( unsigned long ) );
      memmove( (void*)&( tcpMsg[ msgSize + 3*sizeof( unsigned long ) ]), &tmp     , sizeof( unsigned long ) );

      int len = pcktSize + sizeof( int );
      int wlen=0;
	   char * cbuf = (char *)tcpMsg;
      while ( len > 0 )
      {
         int wval;
         if ( ( wval = send( sock, &cbuf[wlen], len, 0 ) ) == ERROR )
         {
            //
            // Error ...
            DataLog( log_level_critical ) << "Sending message=" << hex << mp.msgData().msgId() 
                                 << "(" << _MsgIntegrityMap[ mp.msgData().msgId() ].c_str() << ") " 
                                 << " - Gateway=" << nodeId << " send failed" 
                                 << ", (" << errnoMsg << ")"
                                 << endmsg;
            _FATAL_ERROR( __FILE__, __LINE__, "socket send failed" );
         }

         len -= wval;
         wlen += wval;
      }
   }
   else
   {
      //
      // Error ...                                                               
      DataLog( log_level_router_error ) << "Error sending to gateway=" << hex << nodeId
                                        << ", TCP socket not found." << endmsg;
   }
}

bool Router::sendMessageToSpoofer( const MessagePacket &mp, int priority )
{
   //
   // We can only spoof message packets which have these osCodes ...
   if (    mp.msgData().osCode() == MessageData::DISTRIBUTE_GLOBALLY 
        || mp.msgData().osCode() == MessageData::DISTRIBUTE_LOCALLY )
   {
      map< unsigned long, unsigned long >::iterator mtiter;
      for ( mtiter = _SpooferMsgMap.begin() ;
            mtiter != _SpooferMsgMap.end() ;
            mtiter++ )
      {
         if ( (*mtiter).first == mp.msgData().msgId() )
         {
            map< unsigned long, mqd_t >::iterator tqiter;
            tqiter = _TaskQueueMap.find( (*mtiter).second );
            if ( tqiter == _TaskQueueMap.end() )
            {
               //
               // Error ...
               DataLog( log_level_critical ) << "Sending message=" << hex << (*mtiter).first 
                                    << "(" << _MsgIntegrityMap[ (*mtiter).first ].c_str() << ") " 
                                    << " - Spoofer Task Id=" << (*mtiter).second 
                                    << "(" << taskName( (*mtiter).second ) << ")"
                                    << " not found in task list" << endmsg;
               _FATAL_ERROR( __FILE__, __LINE__, "Spoofer task lookup failed" );
               break;
            }
            
            sendMessage( mp, (*tqiter).second, (*tqiter).first, priority );
            //
            // We only allow one spoofer to register for each message Id ...
            return true;
         }
      }
   }

   return false;
}

void Router::synchUpRemoteNode( unsigned long nodeId )
{
   //
   // Find the first message in the list that has not 
   //  been synched yet.  Synchronize that message and
   //  send a message to myself to do that again.
   map< unsigned long, set< unsigned long > >::iterator mtogiter;
   for ( mtogiter  = _MsgToGatewaySynchMap.begin() ;
         mtogiter != _MsgToGatewaySynchMap.end() ;
         mtogiter++ )
   {
      set< unsigned long >::iterator giter;
      giter = ((*mtogiter).second).find( nodeId );
      if ( giter == ((*mtogiter).second).end() )
      {
         synchUpRemoteNode( nodeId, (*mtogiter).first );
         ((*mtogiter).second).insert( nodeId );

         MessagePacket mp;
         mp.msgData().osCode( MessageData::GATEWAY_MESSAGE_SYNCH );
         mp.msgData().msgId( 0 );
         mp.msgData().nodeId( nodeId );
         mp.updateCRC();

         sendMessage( mp, _RouterQueue, taskIdSelf(), MessageSystemConstant::REMOTE_NODE_SYNCH_PRIORITY );
         return;
      }
   }

   if ( _GatewayConnSynchedMap[ nodeId ] == Router::RemoteComplete )
      _GatewayConnSynchedMap[ nodeId ] = Router::Synched;
   else if ( _GatewayConnSynchedMap[ nodeId ] == Router::Inprogress )
      _GatewayConnSynchedMap[ nodeId ] = Router::LocalComplete;

   //
   // Send the message to say that my side of the message synching is complete ...
   MessagePacket mp;
   mp.msgData().osCode( MessageData::GATEWAY_MESSAGE_SYNCH_COMPLETE );
   mp.msgData().msgId( 0 );
   DataLog( log_level_router_info ) << "Gateway (" << hex << nodeId
                 << ") synchronization status : " << dec << gatewayConnStatus( nodeId )
                 << endmsg;

   sendMessageToGateway( nodeId, mp );
}

void Router::synchUpRemoteNode( unsigned long nodeId, unsigned long msgId )
{
   //
   // Send given msgId message to the remote node ...
   //
   map< unsigned long, map< unsigned long, unsigned char > >::iterator mtiter;
   map< unsigned long, string >::iterator miter;

   //
   // Find the message Id in the list ...
   miter = _MsgIntegrityMap.find( msgId );
   if ( miter != _MsgIntegrityMap.end() )
   {
      MessagePacket mp;
      mp.msgData().msgId( (*miter).first );
      mp.msgData().msg( (unsigned char*)((*miter).second.c_str()), (int)((*miter).second.length()) );
   
      //
      // Try to find the message Id in the message/task map ...
      mtiter = _MessageTaskMap.find( (*miter).first );
      
      //
      // If message found ...
      if ( mtiter != _MessageTaskMap.end() )
         mp.msgData().osCode( MessageData::MESSAGE_REGISTER );
      else
         mp.msgData().osCode( MessageData::MESSAGE_NAME_REGISTER );
   
      //
      // Send the packet to the remote gateway ...
      sendMessageToGateway( nodeId, mp );
   }
}

const char *Router::gatewayConnStatus( unsigned long nodeId )
{
   GatewaySynched status = _GatewayConnSynchedMap[ nodeId ];
   switch ( status )
   {
      case Router::NoConn :
         return "Not Connected";
      case Router::Connected :
         return "One-way connection established";
      case Router::Inprogress :
         return "Fully connected, synchronization in progress";
      case Router::LocalComplete :
         return "Local message have been synchronized with the remote node";
      case Router::RemoteComplete :
         return "Remote node messages have been synchronized";
      case Router::Synched :
         return "Local and Remote node synchronizations complete";
      default :
         return "Status unknown";
   };
}


void Router::dumpQueue( unsigned long tId, mqd_t mqueue, DataLog_Stream &out )
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
                                                                      << "(" << _MsgIntegrityMap[ mp.msgData().msgId() ].c_str() << ") " 
                                                                      << " p# "    << hex << mp.msgData().seqNum()
                                                                      << " tot# "  << hex << mp.msgData().totalNum()  
                                                                      << " msg -> ";
      for (int i=0;i<30;i++) 
      {
         out << hex << (int)(unsigned char)buffer[i] << " "; 
      }
      out << endmsg;
#endif // #if BUILD_TYPE!=DEBUG && CPU!=SIMNT

   }
   mq_setattr( mqueue, &old_attr, 0 );
}

void Router::shutdown()
{
   //
   // Close my queues ...
   if ( _RouterQueue != (mqd_t)0 )
   {
      mq_close( _RouterQueue );
      _RouterQueue = (mqd_t)0;
   }
   if ( _TimerQueue != (mqd_t)0 )
   {
      mq_close( _TimerQueue );
      _TimerQueue = (mqd_t)0;
   }

   //
   // Close the other queues ...
   map< unsigned long, mqd_t >::iterator qiter;
   for ( qiter = _TaskQueueMap.begin() ;
         qiter != _TaskQueueMap.end() ;
         qiter++ )
   {
      mq_close( (*qiter).second );
      (*qiter).second = (mqd_t)0;
   }

   //
   // Close the socket connections ...
   map< unsigned long, int >::iterator sockiter;
   for ( sockiter = _InetGatewayMap.begin() ;
         sockiter != _InetGatewayMap.end() ;
         sockiter++ )
   {
      close((*sockiter).second);
      (*sockiter).second = ERROR;
   }
}

void Router::cleanup()
{
   //
   // Remove the delete hook ...
   taskDeleteHookDelete( (FUNCPTR) &Router::taskDeleteHook );

   //
   // Cleanup the map ...
   //  ( no dynamic memory was created, so doing
   //    a clear will clean the map fine )
   _MsgIntegrityMap.clear();
   _MsgToGatewaySynchMap.clear();
   _MessageTaskMap.clear();
   _TaskQueueMap.clear();
   _MessageGatewayMap.clear();
   _InetGatewayMap.clear();
   _SpooferMsgMap.clear();

   _TheRouter = 0;
   _TheRouterTid = 0;
}

void routerInit()
{
   Router::Router_main();
}

void routerDump()
{
	log_level_router_info.logOutput(DataLog_LogDisabled);
	log_level_router_info.consoleOutput(DataLog_ConsoleEnabled);

   if ( Router::globalRouter() )
      Router::globalRouter()->dump( DataLog( log_level_router_info ) );
}

void msgDump()
{
   if ( Router::globalRouter() )
      Router::globalRouter()->msgDump();
}

void msgTaskDump()
{
   if ( Router::globalRouter() )
      Router::globalRouter()->msgTaskDump();
}

void gatewayDump()
{
   if ( Router::globalRouter() )
      Router::globalRouter()->gatewayDump();
}

void remoteMsgDump()
{
   if ( Router::globalRouter() )
      Router::globalRouter()->remoteMsgDump();
}

