/*
 * Copyright (c) 2002 by Gambro BCT, Inc.  All rights reserved.
 *
 * TITLE:      messagesystemconstant.cpp
 *
 */

#include <vxWorks.h>

#include "messagesystemconstant.h"

const unsigned int MessageSystemConstant::DEFAULT_ROUTER_Q_SIZE=100;
const unsigned int MessageSystemConstant::DEFAULT_Q_SIZE=30;
const unsigned int MessageSystemConstant::MAX_MESSAGE_QUEUE_SIZE=400;
const unsigned int MessageSystemConstant::MAX_NUM_RETRIES=1;
const struct timespec MessageSystemConstant::CONNECT_RETRY_DELAY={ 5 /* seconds */, 0 /*nanoseconds*/ };
const struct timespec MessageSystemConstant::RETRY_DELAY={ 1 /* seconds */, 0 /*nanoseconds*/ };
const int MessageSystemConstant::DEFAULT_REGISTER_PRIORITY=17;
const int MessageSystemConstant::DEFAULT_MESSAGE_PRIORITY=15;
const int MessageSystemConstant::DEFAULT_TIMER_MESSAGE_PRIORITY=16;
const int MessageSystemConstant::GATEWAY_CONNECT_PRIORITY=17;
const int MessageSystemConstant::GATEWAY_MESSAGE_PRIORITY=15;
const int MessageSystemConstant::REMOTE_NODE_SYNCH_PRIORITY=2;
const int MessageSystemConstant::MESSAGES_BETWEEN_LOG=100;

