/*
 * Copyright (c) 2002 Gambro BCT, Inc.  All rights reserved.
 *
 * TITLE:      gateway.h, Message System inter-processor communication task
 *
 * ABSTRACT:   This classes supports message routing. The gateway class 
 *             is used to send messages from another processore to the 
 *             local system router.   Only one gateway should be 
 *             created per processor.  The messages are passed using BSD
 *             sockets.
 */

#ifndef _GATEWAY_H_
#define _GATEWAY_H_

#include <fcntl.h>
#include <mqueue.h>
#include <taskLib.h>

#include "messagepacket.h"
#include "sockinet.h"

class Gateway
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
   // This function will act as the main entry point for the gateway task.  This
   //  function will create the Gateway object.  When this function exits, the gateway
   //  task will also be considered as exitted.
   static int Gateway_main( short port );


public:

   Gateway();
   virtual ~Gateway();

   //
   // Function which will create the gateway's sockets, open the router message
   //  queue and set up all the necessary structures to start the processing.  
   bool init( short port );

   //
   // Function which passes along the remote messages received on the socket
   //  connection to the local router's message queue.
   void receiveLoop();

protected:

   //
   // This function sends the message packet to the router's message queue.
   void sendMsgToRouter( const MessagePacket &mp );

   //
   // Closes the socket and message queue.
   void shutdown();

   //
   // Cleans any memory created by this class.
   void cleanup();

protected:

   //
   // The socket connection to the remote router.
   sockinetbuf _ServerSocket;
   sockbuf     _ClientSocket;

   //
   // The local router's message queue.
   mqd_t       _RouterQueue;

};

#endif
