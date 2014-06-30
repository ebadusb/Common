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
#include <set>
#include <stdio.h>
#include <fcntl.h>
#include <mqueue.h>
#include <string.h>
#include <string>
#include <taskLib.h>

#include "datalog.h"
#include "messagepacket.h"
#include "messagesystemconstant.h"


class Router
{
public:
   //
   // This function will act as the main entry point for the router task.  This
   //  function will create the Router object.  When this function exits, the router
   //  task will also be considered as exitted.
   static int Router_main (unsigned int qSize = MessageSystemConstant::DEFAULT_ROUTER_Q_SIZE);

   //
   // Function to get the global router ...
   static WIND_TCB* globalRouterTid ();
   static Router*   globalRouter ();

   //
   // Function called whenever any task gets deleted ...
   static int taskDeleteHook (WIND_TCB* pTcb);

   //
   // Function to handle task level datalog errors ...
   static void datalogErrorHandler (const char* file, int line,
                                    DataLog_ErrorType error,
                                    const char* msg,
                                    int continuable);
private:

   static WIND_TCB* _TheRouterTid;
   static Router*   _TheRouter;

public:

   Router();
   virtual ~Router();

   //
   // Function which will create the router's message queue, and set up all the
   //  necessary structures to start the processing.
   bool init (unsigned int qSize);

   //
   // This function will continue processing indefinitely while it is blocked on the
   //  message queue.  The processing can be terminated by calling the stopLoop function.
   void dispatchMessages ();
   //
   // This function will interrupt the messaging loop, causing the dispatchMessage function
   //  to return.
   void stopLoop () { _StopLoop = true; }

   //
   // Dump the contents of this class
   void dump (DataLog_Stream& outs);

   //
   // Dump the different router structures
   void msgDump ();
   void msgTaskDump ();
   void gatewayDump ();
   void remoteMsgDump ();

   unsigned int messageRemaining (void) const { mq_attr qattributes; mq_getattr(_RouterQueue, &qattributes); return (unsigned int)qattributes.mq_curmsgs; }

protected:

   //
   // This function creates and starts the connection process with all the
   //  systems gateways
   bool initGateways ();

   //
   // This function handles the processing of the message.  This processing includes
   //  determining where it needs to go and/or determining how to update the internal
   //  router structures
   void processMessage (MessagePacket& mp, int priority = 0);

   //
   // This function will be called to register a task in the task maps
   void registerTask (unsigned long tId, const char* qName);
   //
   // This function will remove the task from the task map, and the message maps.  No
   //  references to the passed in taskId will remain in any of the structures.  All
   //  messages registered for by this task will be deregistered.
   void deregisterTask (unsigned long tId);

   //
   // This function is called given an address of a gateway to connect with.  This
   //  function will try to connect for a specified timeout to ensure the function doesn't
   //  block too long.
   void connectWithGateway (const MessagePacket& mp);

   //
   // This function will shutdown the socket connection for the specified gateway.
   void disconnectWithGateway (unsigned long address);

   //
   // This function will make sure the given message Id is unique for the message
   //  name.
   void checkMessageId (unsigned long msgId, const char* mname);

   //
   // Verify that the system has knowledge of this message Id
   void checkMessageId (unsigned long msgId);

   //
   // This function will register a message for the calling task.  The task must already
   //  be registered in the task map before it can register messages.
   void registerMessage (unsigned long msgId, unsigned long tId);
   void registerMessageWithGateway (unsigned long msgId, unsigned long nId);
   void registerSpooferMessage (unsigned long msgId, unsigned long tId);
   //
   // This function will remove one registration of the message from the calling task's
   //  entry in the message map.  When the number of registrations by the task is at 0, the
   //  task's entry will be removed from the given message Id's list.
   void deregisterMessage (unsigned long msgId, unsigned long tId);
   void deregisterMessageWithGateway (unsigned long msgId, unsigned long nId);
   void deregisterSpooferMessage (unsigned long msgId);

   //
   // This function sends the message packet to registered tasks.
   void sendMessage (const MessagePacket& mp, int priority = 0);
   void sendMessage (const MessagePacket& mp, mqd_t mqueue, const unsigned long tId, int priority = 0);

   //
   // This function sends the message packet to the registered gateways.
   void sendMessageToGateways (const MessagePacket& mp);
   //
   // This function sends the message packet to this specific gateway.
   void sendMessageToGateway (unsigned long nodeId, const MessagePacket& mpConst);

   //
   // This function sends the message packet to the Spoofer task.
   bool sendMessageToSpoofer (const MessagePacket& mp, int priority = 0);

   //
   // This function will synchronize the remote node's registered messages with my list
   void synchUpRemoteNode (unsigned long nodeId);
   enum MessageSynchStatus { MsgNoSynch=0, MsgNameSynch=1, MsgRegisterSynch=3 };
   bool synchUpRemoteNode (unsigned long nodeId, unsigned long msgId, map< unsigned long, MessageSynchStatus >& gStatusMap);

   //
   // This function will output the gateway connection status found for the
   //  passed in nodeId in the _GatewayConnSynchedMap.  The map contains an
   //  enum which will be reported as a meaningful character value.
   void gatewayConnStatus (DataLog_Stream& outs, unsigned long nodeId);

   //
   // Dump the contents of the given queue
   void dumpQueue (unsigned long tId, mqd_t mqueue, DataLog_Stream& out);

   //
   // This function will close the router's message queues and socket connections.
   void shutdown ();
   //
   // This function will clean up the router's structures.
   void cleanup ();

protected:

   //
   // The router message queue ...
   mqd_t _RouterQueue;

   //
   // High water mark for my message queue
   int _MessageHighWaterMark;

   //
   // Number of message passed through this queue
   unsigned long _NumMessages;
   // High water mark for my message queue per logging period
   int           _MessageHighWaterMarkPerPeriod;
   int           _PrevMessageHighWaterMarkPerPeriod;

   //
   // The router message queue ...
   mqd_t _TimerQueue;

   //
   // This structure will be used to verify that no different messages with
   //  duplicate ids get passed around in the system.  The message's hashed Id will be mapped
   //  together with the message name.  This imposes an implicite length restriction on
   //  message names of MAX_MESSAGE_SIZE.
   map< unsigned long, string > _MsgIntegrityMap;

   //
   // This structure will be used during synchronization to keep the status of which messages
   //  have been synchronized with the gateways.  The message will need to be registered on the
   //  remote node, even if the remote node already knows about the message, if the message has
   //  a task waiting to receive it.
   map< unsigned long, map< unsigned long, MessageSynchStatus > > _MsgToGatewaySynchMap;

   //
   // This structure will be used to map the message Ids with the tasks that have registered
   //  to receive them.  The map will be indexed on message Id.  The second item in the map
   //  contain another map.  The inner map will be indexed on task Id.  The second item in
   //  the inner map will contain the number of times the task registered for this message.
   //  When the number goes to 0, the task will no longer receive the given message, and the
   //  entry will be removed from the outer map.
   map< unsigned long, map< unsigned long, unsigned char > > _MessageTaskMap;
   //
   // This structure will map the task Id with it's associated Posix message queue. The
   //  router will open a queue for each task as a write only queue.  The task must create
   //  the queue before registering itself with the router.
   map< unsigned long, mqd_t > _TaskQueueMap;
   map< unsigned long, bool >  _TaskQueueActiveMap;
   //
   // This structure will map the message Ids with the set of gateways that are registered
   //  to receive that specific message.  A message will not be passed along to a gateway
   //  that is not registered to receive it.  The map will be indexed on the message Id.
   //  The Set will be indexed by network address.  The network address is associated with
   //  a network address held in the InetGatewayMap.
   map< unsigned long, set< unsigned long > > _MessageGatewayMap;
   //
   // This structure will hold the socket connections for all the given gateways.  The map
   //  will be indexed on gateway inet address.  The second entry in the map will contain
   //  the socket connection to the gateway.
   map< unsigned long, int > _InetGatewayMap;

   enum GatewaySynched { NotConn                =0, LocalRouterConnected=1,  LocalRouterSynched=2,  LocalConnAndSynched=3,
                         RemoteRouterConnected  =4, RemoteRouterSynched=8, RemoteConnAndSynched=12,
                         LocalAndRemoteConnected=5, LocalSynchRemoteConn=7, RemoteSynchLocalConn=13,
                         Synched                =15};
   map< unsigned long, GatewaySynched > _GatewayConnSynchedMap;

   //
   // This structure will hold the message Ids of the messages the spoofer wants.  The map
   //  will be indexed the message Ids to the task Id of the associated spoofer task.
   map< unsigned long, unsigned long > _SpooferMsgMap;

   //
   // This flag, when set to true, will drop the router out of its message loop.
   bool _StopLoop;

   //
   // This number will contain the sequence number of the network packet for tracking purposes.
   unsigned long _NetSequenceNum;

};


#ifdef __cplusplus
extern "C" void routerInit ();
extern "C" void routerInitQ (const char *commandLine);
extern "C" void routerDump ();
extern "C" void msgDump ();
extern "C" void msgTaskDump ();
extern "C" void gatewayDump ();
extern "C" void remoteMsgDump ();
#else
void routerInit ();
void routerInitQ (const char *commandLine);
void routerDump ();
void msgDump ();
void msgTaskDump ();
void gatewayDump ();
void remoteMsgDump ();
#endif

#endif

/* FORMAT HASH d8c81dc6a2052b8267084a508f79b18d */
