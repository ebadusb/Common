/*
 * Copyright (c) 2002 Gambro BCT, Inc.  All rights reserved.
 *
 * TITLE:      router.h, Message Router.
 *
 * ABSTRACT:   This class provides message routing. The router class 
 *             is used to send messages to the registered system tasks.   
 *             Only one router should be started per processor.   
 */

#ifndef _ROUTER_H_
#define _ROUTER_H_

#include <map>
#include <stdio.h>
#include <fcntl.h>
#include <mqueue.h>
#include <string.h>
#include <string>

#include "messagepacket.h"
#include "sockinet.h"


class Router 
{
public:
   //
   // Number of retries for all system operations before we consider the
   //  failure an error
   static const unsigned int MAX_NUM_RETRIES;
   //
   // Time delay between retries ...
   static const struct timespec RETRY_DELAY;
   //
   // default Q size
   static const unsigned int DEFAULT_Q_SIZE;

public:
   //
   // This function will act as the main entry point for the router task.  This
   //  function will create the Router object.  When this function exits, the router
   //  task will also be considered as exitted.
   static int Router_main();

   //
   // Function to get the global router ...
   static Router *globalRouter();

   //
   // Function called whenever any task gets deleted ...
   static int taskDeleteHook( WIND_TCB *pTcb );

private:

   static Router *_TheRouter;

public:

   Router();
   virtual ~Router();

   //
   // Function which will create the router's message queue, and set up all the 
   //  necessary structures to start the processing.  
   bool init();

   //
   // This function will continue processing indefinitely while it is blocked on the
   //  message queue.  The processing can be terminated by calling the stopLoop function.
   void dispatchMessages();
   //
   // This function will interrupt the messaging loop, causing the dispatchMessage function
   //  to return.
   void stopLoop() { _StopLoop = true; }

   //
   // Dump the contents of this class
   void dump();

protected:

   //
   // This function handles the processing of the message.  This processing includes
   //  determining where it needs to go and/or determining how to update the internal 
   //  router structures
   void processMessage( MessagePacket &mp );

   //
   // This function will be called to register a task in the task maps
   void registerTask( unsigned long tId, const char *qName );
   //
   // This function will remove the task from the task map, and the message maps.  No
   //  references to the passed in taskId will remain in any of the structures.  All
   //  messages registered for by this task will be deregistered.
   void deregisterTask( unsigned long tId );

   //
   // This function is called given an address of a gateway to connect with.  This
   //  function will try to connect for a specified timeout to ensure the function doesn't
   //  block too long.
   void connectWithGateway( unsigned long address );

   //
   // This function will shutdown the socket connection for the specified gateway.
   void disconnectWithGateway( unsigned long address );

   //
   // This function will make sure the given message Id is unique for the message
   //  name.
   void checkMessageId( unsigned long msgId, const char *mname );

   //
   // Verify that the system has knowledge of this message Id
   void checkMessageId( unsigned long msgId );

   //
   // This function will register a message for the calling task.  The task must already
   //  be registered in the task map before it can register messages.   
   void registerMessage( unsigned long msgId, unsigned long tId );
   //
   // This function will remove one registration of the message from the calling task's 
   //  entry in the message map.  When the number of registrations by the task is at 0, the
   //  task's entry will be removed from the given message Id's list.
   void deregisterMessage( unsigned long msgId, unsigned long tId );

   //
   // This function sends the message packet to registered tasks.
   void sendMessage( MessagePacket &mp );

   //
   // This function will close the router's message queues and socket connections.
   void shutdown();
   //
   // This function will clean up the router's structures.
   void cleanup();

protected:

   //
   // The router message queue ...
   mqd_t                                                        _RouterQueue;

   //
   // This structure will be used to verify that no different messages with
   //  duplicate ids get passed around in the system.  The message's hashed Id will be mapped
   //  together with the message name.  This imposes an implicite length restriction on 
   //  message names of MAX_MESSAGE_SIZE.
   map< unsigned long, string >  _MsgIntegrityMap;
   //
   // This structure will be used to map the message Ids with the tasks that have registered
   //  to receive them.  The map will be indexed on message Id.  The second item in the map 
   //  contain another map.  The inner map will be indexed on task Id.  The second item in
   //  the inner map will contain the number of times the task registered for this message. 
   //  When the number goes to 0, the task will no longer receive the given message, and the 
   //  entry will be removed from the outer map.
   map< unsigned long, map< unsigned long, unsigned char > >    _MessageTaskMap;
   //
   // This structure will map the task Id with it's associated Posix message queue. The 
   //  router will open a queue for each task as a write only queue.  The task must create
   //  the queue before registering itself with the router.
   map< unsigned long, mqd_t >                                  _TaskQueueMap;
   //
   // This structure will hold the socket connections for all the given gateways.  The map
   //  will be indexed on gateway inet address.  The second entry in the map will contain
   //  the socket connection to the gateway.
   map< unsigned long, sockinetbuf* >                           _InetGatewayMap;

   //
   // This flag, when set to true, will drop the router out of its message loop.
   bool _StopLoop;

};

#endif
