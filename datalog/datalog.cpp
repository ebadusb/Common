/*
 * Copyright (C) 2002 Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: //bctquad3/home/BCT_Development/vxWorks/Common/datalog/rcs/datalog.cpp 1.13 2003/12/05 16:33:05Z jl11312 Exp rm70006 $
 * $Log: datalog.cpp $
 * Revision 1.7  2002/08/28 14:37:07  jl11312
 * - changed handling of critical output to avoid problem with handles referencing deleted tasks
 * Revision 1.6  2002/08/22 20:19:10  jl11312
 * - added network support
 * Revision 1.5  2002/08/15 20:53:54  jl11312
 * - added support for periodic logging
 * Revision 1.4  2002/07/18 21:20:29  jl11312
 * - preliminary implementation
 * Revision 1.3  2002/06/04 20:22:53  jl11312
 * - added stubs for default level related functions
 * Revision 1.2  2002/05/17 18:28:34  jl11312
 * - temporary changes for debug use under vxWorks
 * Revision 1.1  2002/05/17 14:51:29  jl11312
 * Initial revision
 *
 */

#define DATALOG_DECLARE_ERROR_INFORMATION
#include "datalog.h"
#include "datalog_internal.h"
#include "error.h"

DataLog_Map<DataLog_TaskID, DataLog_TaskInfo *> DataLog_CommonData::_tasks;
DataLog_Lock DataLog_CommonData::_tasksLock = datalog_CreateLock();

DataLog_Map<DataLog_InternalID, DataLog_Handle> DataLog_CommonData::_handles;
DataLog_Lock DataLog_CommonData::_handlesLock = datalog_CreateLock();
const DataLog_HandleInfo DataLog_CommonData::_criticalHandleInfo = { 0, DataLog_HandleInfo::CriticalHandle };

DataLog_CommonData::DataLog_CommonData(void)
{
	if ( !isInitialized() )
	{
		_FATAL_ERROR(__FILE__, __LINE__, "datalog system not initialized");
	}

	setCommonDataPtr(); 
}

DataLog_TaskInfo * DataLog_CommonData::findTask(DataLog_TaskID task)
{
	DataLog_TaskID	searchTask = (task == DATALOG_CURRENT_TASK) ? datalog_CurrentTask() : task;

	datalog_LockAccess(_tasksLock);
	if ( _tasks.find(searchTask) == _tasks.end() )
	{
		datalog_ReleaseAccess(_tasksLock);
		datalog_TaskCreated(datalog_CurrentTask());
		datalog_LockAccess(_tasksLock);
	}

	DataLog_TaskInfo * result = _tasks[searchTask];
	datalog_ReleaseAccess(_tasksLock);
	return result;
}

void DataLog_CommonData::addTask(DataLog_TaskInfo * taskInfo)
{
	datalog_LockAccess(_tasksLock);
	if ( _tasks.find(taskInfo->_id) != _tasks.end() )
	{
		DataLog_Critical	errorLog;
		DataLog(errorLog) << "attempt to add duplicate task ignored" << endmsg;
	}
	else
	{
		_tasks[taskInfo->_id] = taskInfo;
	}

	datalog_ReleaseAccess(_tasksLock);
}

void DataLog_CommonData::deleteTask(DataLog_TaskID task)
{
	DataLog_TaskInfo * taskInfo = NULL;

	datalog_LockAccess(_tasksLock);
	if ( _tasks.find(task) == _tasks.end() )
	{
		DataLog_Critical	errorLog;
		DataLog(errorLog) << "attempt to delete non-existent task ignored" << endmsg;
	}
	else
	{
		taskInfo = _tasks[task];
		_tasks.erase(task);
	}

	datalog_ReleaseAccess(_tasksLock);
	if ( taskInfo )
	{
		delete taskInfo->_critical;
		delete taskInfo->_trace;
		delete taskInfo;
	}
}

DataLog_Handle DataLog_CommonData::findHandle(const char * levelName)
{
	DataLog_Handle	result = DATALOG_NULL_HANDLE;

	datalog_LockAccess(_handlesLock);

	DataLog_InternalID levelID = lookupLevelID(levelName);
	if ( levelID != DATALOG_NULL_ID )
	{
		result = _handles[levelID];
	}

	datalog_ReleaseAccess(_handlesLock);
	return result;
}

void DataLog_CommonData::addHandle(const char * levelName, DataLog_Handle handle)
{
	datalog_LockAccess(_handlesLock);
	
	if ( handle->_id == DATALOG_NULL_ID )
	{
		DataLog_Critical	errorLog;
		DataLog(errorLog) << "attempt to add handle with NULL ID ignored" << endmsg;
	}
	else
	{
		registerLevelID(levelName, handle->_id);
		_handles[handle->_id] = handle;
	}

	datalog_ReleaseAccess(_handlesLock);
}

DataLog_TraceBuffer * DataLog_CommonData::getTaskTraceBuffer(DataLog_TaskID taskID)
{
	DataLog_TaskInfo * taskInfo = findTask(taskID);
	return taskInfo->_trace;
}

DataLog_CriticalBuffer * DataLog_CommonData::getTaskCriticalBuffer(DataLog_TaskID taskID)
{
	DataLog_TaskInfo * taskInfo = findTask(taskID);
	return taskInfo->_critical;
}

void DataLog_CommonData::setLocalConnect(const char * fileName)
{
	_commonData->_connectType = LogToFile;

	DataLog_SharedPtr(char)	buffer = (DataLog_SharedPtr(char))datalog_AllocSharedMem(strlen(fileName)+1);
	strcpy(buffer, fileName);
	_commonData->_connectName = buffer;
	_commonData->_connectPort = 0;
}

void DataLog_CommonData::setNetworkConnect(const char * ipAddress, int port)
{
	_commonData->_connectType = LogToNetwork;

	DataLog_SharedPtr(char)	buffer = (DataLog_SharedPtr(char))datalog_AllocSharedMem(strlen(ipAddress)+1);
	strcpy(buffer, ipAddress);
	_commonData->_connectName = buffer;
	_commonData->_connectPort = port;
}

void DataLog_CommonData::initializeCommonData(DataLog_SharedPtr(CommonData) data)
{
	data->_defaultTraceBufferSize = 2048;
	data->_defaultIntBufferSize = 1024;
	data->_defaultCriticalBufferSize = 4096;

	data->_connectType = NotConnected;
	data->_connectName = DATALOG_NULL_SHARED_PTR;
	data->_connectPort = 0;
}

DataLog_TaskErrorHandler * DataLog_CommonData::getTaskErrorHandler(DataLog_TaskID task)
{
	DataLog_TaskInfo * taskInfo = findTask(task);
	return taskInfo->_errorHandler;
}

void DataLog_CommonData::setTaskErrorHandler(DataLog_TaskID task, DataLog_TaskErrorHandler * func)
{
	DataLog_TaskInfo * taskInfo = findTask(task);
	taskInfo->_errorHandler = func;
}

void DataLog_CommonData::setTaskError(DataLog_ErrorType error, const char * file, int line)
{
	const char * message = datalog_ErrorMessage(error);
	int continuable = datalog_ErrorContinuable(error);

	DataLog_Critical	critical;
	DataLog(critical) << "datalog error: " << message <<
                        ((continuable) ? " " : " (fatal) ") <<
                        "@ " << file << ":" << line << endmsg;

	DataLog_TaskInfo * taskInfo = findTask(DATALOG_CURRENT_TASK);

	taskInfo->_errorActiveCount += 1;
	if ( taskInfo->_errorActiveCount >= 5 )
	{
		_FATAL_ERROR(__FILE__, __LINE__, "task error recursion level exceeded");
	}

	taskInfo->_error = error;
	if ( taskInfo->_errorHandler )
	{
		(*taskInfo->_errorHandler)(file, line, error, message, continuable);
	}

	if ( !continuable )
	{
		_FATAL_ERROR(__FILE__, __LINE__, (char *)message);
	}

	taskInfo->_errorActiveCount -= 1;
}

size_t DataLog_CommonData::getCurrentMaxBufferSize(void)
{
	size_t	maxBufferSize;

	if ( _commonData->_defaultTraceBufferSize > _commonData->_defaultIntBufferSize )
	{
		maxBufferSize = ( _commonData->_defaultTraceBufferSize > _commonData->_defaultCriticalBufferSize ) ?
				_commonData->_defaultTraceBufferSize : _commonData->_defaultCriticalBufferSize;
	}
	else
	{
		maxBufferSize = ( _commonData->_defaultIntBufferSize > _commonData->_defaultCriticalBufferSize ) ?
				_commonData->_defaultIntBufferSize : _commonData->_defaultCriticalBufferSize;
	}

	return maxBufferSize;
}

DataLog_Result datalog_Error(DataLog_TaskID task, DataLog_ErrorType * error)
{
	DataLog_CommonData common;
	DataLog_TaskInfo * taskInfo = common.findTask(task);

	*error = taskInfo->_error;
	return DataLog_OK;
}

DataLog_Result datalog_ClearError(DataLog_TaskID task)
{
	DataLog_CommonData common;
	DataLog_TaskInfo * taskInfo = common.findTask(task);

	taskInfo->_error = DataLog_NoError;
	return DataLog_OK;
}

DataLog_Result datalog_GetCurrentLogFileName(char * fileName, int bufferLength)
{
	DataLog_CommonData	common;
	DataLog_Result			result = DataLog_Error;

	if ( common.connectType() != DataLog_CommonData::LogToFile )
	{
		common.setTaskError(DataLog_NotLogToFile, __FILE__, __LINE__);
	}
	else if ( strlen(common.connectName()) >= bufferLength )
	{
		common.setTaskError(DataLog_BufferTooSmall, __FILE__, __LINE__);
	}
	else
	{
		strcpy(fileName, common.connectName());
		result = DataLog_OK;
	}

	return result;
}

const char * datalog_ErrorMessage(DataLog_ErrorType error)
{
	const char * result = "Unknown datalog error";
 
	for (int i=0; i<DataLog_LastError; i++)
	{
		if ( datalog_ErrorInformation[i]._error == error )
		{
			result = datalog_ErrorInformation[i]._message;
			break;
		}
	}

	return result;
}

int datalog_ErrorContinuable(DataLog_ErrorType error)
{
	int result = 0;
 
	for (int i=0; i<DataLog_LastError; i++)
	{
		if ( datalog_ErrorInformation[i]._error == error )
		{
			result = datalog_ErrorInformation[i]._continuable;
			break;
		}
	}

	return result;
}

void datalog_TaskCreated(DataLog_TaskID taskID)
{
	DataLog_CommonData common;
	DataLog_TaskInfo * info = new DataLog_TaskInfo;
	WIND_TCB	* tcb = taskTcb(taskID);

	info->_id = taskID;
	info->_error = DataLog_NoError;

	info->_trace = new DataLog_TraceBuffer(common.getDefaultTraceBufferSize());
	info->_logOutput = DataLog_LogDisabled;
	info->_consoleOutput = DataLog_ConsoleDisabled;

	info->_critical = new DataLog_CriticalBuffer(common.getDefaultCriticalBufferSize());

	info->_defaultLevel.setHandle(&common._criticalHandleInfo);
	info->_defaultHandle = &common._criticalHandleInfo;

	info->_errorHandler = NULL;
	info->_errorActiveCount = 0;

	common.addTask(info);

	DataLog_TaskCreateRecord taskCreateRecord;
	taskCreateRecord._recordType = DataLog_TaskCreateRecordID;
	datalog_GetTimeStamp(&taskCreateRecord._timeStamp);
	taskCreateRecord._taskID = taskID;
	taskCreateRecord._levelID = DATALOG_SYSTEM_LEVEL_ID;

#ifndef DATALOG_NO_NETWORK_SUPPORT
	taskCreateRecord._nodeID = datalog_NodeID();
#endif /* ifndef DATALOG_NO_NETWORK_SUPPORT */

	taskCreateRecord._nameLen = strlen(tcb->name);

	DataLog_Stream & stream = info->_critical->streamWriteStart();
	stream.write((const char *)&taskCreateRecord, sizeof(taskCreateRecord));
	stream.write(tcb->name, taskCreateRecord._nameLen * sizeof(char));
	info->_critical->streamWriteComplete();
}

void datalog_TaskDeleted(DataLog_TaskID taskID)
{
	DataLog_CommonData common;
	DataLog_TaskInfo * taskInfo = common.findTask(taskID);

	//
	// Use the deleted tasks critical buffer for the task deleted log
	// record, then actually delete the task (the task created log record
	// also uses this output buffer, so the create/delete records are
	// guaranteed sequential).
	//
	DataLog_TaskDeleteRecord taskDeleteRecord;

	taskDeleteRecord._recordType = DataLog_TaskDeleteRecordID;
	taskDeleteRecord._taskID = taskID;
	taskDeleteRecord._levelID = DATALOG_SYSTEM_LEVEL_ID;

#ifndef DATALOG_NO_NETWORK_SUPPORT
	taskDeleteRecord._nodeID = datalog_NodeID();
#endif /* ifndef DATALOG_NO_NETWORK_SUPPORT */

	datalog_GetTimeStamp(&taskDeleteRecord._timeStamp);
	taskInfo->_critical->write((DataLog_BufferData *)&taskDeleteRecord, sizeof(taskDeleteRecord));

	common.deleteTask(taskID);
}

DataLog_Result datalog_SetTaskErrorHandler(DataLog_TaskID task, DataLog_TaskErrorHandler * func)
{
	DataLog_CommonData	common;
	common.setTaskErrorHandler(task, func);
	return DataLog_OK;
}

