/*
 * Copyright (C) 2002 Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: K:/BCT_Development/vxWorks/Common/datalog/rcs/datalog_init.cpp 1.8 2003/04/11 15:26:11Z jl11312 Exp jl11312 $
 * $Log: datalog_init.cpp $
 * Revision 1.3  2002/08/22 20:19:11  jl11312
 * - added network support
 * Revision 1.2  2002/08/15 20:53:54  jl11312
 * - added support for periodic logging
 * Revision 1.1  2002/07/18 21:21:00  jl11312
 * Initial revision
 *
 */

#include <vxWorks.h>
#include "datalog.h"
#include "datalog_internal.h"

#ifdef DATALOG_NO_NETWORK_SUPPORT
DataLog_Result datalog_Init(const char * logPath, const char * platformName)
#else /* ifdef DATALOG_NO_NETWORK_SUPPORT */
DataLog_Result datalog_Init(const char * logPath, const char * platformName, const char * nodeName)
#endif /* ifdef DATALOG_NO_NETWORK_SUPPORT */
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
		// The task hooks for create/delete are installed only after the common data
		// pointer is set, so we must add the task info for the current (initialization)
		// task manually.
		//
		datalog_TaskCreated(datalog_CurrentTask());

		DataLog_CommonData common;
		common.setLocalConnect(logPath);

#ifdef DATALOG_NO_NETWORK_SUPPORT
		datalog_StartLocalOutputTask(platformName, NULL);
#else /* ifdef DATALOG_NO_NETWORK_SUPPORT */
		datalog_StartLocalOutputTask(platformName, nodeName);
#endif /* ifdef DATALOG_NO_NETWORK_SUPPORT */

#ifndef DATALOG_NO_NETWORK_SUPPORT
		datalog_StartNetworkTask();
#endif /* ifndef DATALOG_NO_NETWORK_SUPPORT */
	}

	return result;
}

#ifndef DATALOG_NO_NETWORK_SUPPORT
DataLog_Result datalog_InitNet(const char * ipAddress, int port, long connectTimeout, const char * nodeName)
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
		// The task hooks for create/delete are installed only after the common data
		// pointer is set, so we must add the task info for the current (initialization)
		// task manually.
		//
		datalog_TaskCreated(datalog_CurrentTask());

		DataLog_CommonData common;
		common.setNetworkConnect(ipAddress, port);
		datalog_StartNetworkOutputTask(connectTimeout, nodeName);
		datalog_StartNetworkTask();
	}

	return result;
}
#endif /* ifndef DATALOG_NO_NETWORK_SUPPORT */

DataLog_Result datalog_SetDefaultTraceBufferSize(size_t size)
{
	DataLog_CommonData common;
	common.setDefaultTraceBufferSize(size);
   return DataLog_OK;
}

DataLog_Result datalog_SetDefaultIntBufferSize(size_t size)
{
	DataLog_CommonData common;
	common.setDefaultIntBufferSize(size);
   return DataLog_OK;
}

DataLog_Result datalog_SetDefaultCriticalBufferSize(size_t size)
{
	DataLog_CommonData common;
	common.setDefaultCriticalBufferSize(size);
   return DataLog_OK;
}

