/*
 * Copyright (C) 2002 Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: K:/BCT_Development/vxWorks/Common/datalog/rcs/datalog_message_stream.cpp 1.9 2003/04/29 17:07:54Z jl11312 Exp jl11312 $
 * $Log: datalog_message_stream.cpp $
 * Revision 1.8  2003/02/25 16:10:19Z  jl11312
 * - modified buffering scheme to help prevent buffer overruns
 * Revision 1.7  2003/01/31 21:52:19  jl11312
 * - added check for null string pointers for operator <<
 * Revision 1.6  2003/01/31 19:52:51  jl11312
 * - new stream format for datalog
 * Revision 1.5  2002/10/25 16:59:17  jl11312
 * - added new form of errnoMsg stream manipulator which takes an argument for errno
 * Revision 1.4  2002/09/19 21:25:59  jl11312
 * - modified stream functions to not reset stream state when two stream writes occur without endmsg in between
 * - added errnoMsg manipulator function
 * Revision 1.3  2002/08/28 14:37:08  jl11312
 * - changed handling of critical output to avoid problem with handles referencing deleted tasks
 * Revision 1.2  2002/08/15 20:53:56  jl11312
 * - added support for periodic logging
 * Revision 1.1  2002/07/18 21:20:57  jl11312
 * Initial revision
 *
 */

#include "datalog.h"

#include <stdarg.h>
#include <symLib.h>
#include <sysSymTbl.h>
#include <typeinfo>
#include "datalog_internal.h"
#include "datalog_records.h"

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
		case DataLog_HandleInfo::IntHandle:
			result = _handle->_logOutput;
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
		case DataLog_HandleInfo::IntHandle:
			result = _handle->_logOutput;
			((DataLog_HandleInfo *)_handle)->_logOutput = flag;
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
			result = _handle->_consoleOutput;
			break;

		case DataLog_HandleInfo::IntHandle:
			result = DataLog_ConsoleDisabled;
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
			result = _handle->_consoleOutput;
			((DataLog_HandleInfo *)_handle)->_consoleOutput = flag;
			break;

		case DataLog_HandleInfo::IntHandle:
			result = DataLog_ConsoleDisabled;
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

DataLog_Stream & DataLog_Level::operator()(const char * fileName, int lineNumber)
{
	DataLog_CommonData common;
	DataLog_EnabledType logOutput = DataLog_LogEnabled;
	DataLog_ConsoleEnabledType consoleOutput = DataLog_ConsoleDisabled;

	DataLog_StreamOutputRecord  streamOutputRecord;

	streamOutputRecord._recordType = DataLog_StreamOutputRecordID;
	datalog_GetTimeStamp(&streamOutputRecord._timeStamp);

	DataLog_HandleInfo::HandleType handleType = (_handle != DATALOG_NULL_HANDLE) ? _handle->_type : DataLog_HandleInfo::InvalidHandle;
	DataLog_TaskInfo * taskInfo = common.findTask(DATALOG_CURRENT_TASK);

	switch ( handleType )
	{
	case DataLog_HandleInfo::TraceHandle:
		{
			streamOutputRecord._levelID = _handle->_id;
			streamOutputRecord._taskID = datalog_CurrentTask();

			logOutput = _handle->_logOutput;
			consoleOutput = _handle->_consoleOutput;

			logOutput = ( logOutput == DataLog_LogEnabled ) ? DataLog_LogEnabled : taskInfo->_logOutput;
			consoleOutput = ( consoleOutput == DataLog_ConsoleEnabled ) ? DataLog_ConsoleEnabled : taskInfo->_consoleOutput;
		}
		break;

	case DataLog_HandleInfo::CriticalHandle:
		{
			streamOutputRecord._levelID = 0;
			streamOutputRecord._taskID = datalog_CurrentTask();

			consoleOutput = DataLog_ConsoleEnabled;
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
		}
		break;
	}

#ifdef DATALOG_NETWORK_SUPPORT
	streamOutputRecord._nodeID = datalog_NodeID();
#endif /* ifdef DATALOG_NETWORK_SUPPORT */

	streamOutputRecord._fileNameLen = strlen(fileName);
	streamOutputRecord._lineNum = lineNumber;

	if ( taskInfo->_outputStream.find(streamOutputRecord._levelID) == taskInfo->_outputStream.end() )
	{
		taskInfo->_outputStream[streamOutputRecord._levelID] = new DataLog_Stream(*this);
	}

	DataLog_Stream	* stream = taskInfo->_outputStream[streamOutputRecord._levelID];
	stream->setLogOutput(logOutput);
	stream->setConsoleOutput(consoleOutput);
	stream->writeStart(&streamOutputRecord, fileName);

	return *stream;
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
	_handle = &common._criticalHandleInfo;
}

DataLog_Critical::~DataLog_Critical()
{
	_handle = DATALOG_NULL_HANDLE;
}

DataLog_Stream::DataLog_Stream(const DataLog_Level & level)
{
	DataLog_CommonData common;

	_flags = f_defaultflags;
	_precision = 5;

	_flagsChanged = false;
	_precisionChanged = false;
	_logOutput = DataLog_LogDisabled;
	_consoleOutput = DataLog_ConsoleDisabled;

	_critical = (level.getHandle()->_type == DataLog_HandleInfo::CriticalHandle);
	_reserveBuffers = (_critical) ? 0 : common.criticalReserveBuffers();
}

DataLog_Stream::~DataLog_Stream()
{
	if ( _outputChain._head != NULL )
	{
		writeComplete();
	}

	DataLog_BufferManager::addChainToList(DataLog_BufferManager::FreeList, _headerChain);
}

DataLog_Stream & DataLog_Stream::operator << (char c)
{
	if ( _consoleOutput == DataLog_ConsoleEnabled ) putc(c, datalog_ConsoleFile());
	if ( _logOutput == DataLog_LogEnabled) writeArg(SignedChar, &c, sizeof(char));
	return *this;
}

DataLog_Stream & DataLog_Stream::operator << (int val)
{
	if ( _consoleOutput == DataLog_ConsoleEnabled ) printLong((long)val);
	if ( _logOutput == DataLog_LogEnabled) writeArg(SignedInt, &val, sizeof(int));
	return *this;
}

DataLog_Stream & DataLog_Stream::operator << (unsigned int val)
{
	if ( _consoleOutput == DataLog_ConsoleEnabled ) printUnsignedLong((unsigned long)val);
	if ( _logOutput == DataLog_LogEnabled) writeArg(UnsignedInt, &val, sizeof(unsigned int)); return *this;
}

DataLog_Stream & DataLog_Stream::operator << (long val)
{
	if ( _consoleOutput == DataLog_ConsoleEnabled ) printLong(val);
	if ( _logOutput == DataLog_LogEnabled) writeArg(SignedLong, &val, sizeof(long)); return *this;
}

DataLog_Stream & DataLog_Stream::operator << (unsigned long val)
{
	if ( _consoleOutput == DataLog_ConsoleEnabled ) printUnsignedLong(val);
	if ( _logOutput == DataLog_LogEnabled) writeArg(UnsignedLong, &val, sizeof(unsigned long)); return *this;
}

DataLog_Stream & DataLog_Stream::operator << (bool val)
{
	if ( _consoleOutput == DataLog_ConsoleEnabled ) fprintf(datalog_ConsoleFile(), (val) ? "true" : "false");
	if ( _logOutput == DataLog_LogEnabled)
	{
		DataLog_UINT8 byte = (val) ? 1 : 0;
		writeArg(Bool, &byte, 1);
	}

	return *this;
}

DataLog_Stream & DataLog_Stream::operator << (const char * s)
{
	if ( _consoleOutput == DataLog_ConsoleEnabled ) fprintf(datalog_ConsoleFile(), "%s", s);
	if ( _logOutput == DataLog_LogEnabled)
	{
		if (s) writeStringArg(String, s, strlen(s)*sizeof(char));
		else writeStringArg(String, "null", 4*sizeof(char));
	} 

	return *this;
}

DataLog_Stream & DataLog_Stream::operator << (float val)
{
	if ( _consoleOutput == DataLog_ConsoleEnabled ) printDouble((double)val);	
	if ( _logOutput == DataLog_LogEnabled) writeArg(Float, &val, sizeof(float));
	return *this;
}

void DataLog_Stream::writeStart(DataLog_StreamOutputRecord * record, const char * fileName)
{
	if ( _outputChain._head == NULL &&
		  _logOutput == DataLog_LogEnabled )
	{
		//
		// No write in progress so setup for new write
		//
		if ( _headerChain._head != NULL )
		{
			DataLog_BufferManager::addChainToList(DataLog_BufferManager::FreeList, _headerChain);
		}

		DataLog_BufferManager::createChain(_headerChain, _reserveBuffers);
		DataLog_BufferManager::writeToChain(_headerChain, (DataLog_BufferData *)record, sizeof(*record));
		DataLog_BufferManager::writeToChain(_headerChain, (DataLog_BufferData *)fileName, sizeof(char) * record->_fileNameLen);

		_initialFlags = _flags;
		_initialPrecision = _precision;
	}
}

void DataLog_Stream::writeComplete(void)
{
	if ( _outputChain._head != NULL )
	{
		if ( _outputChain._missedBytes == 0 )
		{
			if ( _outputChain._head->_length > 65535 )
			{
				//
				// Record too large.  Free chain and build new chain record for
				// recording the bytes missed.
				//
				unsigned long	length = _outputChain._head->_length;
				DataLog_BufferManager::addChainToList(DataLog_BufferManager::FreeList, _outputChain);

				_outputChain._head = NULL;
				_outputChain._missedBytes = length;
			}
			else
			{
				//
				// We need to update the argument length in the header record.
				//
				DataLog_UINT16	argLength = _outputChain._head->_length - _headerChain._head->_length - 5;
				unsigned long argLengthOffset = _headerChain._head->_length + 3;

				DataLog_BufferManager::modifyChainData(_outputChain, argLengthOffset, (DataLog_BufferData *)&argLength, 2);
			}
		}

		DataLog_BufferManager::addChainToList( (_critical) ? DataLog_BufferManager::CriticalList : DataLog_BufferManager::TraceList, _outputChain);
		_outputChain._head = NULL;

		//
		// Reset initial flags in case application has saved a stream reference and starts
		// writing again without going through writeStart()
		//
		_initialFlags = _flags;
		_initialPrecision = _precision;
	}
}

DataLog_Stream & DataLog_Stream::operator << (double val)
{
	if ( _consoleOutput == DataLog_ConsoleEnabled ) printDouble(val);
	if ( _logOutput == DataLog_LogEnabled) writeArg(Double, &val, sizeof(double)); return *this;
}

void DataLog_Stream::setupOutputChain(void)
{
	DataLog_BufferManager::copyChain(_outputChain, _headerChain);
	DataLog_BufferManager::writeToChain(_outputChain, (DataLog_BufferData *)&_initialFlags, sizeof(_initialFlags));
	DataLog_BufferManager::writeToChain(_outputChain, (DataLog_BufferData *)&_initialPrecision, sizeof(_initialPrecision));

	DataLog_UINT16 argLength = 0;
	DataLog_BufferManager::writeToChain(_outputChain, (DataLog_BufferData *)&argLength, sizeof(argLength));
}

void DataLog_Stream::writeArg(ArgumentType type, const void * data, DataLog_UINT16 size)
{
	if ( _flagsChanged )
	{
		_flagsChanged = false;
		writeArg(FlagSetting, &_flags, sizeof(_flags));
   }

	if ( _precisionChanged )
	{
		_precisionChanged = false;
		writeArg(PrecisionSetting, &_precision, sizeof(_precision));
   }

	if ( _outputChain._head == NULL ) setupOutputChain();
	DataLog_BufferManager::writeToChain(_outputChain, (DataLog_BufferData *)&type, 1);
	DataLog_BufferManager::writeToChain(_outputChain, (DataLog_BufferData *)data, size);
}

void DataLog_Stream::writeStringArg(ArgumentType type, const void * data, DataLog_UINT16 size)
{
	if ( _flagsChanged )
	{
		_flagsChanged = false;
		writeArg(FlagSetting, &_flags, sizeof(_flags));
   }

	if ( _precisionChanged )
	{
		_precisionChanged = false;
		writeArg(PrecisionSetting, &_precision, sizeof(_precision));
   }

	if ( _outputChain._head == NULL ) setupOutputChain();
	DataLog_BufferManager::writeToChain(_outputChain, (DataLog_BufferData *)&type, 1);
	DataLog_BufferManager::writeToChain(_outputChain, (DataLog_BufferData *)&size, 2);
	DataLog_BufferManager::writeToChain(_outputChain, (DataLog_BufferData *)data, size);
}

void DataLog_Stream::printLong(long val)
{
	const char * format;
	if ( _flags & f_oct ) format="%lo";
	else if ( _flags & f_hex ) format="%lx";
	else format="%ld";

	fprintf(datalog_ConsoleFile(), format, val);
}

void DataLog_Stream::printUnsignedLong(unsigned long val)
{
	const char * format;
	if ( _flags & f_oct ) format="%lo";
	else if ( _flags & f_hex ) format="%lx";
	else format="%lu";

	fprintf(datalog_ConsoleFile(), format, val);
}

void DataLog_Stream::printDouble(double val)
{
	const char * format;
	if ( _flags && f_scientific ) format="%.*g";
	else format="%.*f";

	fprintf(datalog_ConsoleFile(), format, _precision, val);
}

void DataLog_Stream::setFlags(unsigned int flagSetting)
{
	DataLog_UINT16 oldFlags = _flags;

	if ( flagSetting & f_basemask ) _flags &= ~f_basemask;
	if ( flagSetting & f_floatmask ) _flags &= ~f_floatmask;
	_flags |= flagSetting;

	if ( oldFlags != _flags ) _flagsChanged = true;
}

void DataLog_Stream::resetFlags(unsigned int flagSetting)
{
	DataLog_UINT16 oldFlags = _flags;
	_flags &= ~flagSetting;

	if ( !(flagSetting & f_basemask) ) _flags |= f_defaultflags & f_basemask;
	if ( !(flagSetting & f_floatmask) ) _flags |= f_defaultflags & f_floatmask;

	if ( oldFlags != _flags ) _flagsChanged = true;
}

DataLog_Stream & manipfunc_setprecision(DataLog_Stream & stream, int param)
{
	stream.precision(param);
	return stream;
}

DataLog_Stream & manipfunc_setflags(DataLog_Stream & stream, int param)
{
	stream.setFlags(param);
	return stream;
}

DataLog_Stream & manipfunc_resetflags(DataLog_Stream & stream, int param)
{
	stream.resetFlags(param);
	return stream;
}

DataLog_Stream & manipfunc_errnoMsg(DataLog_Stream & stream, int param)
{
	bool	decodeOK = false; 

	if ( statSymTbl != NULL )
	{
		SYM_TYPE	type;
		char  	errName[MAX_SYS_SYM_LEN+1];
		int		errValue;

		if ( symFindByValue(statSymTbl, param, errName, &errValue, &type) == OK )
		{
			if ( errValue == param )
			{
				stream << errName;
				decodeOK = true;
			}
		}
	}

	if ( !decodeOK )
	{
		stream << "errno=" << param;
	}

	return stream;
}

DataLog_Stream & endmsg(DataLog_Stream & stream)
{
	if ( stream._consoleOutput == DataLog_ConsoleEnabled ) putc('\n', datalog_ConsoleFile());

	stream.writeComplete();
	return stream;
}

DataLog_Stream & datalog_GetDefaultStream(const char * file, int line)
{
	DataLog_CommonData common;
	DataLog_TaskInfo * taskInfo = common.findTask(DATALOG_CURRENT_TASK);

	return (taskInfo->_defaultLevel)(file, line);
}

