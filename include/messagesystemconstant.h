/*
 * Copyright (c) 2002 by Gambro BCT, Inc.  All rights reserved.
 *
 * TITLE:      messagesystemconstant.h
 *
 * ABSTRACT:   This file initializes the message system constants
 */

#ifndef _MESSAGE_SYSTEM_CONSTANT_H_
#define _MESSAGE_SYSTEM_CONSTANT_H_

#include <time.h>

class MessageSystemConstant
{
public:
   //
   // default Router Q size
   static const unsigned int DEFAULT_ROUTER_Q_SIZE;
   //
   // default Q size
   static const unsigned int DEFAULT_Q_SIZE;
   //
   // Maximum message queue size
   static const unsigned int MAX_MESSAGE_QUEUE_SIZE;
   //
   // Gateway connection delay time in milliseconds
   static const unsigned int CONNECT_DELAY;
   //
   // Number of retries before error for messaging operations ...
   static const unsigned int MAX_NUM_RETRIES;
   //
   // Time delay between retries ...
   static const struct timespec RETRY_DELAY;
   //
   // Default priority of messages ( 1-31 )
   static const int DEFAULT_REGISTER_PRIORITY;
   //
   // Default priority of messages ( 1-31 )
   static const int DEFAULT_MESSAGE_PRIORITY;
   //
   // Default priority of timer message notifications ( 1-31 )
   static const int DEFAULT_TIMER_MESSAGE_PRIORITY;
   //
   // Size of the user data inside the message packets
   static const int MAX_MESSAGE_SIZE=127;

};

#endif
