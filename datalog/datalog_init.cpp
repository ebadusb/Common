/*
 * Copyright (C) 2002 Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: K:/BCT_Development/vxWorks/Common/datalog/rcs/datalog_init.cpp 1.8 2003/04/11 15:26:11Z jl11312 Exp jl11312 $
 * $Log: datalog_init.cpp $
 *
 */

#include <vxWorks.h>
#include "datalog.h"
#include "datalog_internal.h"

DataLog_Result datalog_Init(const char * logPath, const char * platformName)
{
	DataLog_Result	result = DataLog_OK;
	DataLog_CommonData * common = DATALOG_NULL_SHARED_PTR;

	if ( datalog_StartInitialization() != DataLog_OK )
	{
		common = datalog_GetCommonDataPtr();
		common->setTaskError(DataLog_MultipleInitialization, __FILE__, __LINE__);
		result = DataLog_Error;
	}
	else
	{
		void * commonDataArea = datalog_AllocSharedMem(sizeof(DataLog_CommonData));
		common = new(commonDataArea)DataLog_CommonData;

		//
		// The task hooks for create/delete are installed only after the common data
		// pointer is set, so we must add the task info for the current (initialization)
		// task manually.
		//
		datalog_SetCommonDataPtr(common);
		datalog_TaskCreated(datalog_CurrentTask());

		common->setConnect(DataLog_CommonData::LogToFile, logPath);
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
	DataLog_CommonData * common = datalog_GetCommonDataPtr();
	common->defaultTraceBufferSize() = size;
   return DataLog_OK;
}

DataLog_Result datalog_SetDefaultIntBufferSize(size_t size)
{
	DataLog_CommonData * common = datalog_GetCommonDataPtr();
	common->defaultIntBufferSize() = size;
   return DataLog_OK;
}

DataLog_Result datalog_SetDefaultCriticalBufferSize(size_t size)
{
	DataLog_CommonData * common = datalog_GetCommonDataPtr();
	common->defaultCriticalBufferSize() = size;
   return DataLog_OK;
}

