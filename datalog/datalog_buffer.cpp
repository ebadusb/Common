/*
 * Copyright (C) 2002 Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: K:/BCT_Development/vxWorks/Common/datalog/rcs/datalog_buffer.cpp 1.5 2003/01/31 19:52:49 jl11312 Exp jl11312 $
 * $Log: datalog_buffer.cpp $
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

	if ( bytesRead > 0 )
	{
		DataLog_SharedPtr(DataLog_BufferData)	readLoc = _data->_readPtr;

		for (int count=0; count<bytesRead; count++)
		{
			ptr[count] = *readLoc;
			readLoc += 1;
			if ( readLoc >= _data->_endBufferPtr )
			{
				readLoc = _data->_startBufferPtr;
		   }
		}

		_data->_readPtr = readLoc;
	}

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
	_streamWriteBuffer = NULL;
	_streamWriteCompleteCallBack = NULL;

	_streamWriteInProgress = false;
	_streamWriteReleasedToApp = false;

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

		if ( _streamWriteInProgress && _streamWriteReleasedToApp )
		{
			streamWriteComplete();
		}

		if ( _writeStream )
		{
			delete _writeStream;
			_writeStream = NULL;
		}

		if ( _streamWriteBuffer )
		{
			delete[] _streamWriteBuffer;
			_streamWriteBuffer = NULL;
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

	DataLog_SharedPtr(DataLog_BufferData)	writeLoc = _data->_writePtr;
	for (int count=0; count<bytesWritten; count++)
	{
		*writeLoc = ptr[count];
		writeLoc += 1;
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

DataLog_Stream & DataLog_OutputBuffer::streamWriteStart(NotifyStreamWriteComplete * callBack, size_t callBackArgSize)
{
	lockWriteAccess();

	if ( _streamWriteInProgress )
	{
		DataLog_CommonData	common;

		if ( _streamWriteReleasedToApp )
		{
			//
			// Application code must have a stream output statement with no endmsg.  Handle
			// as if endmsg is called now, immediately before the current output statement.
			//
			streamWriteComplete();
			common.setTaskError(DataLog_UnterminatedStreamOutput, __FILE__, __LINE__);
		}
		else
		{
			//
			// If not a result of unterminated stream output, then there must
			// be an internal problem with the use of the stream interface.
			//
			common.setTaskError(DataLog_InternalWriteError, __FILE__, __LINE__);
		}
	}

	_streamWriteInProgress = true;
	_streamWriteReleasedToApp = false;
	_streamWriteCompleteCallBack = callBack;
	_streamWriteCompleteCallBackArgSize = callBackArgSize;

	if ( !_writeStream )
	{
		_writeStream = new DataLog_Stream(this);
	}

	_writeStream->clear();
	_writeStream->seekp(0);

	releaseWriteAccess();
	return *_writeStream;
}

void DataLog_OutputBuffer::streamWriteReleaseToApp(void)
{
	lockWriteAccess();
	_streamWriteReleasedToApp = true;
	releaseWriteAccess();
}

size_t DataLog_OutputBuffer::streamWriteComplete(void)
{
	size_t	result = 0;

	lockWriteAccess();
	if ( !_streamWriteInProgress )
	{
		//
		//	There is an internal problem with the use of the stream interface.
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
		//
		// Lock stream buffer
		//
		_writeStream->freeze(1);

		//
		// Compute start location of data that needs to go to the log.  If the
		// caller specified a stream write complete callback, some bytes at the
		// beginning of the stream may be used for arguments to the call back
		// and should not be written to the log.
		//
		DataLog_BufferData * startData = ((DataLog_BufferData *)(_writeStream->str())) + _streamWriteCompleteCallBackArgSize;
		size_t dataLen = _writeStream->pcount()*sizeof(char) - _streamWriteCompleteCallBackArgSize;

		if ( _streamWriteReleasedToApp )
		{
			//
			//	For application writes, we don't know the length of the app
			// data until the writes have been completed, so we compute the
			// length here and write it to the appropriate location in the
			// output data.
			//
			const DataLog_StreamOutputRecord * streamOutputRecord = (const DataLog_StreamOutputRecord *)startData;
			DataLog_BufferData * appDataLenPtr = startData + sizeof(*streamOutputRecord) + sizeof(char)*streamOutputRecord->_fileNameLen;
			DataLog_UINT16	appDataLen = dataLen - sizeof(*streamOutputRecord) - sizeof(char)*streamOutputRecord->_fileNameLen - sizeof(DataLog_UINT16);

		   memcpy(appDataLenPtr, &appDataLen, sizeof(DataLog_UINT16));
		}

		//
		// Output the record to the console/log as necessary
		//
		DataLog_EnabledType	logOutput = DataLog_LogEnabled;
		if ( _streamWriteCompleteCallBack )
		{
			logOutput = (*_streamWriteCompleteCallBack)((DataLog_BufferData *)_writeStream->str(), _writeStream->pcount()*sizeof(char));
	   }

		if ( logOutput == DataLog_LogEnabled )
		{
			result = write(startData, dataLen);
		}

		_writeStream->freeze(0);
	}

	_streamWriteInProgress = false;
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
	_streamWriteBuffer = new DataLog_BufferData[bufferSize];
	_writeStream = new DataLog_Stream(_streamWriteBuffer, bufferSize, this);
}

size_t DataLog_CriticalBuffer::write(const DataLog_BufferData * ptr, size_t size)
{
	size_t	result = DataLog_OutputBuffer::write(ptr, size);
	datalog_SendSignal("DataLog_CriticalOutput");

   return result;
}

