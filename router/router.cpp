/*
 * Copyright (c) 2002 Gambro BCT, Inc.  All rights reserved.
 *
 * TITLE:      router.cpp, Message Router.
 *
 */

#include <vxWorks.h>

#include <errnoLib.h>
#include <stdio.h>
#include <taskHookLib.h>

#include "datalog.h"
#include "error.h"
#include "gateway.h"
#include "messagesystem.h"
#include "messagesystemconstant.h"
#include "router.h"
#include "systemoverrides.h"
#include "tcpconnect.h"


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

int Router::taskCreateHook( WIND_TCB *pTcb )
{
   MessageSystem::MsgSystem( 0 );

   return 1;
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
   _TimerQueue( 0 ),
   _MsgIntegrityMap(),
   _MessageTaskMap(),
   _TaskQueueMap(),
   _InetGatewayMap(),
   _GatewayConnAtmptMap(),
   _SpooferMsgMap(),
   _StopLoop( false )
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
   // Add the task create to initialize tasks correctly and delete hooks 
   //  to catch all task deletion and keep the task lists up to date ...
   taskCreateHookAdd( (FUNCPTR) &Router::taskCreateHook );
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
           && retries++ < MessageSystemConstant::MAX_NUM_RETRIES ) 
      nanosleep( &MessageSystemConstant::RETRY_DELAY, 0 );
   if ( _RouterQueue == (mqd_t)ERROR )
   {
      //
      // Error ...
      DataLog_Critical criticalLog;
      DataLog(criticalLog) << "Router message queue open failed" << endmsg;
      _FATAL_ERROR( __FILE__, __LINE__, "Router message queue open failed" );
      return false;
   }

   //
   // Connect up with our timer task ...
   //  don't do anything else until we connnect.
   while ( ( _TimerQueue = mq_open( "timertask", O_WRONLY ) ) == (mqd_t)ERROR ) 
      nanosleep( &MessageSystemConstant::RETRY_DELAY, 0 );

   //
   // If not opened ...
   if ( _TimerQueue == (mqd_t)ERROR )
   {
      //
      // Error ...
      DataLog_Critical criticalLog;
      DataLog(criticalLog) << "Timer message queue open failed" << endmsg;
      _FATAL_ERROR( __FILE__, __LINE__, "Timer message queue open failed" );
      return false;
   }

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
              && retries++ < MessageSystemConstant::MAX_NUM_RETRIES ) 
      {
         cout << "mq_receive failure - sleep for 1 second" << endl;
         nanosleep( &MessageSystemConstant::RETRY_DELAY, 0 );
      }
      if ( size == ERROR || size == 0 )
      {
         //
         // Error ...
         int errorNo = errnoGet();
         DataLog_Critical criticalLog;
         DataLog(criticalLog) << "Dispatching message - mq_receive return size=" << size
                              << " and (" << strerror( errorNo ) << ")" << endmsg;
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
         DataLog_Critical criticalLog;
         DataLog(criticalLog) << "Dispatching message - message CRC validation failed for MsgId=" << hex << mp.msgData().msgId() 
                              << ", CRC=" << crc << " and should be " << mp.crc() << endmsg;
         _FATAL_ERROR( __FILE__, __LINE__, "CRC check failed" );
      }  

      processMessage( mp, priority );

   } while ( _StopLoop == false );

}

void Router::dump( ostream &outs )
{
   outs << "------------------------- Router DUMP -----------------------------" << endl;

   // mq_attr qattributes;
   // if ( _RouterQueue != (mqd_t)0 ) mq_getattr( _RouterQueue, &qattributes );
   outs << " RouterQueue: " << hex << (long)_RouterQueue 
        // << "  flags " << qattributes.mq_flags
        // << "  size " << qattributes.mq_curmsgs
        // << "  maxsize " << qattributes.mq_maxmsg 
        << endl;
   // if ( _TimerQueue != (mqd_t)0 ) mq_getattr( _TimerQueue, &qattributes );
   outs << " MsgSysTimerQueue: " << hex << (long)_TimerQueue 
        // << "  flags " << qattributes.mq_flags
        // << "  size " << qattributes.mq_curmsgs
        // << "  maxsize " << qattributes.mq_maxmsg 
        << endl;

   outs << " Message Integrity Map: size " << dec << _MsgIntegrityMap.size() << endl;
   map< unsigned long, string >::iterator miiter;                                 // _MsgIntegrityMap;
   for ( miiter  = _MsgIntegrityMap.begin() ;
         miiter != _MsgIntegrityMap.end() ;
         miiter++ )
   {
      outs << "  Mid " << hex << (*miiter).first << " " << (*miiter).second << endl;
   }
   outs << " Message Task Map: size " << _MessageTaskMap.size() << endl;
   map< unsigned long, map< unsigned long, unsigned char > >::iterator mtiter;    // _MessageTaskMap;
   map< unsigned long, unsigned char >::iterator triter;                          
   for ( mtiter  = _MessageTaskMap.begin() ;
         mtiter != _MessageTaskMap.end() ;
         mtiter++ )
   {
      outs << "  Mid " << hex << (*mtiter).first << endl;
      for ( triter  = ((*mtiter).second).begin() ;
            triter != ((*mtiter).second).end() ;
            triter++ )
      {
         outs << "    Tid " << hex << (*triter).first << " #regs " << (int)(*triter).second << endl;
      }
   }
   outs << " Task Queue Map: size " << dec << _TaskQueueMap.size() << endl;
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
           << endl;
   }
   outs << " Message Gateway Map: size " << dec << _MessageGatewayMap.size() << endl;
   map< unsigned long, set< unsigned long > >::iterator mgiter;                   // _MessageGatewayMap;
   for ( mgiter  = _MessageGatewayMap.begin() ;
         mgiter != _MessageGatewayMap.end() ;
         mgiter++ )
   {
      outs << "  Mid " << hex << (*mgiter).first << " Gateway Set: size " << dec << (long)(*mgiter).second.size() << endl;
      set< unsigned long >::iterator giter;                                      // GatewaySet;
      for ( giter  = (*mgiter).second.begin() ;
            giter != (*mgiter).second.end() ;
            giter++ )
      {
         outs << "   Gateway address " << hex << (*giter) << endl;
      }
   }
   outs << " Inet Gateway Map: size " << dec << _InetGatewayMap.size() << endl;
   map< unsigned long, sockinetbuf* >::iterator igiter;                            // _InetGatewayMap;
   for ( igiter  = _InetGatewayMap.begin() ;
         igiter != _InetGatewayMap.end() ;
         igiter++ )
   {
      outs << "  Address " << hex << (*igiter).first << endl;
   }
   outs << " Gateway Connection Attempts Map: size " << dec << _GatewayConnAtmptMap.size() << endl;
   map< unsigned long, unsigned int >::iterator gcaiter;                           // _GatewayConnAtmptMap;
   for ( gcaiter  = _GatewayConnAtmptMap.begin() ;
         gcaiter != _GatewayConnAtmptMap.end() ;
         gcaiter++ )
   {
      outs << "  Address " << hex << (*gcaiter).first << " conn attempts: " << (*gcaiter).second << endl;
   }
   outs << " Spoofer Message Map: size " << dec << _SpooferMsgMap.size() << endl;
   map< unsigned long, unsigned long >::iterator smiter;                            // _SpooferMsgMap;
   for ( smiter  = _SpooferMsgMap.begin() ;
         smiter != _SpooferMsgMap.end() ;
         smiter++ )
   {
      outs << "  Mid " << hex << (*smiter).first << " Spoofer Tid " << hex << (*smiter).second << endl;
   }
   outs << " StopLoop " << dec << _StopLoop << endl;
   outs << "-------------------------------------------------------------------" << endl;

}

bool Router::initGateways()
{
   //
   // Spawn all gateways and give myself messages informing me to connect
   //  to the gateways ...
   short remoteport=0;
   unsigned long netAddress=0;
   char gateName[17];

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
            int errorNo = errnoGet();
            DataLog_Critical criticalLog;
            DataLog(criticalLog) << "Router init - could not spawn gateway for address -> " << hex << netAddress 
                                 << ", (" << strerror( errorNo ) << ")"
                                 << endmsg;
            _FATAL_ERROR( __FILE__, __LINE__, "Gateway spawn error" );
            return false;
         }
   
         //
         // Send the message packet to connect to the gateways ...
         unsigned long timeDelay = ( MessageSystemConstant::CONNECT_DELAY * 4 );
         unsigned long timerMsgId;
         crcgen32( &timerMsgId, (const unsigned char*)gateName, strlen( gateName ) );
         checkMessageId( timerMsgId, gateName );

         MessagePacket mp;
         mp.msgData().osCode( MessageData::GATEWAY_CONNECT );
         mp.msgData().msgId( timerMsgId );
         mp.msgData().nodeId( netAddress );
         mp.msgData().taskId( taskIdSelf() );
         mp.msgData().msgLength( sizeof( short ) );
         mp.msgData().totalNum( 1 );
         mp.msgData().seqNum( 1 );
         mp.msgData().packetLength( sizeof( short ) );
         mp.msgData().msg( (unsigned char*)&timeDelay, 
                           sizeof( unsigned int ) );
         mp.updateCRC();
         sendMessage( mp, _TimerQueue, taskIdSelf(), 0 );
         _GatewayConnAtmptMap[ netAddress ] = 0;

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
   map< unsigned long, sockinetbuf* >::iterator sockiter;
   sockiter = _InetGatewayMap.find( mp.msgData().nodeId() );

   //
   // If not found ...
   //
   if ( sockiter == _InetGatewayMap.end() )
   {
      //
      // Try to connect ...
      sockinetbuf *socketbuffer = new sockinetbuf( sockbuf::sock_stream );
      if ( !socketbuffer )
      {
         _FATAL_ERROR( __FILE__, __LINE__,"Create socket buffer failed");
         return;
      }
   
      struct timeval tv;
      tv.tv_sec = 0;
      tv.tv_usec = MessageSystemConstant::CONNECT_DELAY * 1000 /* milliseconds to microseconds */;
      short port = getGatewayPort();
      int status = socketbuffer->connectWithTimeout( ntohl( mp.msgData().nodeId() ), port, &tv );

      //
      // If connected, add to list ...
      if ( status == 0 )
      {
         socketbuffer->keepalive( 1 );
         socketbuffer->nodelay( 1 );
         _InetGatewayMap[ mp.msgData().nodeId() ] = socketbuffer;

         //
         // Synch up the remote node's message list ...
         synchUpRemoteNode( socketbuffer );

         //
         // Stop my notification timer ...
         unsigned long interval=0;
         MessagePacket newMP( mp );
         newMP.msgData().msg( (unsigned char*)&interval, sizeof( unsigned long ) ); 
         newMP.updateCRC();
         sendMessage( newMP, _TimerQueue, taskIdSelf(), 0 );
      }
      else 
      {
         //
         // Increment the connection attempts flag for this gatway ...
         _GatewayConnAtmptMap[ mp.msgData().nodeId() ]++;

#ifndef SIMNT
         //
         // Log the connection failure after every 20 attempts ...
         if ( _GatewayConnAtmptMap[ mp.msgData().nodeId() ]%20 == 0 )
         {
            DataLog_Level elog( LOG_ERROR );
            DataLog( elog ) << "Connect with gateway=" << hex << mp.msgData().nodeId() 
                                 << " failed with error-" << strerror( status ) << endmsg;
         }
#endif

         //
         // ... give back my socket memory.
         delete socketbuffer;
      }
   }
   else
   {
      //
      // ... error, the gateway has already established a connection
      //
      // Error ...
      DataLog_Critical criticalLog;
      DataLog(criticalLog) << "Connect with gateway=" << hex << mp.msgData().nodeId() << " - already connected" << endmsg;
      _FATAL_ERROR( __FILE__, __LINE__, "Gateway connect failed" );
   }
}

void Router::disconnectWithGateway( unsigned long address )
{
   //
   // Find gateway in list ...
   map< unsigned long, sockinetbuf* >::iterator sockiter;
   sockiter = _InetGatewayMap.find( address );

   //
   // If found ...
   //
   if ( sockiter != _InetGatewayMap.end() )
   {
      //
      // disconnect the socket interface
      ((sockinetbuf*)(*sockiter).second)->close();
      delete (*sockiter).second;

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
      while ( ( tQueue = mq_open( qName, O_WRONLY ) ) == (mqd_t)ERROR 
              && retries++ < MessageSystemConstant::MAX_NUM_RETRIES ) nanosleep( &MessageSystemConstant::RETRY_DELAY, 0 );

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
         DataLog_Critical criticalLog;
         DataLog(criticalLog) << "Register task=" << hex << tId << " - message queue open failed" 
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
         DataLog_Critical criticalLog;
         DataLog(criticalLog) << "Check message Id - Message name=" << mname << " to Id=" << hex << msgId << " integrity check failed" << endmsg;
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
      _MsgIntegrityMap[ msgId ] = mname;
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
      DataLog_Critical criticalLog;
      DataLog(criticalLog) << "Check message Id - Message Id=" << hex << msgId << " unregistered use in message system" << endmsg;
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
   DataLog_Level slog( "Spoofer" );
   slog( __FILE__, __LINE__ ) << "Spoofer task " << hex << tId 
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
         mp.msgData().msg( (const unsigned char *)messageName.data(), messageName.length() );

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
   DataLog_Level slog( "Spoofer" );
   slog( __FILE__, __LINE__ ) << "Deregistering Spoofer for message " << hex << msgId << endmsg;

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
            DataLog_Critical criticalLog;
            DataLog(criticalLog) << "Sending message=" << hex << mp.msgData().msgId() << "- Task Id=" << (*titer).first << " not found in task list" << endmsg;
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
      int errorNo = errnoGet();
      DataLog_Critical criticalLog;
      DataLog(criticalLog) << "Sending message=" << hex << mp.msgData().msgId() << " - Task Id=" << tId 
                           << " queue full (" << dec << qattributes.mq_curmsgs << " messages)" 
                           << ", (" << strerror( errorNo ) << ")"
                           << endmsg;
      _FATAL_ERROR( __FILE__, __LINE__, "Message queue full" );
   }

   //
   // Send message to the task ...
   unsigned int retries=0;
   while (    mq_send( mqueue , &mp, sizeof( MessagePacket ), priority ) == ERROR 
           && retries++ < MessageSystemConstant::MAX_NUM_RETRIES ) 
      nanosleep( &MessageSystemConstant::RETRY_DELAY, 0 );
   if ( retries == MessageSystemConstant::MAX_NUM_RETRIES )
   {
      //
      // Error ...
      int errorNo = errnoGet();
      DataLog_Critical criticalLog;
      DataLog(criticalLog) << "Sending message=" << hex << mp.msgData().msgId() 
                           << " - Task Id=" << tId << " send failed" 
                           << ", (" << strerror( errorNo ) << ")"
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
             || mpConst.msgData().osCode() == MessageData::MESSAGE_DEREGISTER )
        && mpConst.msgData().nodeId() == 0 )
   {
      map< unsigned long, sockinetbuf* >::iterator sockiter;

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
                  sendMessageToGateway( ((*sockiter).second), mpConst );
               }
               else
               {
                  //
                  // Error ...
                  DataLog_Critical criticalLog;
                  DataLog(criticalLog) << "Gateway not found=" << hex << (*gatewayiter) << " - Gateway registered for a message, but no active connection" << endmsg;
                  _FATAL_ERROR( __FILE__, __LINE__, "Gateway lookup failed" );
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
            // Send the message to the gateway ...
            sendMessageToGateway( ((*sockiter).second), mpConst );
         }
      }
   } 
}

void Router::sendMessageToGateway( sockinetbuf *sockbuffer, const MessagePacket &mpConst )
{
   MessagePacket mp( mpConst );
   //
   // Assign the message packet this nodes network address
   mp.msgData().nodeId( getNetworkAddress() );
   mp.updateCRC();

   unsigned int retries=0;
   while (    sockbuffer->send( &mp, sizeof( MessagePacket ), 0) == ERROR 
           && retries++ < MessageSystemConstant::MAX_NUM_RETRIES ) 
      nanosleep( &MessageSystemConstant::RETRY_DELAY, 0 );
   if ( retries == MessageSystemConstant::MAX_NUM_RETRIES )
   {
      //
      // Error ...
      int errorNo = errnoGet();
      DataLog_Critical criticalLog;
      DataLog(criticalLog) << "Sending message=" << hex << mp.msgData().msgId() 
                           << " - Gateway=" << sockbuffer->peerhost() 
                           << " (" << mp.msgData().nodeId() << ") send failed" 
                           << ", (" << strerror( errorNo ) << ")"
                           << endmsg;
      _FATAL_ERROR( __FILE__, __LINE__, "socket send failed" );
   }
   sockbuffer->flush_all();
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
            tqiter = _TaskQueueMap.find( mp.msgData().taskId() );
            if ( tqiter == _TaskQueueMap.end() )
            {
               //
               // Error ...
               DataLog_Critical criticalLog;
               DataLog(criticalLog) << "Sending message=" << hex << mp.msgData().msgId() << " - Task Id=" << mp.msgData().taskId() << " not found in task list" << endmsg;
               _FATAL_ERROR( __FILE__, __LINE__, "Task lookup failed" );
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

void Router::synchUpRemoteNode( sockinetbuf *sockbuffer )
{
   //
   // Send all my known messages to the remote node ...
   //

   map< unsigned long, map< unsigned long, unsigned char > >::iterator mtiter;
   map< unsigned long, string >::iterator miter;
   MessagePacket mp;

   //
   // Iterate through the message Ids in the list ...
   for ( miter=_MsgIntegrityMap.begin() ;
         miter!=_MsgIntegrityMap.end() ;
         miter++ )
   {
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
      sendMessageToGateway( sockbuffer, mp );
   }

}

short Router::getGatewayPort()
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
         paiter++ )
   {
      if ( (*paiter).second == getNetworkAddress() )
         return (*paiter).first;
   }
   return 0;
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
   map< unsigned long, sockinetbuf* >::iterator sockiter;
   for ( sockiter = _InetGatewayMap.begin() ;
         sockiter != _InetGatewayMap.end() ;
         sockiter++ )
   {
      ((sockinetbuf*)(*sockiter).second)->close();
      delete (*sockiter).second;
      (*sockiter).second = 0;
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
   if ( Router::globalRouter() )
      Router::globalRouter()->dump( cout );
}

