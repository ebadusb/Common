/*
 * Copyright (C) 2002 Gambro BCT, Inc.  All rights reserved.
 *
 * $Header$
 * $Log: datalog_network.cpp $
 * Revision 1.5  2003/12/09 14:14:28Z  jl11312
 * - corrected time stamp problem (IT 6668)
 * - removed obsolete code/data types (IT 6664)
 * Revision 1.4  2003/03/27 16:27:02Z  jl11312
 * - added support for new datalog levels
 * Revision 1.3  2003/02/25 16:10:21Z  jl11312
 * - modified buffering scheme to help prevent buffer overruns
 * Revision 1.2  2002/09/23 15:35:36  jl11312
 * - fixed port number setting
 * Revision 1.1  2002/08/22 20:19:02  jl11312
 * Initial revision
 *
 */

#include "datalog.h"

#include <fcntl.h>
#include "datalog_internal.h"

#ifdef DATALOG_LEVELS_INIT_SUPPORT
# include "datalog_levels.h"
#endif /* ifdef DATALOG_LEVELS_INIT_SUPPORT */

DataLog_NetworkTask::DataLog_NetworkTask(int port)
{
	_port = port;
}

void DataLog_NetworkTask::main(void)
{
	int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
   sockaddr_in	addr;

	memset(&addr , 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(_port);

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

#ifdef DATALOG_LEVELS_INIT_SUPPORT
			DataLog(log_level_datalog_info)
#else /* ifdef DATALOG_LEVELS_INIT_SUPPORT */
			DataLog_Default
#endif /* ifdef DATALOG_LEVELS_INIT_SUPPORT */
				<< "DataLog network task accepted connection from " << ipAddress << endmsg;

			datalog_StartNetworkClientTask(clientSocket, &clientAddr);
		}
		else
		{
#ifdef DATALOG_LEVELS_INIT_SUPPORT
			DataLog(log_level_datalog_error)
#else /* ifdef DATALOG_LEVELS_INIT_SUPPORT */
			DataLog_Default
#endif /* ifdef DATALOG_LEVELS_INIT_SUPPORT */
				<< "DataLog network task accept call failed, errno=" << errnoGet() << endmsg;

			taskDelay(10*sysClkRateGet());
		}
	}
}

