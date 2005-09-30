/*
 * Copyright (C) 2002 Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: //bctquad3/home/BCT_Development/vxWorks/Common/datalog/rcs/datalog_init.cpp 1.10 2005/09/29 21:59:56Z ms10234 Exp ms10234 $
 * $Log: datalog_init.cpp $
 * Revision 1.9  2003/11/10 17:46:09Z  jl11312
 * - corrections from data log unit tests (see IT 6598)
 * Revision 1.8  2003/04/11 15:26:11Z  jl11312
 * - added support for pre-initialized critical levels and handles
 * Revision 1.7  2003/03/27 16:26:59Z  jl11312
 * - added support for new datalog levels
 * Revision 1.6  2003/02/25 20:43:02Z  jl11312
 * - added support for logging platform specific information in log header
 * Revision 1.5  2003/02/25 16:10:11  jl11312
 * - modified buffering scheme to help prevent buffer overruns
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

#ifdef DATALOG_LEVELS_INIT_SUPPORT
#define DATALOG_LEVELS_INIT_TABLE
#include "datalog_levels.h"

static void datalog_InitLevels(void)
{
	datalog_CreateCriticalLevel(&log_handle_critical);
	log_level_critical.setHandle(log_handle_critical);

	for (int level=0; level<sizeof(initTable)/sizeof(initTable[0]); level+=1)
	{
		datalog_CreateLevel(initTable[level].name, initTable[level].handle);
		initTable[level].level->setHandle(*initTable[level].handle);

		if ( !initTable[level].enabled )
		{
			initTable[level].level->logOutput(DataLog_LogDisabled);
		}
	}
}
#endif /* ifdef DATALOG_LEVELS_INIT_SUPPORT */

#ifdef DATALOG_NETWORK_SUPPORT
DataLog_Result datalog_Init(size_t bufferSizeKBytes, size_t criticalReserveKBytes, const char * logPath, const char * platformName, const char * nodeName, const char * platformInfo, bool allowReset)
#else /* ifdef DATALOG_NETWORK_SUPPORT */
DataLog_Result datalog_Init(size_t bufferSizeKBytes, size_t criticalReserveKBytes, const char * logPath, const char * platformName, const char * platformInfo, bool allowReset)
#endif /* ifdef DATALOG_NETWORK_SUPPORT */
{
	DataLog_Result	result = DataLog_OK;

	assert(1024*bufferSizeKBytes > 10*DataLog_BufferSize);
	assert(criticalReserveKBytes < bufferSizeKBytes);
	assert(logPath != NULL);
	assert(platformName != NULL);

#ifdef DATALOG_NETWORK_SUPPORT
	assert(nodeName != NULL);
#endif

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
		common.setPersistSystemInfo(allowReset);
		common.setPlatformName(platformName);
		common.setPlatformInfo(platformInfo);
		taskUnlock();

#ifdef DATALOG_NETWORK_SUPPORT
		common.setPlatformName(nodeName);
		datalog_StartLocalOutputTask(platformName, nodeName, platformInfo);
#else /* ifdef DATALOG_NETWORK_SUPPORT */
		datalog_StartLocalOutputTask(platformName, NULL, platformInfo);
#endif /* ifdef DATALOG_NETWORK_SUPPORT */

#ifdef DATALOG_LEVELS_INIT_SUPPORT
		datalog_InitLevels();
#endif /* ifdef DATALOG_LEVELS_INIT_SUPPORT */

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

	assert(1024*bufferSizeKBytes > 10*DataLog_BufferSize);
	assert(criticalReserveKBytes < bufferSizeKBytes);
	assert(ipAddress != NULL);
	assert(nodeName != NULL);

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

#ifdef DATALOG_LEVELS_INIT_SUPPORT
		datalog_InitLevels();
#endif /* ifdef DATALOG_LEVELS_INIT_SUPPORT */
	}

	return result;
}
#endif /* ifdef DATALOG_NETWORK_SUPPORT */

