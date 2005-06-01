/*
 * Copyright (C) 2002 Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: //bctquad3/home/BCT_Development/vxWorks/Common/datalog/rcs/datalog_periodic.cpp 1.6 2005/05/31 20:26:46Z jheiusb Exp ms10234 $
 * $Log: datalog_periodic.cpp $
 * Revision 1.5  2004/10/26 20:19:14Z  rm70006
 * Ported datalog code to be compatible with windows compiler.  No functional changes made.  Re-ran unit test and it passed.
 * Revision 1.4  2003/10/03 12:35:09Z  jl11312
 * - improved DataLog_Handle lookup time
 * - modified datalog signal handling to eliminate requirement for a name lookup and the semaphore lock/unlock that went with it
 * Revision 1.3  2003/02/25 16:10:24Z  jl11312
 * - modified buffering scheme to help prevent buffer overruns
 * Revision 1.2  2003/01/31 19:52:51  jl11312
 * - new stream format for datalog
 * Revision 1.1  2002/08/15 21:20:57  jl11312
 * Initial revision
 *
 */

#include "datalog.h"
#include "datalog_internal.h"
#include "datalog_records.h"

DataLog_PeriodicTask::DataLog_PeriodicTask(DataLog_SetHandle set)
{
	_isExiting = false;
	_exitCode = 0;
	_set = set;
}

void DataLog_PeriodicTask::exit(int code)
{
	_exitCode = code;
	_isExiting = true;
}

int DataLog_PeriodicTask::main(void)
{
	DataLog_CommonData common;
	size_t	tempBufferSize = 0;
	DataLog_BufferData * tempBuffer = NULL;

	DataLog_List<DataLog_PeriodicItemBase *>::iterator itemsIter;

	while ( !_isExiting )
	{
		datalog_WaitSignal(_set->_writeSignal, -1);
		if ( datalog_WaitSignal(_set->_periodUpdateSignal, 0) )
		{
			datalog_SetupPeriodicSignal(_set->_writeSignal, _set->_logIntervalMilliSec);
		}

		datalog_LockAccess(_set->_outputLock,WAIT_FOREVER);
		datalog_LockAccess(_set->_lock,WAIT_FOREVER);
		itemsIter = _set->_items.begin();
		while ( itemsIter != _set->_items.end() )
		{
			(*itemsIter)->updateItem();
			++itemsIter;
		}

		if ( datalog_WaitSignal(_set->_modifiedSignal, 0) )
		{
			size_t	newSize = 0;

			itemsIter = _set->_items.begin();
			while ( itemsIter != _set->_items.end() )
			{
				//
				// Each item record requires the item size, key code, and item data
				//
				newSize += (*itemsIter)->currentAllocSize() + sizeof(DataLog_InternalID) + sizeof(DataLog_UINT16);
				++itemsIter;
			}

			//
			// Round newSize up to multiple of 256 and allocate new buffers if necessary
			//
			newSize = (newSize & ~0xff) + 256;
			if ( newSize > tempBufferSize )
			{
				if ( tempBuffer )
				{
					delete[] tempBuffer;
				}

				tempBuffer = new DataLog_BufferData[newSize];
				tempBufferSize = newSize;
		   }
		}

		size_t writeSize = 0;
		size_t itemCount = 0; 
		bool writeOK = true;
 
		itemsIter = _set->_items.begin();
		while ( itemsIter != _set->_items.end() && writeOK )
		{
			if ( (*itemsIter)->itemChanged() )
			{
				//
				// Account for space used by header information for item
				//
				DataLog_BufferData * header = &tempBuffer[writeSize];
				writeSize += sizeof(DataLog_InternalID) + sizeof(DataLog_UINT16);

				//
				// Write item data to buffer
				//
				DataLog_InternalID keyCode = (*itemsIter)->getKeyCode();
				DataLog_UINT16	itemReadSize = 0;
				if ( writeSize < tempBufferSize )
			   {
					itemReadSize = (*itemsIter)->getData(&tempBuffer[writeSize], tempBufferSize-writeSize);
					itemCount += 1;
			   }

				if ( itemReadSize != (*itemsIter)->currentSize() )
				{
					writeOK = false;
				}
				else
				{
					writeSize += itemReadSize;
					memcpy(header, &itemReadSize, sizeof(DataLog_UINT16));
					memcpy(&header[sizeof(DataLog_UINT16)], &keyCode, sizeof(DataLog_InternalID));
				}
			}

			++itemsIter;
		}

		datalog_ReleaseAccess(_set->_lock);
		datalog_ReleaseAccess(_set->_outputLock);

		if ( !writeOK )
		{
			DataLog_CommonData	common;
			common.setTaskError(DataLog_PeriodicWriteError, __FILE__, __LINE__);
		}
		else if ( writeSize > 0 )
		{
			DataLog_PeriodicOutputRecord	periodicOutputRecord;

			periodicOutputRecord._recordType = DataLog_PeriodicOutputRecordID;
			datalog_GetTimeStamp(&periodicOutputRecord._timeStamp);
			periodicOutputRecord._setID = _set->_id;

#ifdef DATALOG_NETWORK_SUPPORT
			periodicOutputRecord._nodeID = datalog_NodeID();
#endif /* ifdef DATALOG_NETWORK_SUPPORT */

			periodicOutputRecord._itemCount = itemCount;

			DataLog_BufferChain	outputChain;

			DataLog_BufferManager::createChain(outputChain, common.criticalReserveBuffers());
			DataLog_BufferManager::writeToChain(outputChain, (DataLog_BufferData *)&periodicOutputRecord, sizeof(periodicOutputRecord));
			DataLog_BufferManager::writeToChain(outputChain, (DataLog_BufferData *)tempBuffer, writeSize);
			DataLog_BufferManager::addChainToList(DataLog_BufferManager::TraceList, outputChain);
		}
	}

	return _exitCode;
}

DataLog_Result datalog_CreatePeriodicSet(const char * setName, DataLog_SetHandle * handle)
{
	DataLog_Result	result = DataLog_OK;
	DataLog_CommonData common;
	DataLog_SetInfo *	setInfo = new DataLog_SetInfo;

	setInfo->_id = common.getNextInternalID();
	setInfo->_logIntervalMilliSec = 0;

	size_t	setNameLen = strlen(setName);
	const char * writeSuffix = "_W";
	const char * modifiedSuffix = "_M";
	const char * periodUpdateSuffix = "_P";

   char * signalName = new char[setNameLen+strlen(writeSuffix)+1];
   strcpy(signalName, setName);
   strcat(signalName, writeSuffix);
	setInfo->_writeSignal = datalog_CreateSignal(signalName);

	signalName = new char[setNameLen+strlen(modifiedSuffix)+1];
   strcpy(signalName, setName);
   strcat(signalName, modifiedSuffix);
   setInfo->_modifiedSignal = datalog_CreateSignal(signalName);

	signalName = new char[setNameLen+strlen(modifiedSuffix)+1];
   strcpy(signalName, setName);
   strcat(signalName, periodUpdateSuffix);
   setInfo->_periodUpdateSignal = datalog_CreateSignal(signalName);

	setInfo->_lock = datalog_CreateMLock();
	setInfo->_outputLock = datalog_CreateMLock();

	datalog_StartPeriodicLogTask(setInfo);

	DataLog_PeriodicSetRecord	periodicSetRecord;
	periodicSetRecord._recordType = DataLog_PeriodicSetRecordID;
	datalog_GetTimeStamp(&periodicSetRecord._timeStamp);
	periodicSetRecord._setID = setInfo->_id;

#ifdef DATALOG_NETWORK_SUPPORT
	periodicSetRecord._nodeID = datalog_NodeID();
#endif /* ifdef DATALOG_NETWORK_SUPPORT */

	periodicSetRecord._nameLen = strlen(setName);

	DataLog_BufferChain	outputChain;
	DataLog_BufferManager::createChain(outputChain);

	DataLog_BufferManager::writeToChain(outputChain, (DataLog_BufferData *)&periodicSetRecord, sizeof(periodicSetRecord));
	DataLog_BufferManager::writeToChain(outputChain, (DataLog_BufferData *)setName, periodicSetRecord._nameLen * sizeof(char));
	DataLog_BufferManager::addChainToList(DataLog_BufferManager::CriticalList, outputChain);

	if ( outputChain._missedBytes > 0 )
	{
		common.setTaskError(DataLog_PeriodicSetRecordWriteFailed, __FILE__, __LINE__);
		result = DataLog_Error;
	}

	*handle = setInfo;
	return result;
}

DataLog_Result datalog_GetPeriodicOutputInterval(DataLog_SetHandle handle, long * milliSeconds)
{
	*milliSeconds = handle->_logIntervalMilliSec;
	return DataLog_OK;
}

DataLog_Result datalog_SetPeriodicOutputInterval(DataLog_SetHandle handle, long milliSeconds)
{
	handle->_logIntervalMilliSec = milliSeconds;
	datalog_SendSignal(handle->_periodUpdateSignal);
	datalog_SendSignal(handle->_writeSignal);
	
	return DataLog_OK;
}

DataLog_Result datalog_ForcePeriodicOutput(DataLog_SetHandle handle)
{
	datalog_SendSignal(handle->_periodUpdateSignal);
	datalog_SendSignal(handle->_writeSignal);

	return DataLog_OK;
}

DataLog_Result datalog_DisablePeriodicOutput(DataLog_SetHandle handle)
{
	datalog_LockAccess(handle->_outputLock,WAIT_FOREVER);
	return DataLog_OK;
}

DataLog_Result datalog_EnablePeriodicOutput(DataLog_SetHandle handle)
{
	datalog_ReleaseAccess(handle->_outputLock);
	return DataLog_OK;
}

DataLog_PeriodicItemBase::DataLog_PeriodicItemBase(DataLog_SetHandle set, size_t size, const char * key, const char * description, const char * format)
{
	_data = malloc(size);
	memset(_data, 0, size);
	_size = 0;
	_allocSize = size;

	_oldData = NULL;
	_oldSize = 0;
	_oldAllocSize = 0;

	_set = set;
	_lock = datalog_CreateMLock();

	DataLog_CommonData common;
	_keyCode = common.getNextInternalID();

	datalog_LockAccess(set->_lock,WAIT_FOREVER);
	set->_items.push_back(this);
	datalog_ReleaseAccess(set->_lock);
	datalog_SendSignal(set->_modifiedSignal);

	DataLog_PeriodicItemRecord periodicItemRecord;
	periodicItemRecord._recordType = DataLog_PeriodicItemRecordID;
	datalog_GetTimeStamp(&periodicItemRecord._timeStamp);
	periodicItemRecord._keyCode = _keyCode;

#ifdef DATALOG_NETWORK_SUPPORT
	periodicItemRecord._nodeID = datalog_NodeID();
#endif /* ifdef DATALOG_NETWORK_SUPPORT */

	periodicItemRecord._keyLen = strlen(key);
	periodicItemRecord._descLen = strlen(description);
	periodicItemRecord._formatLen = strlen(format);

	DataLog_BufferChain	outputChain;
	DataLog_BufferManager::createChain(outputChain);

	DataLog_BufferManager::writeToChain(outputChain, (DataLog_BufferData *)&periodicItemRecord, sizeof(periodicItemRecord));
	DataLog_BufferManager::writeToChain(outputChain, (DataLog_BufferData *)key, periodicItemRecord._keyLen * sizeof(char));
	DataLog_BufferManager::writeToChain(outputChain, (DataLog_BufferData *)description, periodicItemRecord._descLen * sizeof(char));
	DataLog_BufferManager::writeToChain(outputChain, (DataLog_BufferData *)format, periodicItemRecord._formatLen * sizeof(char));
	DataLog_BufferManager::addChainToList(DataLog_BufferManager::CriticalList, outputChain);

	if ( outputChain._missedBytes > 0 )
	{
		common.setTaskError(DataLog_PeriodicItemRecordWriteFailed, __FILE__, __LINE__);
	}
}

DataLog_PeriodicItemBase::~DataLog_PeriodicItemBase()
{
	if ( _data )
	{
		free(_data);
		_data = NULL;
		_allocSize = 0;
	}

	if ( _oldData )
	{
		free(_oldData);
		_oldData = NULL;
		_oldAllocSize = 0;
	}
}

bool DataLog_PeriodicItemBase::itemChanged(void)
{
	bool	changed = false;

	datalog_LockAccess(_lock,WAIT_FOREVER);
	if ( !_oldData ||
		  _size != _oldSize ||
		  memcmp(_data, _oldData, _size) != 0 )
	{
		changed = true;
	}

	if ( changed )
	{
		if ( _oldAllocSize < _size )
		{
			if ( _oldData ) free(_oldData);

			_oldData = malloc(_allocSize);
			_oldAllocSize = _allocSize;
		}

		_oldSize = _size;
		memcpy(_oldData, _data, _size);
	}

	datalog_ReleaseAccess(_lock);
	return changed;
}

void DataLog_PeriodicItemBase::copyItemData(size_t size, const void * data)
{
	if ( size > _allocSize )
	{
		free(_data);
		_allocSize = size+BUFFER_SIZE_INC;
		_data = malloc(_allocSize);

		datalog_SendSignal(_set->_modifiedSignal);
	}

	_size = size;
	memcpy(_data, data, size);
}

void DataLog_PeriodicItemBase::setItemData(size_t size, const void * data)
{
	datalog_LockAccess(_lock,WAIT_FOREVER);
	copyItemData(size, data);
	datalog_ReleaseAccess(_lock);
}

void DataLog_PeriodicItemBase::setItemString(const char * str)
{
	datalog_LockAccess(_lock,WAIT_FOREVER);
	if ( !str )
	{
		copyItemData(1, "\0");
	}
	else
	{
		copyItemData(strlen(str), (const void *)str);
	}

	datalog_ReleaseAccess(_lock);
}

size_t DataLog_PeriodicItemBase::getData(void * buffer, size_t maxSize)
{
	size_t	result = 0;

	if ( _oldSize <= maxSize )
	{
		datalog_LockAccess(_lock,WAIT_FOREVER);
		memcpy(buffer, _oldData, _oldSize);
		result = _oldSize;
		datalog_ReleaseAccess(_lock);
	}

	return result;
}

