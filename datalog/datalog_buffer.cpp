/*
 * Copyright (C) 2002 Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: K:/BCT_Development/vxWorks/Common/datalog/rcs/datalog_buffer.cpp 1.5 2003/01/31 19:52:49 jl11312 Exp jl11312 $
 * $Log: datalog_buffer.cpp $
 * Revision 1.4  2002/10/08 14:42:45  jl11312
 * - added code to handle case for application saving a reference to a data log stream and performing multiple message writes
 * Revision 1.3  2002/09/23 13:55:15  jl11312
 * - removed obsolete UnterminatedStreamOutput error
 * Revision 1.2  2002/09/19 21:25:58  jl11312
 * - modified stream functions to not reset stream state when two stream writes occur without endmsg in between
 * - added errnoMsg manipulator function
 * Revision 1.1  2002/08/15 21:20:59  jl11312
 * Initial revision
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

#include <vxWorks.h>
#include "datalog.h"
#include "datalog_internal.h"

DataLog_Buffer::DataLog_Buffer(void)
{
	_data = DATALOG_NULL_SHARED_PTR;
}

DataLog_Buffer::~DataLog_Buffer()
{
	if ( _data != DATALOG_NULL_SHARED_PTR )
	{
		lockBufferList();
		_data->_refCount -= 1;
		releaseBufferList();
	}

	_data = DATALOG_NULL_SHARED_PTR;
}

size_t DataLog_Buffer::bytesBuffered(void)
{
	size_t	result = 0;

	if ( _data != DATALOG_NULL_SHARED_PTR )
	{
		//
		// Temporary copies are needed to insure that values don't change between
		// the if statement which determines the order of computation and the
		// computation itself.
		//
		DataLog_SharedPtr(DataLog_BufferData)	writePtr = _data->_writePtr;
		DataLog_SharedPtr(DataLog_BufferData)	readPtr = _data->_readPtr;
	
		if ( writePtr >= readPtr )
		{
			result = writePtr-readPtr;
	   }
	   else
	   {
			result = _data->_bufferSize - (readPtr-writePtr);
	   }
	}

	return result;
}

bool DataLog_InputBuffer::attachToFirstBuffer(void)
{
	DataLog_SharedPtr(SharedBufferData)	data;

	lockBufferList();
	data = getBufferListHead();
	reattach(data);
	releaseBufferList();

	return ( _data != DATALOG_NULL_SHARED_PTR );
}

bool DataLog_InputBuffer::attachToNextBuffer(void)
{
	DataLog_SharedPtr(SharedBufferData)	data = DATALOG_NULL_SHARED_PTR;

	lockBufferList();
	if ( _data )
	{
		//
		// Scan to next buffer in list
		//
		data = _data->_next;
	}

	reattach(data);
	releaseBufferList();

	return ( _data != DATALOG_NULL_SHARED_PTR );
}

void DataLog_InputBuffer::reattach(DataLog_SharedPtr(SharedBufferData) data)
{
	//
	// An empty buffer with a referenced count of 0 can be deleted at this point
	//
	if ( _data )
	{
		_data->_refCount -= 1;
		if ( _data->_refCount <= 0 &&
			  bytesBuffered() == 0 )
		{
			if ( _data->_next ) _data->_next->_prev = _data->_prev;
			if ( _data->_prev ) _data->_prev->_next = _data->_next;

			if ( _data == getBufferListHead() ) setBufferListHead(_data->_next);
			if ( _data == getBufferListTail() ) setBufferListTail(_data->_prev);

			datalog_FreeSharedMem(_data);
		}
	}

	//
	// Attach to new buffer data area and bump reference count to show
	// it is being used.
	//
	_data = data;
	if ( _data ) _data->_refCount += 1;
}

size_t DataLog_InputBuffer::read(DataLog_BufferData * ptr, size_t maxSize)
{
	size_t	bytesAvail = bytesBuffered();
	size_t	bytesRead = (maxSize < bytesAvail) ? maxSize : bytesAvail;

	int	count = 0;
	DataLog_SharedPtr(DataLog_BufferData)	readLoc = _data->_readPtr;

	while ( count < bytesRead )
	{
		//
		// Determine largest block of data that can be handled with a single
		// memcpy() call.  Because a circular buffer is used, the full read
		// operation may require two copies.  Note that we don't consider the
		// possibility of copying past _data->_writePtr, since we have already
		// computed the number of bytes to be read above.
		//
		int	maxBlockCount = _data->_endBufferPtr - readLoc;
		if ( maxBlockCount > bytesRead-count )
		{
			maxBlockCount = bytesRead-count;
		}

		memcpy(&ptr[count], readLoc, maxBlockCount);
		count += maxBlockCount;
	   readLoc += maxBlockCount;
		if ( readLoc >= _data->_endBufferPtr )
		{
			readLoc = _data->_startBufferPtr;
		}
	}

	_data->_readPtr = readLoc;
	return bytesRead;
}

DataLog_OutputBuffer::DataLog_OutputBuffer(size_t size)
{
	DataLog_SharedPtr(DataLog_BufferData) data;
	data = (DataLog_SharedPtr(DataLog_BufferData))datalog_AllocSharedMem(size+sizeof(SharedBufferData));
	_data = (DataLog_SharedPtr(SharedBufferData))data;

	DataLog_SharedPtr(DataLog_BufferData)	buffer = data + sizeof(SharedBufferData);
	_data->_startBufferPtr = buffer;
	_data->_endBufferPtr = buffer+size;
	_data->_readPtr = buffer;
	_data->_writePtr = buffer;

	_data->_bufferSize = size;
	_data->_bytesWritten = 0;
	_data->_bytesMissed = 0;

	_data->_prev = DATALOG_NULL_SHARED_PTR;
	_data->_next = DATALOG_NULL_SHARED_PTR;
	_data->_refCount = 1;

	addBufferToList();

	_writeStream = NULL;

	_rawStreamWriteInProgress = false;
	_appStreamWriteInProgress = false;
	_streamWriteRestartPos = false;

#ifdef DATALOG_MULTITHREADED
	_writeLock = datalog_CreateLock();
#endif /* ifdef DATALOG_MULTITHREADED */
}

DataLog_OutputBuffer::~DataLog_OutputBuffer()
{
	//
	// The destructor should not free shared memory for _data, since it may still
	// contain log data that has not been processed by the log output task.  Access
	// routines from the DataLog_InputBuffer class (used by the log output task)
	// will handle freeing the memory when the referenced count reaches zero and
	// all remaining data has been read from the buffer.
	//
	if ( _data != DATALOG_NULL_SHARED_PTR )
	{
		lockWriteAccess();

		if ( _appStreamWriteInProgress &&
			  _writeStream->pcount() > _streamWriteRestartPos )
		{
			streamAppWriteComplete();
		}

		if ( _writeStream )
		{
			delete _writeStream;
			_writeStream = NULL;
		}

#ifdef DATALOG_MULTITHREADED
		datalog_DeleteLock(_writeLock);
#endif /* ifdef DATALOG_MULTITHREADED */
	}
}

size_t DataLog_OutputBuffer::write(const DataLog_BufferData * ptr, size_t size)
{
	size_t	bytesFree = _data->_bufferSize-bytesBuffered()-1;
	size_t	bytesWritten = (size <= bytesFree) ? size : 0;

	lockWriteAccess();

	int	count = 0;
	DataLog_SharedPtr(DataLog_BufferData)	writeLoc = _data->_writePtr;

	while ( count < bytesWritten )
	{
		//
		// Determine largest block of data that can be handled with a single
		// memcpy() call.  Because a circular buffer is used, the full write
		// operation may require two copies.  Note that we don't consider the
		// possibility of copying past _data->_readPtr, since we have already
		// checked above that there is room for the data.
		//
		int	maxBlockCount = _data->_endBufferPtr - writeLoc;
		if ( maxBlockCount > bytesWritten-count )
		{
			maxBlockCount = bytesWritten-count;
		}

		memcpy(writeLoc, &ptr[count], maxBlockCount);
		count += maxBlockCount;
	   writeLoc += maxBlockCount;
		if ( writeLoc >= _data->_endBufferPtr )
		{
			writeLoc = _data->_startBufferPtr;
		}
	}

	_data->_writePtr = writeLoc;
	_data->_bytesWritten += bytesWritten;
	_data->_bytesMissed += size-bytesWritten;
	releaseWriteAccess();

	if ( bytesWritten != size )
	{
		datalog_SendSignal("DataLog_DataLost");
	}

	datalog_SendSignal("DataLog_Output");
	return bytesWritten;
}

DataLog_Stream & DataLog_OutputBuffer::streamAppWriteStart(const DataLog_StreamOutputRecord & header, const char * fileName, DataLog_EnabledType logOutput, DataLog_ConsoleEnabledType consoleOutput)
{
	lockWriteAccess();
	if ( _rawStreamWriteInProgress )
	{
		//
		// Shouldn't be in this function if a stream write is already in progress
		//
		DataLog_CommonData	common;
		common.setTaskError(DataLog_InternalWriteError, __FILE__, __LINE__);
	}

	if ( !_writeStream )
	{
		_writeStream = new DataLog_Stream(this);
	}

	_writeStream->clear();
	_writeStream->setLogOutput(logOutput);
	_writeStream->setConsoleOutput(consoleOutput);
	_writeStream->rawWrite(&header, sizeof(DataLog_StreamOutputRecord));
	_writeStream->rawWrite(fileName, sizeof(char)*header._fileNameLen);

	DataLog_UINT16	flags = _writeStream->getFlags();
	DataLog_UINT8  precision = _writeStream->getPrecision();
	_writeStream->rawWrite(&flags, sizeof(flags));
	_writeStream->rawWrite(&precision, sizeof(precision));

	//
	// This 2 byte value will be replaced by the argument length when the
	// stream data is written to the log.
	//
	DataLog_UINT16	dummy = 0;
	_writeStream->rawWrite(&dummy, sizeof(dummy));

	_appStreamWriteInProgress = true;
	_appConsoleOutput = consoleOutput;
	_streamWriteRestartPos = _writeStream->pcount();

	releaseWriteAccess();
	return *_writeStream;
}

size_t DataLog_OutputBuffer::streamAppWriteComplete(void)
{
	size_t	result = 0;

	lockWriteAccess();
	if ( !_appStreamWriteInProgress )
	{
		//
		//	There is an internal problem with the use of the stream interface, or
		// an application has saved and re-used a data log stream reference which
		// is no longer valid.
		//
		DataLog_CommonData  common;
		common.setTaskError(DataLog_InternalWriteError, __FILE__, __LINE__);
	}
	else if ( _writeStream->fail() )
	{
		//
		// Problem while writing data to the stream.  Simply indicate that data was
		// lost since we don't want to write possibly corrupted data to the log file
		//
		_data->_bytesMissed += _writeStream->pcount();
		datalog_SendSignal("DataLog_DataLost");
	}
	else
	{
		DataLog_BufferData * startData = (DataLog_BufferData *)_writeStream->data();
		size_t dataLen = _writeStream->pcount();
		DataLog_UINT16 argLength = 0;

		if ( _appStreamWriteInProgress )
		{
			//
			//	For application writes, we don't know the length of the app
			// data until the writes have been completed, so we compute the
			// length here and write it to the appropriate location in the
			// output data.
			//
			const DataLog_StreamOutputRecord * streamOutputRecord = (const DataLog_StreamOutputRecord *)startData;
			size_t lengthOffset = sizeof(DataLog_StreamOutputRecord) + sizeof(char)*streamOutputRecord->_fileNameLen + 3;
			argLength = _writeStream->pcount() - lengthOffset - 2;

		   memcpy(&startData[lengthOffset], &argLength, sizeof(DataLog_UINT16));
		}

		//
		// Output the record to the log.  Note that if log output was disabled, 
		// the stream will not contain any argument data.
		//
		if ( _rawStreamWriteInProgress ||
			  _appStreamWriteInProgress && argLength > 0 )
		{
			result = write(startData, dataLen);
		}
	}

	_writeStream->seekp(_streamWriteRestartPos);
	releaseWriteAccess();

	if ( _appConsoleOutput == DataLog_ConsoleEnabled )
	{
		putchar('\n');
	}

	return result;
}

void DataLog_OutputBuffer::partialWrite(const DataLog_BufferData * ptr, size_t size)
{
	lockWriteAccess();

	if ( !_writeStream )
	{
		_writeStream = new DataLog_Stream(this);
	}

	_appStreamWriteInProgress = false;
	if ( !_rawStreamWriteInProgress )
	{
		_writeStream->clear();
		_rawStreamWriteInProgress = true;
	}

	_writeStream->rawWrite(ptr, size);
	releaseWriteAccess();
}

size_t DataLog_OutputBuffer::partialWriteComplete(void)
{
	size_t	result = 0;

	lockWriteAccess();
	if ( !_rawStreamWriteInProgress )
	{
		//
		//	There is an internal problem with the use of the partial write interface.
		//
		DataLog_CommonData  common;
		common.setTaskError(DataLog_InternalWriteError, __FILE__, __LINE__);
	}
	else if ( _writeStream->fail() )
	{
		//
		// Problem while writing data to the stream.  Simply indicate that data was
		// lost since we don't want to write possibly corrupted data to the log file
		//
		_data->_bytesMissed += _writeStream->pcount();
		datalog_SendSignal("DataLog_DataLost");
	}
	else
	{
		result = write((DataLog_BufferData *)_writeStream->data(), _writeStream->pcount());
	}

	_rawStreamWriteInProgress = false;
	releaseWriteAccess();

	return result;
}

void DataLog_OutputBuffer::lockWriteAccess(void)
{
#ifdef DATALOG_MULTITHREADED
	datalog_LockAccess(_writeLock);
#endif /* ifdef DATALOG_MULTITHREADED */
}

void DataLog_OutputBuffer::releaseWriteAccess(void)
{
#ifdef DATALOG_MULTITHREADED
	datalog_ReleaseAccess(_writeLock);
#endif /* ifdef DATALOG_MULTITHREADED */
}

void DataLog_OutputBuffer::addBufferToList(void)
{
	//
	// Standard buffers are added to the end of the buffer list.  This
	// function is overridden for critical buffers, which are inserted at
	// the head of the list.
	//
	lockBufferList();
	if ( getBufferListHead() == DATALOG_NULL_SHARED_PTR )
	{
		//
		// Empty list - insert as first element
		//
		setBufferListHead(_data);
		setBufferListTail(_data);

		_data->_prev = DATALOG_NULL_SHARED_PTR;
		_data->_next = DATALOG_NULL_SHARED_PTR;
	}
	else
	{
		//
		// List has at least one item - make this buffer the new end of list
		//
		DataLog_SharedPtr(SharedBufferData)	oldTail = getBufferListTail();

		setBufferListTail(_data);
		_data->_prev = oldTail;
		_data->_next = DATALOG_NULL_SHARED_PTR;
		oldTail->_next = _data;
	}

	releaseBufferList();
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
	_writeStream = new DataLog_Stream(this);
}

size_t DataLog_CriticalBuffer::write(const DataLog_BufferData * ptr, size_t size)
{
	size_t	result = DataLog_OutputBuffer::write(ptr, size);
	datalog_SendSignal("DataLog_CriticalOutput");

   return result;
}

