/*
 * Copyright (C) 2002 Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: //bctquad3/home/BCT_Development/vxWorks/Common/datalog/rcs/datalog_periodic.cpp 1.6 2005/05/31 20:26:46Z jheiusb Exp ms10234 $
 * $Log: datalog_periodic.cpp $
 * Revision 1.1  2002/08/15 21:20:57  jl11312
 * Initial revision
 *
 */

#include "datalog.h"
#include "datalog_internal.h"

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
	size_t	tempBufferSize = 0;
	DataLog_BufferData * tempBuffer = NULL;

	DataLog_List<DataLog_PeriodicItemBase *>::iterator itemsIter;
	DataLog_PeriodicBuffer * logBuffer = NULL;

	while ( !_isExiting )
	{
		datalog_WaitSignal(_set->_writeSignalName, -1);
		if ( datalog_WaitSignal(_set->_periodUpdateSignalName, 0) )
		{
			datalog_SetupPeriodicSignal(_set->_writeSignalName, _set->_logInterval);
		}

		datalog_LockAccess(_set->_outputLock);
		datalog_LockAccess(_set->_lock);
		itemsIter = _set->_items.begin();
		while ( itemsIter != _set->_items.end() )
		{
			(*itemsIter)->updateItem();
			++itemsIter;
		}

		if ( datalog_WaitSignal(_set->_modifiedSignalName, 0) )
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
			if ( newSize > tempBufferSize || !logBuffer )
			{
				if ( tempBuffer )
				{
					delete[] tempBuffer;
				}

				if ( logBuffer )
				{
					delete logBuffer;
				}

				tempBuffer = new DataLog_BufferData[newSize];
				tempBufferSize = newSize;

				logBuffer = new DataLog_PeriodicBuffer(tempBufferSize);
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
			periodicOutputRecord._setID = _set->_id;;

#ifndef DATALOG_NO_NETWORK_SUPPORT
			periodicOutputRecord._nodeID = datalog_NodeID();
#endif /* ifndef DATALOG_NO_NETWORK_SUPPORT */

			periodicOutputRecord._itemCount = itemCount;
			
			logBuffer->partialWrite((DataLog_BufferData *)&periodicOutputRecord, sizeof(periodicOutputRecord));
			logBuffer->partialWrite((DataLog_BufferData *)tempBuffer, writeSize);
			logBuffer->partialWriteComplete();
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
	setInfo->_logInterval = 0;

	size_t	setNameLen = strlen(setName);
	const char * writeSuffix = "_W";
	const char * modifiedSuffix = "_M";
	const char * periodUpdateSuffix = "_P";

   char * signalName = new char[setNameLen+strlen(writeSuffix)+1];
   strcpy(signalName, setName);
   strcat(signalName, writeSuffix);
	setInfo->_writeSignalName = signalName;

	signalName = new char[setNameLen+strlen(modifiedSuffix)+1];
   strcpy(signalName, setName);
   strcat(signalName, modifiedSuffix);
   setInfo->_modifiedSignalName = signalName;

	signalName = new char[setNameLen+strlen(modifiedSuffix)+1];
   strcpy(signalName, setName);
   strcat(signalName, periodUpdateSuffix);
   setInfo->_periodUpdateSignalName = signalName;

	setInfo->_lock = datalog_CreateLock();
	setInfo->_outputLock = datalog_CreateLock();
	
	datalog_StartPeriodicLogTask(setInfo);

	DataLog_PeriodicSetRecord	periodicSetRecord;
	periodicSetRecord._recordType = DataLog_PeriodicSetRecordID;
	datalog_GetTimeStamp(&periodicSetRecord._timeStamp);
	periodicSetRecord._setID = setInfo->_id;

#ifndef DATALOG_NO_NETWORK_SUPPORT
	periodicSetRecord._nodeID = datalog_NodeID();
#endif /* ifndef DATALOG_NO_NETWORK_SUPPORT */

	periodicSetRecord._nameLen = strlen(setName);

	DataLog_CriticalBuffer * buffer = common.getTaskCriticalBuffer(DATALOG_CURRENT_TASK);

	buffer->partialWrite((DataLog_BufferData *)&periodicSetRecord, sizeof(periodicSetRecord));
	buffer->partialWrite((DataLog_BufferData *)setName, periodicSetRecord._nameLen * sizeof(char));
	size_t writeSize = buffer->partialWriteComplete();

	if ( writeSize != sizeof(periodicSetRecord) + periodicSetRecord._nameLen * sizeof(char) )
	{
		common.setTaskError(DataLog_PeriodicSetRecordWriteFailed, __FILE__, __LINE__);
		result = DataLog_Error;
	}

	*handle = setInfo;
	return result;
}

DataLog_Result datalog_GetPeriodicOutputInterval(DataLog_SetHandle handle, double * seconds)
{
	*seconds = handle->_logInterval;
	return DataLog_OK;
}

DataLog_Result datalog_SetPeriodicOutputInterval(DataLog_SetHandle handle, double seconds)
{
	handle->_logInterval = seconds;
	datalog_SendSignal(handle->_periodUpdateSignalName);
	datalog_SendSignal(handle->_writeSignalName);
	
	return DataLog_OK;
}

DataLog_Result datalog_ForcePeriodicOutput(DataLog_SetHandle handle)
{
	datalog_SendSignal(handle->_periodUpdateSignalName);
	datalog_SendSignal(handle->_writeSignalName);

	return DataLog_OK;
}

DataLog_Result datalog_DisablePeriodicOutput(DataLog_SetHandle handle)
{
	datalog_LockAccess(handle->_outputLock);
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
	_lock = datalog_CreateLock();

	DataLog_CommonData common;
	_keyCode = common.getNextInternalID();

	datalog_LockAccess(set->_lock);
	set->_items.push_back(this);
	datalog_ReleaseAccess(set->_lock);
	datalog_SendSignal(set->_modifiedSignalName);

	DataLog_PeriodicItemRecord periodicItemRecord;
	periodicItemRecord._recordType = DataLog_PeriodicItemRecordID;
	datalog_GetTimeStamp(&periodicItemRecord._timeStamp);
	periodicItemRecord._keyCode = _keyCode;

#ifndef DATALOG_NO_NETWORK_SUPPORT
	periodicItemRecord._nodeID = datalog_NodeID();
#endif /* ifndef DATALOG_NO_NETWORK_SUPPORT */

	periodicItemRecord._keyLen = strlen(key);
	periodicItemRecord._descLen = strlen(description);
	periodicItemRecord._formatLen = strlen(format);

	DataLog_CriticalBuffer * buffer = common.getTaskCriticalBuffer(DATALOG_CURRENT_TASK);

	buffer->partialWrite((DataLog_BufferData *)&periodicItemRecord, sizeof(periodicItemRecord));
	buffer->partialWrite((DataLog_BufferData *)key, periodicItemRecord._keyLen * sizeof(char));
	buffer->partialWrite((DataLog_BufferData *)description, periodicItemRecord._descLen * sizeof(char));
	buffer->partialWrite((DataLog_BufferData *)format, periodicItemRecord._formatLen * sizeof(char));
	size_t writeSize = buffer->partialWriteComplete();

	if ( writeSize != sizeof(periodicItemRecord) +
                       periodicItemRecord._keyLen * sizeof(char) +
                       periodicItemRecord._descLen * sizeof(char) +
							  periodicItemRecord._formatLen * sizeof(char) )
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

	datalog_LockAccess(_lock);
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

		datalog_SendSignal(_set->_modifiedSignalName);
	}

	_size = size;
	memcpy(_data, data, size);
}

void DataLog_PeriodicItemBase::setItemData(size_t size, const void * data)
{
	datalog_LockAccess(_lock);
	copyItemData(size, data);
	datalog_ReleaseAccess(_lock);
}

void DataLog_PeriodicItemBase::setItemString(const char * str)
{
	datalog_LockAccess(_lock);
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
		datalog_LockAccess(_lock);
		memcpy(buffer, _oldData, _oldSize);
		result = _oldSize;
		datalog_ReleaseAccess(_lock);
	}

	return result;
}

