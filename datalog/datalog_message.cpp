/*
 * Copyright (C) 2002 Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: K:/BCT_Development/vxWorks/Common/datalog/rcs/datalog_message.cpp 1.5 2003/02/25 16:10:14Z jl11312 Exp jl11312 $
 * $Log: datalog_message.cpp $
 * Revision 1.2  2002/08/15 20:53:55  jl11312
 * - added support for periodic logging
 * Revision 1.1  2002/07/18 21:20:54  jl11312
 * Initial revision
 *
 */

#include "datalog.h"
#include "datalog_internal.h"

DataLog_Result datalog_CreateLevel(const char * levelName, DataLog_Handle * handle)
{
	DataLog_Result result = DataLog_OK;
	DataLog_CommonData common;
	const DataLog_HandleInfo * handleInfo = common.findHandle(levelName);

	if ( !handleInfo )
	{
		//
		// Create new log level
		//
		DataLog_HandleInfo * newHandle = new DataLog_HandleInfo;
		newHandle->_id = common.getNextInternalID();
		newHandle->_type = DataLog_HandleInfo::TraceHandle;
		newHandle->_traceData._logOutput = DataLog_LogEnabled;
		newHandle->_traceData._consoleOutput = DataLog_ConsoleDisabled;
		common.addHandle(levelName, newHandle);
		handleInfo = newHandle;

		//
		// Write record to log showing addition of new log level
		//
	   DataLog_LogLevelRecord logLevelRecord;
	   logLevelRecord._recordType = DataLog_LogLevelRecordID;
		datalog_GetTimeStamp(&logLevelRecord._timeStamp);
	   logLevelRecord._levelID = handleInfo->_id;

#ifndef DATALOG_NO_NETWORK_SUPPORT
		logLevelRecord._nodeID = datalog_NodeID();
#endif /* ifndef DATALOG_NO_NETWORK_SUPPORT */

		logLevelRecord._nameLen = strlen(levelName);

		DataLog_CriticalBuffer * buffer = common.getTaskCriticalBuffer(DATALOG_CURRENT_TASK);
		DataLog_Stream & stream = buffer->streamWriteStart();

		stream.write(&logLevelRecord, sizeof(logLevelRecord));
		stream.write(levelName, logLevelRecord._nameLen * sizeof(char));
		size_t writeSize = buffer->streamWriteComplete();
 
		if ( writeSize != sizeof(logLevelRecord) + logLevelRecord._nameLen * sizeof(char) )
		{
			common.setTaskError(DataLog_LevelRecordWriteFailed, __FILE__, __LINE__);
			result = DataLog_Error;
		}
	}

	*handle = handleInfo;
	return result;
}

DataLog_Handle datalog_GetCriticalHandle(void)
{
	DataLog_CommonData	common;
	return &common._criticalHandleInfo;
}

DataLog_Result datalog_SetDefaultLevel(DataLog_Handle handle)
{
	DataLog_CommonData	common;
	DataLog_TaskInfo * taskInfo = common.findTask(DATALOG_CURRENT_TASK);

	taskInfo->_defaultHandle = handle;
	taskInfo->_defaultLevel.setHandle(handle);
	return DataLog_OK;
}

DataLog_Result datalog_GetTaskOutputOptions(DataLog_TaskID task, DataLog_EnabledType * log, DataLog_ConsoleEnabledType * console)
{
	DataLog_CommonData	common;
	DataLog_TaskInfo * taskInfo = common.findTask(DATALOG_CURRENT_TASK);

	*log = taskInfo->_logOutput;
	*console = taskInfo->_consoleOutput;
	return DataLog_OK;
}

DataLog_Result datalog_SetTaskOutputOptions(DataLog_TaskID task, DataLog_EnabledType log, DataLog_ConsoleEnabledType console)
{
	DataLog_CommonData	common;
	DataLog_TaskInfo * taskInfo = common.findTask(DATALOG_CURRENT_TASK);

	taskInfo->_logOutput = log;
	taskInfo->_consoleOutput = console;
	return DataLog_OK;
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
			DataLog_CommonData common;
			common.setTaskError(DataLog_InvalidHandle, __FILE__, __LINE__);
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
		((DataLog_HandleInfo *)handle)->_traceData._logOutput = log;
		((DataLog_HandleInfo *)handle)->_traceData._consoleOutput = console;
		break;

	case DataLog_HandleInfo::IntHandle:
		((DataLog_HandleInfo *)handle)->_intData._logOutput = log;
		break;

	case DataLog_HandleInfo::CriticalHandle:
		break;

	default:
		{
			DataLog_CommonData common;
			common.setTaskError(DataLog_InvalidHandle, __FILE__, __LINE__);
			result = DataLog_Error;
		}
		break;
	}

	return result;
}

