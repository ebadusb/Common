/*
 * Copyright (C) 2002 Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: K:/BCT_Development/vxWorks/Common/datalog/rcs/datalog_init.cpp 1.8 2003/04/11 15:26:11Z jl11312 Exp jl11312 $
 * $Log: datalog_init.cpp $
 * Revision 1.4  2003/02/06 20:41:30  jl11312
 * - added support for binary record type
 * - added support for symbolic node names in networked configurations
 * - enabled compression/encryption of log files
 * Revision 1.3  2002/08/22 20:19:11  jl11312
 * - added network support
 * Revision 1.2  2002/08/15 20:53:54  jl11312
 * - added support for periodic logging
 * Revision 1.1  2002/07/18 21:21:00  jl11312
 * Initial revision
 *
 */

#include "datalog.h"
#include "datalog_internal.h"

#ifdef DATALOG_NETWORK_SUPPORT
DataLog_Result datalog_Init(size_t bufferSizeKBytes, size_t criticalReserveKBytes, const char * logPath, const char * platformName, const char * nodeName)
#else /* ifdef DATALOG_NETWORK_SUPPORT */
DataLog_Result datalog_Init(size_t bufferSizeKBytes, size_t criticalReserveKBytes, const char * logPath, const char * platformName)
#endif /* ifdef DATALOG_NETWORK_SUPPORT */
{
	DataLog_Result	result = DataLog_OK;

	if ( !DataLog_CommonData::startInitialization() )
	{
		DataLog_CommonData common;
		common.setTaskError(DataLog_MultipleInitialization, __FILE__, __LINE__);
		result = DataLog_Error;
	}
	else
	{
		//
		// Allocate buffer space use for data log
		//
		DataLog_BufferManager::initialize(bufferSizeKBytes);

		//
		// The task hooks for create/delete are installed only after the common data
		// pointer is set, so we must add the task info for any existing tasks manually.
		// The current task is locked so that we will not get tasks created or
		// deleted during this logging phase.
		//
		taskLock();
		int	idList[64];
		int	idCount = taskIdListGet(idList, 64);

		for ( int i=0; i<idCount; i++ )
		{
			datalog_TaskCreated(idList[i]);
		}

		DataLog_CommonData common;
		common.setLocalConnect(logPath);
		common.setCriticalReserveBuffers((criticalReserveKBytes*1024+DataLog_BufferSize-1)/DataLog_BufferSize);
		taskUnlock();

#ifdef DATALOG_NETWORK_SUPPORT
		datalog_StartLocalOutputTask(platformName, nodeName);
#else /* ifdef DATALOG_NETWORK_SUPPORT */
		datalog_StartLocalOutputTask(platformName, NULL);
#endif /* ifdef DATALOG_NETWORK_SUPPORT */

#ifdef DATALOG_NETWORK_SUPPORT
		datalog_StartNetworkTask();
#endif /* ifdef DATALOG_NETWORK_SUPPORT */
	}

	return result;
}

#ifdef DATALOG_NETWORK_SUPPORT
DataLog_Result datalog_InitNet(size_t bufferSizeKBytes, size_t criticalReserveKBytes, const char * ipAddress, int port, long connectTimeout, const char * nodeName)
{
	DataLog_Result	result = DataLog_OK;

	if ( !DataLog_CommonData::startInitialization() )
	{
		DataLog_CommonData common;
		common.setTaskError(DataLog_MultipleInitialization, __FILE__, __LINE__);
		result = DataLog_Error;
	}
	else
	{
		//
		// Allocate buffer space use for data log
		//
		DataLog_BufferManager::initialize(bufferSizeKBytes);

		//
		// The task hooks for create/delete are installed only after the common data
		// pointer is set, so we must add the task info for any existing tasks manually.
		// The current task is locked so that we will not get tasks created or
		// deleted during this logging phase.
		//
		taskLock();
		int	idList[64];
		int	idCount = taskIdListGet(idList, 64);

		for ( int i=0; i<idCount; i++ )
		{
			datalog_TaskCreated(idList[i]);
		}

		DataLog_CommonData common;
		common.setNetworkConnect(ipAddress, port);
		common.setCriticalReserveBuffers((criticalReserveKBytes*1024+DataLog_BufferSize-1)/DataLog_BufferSize);
		taskUnlock();

		datalog_StartNetworkOutputTask(connectTimeout, nodeName);
		datalog_StartNetworkTask();
	}

	return result;
}
#endif /* ifdef DATALOG_NETWORK_SUPPORT */

