/*
 * Copyright (c) 1999 by Cobe BCT, Inc.  All rights reserved.
 *
 * $Header: Q:/home1/COMMON_PROJECT/Source/INCLUDE/rcs/PreRegisterMsgs.h 1.3 1999/07/26 22:30:10 TD10216 Exp $
 * $Log: PreRegisterMsgs.h $
 * Revision 1.3  1999/07/26 22:30:10  TD10216
 * IT4154
 * Revision 1.2  1999/07/25 02:28:37  TD10216
 * IT4154
 * Revision 1.1  1999/07/24 20:50:41  TD10216
 * Initial revision
 *
 *
 * ABSTRACT:Code shared between tcp_gate and everest_logger which
 * doesnt really belong in any of our libraries.
 *
 *
*/

#include <ctype.h>

// SPECIFICATION:	Read File to get pre-registered Everest messages
//					Parameter: none
//
// ERROR HANDLING:	none
static void PreRegisterMsgs( int RegisterRegistrationMsg )
{
	TCPGate_Reg_ReqMsg aRegMsg;
	FILE *EnumDefFile;

	char *EVERESTMAP = getenv("EVERESTMAP");
	if(!EVERESTMAP)
	{
        _FATAL_ERROR( __FILE__,__LINE__, TRACE_GATEWAY, errno,  
            "EVERESTMAP environment variable not set from (?globvars?)");
		return;
	}
	if((EnumDefFile = fopen(EVERESTMAP,"r")) == NULL)
	{
        _FATAL_ERROR( __FILE__,__LINE__, TRACE_GATEWAY, errno,  
            "Cant open EverstMsgMap file");
		return;
	}
	if(RegisterRegistrationMsg)
	{
		aRegMsg.Msg_Struct.Reg_Struct.count = 1;
		// catch and log all external tcpgate msgs which are registered
		aRegMsg.Msg_Struct.Reg_Struct.MsgIds[0] = TcpgateRegistration;
	}
	else
		aRegMsg.Msg_Struct.Reg_Struct.count = 0;

	char buf[80];
	while(fgets(buf,sizeof(buf),EnumDefFile) != NULL)
	{
		// only register MAXREG messages at a time
		if(aRegMsg.Msg_Struct.Reg_Struct.count == MAXREG - 1)
		{
			MsgRegistrar.RegisterMessages( &aRegMsg, RegisterMsgs::DontReply);
			aRegMsg.Msg_Struct.Reg_Struct.count = 0;
		}
		// lose leading spaces
		int i = 0;
		while(isspace(buf[i]))
			++i;
		// ignore entire lines beginning with '#'
		if(buf[i] == '#')
		{
			continue;
		}
		// get the message number,ignore trailing cruft
		while(isdigit(buf[i]))
			++i;
		buf[i] = '\0';	
		int b = atoi(buf);
		aRegMsg.Msg_Struct.Reg_Struct.MsgIds[aRegMsg.Msg_Struct.Reg_Struct.count++] 
			= (short)b;
	}
	fclose(EnumDefFile);
#if 0	// for debugging
	for(int j = 0; j < aRegMsg.Msg_Struct.Reg_Struct.count; j++)
	{
		char eString[80];
		sprintf(eString,"Registered %d",
			aRegMsg.Msg_Struct.Reg_Struct.MsgIds[j]);
		_LOG_ERROR(__FILE__,__LINE__,TRACE_DISPATCHER,0,eString);
	}
#endif
	// catch remainder or all if < MAXREG - 1
	if(aRegMsg.Msg_Struct.Reg_Struct.count)	
		MsgRegistrar.RegisterMessages( &aRegMsg, RegisterMsgs::DontReply);
}
