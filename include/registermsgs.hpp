/*******************************************************************
 * Copyright (c) 1999 Cobe BCT, Inc.
 *
 * PURPOSE: register for multiple tcp_gate messages
 * CHANGELOG:
 * $Header: Q:/home1/COMMON_PROJECT/Source/ROUTER/rcs/DEBUG.HPP 1.1 1999/05/24 23:29:26 TD10216 Exp $
 * $Log: DEBUG.HPP $
 *******************************************************************/

#include "intmsgs.h"
#include "buffmsgs.h"
#include "msghdr.h"
#include "an2msgs.h"
#include "error.h"
#include "reply.hpp"

class RegisterMsgs
{
	private:
		// array just as large as # of buffmsgs and in same order
		// 0=not registered for, ~0=registered for
		// initialized by constructor, updated by RegisterMessages()
		// read by MessageRegistered()
		short RegisteredMsgs[LAST_INT32_MESSAGE+1];
	public:
		// used as 2nd parameter to RegisterMessages
		enum { DontReply = 0, DoReply = 1 };
		RegisterMsgs(void);
		// SPECIFICATION:   Add Messages to RegisteredMessages array
		//                      parameter msg - a TCPGate_Reg_Msg
		//
		// ERROR HANDLING:  Calls fatalError().
		void RegisterMessages(TCPGate_Reg_ReqMsg *msg, int Reply);


		// SPECIFICATION:	Determine if a message should be forwarded
		//						parameter msg - a valid focus MsgId
		//
		// ERROR HANDLING:	Calls fatalError().
		int MessageRegistered(short msgID);
};
