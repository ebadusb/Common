/*
 * Copyright (C) 2002 Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: K:/BCT_Development/vxWorks/Common/datalog/rcs/datalog_message.cpp 1.5 2003/02/25 16:10:14Z jl11312 Exp jl11312 $
 * $Log: datalog_message.cpp $
 *
 */

#include "datalog.h"
#include "datalog_internal.h"

DataLog_Result datalog_CreateLevel(const char * levelName, DataLog_Handle * handle)
{
	DataLog_Result result = DataLog_OK;
	DataLog_CommonData * common = datalog_GetCommonDataPtr();
	DataLog_CommonData::LevelInfoPtr levelInfo = common->findLevel(levelName);

	if ( levelInfo == DATALOG_NULL_SHARED_PTR )
	{
		levelInfo = (DataLog_CommonData::LevelInfoPtr)datalog_AllocSharedMem(sizeof(DataLog_LevelInfo));

		char * buffer = (char *)datalog_AllocSharedMem(strlen(levelName)+1);
		strcpy(buffer,levelName);
		levelInfo->_name = buffer;
		common->addLevel(levelInfo);

		levelInfo->_handle = (DataLog_Handle)datalog_AllocSharedMem(sizeof(DataLog_HandleInfo));
		levelInfo->_handle->_type = DataLog_HandleInfo::TraceHandle;
		levelInfo->_handle->_traceData._id = levelInfo->_id;
		levelInfo->_handle->_traceData._logOutput = DataLog_LogEnabled;
		levelInfo->_handle->_traceData._consoleOutput = DataLog_ConsoleDisabled;

	   DataLog_LogLevelRecord logLevelRecord;
	   logLevelRecord._recordType = DataLog_LogLevelRecordID;
		datalog_GetTimeStamp(&logLevelRecord._timeStamp);
	   logLevelRecord._levelID = levelInfo->_id;

#ifndef DATALOG_NO_NETWORK_SUPPORT
		logLevelRecord._nodeID = datalog_NodeID();
#endif /* ifndef DATALOG_NO_NETWORK_SUPPORT */

		logLevelRecord._nameLen = strlen(levelName);

		common->internalBuffer()->partialWriteStart(); 
		size_t writeSize = common->internalBuffer()->partialWrite((DataLog_BufferData *)&logLevelRecord, sizeof(logLevelRecord));
		writeSize += common->internalBuffer()->partialWrite((DataLog_BufferData *)levelName, logLevelRecord._nameLen * sizeof(char));
		common->internalBuffer()->partialWriteComplete();

		if ( writeSize != sizeof(logLevelRecord) + logLevelRecord._nameLen * sizeof(char) )
		{
			common->setTaskError(DataLog_LevelRecordWriteFailed, __FILE__, __LINE__);
			result = DataLog_Error;
		}
	}

	*handle = levelInfo->_handle;
	return result;
}

DataLog_Handle datalog_GetCriticalHandle(void)
{
	DataLog_Handle result = DATALOG_NULL_HANDLE;
	DataLog_CommonData * common = datalog_GetCommonDataPtr();
	DataLog_CommonData::TaskInfoPtr	taskInfo = common->findTask(DATALOG_CURRENT_TASK);

	if ( taskInfo == DATALOG_NULL_SHARED_PTR )
	{
		common->setTaskError(DataLog_NoSuchTask, __FILE__, __LINE__);
   }
   else
	{
		result = taskInfo->_criticalHandle;
	}

	return result;	
}

DataLog_Result datalog_GetDefaultLevel(DataLog_Handle * handle)
{
	DataLog_Result result = DataLog_OK;
	DataLog_CommonData * common = datalog_GetCommonDataPtr();
	DataLog_CommonData::TaskInfoPtr	taskInfo = common->findTask(DATALOG_CURRENT_TASK);

	if ( taskInfo == DATALOG_NULL_SHARED_PTR )
	{
		common->setTaskError(DataLog_NoSuchTask, __FILE__, __LINE__);
		result = DataLog_Error;
   }
   else
	{
		*handle = taskInfo->_defaultHandle;
	}

	return result;
}

DataLog_Result datalog_SetDefaultLevel(DataLog_Handle handle)
{
	DataLog_Result result = DataLog_OK;
	DataLog_CommonData * common = datalog_GetCommonDataPtr();
	DataLog_CommonData::TaskInfoPtr	taskInfo = common->findTask(DATALOG_CURRENT_TASK);

	if ( taskInfo == DATALOG_NULL_SHARED_PTR )
	{
		common->setTaskError(DataLog_NoSuchTask, __FILE__, __LINE__);
		result = DataLog_Error;
   }
   else
	{
		taskInfo->_defaultHandle = handle;
		taskInfo->_defaultLevel.setHandle(handle);
	}

	return result;
}

DataLog_Result datalog_GetTaskOutputOptions(DataLog_TaskID task, DataLog_EnabledType * log, DataLog_ConsoleEnabledType * console)
{
	DataLog_Result result = DataLog_OK;
	DataLog_CommonData * common = datalog_GetCommonDataPtr();
	DataLog_CommonData::TaskInfoPtr	taskInfo = common->findTask(DATALOG_CURRENT_TASK);

	if ( taskInfo == DATALOG_NULL_SHARED_PTR )
	{
		common->setTaskError(DataLog_NoSuchTask, __FILE__, __LINE__);
		result = DataLog_Error;
   }
   else
	{
		*log = taskInfo->_logOutput;
		*console = taskInfo->_consoleOutput;
	}

	return result;
}

DataLog_Result datalog_SetTaskOutputOptions(DataLog_TaskID task, DataLog_EnabledType log, DataLog_ConsoleEnabledType console)
{
	DataLog_Result result = DataLog_OK;
	DataLog_CommonData * common = datalog_GetCommonDataPtr();
	DataLog_CommonData::TaskInfoPtr	taskInfo = common->findTask(DATALOG_CURRENT_TASK);

	if ( taskInfo == DATALOG_NULL_SHARED_PTR )
	{
		common->setTaskError(DataLog_NoSuchTask, __FILE__, __LINE__);
		result = DataLog_Error;
   }
   else
	{
		taskInfo->_logOutput = log;
		taskInfo->_consoleOutput = console;
	}

	return result;
}

DataLog_Result datalog_GetLevelOutputOptions(DataLog_Handle handle, DataLog_EnabledType * log, DataLog_ConsoleEnabledType * console)
{
	DataLog_Result result = DataLog_OK;

	switch ( handle->_type )
	{
	case DataLog_HandleInfo::TraceHandle:
		*log = handle->_traceData._logOutput;
		*console = handle->_traceData._consoleOutput;
		break;

	case DataLog_HandleInfo::IntHandle:
		*log = handle->_intData._logOutput;
		*console = DataLog_ConsoleDisabled;
		break;

	case DataLog_HandleInfo::CriticalHandle:
		*log = DataLog_LogEnabled;
		*console = DataLog_ConsoleDisabled;
		break;

	default:
		{
			DataLog_CommonData * common = datalog_GetCommonDataPtr();
			common->setTaskError(DataLog_InvalidHandle, __FILE__, __LINE__);
			result = DataLog_Error;
		}
		break;
	}

	return result;
}

DataLog_Result datalog_SetLevelOutputOptions(DataLog_Handle handle, DataLog_EnabledType log, DataLog_ConsoleEnabledType console)
{
	DataLog_Result result = DataLog_OK;

	switch ( handle->_type )
	{
	case DataLog_HandleInfo::TraceHandle:
		handle->_traceData._logOutput = log;
		handle->_traceData._consoleOutput = console;
		break;

	case DataLog_HandleInfo::IntHandle:
		handle->_intData._logOutput = log;
		break;

	case DataLog_HandleInfo::CriticalHandle:
		break;

	default:
		{
			DataLog_CommonData * common = datalog_GetCommonDataPtr();
			common->setTaskError(DataLog_InvalidHandle, __FILE__, __LINE__);
			result = DataLog_Error;
		}
		break;
	}

	return result;
}

