/*
 * Copyright (C) 2002 Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: K:/BCT_Development/vxWorks/Common/datalog/rcs/datalog_init.cpp 1.8 2003/04/11 15:26:11Z jl11312 Exp jl11312 $
 * $Log: datalog_init.cpp $
 * Revision 1.1  2002/07/18 21:21:00  jl11312
 * Initial revision
 *
 */

#include <vxWorks.h>
#include "datalog.h"
#include "datalog_internal.h"

DataLog_Result datalog_Init(const char * logPath, const char * platformName)
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
		common.setConnect(DataLog_CommonData::LogToFile, logPath);
		datalog_StartOutputTask(platformName);
	}

	return result;
}

DataLog_Result datalog_InitNet(const char * ipAddress, double seconds)
{
	assert(0);
	return DataLog_OK;
}

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

