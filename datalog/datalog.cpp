/*
 * Copyright (C) 2002 Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: //bctquad3/home/BCT_Development/vxWorks/Common/datalog/rcs/datalog.cpp 1.13 2003/12/05 16:33:05Z jl11312 Exp rm70006 $
 * $Log: datalog.cpp $
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

#include <typeinfo>

DataLog_OutputBuffer::DataLog_OutputBuffer(size_t bufferSize)
{
	_startBufferPtr = (SharedBufferPtr)datalog_AllocSharedMem(bufferSize);
	_endBufferPtr = _startBufferPtr+bufferSize;
	_readPtr = _writePtr = _startBufferPtr;

	_bufferSize = bufferSize;
	_bytesWritten = _bytesMissed = 0;
	_partialWriteInProgress = false;

	_stream = NULL;
	_streamBuffer = NULL;
	_streamBufferSize = 0;
	_streamBufferInUse = false;

#ifdef DATALOG_MULTITHREADED
	_lock = datalog_CreateLock();
	_streamLock = datalog_CreateLock();
#endif /* ifdef DATALOG_MULTITHREADED */
}

DataLog_OutputBuffer::~DataLog_OutputBuffer()
{
	datalog_FreeSharedMem(_startBufferPtr);
}

size_t DataLog_OutputBuffer::read(DataLog_BufferData * ptr, size_t maxSize)
{
	size_t	bytesAvail = bytesBuffered();
	size_t	bytesRead = (maxSize < bytesAvail) ? maxSize : bytesAvail;

	if ( _partialWriteInProgress )
	{
		//
		// Don't allow reading the buffer until we have a complete message
		//
		bytesRead = 0;
	}

	SharedBufferPtr	readLoc = _readPtr;
	for (int count=0; count<bytesRead; count++)
	{
		ptr[count] = *readLoc;
		readLoc += 1;
		if ( readLoc >= _endBufferPtr )
		{
			readLoc = _startBufferPtr;
		}
	}

	_readPtr = readLoc;
	return bytesRead;
}

size_t DataLog_OutputBuffer::write(DataLog_BufferData * ptr, size_t size)
{
	lockWriteAccess();

	size_t	bytesFree = _bufferSize-bytesBuffered()-1;
	size_t	bytesWritten = (size <= bytesFree) ? size : 0;

	if ( _partialWriteInProgress )
	{
		DataLog_CommonData * common = datalog_GetCommonDataPtr();
		common->setTaskError(DataLog_InternalWriteError, __FILE__, __LINE__);
		bytesWritten = 0;
	}
	else
	{
		doWrite(ptr, bytesWritten);
	}

	_bytesWritten += bytesWritten;
	_bytesMissed += size-bytesWritten;
	releaseWriteAccess();

	if ( bytesWritten != size )
	{
		datalog_SendSignal("DataLog_DataLost");
	}

	datalog_SendSignal("DataLog_Output");
	return bytesWritten;
}

void DataLog_OutputBuffer::partialWriteStart(void)
{
	if ( _partialWriteInProgress && _streamBufferInUse )
	{
		//
		// User code must have a stream output statement with no endmsg.  Handle
		// as if endmsg is called now, immediately before the current output statement.
		//
		releaseStreamBuffer();
	}
	else if ( !_partialWriteInProgress )
	{
		lockWriteAccess();
		_partialWriteInProgress = true;
		_partialWriteOverflow = false;
		_partialWriteStartPtr = _writePtr;
		_partialWriteBytesWritten = 0;
	}
}

size_t DataLog_OutputBuffer::partialWrite(DataLog_BufferData * ptr, size_t size)
{
	if ( !_partialWriteInProgress )
	{
		partialWriteStart();
	}

	size_t	bytesFree = _bufferSize-bytesBuffered()-1;
	size_t	bytesWritten = (size <= bytesFree) ? size : 0;
	doWrite(ptr, bytesWritten);

	if ( bytesWritten != size )
	{
		_partialWriteOverflow = true;
	}

	_partialWriteBytesWritten += bytesWritten;
	_bytesMissed += size-bytesWritten;
	return bytesWritten;
}

void DataLog_OutputBuffer::partialWriteComplete(void)
{
	if ( !_partialWriteInProgress )
	{
		DataLog_CommonData * common = datalog_GetCommonDataPtr();
		common->setTaskError(DataLog_InternalWriteError, __FILE__, __LINE__);
	}
	else
	{
		if ( _partialWriteOverflow )
		{
			_writePtr = _partialWriteStartPtr;
			_bytesMissed += _partialWriteBytesWritten;
			datalog_SendSignal("DataLog_DataLost");
		}
		else
		{
			_bytesWritten += _partialWriteBytesWritten;
	   }

		_partialWriteInProgress = false;
		releaseWriteAccess();
		datalog_SendSignal("DataLog_Output");
   }
}

void DataLog_OutputBuffer::doWrite(DataLog_BufferData * ptr, size_t bytesWritten)
{
	SharedBufferPtr	writeLoc = _writePtr;
	for (int count=0; count<bytesWritten; count++)
	{
		*writeLoc = ptr[count];
		writeLoc += 1;
		if ( writeLoc >= _endBufferPtr )
		{
			writeLoc = _startBufferPtr;
		}
	}

	_writePtr = writeLoc;
}

DataLog_Stream & DataLog_OutputBuffer::lockStreamBuffer(const DataLog_StreamOutputRecord * streamOutputRecord, const char * fileName, DataLog_EnabledType logOutput, DataLog_ConsoleEnabledType consoleOutput)
{
	if ( logOutput == DataLog_LogEnabled )
	{
		//
		// Need to start the log write before accessing internal stream.  This
		// makes sure that any previous stream output is flushed before starting
		// the new output.
		//
		partialWriteStart();
		partialWrite((DataLog_BufferData *)streamOutputRecord, sizeof(*streamOutputRecord));  
		partialWrite((DataLog_BufferData *)fileName, streamOutputRecord->_fileNameLen * sizeof(char));
	}

#ifdef DATALOG_MULTITHREADED
	datalog_LockAccess(_streamLock);
#endif /* ifdef DATALOG_MULTITHREADED */

	if ( !_streamBuffer )
	{
		_streamBufferSize = _bufferSize;
		_streamBuffer = new DataLog_BufferData[_streamBufferSize];
		_stream = new DataLog_Stream(_streamBuffer, _streamBufferSize, this);
	}

	_streamLogOutput = logOutput;
	_streamConsoleOutput = consoleOutput;
	_streamBufferInUse = true;
	_stream->seekp(0);

#ifdef DATALOG_MULTITHREADED
	datalog_ReleaseAccess(_streamLock);
#endif /* ifdef DATALOG_MULTITHREADED */

	return *_stream;
}

void DataLog_OutputBuffer::releaseStreamBuffer(void)
{
#ifdef DATALOG_MULTITHREADED
	datalog_LockAccess(_streamLock);
#endif /* ifdef DATALOG_MULTITHREADED */

	if ( _streamBufferInUse )
	{
		DataLog_UINT16	outputLen = _stream->pcount();

		if ( _streamLogOutput == DataLog_LogEnabled )
		{
			partialWrite((DataLog_BufferData *)&outputLen, sizeof(outputLen));
			partialWrite(_streamBuffer, outputLen*sizeof(char));
			partialWriteComplete();
	   }

		if ( _streamConsoleOutput == DataLog_ConsoleEnabled )
		{
			cout.write((char *)_streamBuffer, outputLen*sizeof(char));
			cout << endl;
		}

		_streamBufferInUse = false;
	}

#ifdef DATALOG_MULTITHREADED
	datalog_ReleaseAccess(_streamLock);
#endif /* ifdef DATALOG_MULTITHREADED */
}

void DataLog_OutputBuffer::lockWriteAccess(void)
{
#ifdef DATALOG_MULTITHREADED
	datalog_LockAccess(_lock);
#endif /* ifdef DATALOG_MULTITHREADED */
}

void DataLog_OutputBuffer::releaseWriteAccess(void)
{
#ifdef DATALOG_MULTITHREADED
	datalog_ReleaseAccess(_lock);
#endif /* ifdef DATALOG_MULTITHREADED */
}

size_t DataLog_OutputBuffer::bytesBuffered(void)
{
	//
	// Temporary copies are needed to insure that values don't change between
	// the if statement which determines the order of computation and the
	// computation itself.
	//
	SharedBufferPtr	writePtr = _writePtr;
	SharedBufferPtr	readPtr = _readPtr;
	size_t	result;
	
	if ( writePtr >= readPtr )
	{
		result = writePtr-readPtr;
	}
	else
	{
		result = _bufferSize - (readPtr-writePtr);
	}

	return result;
}

DataLog_TraceBuffer::DataLog_TraceBuffer(size_t bufferSize)
				: DataLog_OutputBuffer(bufferSize)
{
}

#ifndef DATALOG_NO_NETWORK_SUPPORT
DataLog_ClientBuffer::DataLog_ClientBuffer(size_t bufferSize)
				: DataLog_OutputBuffer(bufferSize)
{
}
#endif /* ifndef DATALOG_NO_NETWORK_SUPPORT */

DataLog_PeriodicBuffer::DataLog_PeriodicBuffer(size_t bufferSize)
				: DataLog_OutputBuffer(bufferSize)
{
}

DataLog_IntBuffer::DataLog_IntBuffer(size_t bufferSize)
				: DataLog_OutputBuffer(bufferSize)
{
}

DataLog_CriticalBuffer::DataLog_CriticalBuffer(size_t bufferSize)
				: DataLog_OutputBuffer(bufferSize)
{
}

size_t DataLog_CriticalBuffer::write(DataLog_BufferData * ptr, size_t size)
{
	size_t	result = DataLog_OutputBuffer::write(ptr, size);
	datalog_SendSignal("DataLog_CriticalOutput");

   return result;
}

void DataLog_CriticalBuffer::partialWriteComplete(void)
{
	DataLog_OutputBuffer::partialWriteComplete();
	datalog_SendSignal("DataLog_CriticalOutput");
}

DataLog_CommonData::DataLog_CommonData(void)
{
	_connectType = NotConnected;
	_connectName = NULL;

	_tasksLock = datalog_CreateLock();

	_buffersLock = datalog_CreateLock();
	_savedBytesWritten = 0;
	_savedBytesMissed = 0;

	_setsLock = datalog_CreateLock();
	_setNumber = 0;

	_levelsLock = datalog_CreateLock();
	_levelNumber = 0;

	_defaultTraceBufferSize = 4096;
	_defaultIntBufferSize = 2048;
	_defaultCriticalBufferSize = 4096;

	createInternalBuffer();
}

DataLog_CommonData::~DataLog_CommonData(void)
{
	delete[] _connectName;
}

void DataLog_CommonData::setConnect(ConnectType type, const char * name)
{
	_connectType = type;

	char	* buffer = (char *)malloc(strlen(name)+1);
	strcpy(buffer, name);
	_connectName = buffer;
}

DataLog_CommonData::TaskInfoPtr DataLog_CommonData::findTask(DataLog_TaskID task)
{
	TaskInfoPtr	result = DATALOG_NULL_SHARED_PTR;
	DataLog_TaskID	searchTask = (task == DATALOG_CURRENT_TASK) ? datalog_CurrentTask() : task;

	datalog_LockAccess(_tasksLock);
	if ( _tasks.find(searchTask) != _tasks.end() )
	{
		result = _tasks[searchTask];
	}

	datalog_ReleaseAccess(_tasksLock);
	return result;
}

void DataLog_CommonData::addTask(TaskInfoPtr taskInfo)
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
		addBuffer(taskInfo->_trace);
		addBuffer(taskInfo->_critical);
	}

	datalog_ReleaseAccess(_tasksLock);
}

void DataLog_CommonData::deleteTask(DataLog_TaskID task)
{
	TaskInfoPtr	taskInfo = DATALOG_NULL_SHARED_PTR;

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
	if ( taskInfo != DATALOG_NULL_SHARED_PTR )
	{
		taskInfo->_critical->_deleteOnEmpty = true;
		taskInfo->_trace->_deleteOnEmpty = true;
		datalog_SendSignal("DataLog_Output");

		datalog_DeleteLock(taskInfo->_lock);
		datalog_FreeSharedMem((void *)taskInfo->_name);
		datalog_FreeSharedMem(taskInfo);
	}
}

DataLog_CommonData::BufferInfoPtr DataLog_CommonData::getTaskTraceBuffer(DataLog_TaskID taskID)
{
	BufferInfoPtr	result;
	TaskInfoPtr taskInfo = findTask(taskID);

	if ( taskInfo == DATALOG_NULL_SHARED_PTR )
	{
		setTaskError(DataLog_NoSuchTask, __FILE__, __LINE__);
		result = _internalBuffer;
	}
	else
	{
		result = taskInfo->_trace;
	}

	return result;
}

void DataLog_CommonData::addBuffer(BufferInfoPtr bufferInfo)
{
	datalog_LockAccess(_buffersLock);
	if ( typeid(*(bufferInfo->_buffer)) == typeid(DataLog_CriticalBuffer) )
	{
		_buffers.push_front(bufferInfo);
	}
	else
	{
		_buffers.push_back(bufferInfo);
	}

	datalog_ReleaseAccess(_buffersLock);
}

void DataLog_CommonData::deleteBuffer(BufferInfoPtr bufferInfo)
{
	datalog_LockAccess(_buffersLock);
	_buffers.remove(bufferInfo);
	datalog_ReleaseAccess(_buffersLock);

	_savedBytesWritten += bufferInfo->_buffer->bytesWritten();
	_savedBytesMissed += bufferInfo->_buffer->bytesMissed();

	bufferInfo->_buffer->~DataLog_OutputBuffer();
	datalog_FreeSharedMem(bufferInfo->_buffer);
	datalog_FreeSharedMem(bufferInfo);
}

DataLog_CommonData::BufferInfoPtr DataLog_CommonData::findFirstBuffer(void)
{
	BufferInfoPtr	result = DATALOG_NULL_SHARED_PTR;

	datalog_LockAccess(_buffersLock);
	_buffersIter = _buffers.begin();
	if ( _buffersIter != _buffers.end() )
	{
		result = *_buffersIter;
	}

	datalog_ReleaseAccess(_buffersLock); 
	return result;
}

DataLog_CommonData::BufferInfoPtr DataLog_CommonData::findNextBuffer(void)
{
	BufferInfoPtr	result = DATALOG_NULL_SHARED_PTR; 

	datalog_LockAccess(_buffersLock);
	++_buffersIter;
	if ( _buffersIter != _buffers.end() )
	{
		result = *_buffersIter;
	}

	datalog_ReleaseAccess(_buffersLock);
	return result;
}

DataLog_SetHandle DataLog_CommonData::findSet(const char * name)
{
	DataLog_SetHandle	result = DATALOG_NULL_SHARED_PTR;

	datalog_LockAccess(_setsLock);
	if ( _sets.find(name) != _sets.end() )
	{
		result = _sets[name];
	}

	datalog_ReleaseAccess(_setsLock);
	return result;
}

void DataLog_CommonData::addSet(DataLog_SetHandle setInfo)
{
	datalog_LockAccess(_setsLock);
	if ( _sets.find(setInfo->_name) != _sets.end() )
	{
		DataLog_Critical	errorLog;
		DataLog(errorLog) << "attempt to add duplicate set ignored" << endmsg;
	}
	else
	{
		_setNumber += 1;
		setInfo->_id = _setNumber;
		_sets[setInfo->_name] = setInfo;
	}

	datalog_ReleaseAccess(_setsLock);
}

DataLog_CommonData::LevelInfoPtr DataLog_CommonData::findLevel(const char * name)
{
	LevelInfoPtr	result = DATALOG_NULL_SHARED_PTR;

	datalog_LockAccess(_levelsLock);
	if ( _levels.find(name) != _levels.end() )
	{
		result = _levels[name];
	}

	datalog_ReleaseAccess(_levelsLock);
	return result;
}

void DataLog_CommonData::addLevel(LevelInfoPtr levelInfo)
{
	datalog_LockAccess(_levelsLock);
	if ( _levels.find(levelInfo->_name) != _levels.end() )
	{
		DataLog_Critical	errorLog;
		DataLog(errorLog) << "attempt to add duplicate level ignored" << endmsg;
	}
	else
	{
		_levelNumber += 1;
		levelInfo->_id = _levelNumber;
		_levels[levelInfo->_name] = levelInfo;
	}

	datalog_ReleaseAccess(_levelsLock);
}

DataLog_TaskErrorHandler * DataLog_CommonData::getTaskErrorHandler(DataLog_TaskID task)
{
	DataLog_TaskErrorHandler * result = NULL;
	TaskInfoPtr	taskInfo = findTask(task);
	if ( taskInfo != DATALOG_NULL_SHARED_PTR )
	{
		result = taskInfo->_errorHandler;
	}

	return result;
}

void DataLog_CommonData::setTaskErrorHandler(DataLog_TaskID task, DataLog_TaskErrorHandler * func)
{
	TaskInfoPtr	taskInfo = findTask(task);
	if ( taskInfo != DATALOG_NULL_SHARED_PTR )
	{
		taskInfo->_errorHandler = func;
	}
}

void DataLog_CommonData::setTaskError(DataLog_ErrorType error, const char * file, int line)
{
	TaskInfoPtr taskInfo = findTask(DATALOG_CURRENT_TASK);
	const char * message = datalog_ErrorMessage(error);
	int continuable = datalog_ErrorContinuable(error);

	if ( taskInfo != DATALOG_NULL_SHARED_PTR )
	{
		taskInfo->_error = error;
		if ( !taskInfo->_errorHandlerActive && taskInfo->_errorHandler )
		{
			taskInfo->_errorHandlerActive = true;
			(*taskInfo->_errorHandler)(file, line, error, message, continuable);
			taskInfo->_errorHandlerActive = false;
		}
	}

	if ( !continuable )
	{
		abort();
	}
}

size_t DataLog_CommonData::currentMaxBufferSize(void)
{
	size_t	maxBufferSize;

	if ( _defaultTraceBufferSize > _defaultIntBufferSize )
	{
		maxBufferSize = ( _defaultTraceBufferSize > _defaultCriticalBufferSize ) ?
				_defaultTraceBufferSize : _defaultCriticalBufferSize;
	}
	else
	{
		maxBufferSize = ( _defaultIntBufferSize > _defaultCriticalBufferSize ) ?
				_defaultIntBufferSize : _defaultCriticalBufferSize;
	}

	return maxBufferSize;
}

void DataLog_CommonData::createInternalBuffer(void)
{
	size_t	internalBufferSize = currentMaxBufferSize();
	_internalBuffer = (DataLog_CommonData::BufferInfoPtr)datalog_AllocSharedMem(sizeof(DataLog_BufferInfo));

	void * bufferArea = datalog_AllocSharedMem(sizeof(DataLog_TraceBuffer));
	_internalBuffer->_buffer = new(bufferArea) DataLog_TraceBuffer(internalBufferSize);
	_internalBuffer->_deleteOnEmpty = false;
	addBuffer(_internalBuffer);
}

DataLog_Result datalog_Error(DataLog_TaskID task, DataLog_ErrorType * error)
{
	DataLog_Result	result = DataLog_OK;
	DataLog_CommonData * common = datalog_GetCommonDataPtr();
	DataLog_CommonData::TaskInfoPtr	taskInfo = common->findTask(task);

	if ( taskInfo == DATALOG_NULL_SHARED_PTR )
	{
		common->setTaskError(DataLog_NoSuchTask, __FILE__, __LINE__);
		result = DataLog_Error;
	}
	else
	{
		*error = taskInfo->_error;
	}

	return result;
}

DataLog_Result datalog_ClearError(DataLog_TaskID task)
{
	DataLog_Result	result = DataLog_OK;
	DataLog_CommonData * common = datalog_GetCommonDataPtr();
	DataLog_CommonData::TaskInfoPtr	taskInfo = common->findTask(task);

	if ( taskInfo != DATALOG_NULL_SHARED_PTR )
	{
		taskInfo->_error = DataLog_NoError;
	}
	else
	{
		common->setTaskError(DataLog_NoSuchTask, __FILE__, __LINE__);
		result = DataLog_Error;
	}

	return result;
}

const char * datalog_ErrorMessage(DataLog_ErrorType error)
{
	const char * result = NULL;
 
	for (int i=0; i<DataLog_LastError && !result; i++)
	{
		if ( datalog_ErrorInformation[i]._error == error )
		{
			result = datalog_ErrorInformation[i]._message;
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
	DataLog_CommonData::TaskInfoPtr info = (DataLog_CommonData::TaskInfoPtr)datalog_AllocSharedMem(sizeof(DataLog_TaskInfo));
	WIND_TCB	* tcb = taskTcb(taskID);
	DataLog_CommonData * common = datalog_GetCommonDataPtr();

	info->_id = taskID;

	DataLog_SharedPtr(char)	nameBuffer = (DataLog_SharedPtr(char))datalog_AllocSharedMem(strlen(tcb->name)+1);
	strcpy(nameBuffer, tcb->name);
	info->_name = nameBuffer;

	info->_error = DataLog_NoError;
	info->_lock = datalog_CreateLock();

	info->_trace = (DataLog_CommonData::BufferInfoPtr)datalog_AllocSharedMem(sizeof(DataLog_BufferInfo));

	void * traceBufferArea = datalog_AllocSharedMem(sizeof(DataLog_TraceBuffer));
	info->_trace->_buffer = new(traceBufferArea) DataLog_TraceBuffer(common->defaultTraceBufferSize());
	info->_trace->_deleteOnEmpty = false;

	info->_logOutput = DataLog_LogDisabled;
	info->_consoleOutput = DataLog_ConsoleDisabled;

	info->_critical = (DataLog_CommonData::BufferInfoPtr)datalog_AllocSharedMem(sizeof(DataLog_BufferInfo));

	void * criticalBufferArea = datalog_AllocSharedMem(sizeof(DataLog_CriticalBuffer));
	DataLog_SharedPtr(DataLog_CriticalBuffer) criticalBuffer = new(criticalBufferArea) DataLog_CriticalBuffer(common->defaultCriticalBufferSize());
	info->_critical->_buffer = criticalBuffer;
	info->_critical->_deleteOnEmpty = false;

	info->_criticalHandle = (DataLog_Handle)datalog_AllocSharedMem(sizeof(DataLog_HandleInfo));
	info->_criticalHandle->_type = DataLog_HandleInfo::CriticalHandle;
	info->_criticalHandle->_criticalData._id = taskID;
	info->_criticalHandle->_criticalData._buffer = criticalBuffer;

	info->_errorHandler = NULL;
	info->_errorHandlerActive = false;

	common->addTask(info);
	info->_defaultLevel.setHandle(info->_criticalHandle);
	info->_defaultHandle = info->_criticalHandle;

	DataLog_TaskCreateRecord taskCreateRecord;
	taskCreateRecord._recordType = DataLog_TaskCreateRecordID;
	datalog_GetTimeStamp(&taskCreateRecord._timeStamp);
	taskCreateRecord._taskID = taskID;
	taskCreateRecord._levelID = DATALOG_SYSTEM_LEVEL_ID;

#ifndef DATALOG_NO_NETWORK_SUPPORT
	taskCreateRecord._nodeID = datalog_NodeID();
#endif /* ifndef DATALOG_NO_NETWORK_SUPPORT */

	taskCreateRecord._nameLen = strlen(info->_name);

	info->_critical->_buffer->partialWriteStart();
	info->_critical->_buffer->partialWrite((DataLog_BufferData *)&taskCreateRecord, sizeof(taskCreateRecord));
	info->_critical->_buffer->partialWrite((DataLog_BufferData *)info->_name, taskCreateRecord._nameLen * sizeof(char));
	info->_critical->_buffer->partialWriteComplete();
}

void datalog_TaskDeleted(DataLog_TaskID taskID)
{
	DataLog_CommonData * common = datalog_GetCommonDataPtr();
	DataLog_CommonData::TaskInfoPtr taskInfo = common->findTask(taskID);

	//
	// Use the deleted tasks critical buffer for the task deleted log
	// record, then actually delete the task (the task created log record
	// also uses this output buffer, so the create/delete records are
	// guaranteed sequential).
	//
	if ( taskInfo != DATALOG_NULL_SHARED_PTR )
	{
		DataLog_TaskDeleteRecord taskDeleteRecord;

		taskDeleteRecord._recordType = DataLog_TaskDeleteRecordID;
		taskDeleteRecord._taskID = taskID;
		taskDeleteRecord._levelID = DATALOG_SYSTEM_LEVEL_ID;

#ifndef DATALOG_NO_NETWORK_SUPPORT
		taskDeleteRecord._nodeID = datalog_NodeID();
#endif /* ifndef DATALOG_NO_NETWORK_SUPPORT */

		datalog_GetTimeStamp(&taskDeleteRecord._timeStamp);
		taskInfo->_critical->_buffer->write((DataLog_BufferData *)&taskDeleteRecord, sizeof(taskDeleteRecord));
	}

	common->deleteTask(taskID);
}

