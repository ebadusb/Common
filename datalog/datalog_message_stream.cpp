/*
 * Copyright (C) 2002 Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: K:/BCT_Development/vxWorks/Common/datalog/rcs/datalog_message_stream.cpp 1.9 2003/04/29 17:07:54Z jl11312 Exp jl11312 $
 * $Log: datalog_message_stream.cpp $
 *
 */

#include <vxWorks.h>
#include <stdarg.h>
#include <typeinfo>
#include "datalog.h"
#include "datalog_internal.h"

DataLog_Level::DataLog_Level(void)
{
	_handle = DATALOG_NULL_SHARED_PTR;
}

DataLog_Result DataLog_Level::initialize(const char * levelName)
{
	return datalog_CreateLevel(levelName, &_handle);
}

DataLog_Level::DataLog_Level(const char * levelName)
{
	_handle = DATALOG_NULL_HANDLE;
	if ( initialize(levelName) != DataLog_OK )
	{
		DataLog_CommonData * common = datalog_GetCommonDataPtr();
		common->setTaskError(DataLog_LevelConstructorFailed, __FILE__, __LINE__);
	}
}

DataLog_Level::~DataLog_Level()
{
	_handle = DATALOG_NULL_HANDLE;
}

DataLog_EnabledType DataLog_Level::logOutput(void)
{
	DataLog_EnabledType result = DataLog_LogEnabled;
 
	if ( _handle == DATALOG_NULL_HANDLE )
	{
		DataLog_CommonData * common = datalog_GetCommonDataPtr();
		common->setTaskError(DataLog_LevelNotInitialized, __FILE__, __LINE__);
	}
	else
	{
		switch ( _handle->_type )
		{
		case DataLog_HandleInfo::TraceHandle:
			result = _handle->_traceData._logOutput;
			break;

		case DataLog_HandleInfo::CriticalHandle:
			result = DataLog_LogEnabled;
			break;

		default:
			DataLog_CommonData * common = datalog_GetCommonDataPtr();
			common->setTaskError(DataLog_InvalidHandle, __FILE__, __LINE__);
			break;
		}
	}

	return result;
}

DataLog_EnabledType DataLog_Level::logOutput(DataLog_EnabledType flag)
{
	DataLog_EnabledType result = DataLog_LogEnabled;
 
	if ( _handle == DATALOG_NULL_HANDLE )
	{
		DataLog_CommonData * common = datalog_GetCommonDataPtr();
		common->setTaskError(DataLog_LevelNotInitialized, __FILE__, __LINE__);		
	}
	else
	{
		switch ( _handle->_type )
		{
		case DataLog_HandleInfo::TraceHandle:
			result = _handle->_traceData._logOutput;
			_handle->_traceData._logOutput = flag;
			break;

		case DataLog_HandleInfo::CriticalHandle:
			result = DataLog_LogEnabled;
			break;

		default:
			DataLog_CommonData * common = datalog_GetCommonDataPtr();
			common->setTaskError(DataLog_InvalidHandle, __FILE__, __LINE__);
			break;
		}
	}

	return result;
}

DataLog_ConsoleEnabledType DataLog_Level::consoleOutput(void)
{
	DataLog_ConsoleEnabledType result = DataLog_ConsoleEnabled;
 
	if ( _handle == DATALOG_NULL_SHARED_PTR )
	{
		DataLog_CommonData * common = datalog_GetCommonDataPtr();
		common->setTaskError(DataLog_LevelNotInitialized, __FILE__, __LINE__);		
	}
	else
	{
		switch ( _handle->_type )
		{
		case DataLog_HandleInfo::TraceHandle:
			result = _handle->_traceData._consoleOutput;
			break;

		case DataLog_HandleInfo::CriticalHandle:
			result = DataLog_ConsoleEnabled;
			break;

		default:
			DataLog_CommonData * common = datalog_GetCommonDataPtr();
			common->setTaskError(DataLog_InvalidHandle, __FILE__, __LINE__);
			break;
		}
	}

	return result;
}

DataLog_ConsoleEnabledType DataLog_Level::consoleOutput(DataLog_ConsoleEnabledType flag)
{
	DataLog_ConsoleEnabledType result = DataLog_ConsoleEnabled;
 
	if ( _handle == DATALOG_NULL_SHARED_PTR )
	{
		DataLog_CommonData * common = datalog_GetCommonDataPtr();
		common->setTaskError(DataLog_LevelNotInitialized, __FILE__, __LINE__);		
	}
	else
	{
		switch ( _handle->_type )
		{
		case DataLog_HandleInfo::TraceHandle:
			result = _handle->_traceData._consoleOutput;
			_handle->_traceData._consoleOutput = flag;
			break;

		case DataLog_HandleInfo::CriticalHandle:
			result = DataLog_ConsoleEnabled;
			break;

		default:
			DataLog_CommonData * common = datalog_GetCommonDataPtr();
			common->setTaskError(DataLog_InvalidHandle, __FILE__, __LINE__);
			break;
		}
	}

	return result;
}

DataLog_Stream & DataLog_Level::operator()(const char * fileName, int lineNumber)
{
	DataLog_CommonData * common = datalog_GetCommonDataPtr();
	DataLog_OutputBuffer * outputBuffer = NULL;
	DataLog_EnabledType logOutput = DataLog_LogEnabled;
	DataLog_ConsoleEnabledType consoleOutput = DataLog_ConsoleDisabled;

	DataLog_StreamOutputRecord streamOutputRecord;
	streamOutputRecord._recordType = DataLog_StreamOutputRecordID;
	datalog_GetTimeStamp(&streamOutputRecord._timeStamp);

	DataLog_HandleInfo::HandleType handleType = (_handle != DATALOG_NULL_HANDLE) ? _handle->_type : DataLog_HandleInfo::InvalidHandle;
	switch ( handleType )
	{
	case DataLog_HandleInfo::TraceHandle:
		{
			streamOutputRecord._levelID = _handle->_traceData._id;
			streamOutputRecord._taskID = datalog_CurrentTask();

			logOutput = _handle->_traceData._logOutput;
			consoleOutput = _handle->_traceData._consoleOutput;

			DataLog_CommonData::TaskInfoPtr	taskInfo = common->findTask(DATALOG_CURRENT_TASK);
			if ( taskInfo != DATALOG_NULL_SHARED_PTR )
			{
				logOutput = ( logOutput == DataLog_LogEnabled ) ? DataLog_LogEnabled : taskInfo->_logOutput;
				consoleOutput = ( consoleOutput == DataLog_ConsoleEnabled ) ? DataLog_ConsoleEnabled : taskInfo->_consoleOutput;
		   }

			outputBuffer = (common->getTaskTraceBuffer(DATALOG_CURRENT_TASK))->_buffer;
		}
		break;

	case DataLog_HandleInfo::CriticalHandle:
		{
			streamOutputRecord._levelID = 0;
			streamOutputRecord._taskID = _handle->_criticalData._id;

			consoleOutput = DataLog_ConsoleEnabled;
			outputBuffer = _handle->_criticalData._buffer;
	   }
		break;

	default:
		{
			/*
			 *	Invalid handle type for stream.  Report the error and choose reasonable
			 * defaults if continuing.
			 */
			common->setTaskError(DataLog_InvalidHandle, __FILE__, __LINE__);

			streamOutputRecord._levelID = 0;
			streamOutputRecord._taskID = DATALOG_CURRENT_TASK;
			outputBuffer = (common->getTaskTraceBuffer(DATALOG_CURRENT_TASK))->_buffer;
		}
		break;
	}

#ifndef DATALOG_NO_NETWORK_SUPPORT
	streamOutputRecord._nodeID = datalog_NodeID();
#endif /* ifndef DATALOG_NO_NETWORK_SUPPORT */

	streamOutputRecord._fileNameLen = strlen(fileName);
	streamOutputRecord._lineNum = lineNumber;
	DataLog_Stream	& stream = outputBuffer->lockStreamBuffer(&streamOutputRecord, fileName, logOutput, consoleOutput);
	return stream;
}

DataLog_Result DataLog_Level::setAsDefault(void)
{
	DataLog_Result result = DataLog_OK;

	DataLog_CommonData * common = datalog_GetCommonDataPtr();
	DataLog_TaskInfo * taskInfo = common->findTask(DATALOG_CURRENT_TASK);

	if ( taskInfo == DATALOG_NULL_SHARED_PTR )
	{
		common->setTaskError(DataLog_NoSuchTask, __FILE__, __LINE__);
		result = DataLog_Error;
	}
	else
	{
		taskInfo->_defaultHandle = _handle;
		taskInfo->_defaultLevel.setHandle(_handle);
	}

	return result;
}

DataLog_Critical::DataLog_Critical(void)
{
	DataLog_CommonData * common = datalog_GetCommonDataPtr();
	DataLog_TaskInfo * taskInfo = common->findTask(DATALOG_CURRENT_TASK);

	if ( taskInfo == DATALOG_NULL_SHARED_PTR )
	{
		_handle = DATALOG_NULL_HANDLE;
		common->setTaskError(DataLog_NoSuchTask, __FILE__, __LINE__);
	}
	else
	{
		_handle = taskInfo->_criticalHandle;
	}
}

DataLog_Critical::~DataLog_Critical()
{
	_handle = DATALOG_NULL_HANDLE;
}

DataLog_Stream::DataLog_Stream(DataLog_BufferData * buffer, size_t bufferSize, DataLog_OutputBuffer * output)
				: ostrstream((char *)buffer, bufferSize/sizeof(char)), _output(output)
{
}

ostream & endmsg(ostream & stream)
{
	bool	validStream = false;

	//
	// Compiled code generates GPF when attempting to perform run time type
	// operations on the standard output streams, so we need to check for
	// these as a special case.
	//
	if ( &stream != dynamic_cast<ostream *>(&cout) &&
		  &stream != dynamic_cast<ostream *>(&clog) &&
		  &stream != dynamic_cast<ostream *>(&cerr) )
	{
		DataLog_Stream * dstream = dynamic_cast<DataLog_Stream *>(&stream);
		if ( dstream )
		{
			dstream->_output->releaseStreamBuffer();
			validStream = true;
	   }
	}

	if ( !validStream )
	{
		stream << "\n";
	}

	return stream;
}

ostream & datalog_GetDefaultStream(const char * file, int line)
{
	DataLog_CommonData * common = datalog_GetCommonDataPtr();
	DataLog_CommonData::TaskInfoPtr	taskInfo = common->findTask(DATALOG_CURRENT_TASK);

	if ( taskInfo == DATALOG_NULL_SHARED_PTR )
	{
		common->setTaskError(DataLog_NoSuchTaskFatal, __FILE__, __LINE__);
   }

	return (taskInfo->_defaultLevel)(file, line);
}
