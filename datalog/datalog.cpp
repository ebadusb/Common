/*
 * Copyright (C) 2002 Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: //bctquad3/home/BCT_Development/vxWorks/Common/datalog/rcs/datalog.cpp 1.16 2005/09/29 21:59:50Z ms10234 Exp $
 * $Log: datalog.cpp $
 * Revision 1.16  2005/09/29 21:59:50Z  ms10234
 * IT42 - allow generation of new log files without rebooting
 * Revision 1.15  2005/05/31 20:26:41Z  jheiusb
 * it32 Make changes to common to accommodate the Trima 5.2 vxWorks  5.5 port
 * Revision 1.14  2004/10/26 20:18:45Z  rm70006
 * Ported datalog code to be compatible with windows compiler.  No functional changes made.  Re-ran unit test and it passed.
 * Revision 1.13  2003/12/05 16:33:05Z  jl11312
 * - removed non-portable calls
 * Revision 1.12  2003/10/03 12:34:58Z  jl11312
 * - improved DataLog_Handle lookup time
 * - modified datalog signal handling to eliminate requirement for a name lookup and the semaphore lock/unlock that went with it
 * Revision 1.11  2003/03/27 16:26:55Z  jl11312
 * - added support for new datalog levels
 * Revision 1.10  2003/02/25 16:10:06Z  jl11312
 * - modified buffering scheme to help prevent buffer overruns
 * Revision 1.9  2003/01/31 19:52:49  jl11312
 * - new stream format for datalog
 * Revision 1.8  2002/09/23 13:54:58  jl11312
 * - added access function for current log file name
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
#include "datalog_records.h"
#include "error.h"
#include <string>

#ifdef DATALOG_LEVELS_INIT_SUPPORT
# include "datalog_levels.h"
#endif /* ifdef DATALOG_LEVELS_INIT_SUPPORT */


DataLog_Map<DataLog_TaskID, DataLog_TaskInfo *> DataLog_CommonData::_tasks;
DataLog_Lock DataLog_CommonData::_tasksLock = datalog_CreateMLock();

const DataLog_HandleInfo DataLog_CommonData::_criticalHandleInfo = { 0, DataLog_HandleInfo::CriticalHandle, DataLog_LogEnabled, DataLog_ConsoleDisabled };

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

	datalog_LockAccess(_tasksLock,WAIT_FOREVER);
	if ( _tasks.find(searchTask) == _tasks.end() )
	{
		datalog_ReleaseAccess(_tasksLock);
		datalog_TaskCreated(datalog_CurrentTask());
		datalog_LockAccess(_tasksLock,WAIT_FOREVER);
	}

	DataLog_TaskInfo * result = _tasks[searchTask];
	datalog_ReleaseAccess(_tasksLock);
	return result;
}

void DataLog_CommonData::addTask(DataLog_TaskInfo * taskInfo)
{
	datalog_LockAccess(_tasksLock,WAIT_FOREVER);
	_tasks[taskInfo->_id] = taskInfo;
	datalog_ReleaseAccess(_tasksLock);
}

void DataLog_CommonData::deleteTask(DataLog_TaskID task)
{
	DataLog_TaskInfo * taskInfo = NULL;

	datalog_LockAccess(_tasksLock,WAIT_FOREVER);
	if ( _tasks.find(task) != _tasks.end() )
	{
		taskInfo = _tasks[task];
		_tasks.erase(task);
	}

	datalog_ReleaseAccess(_tasksLock);

	if ( taskInfo )
	{
		DataLog_Map<DataLog_InternalID, DataLog_Stream *>::iterator streamsIter;
		for ( streamsIter = taskInfo->_outputStream.begin(); streamsIter != taskInfo->_outputStream.end(); ++streamsIter )
		{
			delete streamsIter->second;
		}

		delete taskInfo;
	}
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

void DataLog_CommonData::setPersistSystemInfo(bool flag)
{
	_commonData->_persistSystemInfo = flag;

	if ( flag )
	{
		DataLog_BufferManager::createChain(_commonData->_persistedSystemInfoChain);
	}
}

void DataLog_CommonData::setPlatformName(const char * platformName)
{
	DataLog_SharedPtr(char)	buffer = (DataLog_SharedPtr(char))datalog_AllocSharedMem(strlen(platformName)+1);
	strcpy(buffer, platformName);
	_commonData->_platformName = buffer;
}

void DataLog_CommonData::setPlatformInfo(const char * platformInfo)
{
	DataLog_SharedPtr(char)	buffer = (DataLog_SharedPtr(char))datalog_AllocSharedMem(strlen(platformInfo)+1);
	strcpy(buffer, platformInfo);
	_commonData->_platformInfo = buffer;
}

void DataLog_CommonData::setNodeName(const char * nodeName)
{
	DataLog_SharedPtr(char)	buffer = (DataLog_SharedPtr(char))datalog_AllocSharedMem(strlen(nodeName)+1);
	strcpy(buffer, nodeName);
	_commonData->_nodeName = buffer;
}

void DataLog_CommonData::initializeCommonData(DataLog_SharedPtr(CommonData) data)
{
	data->_connectType = NotConnected;
	data->_connectName = DATALOG_NULL_SHARED_PTR;
	data->_connectPort = 0;
	data->_criticalReserveBuffers = 0;
}

void DataLog_CommonData::outputSystemInfo(void)
{
	DataLog_BufferChain systemInfoChain;
	DataLog_BufferManager::copyChain(systemInfoChain,_commonData->_persistedSystemInfoChain); 
	DataLog_BufferManager::addChainToList(DataLog_BufferManager::CriticalList,systemInfoChain);
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

#ifdef DATALOG_LEVELS_INIT_SUPPORT
	DataLog(log_level_datalog_error)
#else /* ifdef DATALOG_LEVELS_INIT_SUPPORT */
	DataLog_Default
#endif /* ifdef DATALOG_LEVELS_INIT_SUPPORT */
		<< "datalog error: " << message <<
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

DataLog_Result datalog_StartNewLogFile(const char *newLogFileName)
{
	DataLog_CommonData	common;
	DataLog_Result			result = DataLog_Error;

	if ( !common.persistSystemInfo() )
	{
		common.setTaskError(DataLog_StartNewLogNotAllowed, __FILE__, __LINE__);
	}
	else if ( common.connectType() != DataLog_CommonData::LogToFile )
	{
		common.setTaskError(DataLog_NotLogToFile, __FILE__, __LINE__);
	}
	else
	{
		datalog_FreeSharedMem( (void*)common.connectName() );
		common.setLocalConnect(newLogFileName);
		result = DataLog_OK;

#ifdef DATALOG_NETWORK_SUPPORT
		datalog_StartLocalOutputTask(common.platformName(), common.nodeName(), common.platformInfo());
#else /* ifdef DATALOG_NETWORK_SUPPORT */
		datalog_StartLocalOutputTask(common.platformName(), NULL, common.platformInfo());
#endif /* ifdef DATALOG_NETWORK_SUPPORT */

		common.outputSystemInfo();
	}

	return result;
}

DataLog_Result datalog_GetCurrentLogFileName(char * fileName, size_t bufferLength)
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
	bool saveInfo = common.persistSystemInfo();

	info->_id = taskID;
	info->_error = DataLog_NoError;

	info->_logOutput = DataLog_LogDisabled;
	info->_consoleOutput = DataLog_ConsoleDisabled;

	info->_defaultLevel.setHandle(&common._criticalHandleInfo);
	info->_defaultHandle = &common._criticalHandleInfo;
	info->_outputStream[DATALOG_NULL_ID] = new DataLog_Stream(info->_defaultLevel);

	info->_errorHandler = NULL;
	info->_errorActiveCount = 0;

	common.addTask(info);

	DataLog_TaskCreateRecord taskCreateRecord;
	taskCreateRecord._recordType = DataLog_TaskCreateRecordID;
	datalog_GetTimeStamp(&taskCreateRecord._timeStamp);
	taskCreateRecord._taskID = taskID;
	taskCreateRecord._levelID = DATALOG_SYSTEM_LEVEL_ID;

#ifdef DATALOG_NETWORK_SUPPORT
	taskCreateRecord._nodeID = datalog_NodeID();
#endif /* ifdef DATALOG_NETWORK_SUPPORT */

	const char * taskName = datalog_TaskName(taskID);
	taskCreateRecord._nameLen = (taskName) ? strlen(taskName) : 0;
	
	DataLog_BufferChain	outputChain;
	DataLog_BufferManager::createChain(outputChain);

	DataLog_BufferManager::writeToChain(outputChain, (DataLog_BufferData *)&taskCreateRecord, sizeof(taskCreateRecord));

	if ( saveInfo ) 
		DataLog_BufferManager::writeToChain(common.systemInfoChain(), 
														(DataLog_BufferData *)&taskCreateRecord, 
														sizeof(taskCreateRecord));
	if ( taskName )
	{
		DataLog_BufferManager::writeToChain(outputChain, (DataLog_BufferData *)taskName, taskCreateRecord._nameLen * sizeof(char));
		if ( saveInfo ) 
			DataLog_BufferManager::writeToChain(common.systemInfoChain(), 
															(DataLog_BufferData *)taskName, 
															taskCreateRecord._nameLen * sizeof(char));
	}

	DataLog_BufferManager::addChainToList(DataLog_BufferManager::CriticalList, outputChain);
}

void datalog_TaskDeleted(DataLog_TaskID taskID)
{
	DataLog_CommonData common;
	DataLog_TaskInfo * taskInfo = common.findTask(taskID);
	bool saveInfo = common.persistSystemInfo();

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

#ifdef DATALOG_NETWORK_SUPPORT
	taskDeleteRecord._nodeID = datalog_NodeID();
#endif /* ifdef DATALOG_NETWORK_SUPPORT */

	datalog_GetTimeStamp(&taskDeleteRecord._timeStamp);

	DataLog_BufferChain	outputChain;
	DataLog_BufferManager::writeToChain(outputChain, (DataLog_BufferData *)&taskDeleteRecord, sizeof(taskDeleteRecord));
	if ( saveInfo ) 
		DataLog_BufferManager::writeToChain(common.systemInfoChain(), 
														(DataLog_BufferData *)&taskDeleteRecord, 
														sizeof(taskDeleteRecord));
	DataLog_BufferManager::addChainToList(DataLog_BufferManager::CriticalList, outputChain);

	common.deleteTask(taskID);
}

DataLog_Result datalog_SetTaskErrorHandler(DataLog_TaskID task, DataLog_TaskErrorHandler * func)
{
	DataLog_CommonData	common;
	common.setTaskErrorHandler(task, func);
	return DataLog_OK;
}

