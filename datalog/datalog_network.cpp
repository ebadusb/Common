/*
 * Copyright (C) 2002 Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: //bctquad3/home/BCT_Development/vxWorks/Common/datalog/rcs/datalog_network.cpp 1.5 2003/12/09 14:14:28Z jl11312 Exp rm70006 $
 * $Log: datalog_network.cpp $
 *
 */

#include <vxWorks.h>
#include <errnoLib.h>
#include <fcntl.h>
#include <ioLib.h>
#include <sysLib.h>

#include "datalog.h"
#include "datalog_internal.h"

DataLog_NetworkTask::DataLog_NetworkTask(int port)
{
	_port = port;
	_isExiting = false;
	_exitCode = 0;
}

void DataLog_NetworkTask::exit(int code)
{
	_exitCode = code;
	_isExiting = true; 
}

int DataLog_NetworkTask::main(void)
{
	DataLog_Critical	critical;
	int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
   sockaddr_in	addr;

	memset(&addr , 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = _port;

   bind(serverSocket, (sockaddr *)&addr, sizeof(addr));
   while ( 1 )
	{
		struct sockaddr_in	clientAddr;
		int 	clientAddrSize = sizeof(clientAddr);
		int	clientSocket;
		char	ipAddress[INET_ADDR_LEN];

		listen(serverSocket, 0);
		clientSocket = accept(serverSocket, (sockaddr *)&clientAddr, &clientAddrSize);

		if ( clientSocket >= 0 )
		{
			inet_ntoa_b(clientAddr.sin_addr, ipAddress);
			DataLog(critical) << "DataLog network task accepted connection from " << ipAddress << endmsg;

			datalog_StartNetworkClientTask(clientSocket, &clientAddr);
		}
		else
		{
			DataLog(critical) << "DataLog network task accept call failed, errno=" << errnoGet() << endmsg;
			taskDelay(10*sysClkRateGet());
		}
	}

	shutdown(serverSocket, 2);
	close(serverSocket);
	return _exitCode;
}

