/*
 * Copyright (c) 1999 by Cobe BCT, Inc.  All rights reserved.
 *
 * $Header: Q:/home1/COMMON_PROJECT/Source/INCLUDE/rcs/PreRegisterMsgs.h 1.3 1999/07/26 22:30:10 TD10216 Exp TD10216 $
 * $Log: PreRegisterMsgs.h $
 * Revision 1.1  1999/07/24 20:50:41  TD10216
 * Initial revision
 *
 *
 * ABSTRACT:Code shared between tcp_gate and everest_logger which
 * doesnt really belong in any of our libraries.
 *
 *
*/

// SPECIFICATION:	Read File to get pre-registered Everest messages
//					Parameter: none
//
// ERROR HANDLING:	none
static void PreRegisterMsgs( int RegisterRegistrationMsg )
{
	TCPGate_Reg_ReqMsg aRegMsg;
	FILE *EnumDefFile;

	char *EVERESTMAP = getenv("EVERESTMAP");
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
		char *p = strchr(buf,'\n');
		if(p)
			*p = '\0';
		int b = atoi(buf);
		aRegMsg.Msg_Struct.Reg_Struct.MsgIds[aRegMsg.Msg_Struct.Reg_Struct.count++] 
			= (short)b;
	}
	fclose(EnumDefFile);
	// catch remainder or all if < MAXREG - 1
	if(aRegMsg.Msg_Struct.Reg_Struct.count)	
		MsgRegistrar.RegisterMessages( &aRegMsg, RegisterMsgs::DontReply);
}
