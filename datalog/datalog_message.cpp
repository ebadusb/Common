/*
 * Copyright (C) 2002 Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: //bctquad3/home/BCT_Development/vxWorks/Common/datalog/rcs/datalog_message.cpp 1.7 2003/11/24 23:06:11Z jl11312 Exp ms10234 $
 * $Log: datalog_message.cpp $
 * Revision 1.5  2003/02/25 16:10:14  jl11312
 * - modified buffering scheme to help prevent buffer overruns
 * Revision 1.4  2003/01/31 19:52:50  jl11312
 * - new stream format for datalog
 * Revision 1.3  2002/08/28 14:37:07  jl11312
 * - changed handling of critical output to avoid problem with handles referencing deleted tasks
 * Revision 1.2  2002/08/15 20:53:55  jl11312
 * - added support for periodic logging
 * Revision 1.1  2002/07/18 21:20:54  jl11312
 * Initial revision
 *
 */

#include "datalog.h"
#include "datalog_internal.h"
#include "datalog_records.h"

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
		newHandle->_logOutput = DataLog_LogEnabled;
		newHandle->_consoleOutput = DataLog_ConsoleDisabled;
		common.addHandle(levelName, newHandle);
		handleInfo = newHandle;

		//
		// Write record to log showing addition of new log level
		//
	   DataLog_LogLevelRecord logLevelRecord;
	   logLevelRecord._recordType = DataLog_LogLevelRecordID;
		datalog_GetTimeStamp(&logLevelRecord._timeStamp);
	   logLevelRecord._levelID = handleInfo->_id;

#ifdef DATALOG_NETWORK_SUPPORT
		logLevelRecord._nodeID = datalog_NodeID();
#endif /* ifdef DATALOG_NETWORK_SUPPORT */

		logLevelRecord._nameLen = strlen(levelName);

		DataLog_BufferChain	outputChain;
		bool	outputOK = false;

		if ( DataLog_BufferManager::createChain(outputChain) &&
			  DataLog_BufferManager::writeToChain(outputChain, (DataLog_BufferData *)&logLevelRecord, sizeof(logLevelRecord)) &&
			  DataLog_BufferManager::writeToChain(outputChain, (DataLog_BufferData *)levelName, logLevelRecord._nameLen * sizeof(char)) )
		{
			outputOK = true;
			DataLog_BufferManager::addChainToList(DataLog_BufferManager::CriticalList, outputChain);
		}
 
		if ( !outputOK )
		{
			common.setTaskError(DataLog_LevelRecordWriteFailed, __FILE__, __LINE__);
			result = DataLog_Error;
		}
	}

	*handle = handleInfo;
	return result;
}

DataLog_Result datalog_CreateCriticalLevel(DataLog_Handle * handle)
{
	DataLog_CommonData	common;
	*handle = &common._criticalHandleInfo;
	return DataLog_OK;
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
		*log = handle->_logOutput;
		*console = handle->_consoleOutput;
		break;

	case DataLog_HandleInfo::IntHandle:
		*log = handle->_logOutput;
		*console = DataLog_ConsoleDisabled;
		break;

	case DataLog_HandleInfo::CriticalHandle:
		*log = DataLog_LogEnabled;
		*console = DataLog_ConsoleEnabled;
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
		((DataLog_HandleInfo *)handle)->_logOutput = log;
		((DataLog_HandleInfo *)handle)->_consoleOutput = console;
		break;

	case DataLog_HandleInfo::IntHandle:
		((DataLog_HandleInfo *)handle)->_logOutput = log;
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

