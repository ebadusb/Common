/*******************************************************************
 * Copyright (c) 1999 Cobe BCT, Inc.
 *
 * PURPOSE: register for multiple tcp_gate messages
 * CHANGELOG:
 * $Header: Q:/home1/COMMON_PROJECT/Source/ROUTER/rcs/DEBUG.HPP 1.1 1999/05/24 2
 * $Log: RegisterMsgs.cpp $
 * Revision 1.2  1999/07/29 15:53:02  TD10216
 * IT4154
 * Revision 1.1  1999/07/24 20:51:35  TD10216
 * Initial revision
 *******************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "intmsgs.h"
#include "buffmsg.hpp"
#include "crc.h"
#include "error.h"
#include "msghdr.h"
#include "sinver.h"
#include "an2msgs.h"
#include "reply.hpp"
#include "RegisterMsgs.hpp"

RegisterMsgs::RegisterMsgs(void)
{
	memset( RegisteredMsgs, 0, sizeof( RegisteredMsgs ) );
}

// SPECIFICATION:	Add Messages to RegisteredMessages array
//						parameter msg - a TCPGate_Reg_Msg
//
// ERROR HANDLING:	Calls fatalError().
void RegisterMsgs::RegisterMessages(TCPGate_Reg_ReqMsg *msg, int Reply)
{
	TCPGate_Reg_Struct *Reg_Struct = &msg->Msg_Struct.Reg_Struct;
	int i;

	//_LOG_ERROR( __FILE__,__LINE__, TRACE_GATEWAY, 
		//Reg_Struct->count, "Registering 'user' number of messages" );
	// register/de-register for ALL messages
	// count == -1 -> register all messages
	// count == 0  -> de-register all messages
	if(Reg_Struct->count == -1 || Reg_Struct->count == 0)
	{
		short v = (Reg_Struct->count == -1) ? 1 : 0;
#if 0
		if(!v)
			_LOG_ERROR( __FILE__,__LINE__, TRACE_GATEWAY, 
				v, "UnRegistering all messages" );
		else
			_LOG_ERROR( __FILE__,__LINE__, TRACE_GATEWAY, 
				v, "Registering all messages" );
#endif
		for(i = 0; i < LAST_INT32_MESSAGE; ++i)
			RegisteredMsgs[i] = v;
	}
	else
	// otherwise, step through the array
	for(i = 0; i < Reg_Struct->count; ++i)
	{
		if(abs(Reg_Struct->MsgIds[i]) >= LAST_INT32_MESSAGE 
		|| abs(Reg_Struct->MsgIds[i]) <= FIRST_BUFFER_MESSAGE)
			_LOG_ERROR( __FILE__,__LINE__, TRACE_GATEWAY, 
			Reg_Struct->MsgIds[i], "Attempt to Register Invalid MsgId" );
		else
			RegisteredMsgs[abs(Reg_Struct->MsgIds[i])] = 
				(Reg_Struct->MsgIds[i] > 0) ? 1 : 0;
	}

	// reply to AN2 host
	if(Reply == DoReply)
	{
		UDP_Reply<SAN2ServiceRequestReply> * service_reply;
		SAN2ServiceRequestReply request_reply;
	
		// Create the reply class in order to set up the reply for request for servi
		// HOST, PORT, STRUCTURE.
		service_reply = new UDP_Reply<SAN2ServiceRequestReply>
		( 
			msg->Msg_Struct.host_struct.szAN2HostIP, 
			msg->Msg_Struct.host_struct.szAN2HostPort, 
			(unsigned short)AN2ServiceRequestReplyMsg
		);
		// Initalize the request reply structure.
		request_reply.iAN2ServiceRequested =  REGISTER_TCP_MSGS;
		request_reply.iFSHostPort = 0;
		request_reply.iFSState = 0;
		request_reply.iAN2ServiceRequestStatus = SERVICE_ALLOWED;
		// Send the IP of the current current_server.
		static char *pFSIP = NULL;
		if(!pFSIP)
			pFSIP = getenv( "AN2IP" );
		strcpy( request_reply.szFSHostIP, pFSIP );
		// Send the reply.
		service_reply->send_message( & request_reply );
		delete service_reply;
	} 
}

// SPECIFICATION:	Determine if a message should be forwarded
//						parameter msg - a valid focus MsgId
//
// ERROR HANDLING:	Calls fatalError().
int RegisterMsgs::MessageRegistered(short msgID)
{
	if(msgID >= LAST_INT32_MESSAGE || msgID <= FIRST_BUFFER_MESSAGE
	||(msgID >= LAST_BUFFER_MESSAGE && msgID <= FIRST_INT32_MESSAGE))
	{
		char eString[256];
		sprintf(eString,"Unknown or Invalid MsgId %d range:(%d)-(%d)",
			msgID,FIRST_BUFFER_MESSAGE,LAST_INT32_MESSAGE);
		_LOG_ERROR( __FILE__,__LINE__, TRACE_GATEWAY, 
			msgID, eString );
		return -1;
	}
	return RegisteredMsgs[msgID];
}
