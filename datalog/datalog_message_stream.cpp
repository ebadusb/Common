/*
 * Copyright (C) 2002 Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: K:/BCT_Development/vxWorks/Common/datalog/rcs/datalog_message_stream.cpp 1.9 2003/04/29 17:07:54Z jl11312 Exp jl11312 $
 * $Log: datalog_message_stream.cpp $
 * Revision 1.1  2002/07/18 21:20:57  jl11312
 * Initial revision
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
		DataLog_CommonData common;
		common.setTaskError(DataLog_LevelConstructorFailed, __FILE__, __LINE__);
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
		DataLog_CommonData common;
		common.setTaskError(DataLog_LevelNotInitialized, __FILE__, __LINE__);
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
			DataLog_CommonData common;
			common.setTaskError(DataLog_InvalidHandle, __FILE__, __LINE__);
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
		DataLog_CommonData common;
		common.setTaskError(DataLog_LevelNotInitialized, __FILE__, __LINE__);		
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
			DataLog_CommonData common;
			common.setTaskError(DataLog_InvalidHandle, __FILE__, __LINE__);
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
		DataLog_CommonData common;
		common.setTaskError(DataLog_LevelNotInitialized, __FILE__, __LINE__);		
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
			DataLog_CommonData common;
			common.setTaskError(DataLog_InvalidHandle, __FILE__, __LINE__);
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
		DataLog_CommonData common;
		common.setTaskError(DataLog_LevelNotInitialized, __FILE__, __LINE__);		
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
			DataLog_CommonData common;
			common.setTaskError(DataLog_InvalidHandle, __FILE__, __LINE__);
			break;
		}
	}

	return result;
}

struct OutputControl
{
	DataLog_EnabledType	logOutput;
	DataLog_ConsoleEnabledType	consoleOutput;
};

DataLog_EnabledType streamCallBack(const DataLog_BufferData * data, size_t size)
{
	OutputControl * control = (OutputControl *)data;

//	if ( control->consoleOutput == DataLog_ConsoleEnabled )
	{
		//
		// Parse the stream output record and send data to stdout
		//
		DataLog_StreamOutputRecord * streamOutputRecord;
		const DataLog_BufferData * fileName;
		DataLog_UINT16 * streamDataLen;
		const DataLog_BufferData * streamData;

		streamOutputRecord = (DataLog_StreamOutputRecord *)&data[sizeof(*control)];
		fileName = &data[sizeof(*control)+sizeof(*streamOutputRecord)];
		streamDataLen = (DataLog_UINT16 *)(fileName + streamOutputRecord->_fileNameLen*sizeof(char));
		streamData = ((DataLog_BufferData *)streamDataLen) + sizeof(DataLog_UINT16);

		fwrite(fileName, sizeof(char), (unsigned int)streamOutputRecord->_fileNameLen, stderr);
		fprintf(stderr, "(%d): ", streamOutputRecord->_lineNum);
		fwrite(streamData, sizeof(char), (unsigned int)(*streamDataLen), stderr);
		fputc('\n', stderr);
	}

	return control->logOutput;
}

DataLog_Stream & DataLog_Level::operator()(const char * fileName, int lineNumber)
{
	DataLog_CommonData common;
	DataLog_OutputBuffer * outputBuffer = NULL;
	OutputControl outputControl = { DataLog_LogEnabled, DataLog_ConsoleDisabled };

	DataLog_StreamOutputRecord streamOutputRecord;
	streamOutputRecord._recordType = DataLog_StreamOutputRecordID;
	datalog_GetTimeStamp(&streamOutputRecord._timeStamp);

	DataLog_HandleInfo::HandleType handleType = (_handle != DATALOG_NULL_HANDLE) ? _handle->_type : DataLog_HandleInfo::InvalidHandle;
	switch ( handleType )
	{
	case DataLog_HandleInfo::TraceHandle:
		{
			streamOutputRecord._levelID = _handle->_id;
			streamOutputRecord._taskID = datalog_CurrentTask();

			outputControl.logOutput = _handle->_traceData._logOutput;
			outputControl.consoleOutput = _handle->_traceData._consoleOutput;

			DataLog_TaskInfo * taskInfo = common.findTask(DATALOG_CURRENT_TASK);
			outputControl.logOutput = ( outputControl.logOutput == DataLog_LogEnabled ) ? DataLog_LogEnabled : taskInfo->_logOutput;
			outputControl.consoleOutput = ( outputControl.consoleOutput == DataLog_ConsoleEnabled ) ? DataLog_ConsoleEnabled : taskInfo->_consoleOutput;
			outputBuffer = taskInfo->_trace;
		}
		break;

	case DataLog_HandleInfo::CriticalHandle:
		{
			streamOutputRecord._levelID = 0;
			streamOutputRecord._taskID = _handle->_criticalData._taskID;

			outputControl.consoleOutput = DataLog_ConsoleEnabled;
			outputBuffer = _handle->_criticalData._buffer;
	   }
		break;

	default:
		{
			/*
			 *	Invalid handle type for stream.  Report the error and choose reasonable
			 * defaults if continuing.
			 */
			common.setTaskError(DataLog_InvalidHandle, __FILE__, __LINE__);

			streamOutputRecord._levelID = 0;
			streamOutputRecord._taskID = DATALOG_CURRENT_TASK;
			outputBuffer = common.getTaskCriticalBuffer(DATALOG_CURRENT_TASK);
		}
		break;
	}

#ifndef DATALOG_NO_NETWORK_SUPPORT
	streamOutputRecord._nodeID = datalog_NodeID();
#endif /* ifndef DATALOG_NO_NETWORK_SUPPORT */

	streamOutputRecord._fileNameLen = strlen(fileName);
	streamOutputRecord._lineNum = lineNumber;
 
	DataLog_Stream	& stream = outputBuffer->streamWriteStart(&streamCallBack, sizeof(outputControl));
	stream.write(&outputControl, sizeof(outputControl));
	stream.write(&streamOutputRecord, sizeof(streamOutputRecord));
	stream.write(fileName, streamOutputRecord._fileNameLen * sizeof(char));

	DataLog_UINT16	dataLen = 0;
	stream.write(&dataLen, sizeof(dataLen));

	outputBuffer->streamWriteReleaseToApp();
	return stream;
}

DataLog_Result DataLog_Level::setAsDefault(void)
{
	DataLog_CommonData common;
	DataLog_TaskInfo * taskInfo = common.findTask(DATALOG_CURRENT_TASK);
	taskInfo->_defaultHandle = _handle;
	taskInfo->_defaultLevel.setHandle(_handle);

	return DataLog_OK;
}

DataLog_Critical::DataLog_Critical(void)
{
	DataLog_CommonData common;
	DataLog_TaskInfo * taskInfo = common.findTask(DATALOG_CURRENT_TASK);

	_handle = taskInfo->_criticalHandle;
}

DataLog_Critical::~DataLog_Critical()
{
	_handle = DATALOG_NULL_HANDLE;
}

DataLog_Stream::DataLog_Stream(DataLog_OutputBuffer * output)
				: ostrstream(), _output(output)
{
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
			dstream->_output->streamWriteComplete();
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
	DataLog_CommonData common;
	DataLog_TaskInfo * taskInfo = common.findTask(DATALOG_CURRENT_TASK);

	return (taskInfo->_defaultLevel)(file, line);
}

