/*
 * Copyright (c) 2002 Gambro BCT, Inc.  All rights reserved.
 *
 * TITLE:      router.cpp, Message Router.
 *
 */

#include <vxWorks.h>
#include <taskHookLib.h>

#include "router.h"

const unsigned int Router::MAX_NUM_RETRIES=1;
const struct timespec Router::RETRY_DELAY={ 1 /* seconds */, 0 /*nanoseconds*/ };
const unsigned int Router::DEFAULT_Q_SIZE=100;
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

Router *Router::globalRouter()
{
   return _TheRouter;
}

int Router::taskDeleteHook( WIND_TCB *pTcb )
{
   if ( _TheRouter )
      _TheRouter->deregisterTask( (unsigned long) pTcb );
   
   return 1;
}

Router::Router()
:  _RouterQueue( 0 ),
   _MsgIntegrityMap(),
   _MessageTaskMap(),
   _TaskQueueMap(),
   _InetGatewayMap(),
   _StopLoop( false )
{
   _MsgIntegrityMap.clear();
   _MessageTaskMap.clear();
   _TaskQueueMap.clear();
   _InetGatewayMap.clear();
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
   //  keep the task lists up to date ...
   taskDeleteHookAdd( (FUNCPTR) &Router::taskDeleteHook );

   struct mq_attr attr;                        // message queue attributes 
   attr.mq_maxmsg =  DEFAULT_Q_SIZE;           // set max number of messages 
   attr.mq_msgsize = sizeof( MessagePacket );  // set message size 

   //
   // open queue
   //
   unsigned int retries=0;
   while (    ( _RouterQueue = mq_open( "router", O_RDWR | O_CREAT , 0666, &attr) ) == (mqd_t)ERROR 
           && retries++ < MAX_NUM_RETRIES ) nanosleep( &Router::RETRY_DELAY, 0 );
   if ( _RouterQueue == (mqd_t)ERROR )
      //
      // Error ...
      return false;

   //
   // Set the static pointer to ensure we only init once ...
   _TheRouter = this;
   return true;
}

void Router::dispatchMessages()
{
   if ( _RouterQueue == 0 )
      return;

   int size=0;
   char buffer[ sizeof( MessagePacket ) ];
   while ( _StopLoop == false )
   {
      //
      // Read the queue entry ...
      unsigned int retries=0;
      while (    ( size = mq_receive( _RouterQueue, &buffer, sizeof( MessagePacket ), 0 ) ) == ERROR 
              && retries++ < MAX_NUM_RETRIES ) nanosleep( &Router::RETRY_DELAY, 0 );
      if ( size == ERROR )
      {
         //
         // Error ...
      }

      //
      // Format the data ...
      MessagePacket mp;
      memmove( &mp, &buffer, sizeof( MessagePacket ) );

      if ( mp.validCRC() == false )
      {
         //
         // Error ...
         cout << "CRC Invalid for MsgId " << mp.msgData().msgId() << endl;
      }  

      processMessage( mp );

   }

}

void Router::dump()
{
   cout << "------------------------- Router DUMP -----------------------------" << endl;
   cout << " RouterQueue: " << hex << (long)_RouterQueue << endl;

   cout << " Message Integrity Map: size " << _MsgIntegrityMap.size() << endl;
   map< unsigned long, string >::iterator miiter;                                 // _MsgIntegrityMap;
   for ( miiter  = _MsgIntegrityMap.begin() ;
         miiter != _MsgIntegrityMap.end() ;
         miiter++ )
   {
      cout << "  Mid " << (*miiter).first << " " << (*miiter).second << endl;
   }
   cout << " Message Task Map: size " << _MessageTaskMap.size() << endl;
   map< unsigned long, map< unsigned long, unsigned char > >::iterator mtiter;    // _MessageTaskMap;
   map< unsigned long, unsigned char >::iterator triter;                          
   for ( mtiter  = _MessageTaskMap.begin() ;
         mtiter != _MessageTaskMap.end() ;
         mtiter++ )
   {
      cout << "  Mid " << (*mtiter).first << endl;
      for ( triter  = ((*mtiter).second).begin() ;
            triter != ((*mtiter).second).end() ;
            triter++ )
      {
         cout << "    Tid " << dec << (*triter).first << " #regs " << (int)(*triter).second << endl;
      }
   }
   cout << " Task Queue Map: size " << _TaskQueueMap.size() << endl;
   map< unsigned long, mqd_t >::iterator tqiter;                                  // _TaskQueueMap;
   for ( tqiter  = _TaskQueueMap.begin() ;
         tqiter != _TaskQueueMap.end() ;
         tqiter++ )
   {
      cout << "  Tid " << dec << (*tqiter).first << " " << (long)(*tqiter).second << endl;
   }
   cout << " Inet Gateway Map: size " << _InetGatewayMap.size() << endl;
   map< unsigned long, sockinetbuf* >::iterator igiter;                            // _InetGatewayMap;
   for ( igiter  = _InetGatewayMap.begin() ;
         igiter != _InetGatewayMap.end() ;
         igiter++ )
   {
      cout << "  Address " << (*igiter).first << endl;
   }
   cout << " StopLoop " << _StopLoop << endl;
   cout << "-------------------------------------------------------------------" << endl;

}

void Router::processMessage( MessagePacket &mp )
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
      checkMessageId( mp.msgData().msgId() );
      break;
   case MessageData::MESSAGE_REGISTER:
      checkMessageId( mp.msgData().msgId(), (const char *)( mp.msgData().msg() ) );
      registerMessage( mp.msgData().msgId(), mp.msgData().taskId() );
      break;
   case MessageData::MESSAGE_DEREGISTER:
      checkMessageId( mp.msgData().msgId(), (const char *)( mp.msgData().msg() ) );
      deregisterMessage( mp.msgData().msgId(), mp.msgData().taskId() );
      break;
   case MessageData::GATEWAY_CONNECT:
      connectWithGateway( mp.msgData().nodeId() );
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
      checkMessageId( mp.msgData().msgId() );
      sendMessage( mp );
      break;
   default:
      break;
   }


}

void Router::connectWithGateway( unsigned long address )
{
   //
   // Try to connect ...

   //
   // If connected, add to list ...

   //
   // If not connected, add message to the queue to try again ...
}

void Router::disconnectWithGateway( unsigned long address )
{
   //
   // Find gateway in list ...

   //
   // If found ...
   //
   //    disconnect the socket interface
   //    remove the entry from the list
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
         cout << " ERROR " << endl;
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
         cout << "Message ID Integrity Check Failure" << endl;
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
      cout << "MsgId " << msgId << " : Message ID Integrity Check Failure" << endl;
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

void Router::sendMessage( MessagePacket &mp )
{
   //
   // Distribute the message to the local tasks
   unsigned int retries;
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
         }
         //
         // If the task was found to be registered ...
         else
         {
            //
            // Check the task's queue to see if it is full or not ...
            mq_attr qattributes;
            if (    mq_getattr( (*tqiter).second, &qattributes ) == ERROR
                 || qattributes.mq_curmsgs >= qattributes.mq_maxmsg )
            {
               //
               // The task's queue is full!
               //
               // Error ...
            }

            //
            // Send message to the task ...
            retries=0;
            while (    mq_send( (*tqiter).second , &mp, sizeof( MessagePacket ), 0 ) == ERROR 
                    && retries++ < MAX_NUM_RETRIES ) nanosleep( &Router::RETRY_DELAY, 0 );
            if ( retries == MAX_NUM_RETRIES )
            {
               //
               // Error ...
            }
         }

      }
   }


   // ... and if the message isn't supposed to be local, distribute
   //  the message over the socket connections to the gateways...
   //
}

void Router::shutdown()
{
   //
   // Close my queue ...
   mq_close( _RouterQueue );
   _RouterQueue = (mqd_t)0;

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

   _TheRouter = 0;
}

