/*
 * Copyright (C) 2002 Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: I:/BCT_Development/vxWorks/Common/datalog/rcs/datalog_buffer.cpp 1.7 2003/11/10 17:46:05Z jl11312 Exp $
 * $Log: datalog_buffer.cpp $
 * Revision 1.7  2003/11/10 17:46:05Z  jl11312
 * - corrections from data log unit tests (see IT 6598)
 * Revision 1.6  2003/02/25 16:10:07Z  jl11312
 * - modified buffering scheme to help prevent buffer overruns
 *
 */

#include "datalog.h"
#include "datalog_internal.h"

bool DataLog_BufferManager::createChain(DataLog_BufferChain & chain, unsigned long reserveBuffers)
{
	chain._head = chain._tail = NULL;
	chain._missedBytes = 0;
	chain._reserveBuffers = reserveBuffers;

	return true;
}

bool DataLog_BufferManager::copyChain(DataLog_BufferChain & dest, const DataLog_BufferChain & source)
{
	bool copyOK = createChain(dest, source._reserveBuffers);
	DataLog_BufferPtr sourceBuffer = source._head;

	while ( copyOK &&
			  sourceBuffer != NULL )
	{
		DataLog_BufferPtr newTail = getFreeBuffer(source._reserveBuffers);
		if ( newTail )
		{
			newTail->_next = newTail->_tail = NULL;
			newTail->_length = 0;

			if ( !dest._head )
			{
				dest._head = newTail;
			}
			else
			{
				dest._tail->_next = newTail;
			}

			dest._tail = newTail;
			dest._head->_tail = newTail;
			dest._head->_length += DataLog_BufferSize;

			memcpy(dest._tail->_data, sourceBuffer->_data, DataLog_BufferSize);
			sourceBuffer = sourceBuffer->_next;
		}
		else
		{
			copyOK = false;
		}
	}

	if ( !copyOK )
	{
		//
		// Return any buffers allocated back to the free list
		//
		addChainToList(FreeList, dest);
		dest._head = NULL;
		dest._missedBytes = (source._head) ? source._head->_length : 1;
	}
	else
	{
		if ( dest._head != NULL )
		{
			dest._head->_length = (source._head) ? source._head->_length : 0;
		}
	}

	return copyOK;
}

bool DataLog_BufferManager::writeToChain(DataLog_BufferChain & chain, DataLog_BufferData * data, size_t size)
{
	bool writeOK = (chain._missedBytes == 0);
	size_t	bytesLeft = size;
	size_t	writePos = 0;

	while ( writeOK &&
			  bytesLeft > 0 )
	{
		size_t bytesAvail;
		if ( !chain._head )
		{
			bytesAvail = 0;
		}
		else
		{
			//
			// Since additional buffers are allocated only when needed, a result of
			// DataLog_BufferSize indicates that the tail buffer is full, and that
			// another buffer must be added to the chain.
			//
			bytesAvail = DataLog_BufferSize - (chain._head->_length % DataLog_BufferSize);
			if ( bytesAvail == DataLog_BufferSize ) bytesAvail = 0;
		}

		if ( bytesAvail == 0 )
		{
			DataLog_BufferPtr newTail = getFreeBuffer(chain._reserveBuffers);
			if ( newTail )
			{
				newTail->_next = newTail->_tail = NULL;
				newTail->_length = 0;

				if ( !chain._head )
				{
					chain._head = chain._tail = newTail;
				}
				else
				{
					chain._tail->_next = newTail;
					chain._tail = newTail;
				}

				chain._head->_tail = newTail;
			}
			else
			{
				writeOK = false;
			}
			
		}

		if ( writeOK )
		{
			size_t offset = (chain._head->_length % DataLog_BufferSize);
			size_t bytesToWrite = (bytesLeft > DataLog_BufferSize-offset) ? DataLog_BufferSize-offset : bytesLeft;

			memcpy(&(chain._tail->_data[offset]), &data[writePos], bytesToWrite);
			chain._head->_length += bytesToWrite;
			writePos += bytesToWrite;
			bytesLeft -= bytesToWrite;
		}
	}

	if ( !writeOK )
	{
		chain._missedBytes += bytesLeft;
	}

	return writeOK;
}

void DataLog_BufferManager::modifyChainData(DataLog_BufferChain & chain, unsigned long offset, DataLog_BufferData * data, size_t size)
{
	DataLog_BufferPtr buffer = chain._head;
	unsigned long	currentBufferPos = 0;

	while ( buffer &&
			  currentBufferPos+DataLog_BufferSize <= offset )
	{
		buffer = buffer->_next;
		currentBufferPos += DataLog_BufferSize;
	}

   size_t readPos = 0;
   int writePos = offset%DataLog_BufferSize;

	while ( buffer &&
			  readPos < size )
	{
		buffer->_data[writePos] = data[readPos];
		readPos += 1;
		writePos += 1;
		if ( writePos >= DataLog_BufferSize )
		{
			writePos -= DataLog_BufferSize;
			buffer = buffer->_next;
		}
	}
}

unsigned long datalog_GetFreeBufferBytes(void)
{
	return DataLog_BufferSize * DataLog_BufferManager::currentBufferCount(DataLog_BufferManager::FreeList);
}

unsigned long datalog_GetTraceBufferBytes(void)
{
	return DataLog_BufferSize * DataLog_BufferManager::currentBufferCount(DataLog_BufferManager::TraceList);
}

unsigned long datalog_GetCriticalBufferBytes(void)
{
	return DataLog_BufferSize * DataLog_BufferManager::currentBufferCount(DataLog_BufferManager::CriticalList);
}

unsigned long datalog_GetTraceWrittenBytes(void)
{
	return DataLog_BufferManager::bytesWritten(DataLog_BufferManager::TraceList);
}

unsigned long datalog_GetTraceMissedBytes(void)
{
	return DataLog_BufferManager::bytesMissed(DataLog_BufferManager::TraceList);
}

unsigned long datalog_GetCriticalWrittenBytes(void)
{
	return DataLog_BufferManager::bytesWritten(DataLog_BufferManager::CriticalList);
}

unsigned long datalog_GetCriticalMissedBytes(void)
{
	return DataLog_BufferManager::bytesMissed(DataLog_BufferManager::CriticalList);
}

#ifdef DATALOG_BUFFER_STATISTICS
unsigned long datalog_GetMinFreeBufferBytes(void)
{
	return DataLog_BufferSize * DataLog_BufferManager::minBufferCount(DataLog_BufferManager::FreeList);
}
#endif /* ifdef DATALOG_BUFFER_STATISTICS */

#ifdef DATALOG_BUFFER_STATISTICS
unsigned long datalog_GetMaxTraceBufferBytes(void)
{
	return DataLog_BufferSize * DataLog_BufferManager::maxBufferCount(DataLog_BufferManager::TraceList);
}
#endif /* ifdef DATALOG_BUFFER_STATISTICS */

#ifdef DATALOG_BUFFER_STATISTICS
unsigned long datalog_GetMaxCriticalBufferBytes(void)
{
	return DataLog_BufferSize * DataLog_BufferManager::maxBufferCount(DataLog_BufferManager::CriticalList);
}
#endif /* ifdef DATALOG_BUFFER_STATISTICS */

#ifdef DATALOG_BUFFER_STATISTICS
unsigned long datalog_GetAvgFreeBufferBytes(void)
{
	return DataLog_BufferSize * DataLog_BufferManager::avgBufferCount(DataLog_BufferManager::FreeList);
}
#endif /* ifdef DATALOG_BUFFER_STATISTICS */

#ifdef DATALOG_BUFFER_STATISTICS
unsigned long datalog_GetAvgTraceBufferBytes(void)
{
	return DataLog_BufferSize * DataLog_BufferManager::avgBufferCount(DataLog_BufferManager::TraceList);
}
#endif /* ifdef DATALOG_BUFFER_STATISTICS */

#ifdef DATALOG_BUFFER_STATISTICS
unsigned long datalog_GetAvgCriticalBufferBytes(void)
{
	return DataLog_BufferSize * DataLog_BufferManager::avgBufferCount(DataLog_BufferManager::CriticalList);
}
#endif /* ifdef DATALOG_BUFFER_STATISTICS */

