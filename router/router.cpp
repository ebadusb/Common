/*
 * Copyright (c) 2002 Gambro BCT, Inc.  All rights reserved.
 *
 * TITLE:      router.cpp, Message Router.
 *
 */

#include <stdio.h>
#include <vxWorks.h>
#include <taskHookLib.h>

#include "error.h"
#include "gateway.h"
#include "messagesystem.h"
#include "router.h"
#include "systemoverrides.h"
#include "tcpconnect.h"


const unsigned int Router::MAX_NUM_RETRIES=1;
const struct timespec Router::RETRY_DELAY={ 1 /* seconds */, 0 /*nanoseconds*/ };
const unsigned int Router::DEFAULT_Q_SIZE=100;
const unsigned int Router::CONNECT_DELAY=500; // milliseconds
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

Router::Router()
:  _RouterQueue( 0 ),
   _MsgIntegrityMap(),
   _MessageTaskMap(),
   _TaskQueueMap(),
   _InetGatewayMap(),
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

   struct mq_attr attr;                        // message queue attributes 
   attr.mq_maxmsg =  DEFAULT_Q_SIZE;           // set max number of messages 
   attr.mq_msgsize = sizeof( MessagePacket );  // set message size 
   attr.mq_flags = 0;

   //
   // open queue
   //
   unsigned int retries=0;
   while (    ( _RouterQueue = mq_open( "router", O_RDWR | O_CREAT , 0666, &attr) ) == (mqd_t)ERROR 
           && retries++ < MAX_NUM_RETRIES ) nanosleep( &Router::RETRY_DELAY, 0 );
   if ( _RouterQueue == (mqd_t)ERROR )
   {
      //
      // Error ...
      _FATAL_ERROR( __FILE__, __LINE__, "Router message queue open failed" );
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
              && retries++ < MAX_NUM_RETRIES ) nanosleep( &Router::RETRY_DELAY, 0 );
      if ( size == ERROR )
      {
         //
         // Error ...
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
         char buffer[256];
         unsigned long crc = mp.crc();
         mp.updateCRC();
         sprintf( buffer,"Dispatching message - message CRC validation failed for MsgId=%lx, CRC=%lx and should be %lx",
                  mp.msgData().msgId(), crc, mp.crc() );
         _FATAL_ERROR( __FILE__, __LINE__, buffer );
      }  

      processMessage( mp, priority );

   } while ( _StopLoop == false );

}

void Router::dump( ostream &outs )
{
   outs << "------------------------- Router DUMP -----------------------------" << endl;
   outs << " RouterQueue: " << hex << (long)_RouterQueue << endl;

   outs << " Message Integrity Map: size " << _MsgIntegrityMap.size() << endl;
   map< unsigned long, string >::iterator miiter;                                 // _MsgIntegrityMap;
   for ( miiter  = _MsgIntegrityMap.begin() ;
         miiter != _MsgIntegrityMap.end() ;
         miiter++ )
   {
      outs << "  Mid " << (*miiter).first << " " << (*miiter).second << endl;
   }
   outs << " Message Task Map: size " << _MessageTaskMap.size() << endl;
   map< unsigned long, map< unsigned long, unsigned char > >::iterator mtiter;    // _MessageTaskMap;
   map< unsigned long, unsigned char >::iterator triter;                          
   for ( mtiter  = _MessageTaskMap.begin() ;
         mtiter != _MessageTaskMap.end() ;
         mtiter++ )
   {
      outs << "  Mid " << (*mtiter).first << endl;
      for ( triter  = ((*mtiter).second).begin() ;
            triter != ((*mtiter).second).end() ;
            triter++ )
      {
         outs << "    Tid " << dec << (*triter).first << " #regs " << (int)(*triter).second << endl;
      }
   }
   outs << " Task Queue Map: size " << _TaskQueueMap.size() << endl;
   map< unsigned long, mqd_t >::iterator tqiter;                                  // _TaskQueueMap;
   for ( tqiter  = _TaskQueueMap.begin() ;
         tqiter != _TaskQueueMap.end() ;
         tqiter++ )
   {
      outs << "  Tid " << dec << (*tqiter).first << " " << (long)(*tqiter).second << endl;
   }
   outs << " Inet Gateway Map: size " << _InetGatewayMap.size() << endl;
   map< unsigned long, sockinetbuf* >::iterator igiter;                            // _InetGatewayMap;
   for ( igiter  = _InetGatewayMap.begin() ;
         igiter != _InetGatewayMap.end() ;
         igiter++ )
   {
      outs << "  Address " << (*igiter).first << endl;
   }
   outs << " StopLoop " << _StopLoop << endl;
   outs << "-------------------------------------------------------------------" << endl;

}

bool Router::initGateways()
{
   //
   // Spawn all gateways and give myself messages informing me to connect
   //  to the gateways ...
   short localport=0;
   short remoteport=0;
   unsigned long netAddress=0;
   char gateName[17];

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
      netAddress = (*paiter).second;
      if ( netAddress == getNetworkAddress() )
         localport = (*paiter).first;
   }

   //
   // Connect to the other nodes ...
   for ( paiter = portAddressMap.begin() ;
         paiter != portAddressMap.end() && localport != 0 ;
         paiter++ )
   {
      netAddress = (*paiter).second;
      if ( netAddress != getNetworkAddress() )
      {
         remoteport = (*paiter).first;
         sprintf( gateName, "tGateway%lx", netAddress );
         if (    taskNameToId( gateName ) != ERROR
              || (    taskNameToId( gateName ) == ERROR
                   && taskSpawn ( gateName , 4, 0, 20000,
                             (FUNCPTR) Gateway::Gateway_main , remoteport,0,0,0,0,0,0,0,0,0) == ERROR 
                 )
            )
         {
            //
            // Error ...
            char buffer[256];
            sprintf( buffer,"Router init - could not spawn gateway for address -> %lx",
                     netAddress );
            _FATAL_ERROR( __FILE__, __LINE__, buffer );
            return false;
         }
   
         //
         // Send the message packet to connect to the gateways ...
         MessagePacket mp;
         mp.msgData().osCode( MessageData::GATEWAY_CONNECT );
         mp.msgData().msgId( 0 );
         mp.msgData().nodeId( netAddress );
         mp.msgData().taskId( taskIdSelf() );
         mp.msgData().msgLength( sizeof( short ) );
         mp.msgData().totalNum( 1 );
         mp.msgData().seqNum( 1 );
         mp.msgData().packetLength( sizeof( short ) );
         mp.msgData().msg( (unsigned char*)&localport, sizeof( short ) );
         mp.updateCRC();
         sendMessage( mp, _RouterQueue, taskIdSelf(), 0 );

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
      registerMessage( mp.msgData().msgId(), mp.msgData().taskId() );
      sendMessageToGateways( mp );
      break;
   case MessageData::MESSAGE_DEREGISTER:
      checkMessageId( mp.msgData().msgId(), (const char *)( mp.msgData().msg() ) );
      deregisterMessage( mp.msgData().msgId(), mp.msgData().taskId() );
      break;
   case MessageData::GATEWAY_CONNECT:
      connectWithGateway( mp );
      break;
   case MessageData::GATEWAY_DISCONNECT:
      disconnectWithGateway( mp.msgData().nodeId() );
      break;
   case MessageData::SPOOFER_REGISTER:
      break;
   case MessageData::SPOOFER_DEREGISTER:
      break;
   case MessageData::MESSAGE_MULTICAST:
   case MessageData::MESSAGE_MULTICAST_LOCAL:
   case MessageData::SPOOFED_MESSAGE:
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
      tv.tv_usec = CONNECT_DELAY * 1000;
      short port;
      memmove( &port, mp.msgData().msg(), sizeof( short ) );
      int status = socketbuffer->connectWithTimeout( ntohl( mp.msgData().nodeId() ), port/*port*/, &tv );

      //
      // If connected, add to list ...
      if ( status == 0 )
      {
         _InetGatewayMap[ mp.msgData().nodeId() ] = socketbuffer;
      }
      //
      // If not connected, add message to the queue to try again ...
      else
      {
         //
         // ... give back my socket memory.
         delete socketbuffer;
         
         sendMessage( mp, _RouterQueue, taskIdSelf(), 0 );
      }
   }
   else
   {
      //
      // ... error, the gateway has already established a connection
      //
      // Error ...
      char buffer[256];
      sprintf( buffer,"Connect with gateway=%lx - already connected", mp.msgData().nodeId() );
      _FATAL_ERROR( __FILE__, __LINE__, buffer );
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
              && retries++ < MAX_NUM_RETRIES ) nanosleep( &Router::RETRY_DELAY, 0 );

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
         char buffer[256];
         sprintf( buffer,"Register task=%lx - message queue open failed", tId );
         _FATAL_ERROR( __FILE__, __LINE__, buffer );
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
         // find the task registration entry ( if it exists )
         map< unsigned long, unsigned char >::iterator triter;
         triter = ( (*mtiter).second ).find( tId );
         if ( triter != ( (*mtiter).second ).end() )
         {
            //
            // Remove the entry ...
            ( (*mtiter).second ).erase( triter );
         }
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
         char buffer[256];
         sprintf( buffer,"Check message Id - Message name=%s to Id=%lx integrity check failed",
                  mname, msgId );
         _FATAL_ERROR( __FILE__, __LINE__, buffer );
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
      char buffer[256];
      sprintf( buffer,"Check message Id - Message Id=%lx unregistered use in message system", msgId );
      _FATAL_ERROR( __FILE__, __LINE__, buffer );
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
      map< unsigned long, unsigned char>::iterator titer;
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

void Router::registerSpooferMessage( unsigned long msgId, unsigned long tId )
{
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
   }

}

void Router::deregisterSpooferMessage( unsigned long msgId)
{
}

void Router::sendMessage( const MessagePacket &mp, int priority )
{
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
            char buffer[256];
            sprintf( buffer,"Sending message=%lx - Task Id=%lx not found in task list",
                     mp.msgData().msgId(), (*titer).first );
            _FATAL_ERROR( __FILE__, __LINE__, buffer );
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
   // ... and if the message isn't supposed to be local, distribute
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
      char buffer[256];
      sprintf( buffer,"Sending message=%lx - Task Id=%lx queue full (%d messages)",
               mp.msgData().msgId(), tId, qattributes.mq_curmsgs );
      _FATAL_ERROR( __FILE__, __LINE__, buffer );
   }

   //
   // Send message to the task ...
   unsigned int retries=0;
   while (    mq_send( mqueue , &mp, sizeof( MessagePacket ), priority ) == ERROR 
           && retries++ < MAX_NUM_RETRIES ) nanosleep( &Router::RETRY_DELAY, 0 );
   if ( retries == MAX_NUM_RETRIES )
   {
      //
      // Error ...
      char buffer[256];
      sprintf( buffer,"Sending message=%lx - Task Id=%lx send failed",
               mp.msgData().msgId(), tId );
      _FATAL_ERROR( __FILE__, __LINE__, buffer );
   }
}

void Router::sendMessageToGateways( const MessagePacket &mpConst )
{
   if (    (    mpConst.msgData().osCode() == MessageData::MESSAGE_MULTICAST
             || mpConst.msgData().osCode() == MessageData::MESSAGE_NAME_REGISTER
             || mpConst.msgData().osCode() == MessageData::MESSAGE_REGISTER
             || mpConst.msgData().osCode() == MessageData::MESSAGE_DEREGISTER )
        && mpConst.msgData().nodeId() == 0 )
   {
      MessagePacket mp( mpConst );
      //
      // Assign the message packet this nodes network address
      mp.msgData().nodeId( getNetworkAddress() );
      mp.updateCRC();

      map< unsigned long, sockinetbuf* >::iterator sockiter;
      for ( sockiter  = _InetGatewayMap.begin() ;
            sockiter != _InetGatewayMap.end() ;
            sockiter++ )
      {
         unsigned int retries=0;
         while (    ((*sockiter).second)->send( &mp, sizeof( MessagePacket ), 0) == ERROR 
                 && retries++ < MAX_NUM_RETRIES ) nanosleep( &Router::RETRY_DELAY, 0 );
         if ( retries == MAX_NUM_RETRIES )
         {
            //
            // Error ...
            char buffer[256];
            sprintf( buffer,"Sending message=%lx - Gateway=%s (%ld) send failed",
                     mp.msgData().msgId(), 
                     ((sockinetbuf*)(*sockiter).second)->peerhost(), 
                     (*sockiter).first );
            _FATAL_ERROR( __FILE__, __LINE__, buffer );
         }
      }
   }
}

void Router::sendMessageToSpoofer( const MessagePacket &mp )
{
   /*
   if ( _SpooferQueue != (mqd_t)0 )
   {
      //
      // Check the task's queue to see if it is full or not ...
      mq_attr qattributes;
      if ( mq_getattr( _SpooferQueue, &qattributes ) == ERROR
           || qattributes.mq_curmsgs >= qattributes.mq_maxmsg )
      {
         //
         // The spoofer's queue is full!
         //
         // Error ...
         char buffer[256];
         sprintf( buffer,"",);
         _FATAL_ERROR( __FILE__, __LINE__, buffer );
      }

      //
      // Send message to the task ...
      unsigned int retries=0;
      while ( mq_send( _SpooferQueue , &mp, sizeof( MessagePacket ), 0 ) == ERROR 
              && retries++ < MAX_NUM_RETRIES ) nanosleep( &Router::RETRY_DELAY, 0 );
      if ( retries == MAX_NUM_RETRIES )
      {
         //
         // Error ...
         char buffer[256];
         sprintf( buffer,"",);
         _FATAL_ERROR( __FILE__, __LINE__, buffer );
      }
   }
   */
}


void Router::shutdown()
{
   //
   // Close my queue ...
   if ( _RouterQueue != (mqd_t)0 )
   {
      mq_close( _RouterQueue );
      _RouterQueue = (mqd_t)0;
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
   _InetGatewayMap.clear();
   _SpooferMsgMap.clear();

   _TheRouter = 0;
   _TheRouterTid = 0;
}

