/*
 * Copyright (C) 2002 Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: K:/BCT_Development/vxWorks/Common/datalog/rcs/datalog_port_vxworks.cpp 1.9 2003/10/16 14:57:40Z jl11312 Exp jl11312 $
 * $Log: datalog_port_vxworks.cpp $
 * Revision 1.5  2003/02/25 16:10:26Z  jl11312
 * - modified buffering scheme to help prevent buffer overruns
 * Revision 1.4  2003/01/10 14:21:46  jl11312
 * - corrected conditional compile problem for simulator
 * Revision 1.3  2003/01/08 15:37:48  jl11312
 * - corrected timestamp operation
 * Revision 1.2  2002/08/15 20:53:58  jl11312
 * - added support for periodic logging
 * Revision 1.1  2002/07/18 21:20:52  jl11312
 * Initial revision
 *
 */

#include "datalog.h"

#include <intLib.h>
#include <taskHookLib.h>
#include <string>
#include <sysLib.h>
#include <timers.h>
#include "datalog_internal.h"
#include "error.h"

#ifdef DATALOG_LEVELS_INIT_SUPPORT
# include "datalog_levels.h"
#endif /* ifdef DATALOG_LEVELS_INIT_SUPPORT */


//
// Local functions
//
struct SignalInfo;
static SignalInfo * getSignalInfo(const char * signalName);
static void timerNotify(timer_t timerID, SignalInfo * signalInfo);
static time_t markTimeStampStart(void);

//
// Initialization related functions
//
static SEM_ID	initializationDataLock = semBCreate(SEM_Q_PRIORITY, SEM_FULL);
static bool initializationStarted;

bool DataLog_CommonData::startInitialization(void)
{
	bool result = false;

	semTake(initializationDataLock, WAIT_FOREVER);
	if ( !initializationStarted )
	{
		initializationStarted = true;
		result = true;
	}

	semGive(initializationDataLock);
	return result;
}

bool DataLog_CommonData::isInitialized(void)
{
	semTake(initializationDataLock, WAIT_FOREVER);
	bool result = initializationStarted;
	semGive(initializationDataLock);

	return result;
}

//
// Internal ID related functions
//
static SEM_ID	internalIDLock = semBCreate(SEM_Q_PRIORITY, SEM_FULL);

DataLog_InternalID DataLog_CommonData::getNextInternalID(void)
{
	static DataLog_InternalID	id = DATALOG_NULL_ID;

	semTake(internalIDLock, WAIT_FOREVER);

	id += 1;
	DataLog_InternalID	result = id;
	semGive(internalIDLock);

	return result;	
}

//
// Level ID related functions
//
static SEM_ID	levelIDLock = semBCreate(SEM_Q_PRIORITY, SEM_FULL);
static map<string, DataLog_InternalID>	levelIDMap;

DataLog_InternalID DataLog_CommonData::lookupLevelID(const char * levelName)
{
	DataLog_InternalID result = DATALOG_NULL_ID;

	semTake(levelIDLock, WAIT_FOREVER);
	if ( levelIDMap.find(levelName) != levelIDMap.end() )
	{
		result = levelIDMap[levelName];
	}

	semGive(levelIDLock);
	return result;
}

void DataLog_CommonData::registerLevelID(const char * levelName, DataLog_InternalID id)
{
	semTake(levelIDLock, WAIT_FOREVER);
	if ( levelIDMap.find(levelName) != levelIDMap.end() )
	{
#ifdef DATALOG_LEVELS_INIT_SUPPORT
		DataLog(log_level_datalog_error)
#else /* ifdef DATALOG_LEVELS_INIT_SUPPORT */
		DataLog_Default
#endif /* ifdef DATALOG_LEVELS_INIT_SUPPORT */
			<< "attempt to register duplicate level name ignored" << endmsg;
	}
	else
	{
		levelIDMap[levelName] = id;
	}

	semGive(levelIDLock);
}

//
// Common data area related functions
//
static SEM_ID	commonDataLock = semBCreate(SEM_Q_PRIORITY, SEM_FULL);

void DataLog_CommonData::setCommonDataPtr(void)
{
	static DataLog_SharedPtr(CommonData)	commonData = DATALOG_NULL_SHARED_PTR;

	//
	// If commonData has been set, it will never change so we don't
	// need to acquire the semaphore.  Otherwise, we need to acquire
	// the semaphore and allocate the common data area.
	//
	if ( commonData == DATALOG_NULL_SHARED_PTR )
	{
		semTake(commonDataLock, WAIT_FOREVER);
		if ( commonData == DATALOG_NULL_SHARED_PTR )
		{
			commonData = (DataLog_SharedPtr(CommonData))datalog_AllocSharedMem(sizeof(CommonData));
			initializeCommonData(commonData);

			taskCreateHookAdd((FUNCPTR)datalog_TaskCreated);
			taskDeleteHookAdd((FUNCPTR)datalog_TaskDeleted);
	   }

		semGive(commonDataLock);
	}

	_commonData = commonData;
}

//
// Signal related functions
//
struct SignalInfo
{
	SEM_ID	_semID;

	bool		_timerCreated;
	timer_t 	_timerID;
};

static SEM_ID	signalDataLock = semBCreate(SEM_Q_PRIORITY, SEM_FULL);
static map<string, SignalInfo *>	signalMap;

static SignalInfo * getSignalInfo(const char * signalName)
{
	//
	//	Create signal if necessary
	//
	SignalInfo *	result;

	semTake(signalDataLock, WAIT_FOREVER);
	if ( signalMap.find(signalName) == signalMap.end() )
	{
		result = new SignalInfo;
		result->_semID = semBCreate(SEM_Q_PRIORITY, SEM_EMPTY);
		result->_timerCreated = false;
		signalMap[signalName] = result;
	}

	result = signalMap[signalName];
	semGive(signalDataLock);
	return result;
}

static void timerNotify(timer_t timerID, SignalInfo * signalInfo)
{
	semGive(signalInfo->_semID);
}

bool datalog_WaitSignal(const char * signalName, long milliSeconds)
{
	SignalInfo * signalInfo = getSignalInfo(signalName);
	int timeout = WAIT_FOREVER;

	if (milliSeconds >= 0)
	{
		timeout = milliSeconds * sysClkRateGet() / 1000;
	} 

	STATUS result = semTake(signalInfo->_semID, timeout);
	return (result == OK) ? true : false;
}

void datalog_SendSignal(const char * signalName)
{
	SignalInfo * signalInfo = getSignalInfo(signalName);
	semGive(signalInfo->_semID);
}

void datalog_SetupPeriodicSignal(const char * signalName, long milliSeconds)
{
	SignalInfo * signalInfo = getSignalInfo(signalName);
	if ( !signalInfo->_timerCreated )
	{
		if ( timer_create(CLOCK_REALTIME, NULL, &(signalInfo->_timerID)) != OK )
		{
			_FATAL_ERROR(__FILE__, __LINE__, "timerCreate failed");
		}

		timer_connect(signalInfo->_timerID, (VOIDFUNCPTR)timerNotify, (int)signalInfo);
		signalInfo->_timerCreated = true;
	}

	timer_cancel(signalInfo->_timerID);
	if ( milliSeconds > 0 )
	{
		struct itimerspec	period;

		period.it_value.tv_sec = period.it_interval.tv_sec = (time_t)(milliSeconds/1000);
		period.it_value.tv_nsec = period.it_interval.tv_nsec = (long)((milliSeconds%1000) * 1e6);
		timer_settime(signalInfo->_timerID, 0, &period, NULL);
	}
}

//
// Time stamp related functions
//
static SEM_ID 	timeDataLock = semBCreate(SEM_Q_PRIORITY, SEM_FULL);

#if (CPU != SIMNT)
static long			nanoSecPerTimestampTick;
#endif

static struct timespec	clockStart;
static time_t				timeStart = markTimeStampStart();

static time_t markTimeStampStart(void)
{
	int	oldLevel;

#if (CPU != SIMNT)
	//
	// Make sure time stamp clock is enabled and period set correctly (for the
	// 8253 driver - sysTimestampPeriod() also sets an internal period value
	// needed for proper operation).
	//
	sysTimestampEnable();
	sysTimestampPeriod();
#endif /* if (CPU != SIMNT) */

	//
	// Save start time for real-time clock.  All timestamp information in the
	// log file is relative to this start time.
	//
	semTake(timeDataLock, WAIT_FOREVER);
	clock_gettime(CLOCK_REALTIME, &clockStart);

#if (CPU != SIMNT)
	nanoSecPerTimestampTick = 1000000000/sysTimestampFreq();
#endif /* if (CPU != SIMNT) */

	semGive(timeDataLock);

	return time(NULL);
}

void datalog_GetTimeStampStart(DataLog_TimeStampStart * start)
{
	struct tm	tmVal;
	gmtime_r(&timeStart, &tmVal);

	start->_day = tmVal.tm_mday;
	start->_month = tmVal.tm_mon + 1;
	start->_year = tmVal.tm_year + 1900;
	start->_hour = tmVal.tm_hour;
	start->_minute = tmVal.tm_min;
	start->_second = tmVal.tm_sec;
}

void datalog_GetTimeStamp(DataLog_TimeStamp * stamp)
{
	struct timespec	clockCurrent;

#if (CPU != SIMNT)
	int oldLevel;
	UINT32 timestampCurrent;

	oldLevel = intLock();
	timestampCurrent = sysTimestamp();
#endif /* if (CPU != SIMNT) */

	clock_gettime(CLOCK_REALTIME, &clockCurrent);

#if (CPU != SIMNT)
	intUnlock(oldLevel);
#endif /* if (CPU != SIMNT) */

	long		seconds;
	long		nanoseconds;

	seconds = clockCurrent.tv_sec - clockStart.tv_sec;
	nanoseconds = clockCurrent.tv_nsec - clockStart.tv_nsec;

#if (CPU != SIMNT)
	nanoseconds += timestampCurrent * nanoSecPerTimestampTick;
#endif /* if (CPU != SIMNT) */

	if ( nanoseconds < 0 )
	{
		nanoseconds += 1000000000;
		seconds -= 1;
	}

	stamp->_seconds = (DataLog_UINT32)seconds;
	stamp->_nanoseconds = (DataLog_UINT32)nanoseconds;
}

DataLog_TaskID datalog_CurrentTask(void)
{
	return taskIdSelf();
}

const char * datalog_TaskName(DataLog_TaskID taskID)
{
	return taskName((taskID == DATALOG_CURRENT_TASK) ? taskIdSelf() : taskID);
}

DataLog_SharedPtr(void) datalog_AllocSharedMem(size_t size)
{
	return malloc(size);
}

void datalog_FreeSharedMem(DataLog_SharedPtr(void) ptr)
{
	free(ptr);
}

DataLog_Lock datalog_CreateLock(void)
{
	return semBCreate(SEM_Q_PRIORITY, SEM_FULL);
}

void datalog_DeleteLock(DataLog_Lock lock)
{
	semDelete(lock);  
}

void datalog_LockAccess(DataLog_Lock lock)
{
	semTake(lock, WAIT_FOREVER);
}

void datalog_ReleaseAccess(DataLog_Lock lock)
{
	semGive(lock);
}

//
// Buffer manager
//
struct DataLog_BufferList
{
	DataLog_BufferPtr _head;
	DataLog_BufferPtr _tail;

	volatile unsigned long _currentBufferCount;
	volatile unsigned long _bytesWritten;
	volatile unsigned long _bytesMissed;

#ifdef DATALOG_BUFFER_STATISTICS
	volatile unsigned long _minBufferCount;
	volatile unsigned long _maxBufferCount;

	volatile unsigned long _sumBufferCountSamples;
	volatile unsigned long _numBufferCountSamples;
#endif /* ifdef DATALOG_BUFFER_STATISTICS */
};

static DataLog_BufferList	traceList;
static DataLog_BufferList	criticalList;
static DataLog_BufferList	freeList;

void DataLog_BufferManager::initialize(size_t bufferSizeKBytes)
{
	unsigned long	bufferCount = (bufferSizeKBytes*1024+DataLog_BufferSize-1)/DataLog_BufferSize;
	DataLog_Buffer	* buffer = new DataLog_Buffer[bufferCount];

	freeList._head = &buffer[0];
	freeList._tail = &buffer[bufferCount-1];
	freeList._currentBufferCount = bufferCount;
	freeList._bytesMissed = freeList._bytesWritten = 0;

#ifdef DATALOG_BUFFER_STATISTICS
	freeList._minBufferCount = freeList._maxBufferCount = bufferCount;
	freeList._sumBufferCountSamples = freeList._numBufferCountSamples = 0;
#endif /* ifdef DATALOG_BUFFER_STATISTICS */

	for ( unsigned long i=0; i<bufferCount; i++ )
	{
		buffer[i]._next = (i<bufferCount-1) ? &buffer[i+1] : NULL;
		buffer[i]._length = 0;
	}

	traceList._head = traceList._tail = NULL;
	traceList._currentBufferCount = 0;
	traceList._bytesMissed = traceList._bytesWritten = 0;

#ifdef DATALOG_BUFFER_STATISTICS
	traceList._minBufferCount = traceList._maxBufferCount = 0;
	traceList._sumBufferCountSamples = traceList._numBufferCountSamples = 0;
#endif /* ifdef DATALOG_BUFFER_STATISTICS */

	criticalList._head = criticalList._tail = NULL;
	criticalList._currentBufferCount = 0;
	criticalList._bytesMissed = criticalList._bytesWritten = 0;

#ifdef DATALOG_BUFFER_STATISTICS
	criticalList._minBufferCount = criticalList._maxBufferCount = 0;
	criticalList._sumBufferCountSamples = criticalList._numBufferCountSamples = 0;
#endif /* ifdef DATALOG_BUFFER_STATISTICS */

}

DataLog_BufferPtr DataLog_BufferManager::getFreeBuffer(unsigned long reserveBuffers)
{
	DataLog_BufferPtr result = NULL;

	int level = intLock();
	if ( freeList._currentBufferCount > reserveBuffers )
	{
		result = freeList._head;
		freeList._head = freeList._head->_next;
		freeList._currentBufferCount -= 1;
	}

#ifdef DATALOG_BUFFER_STATISTICS
	if ( freeList._currentBufferCount < freeList._minBufferCount ) freeList._minBufferCount = freeList._currentBufferCount;
	freeList._numBufferCountSamples += 1;
	freeList._sumBufferCountSamples += freeList._currentBufferCount;
#endif /* ifdef DATALOG_BUFFER_STATISTICS */

	intUnlock(level);
	return result;
}

bool DataLog_BufferManager::getNextChain(DataLog_BufferChain & chain)
{
	int level = intLock();

	DataLog_BufferList * list = (criticalList._head != NULL) ? &criticalList : &traceList;
	chain._head = list->_head;
	if ( chain._head )
	{
		int	chainBufferCount = (chain._head->_length+DataLog_BufferSize-1)/DataLog_BufferSize;
		chain._tail = chain._head->_tail;

		list->_head = chain._tail->_next;
		chain._tail->_next = NULL;
		if ( !list->_head )
		{
			list->_tail = NULL;
		}

		list->_currentBufferCount -= chainBufferCount;

#ifdef DATALOG_BUFFER_STATISTICS
		list->_numBufferCountSamples += 1;
		list->_sumBufferCountSamples += list->_currentBufferCount;
#endif /* ifdef DATALOG_BUFFER_STATISTICS */

   }

	intUnlock(level);
	return (chain._head != NULL);
}

inline DataLog_BufferList * getListPtr(DataLog_BufferManager::BufferList list)
{
	DataLog_BufferList * bufferList = NULL;

	switch ( list )
	{
	case DataLog_BufferManager::TraceList: 	bufferList = &traceList; break;
	case DataLog_BufferManager::CriticalList: bufferList = &criticalList; break;
	case DataLog_BufferManager::FreeList: 		bufferList = &freeList; break;
	default:												_FATAL_ERROR(__FILE__, __LINE__, "invalid buffer list");
	}

	return bufferList;
}

void DataLog_BufferManager::addChainToList(DataLog_BufferManager::BufferList list, const DataLog_BufferChain & chain)
{
	if ( chain._head != NULL)
	{
		DataLog_BufferList * bufferList = getListPtr(list);
		bool signalNeeded = false;

		if ( chain._missedBytes != 0 && list != FreeList )
		{
			//
			// Log data was lost while writing to this buffer chain.  Update the bytes
			// missed for the specified buffer list, then return the chain to the free
			// list instead.
			//
			int level = intLock();
			bufferList->_bytesMissed += chain._missedBytes + chain._head->_length;
			intUnlock(level);

			bufferList = &freeList;
			datalog_SendSignal("DataLog_DataLost");
		}

		int level = intLock();
		if ( bufferList->_tail )
		{
			//
			// Add buffer to end of existing linked list
			//
			bufferList->_tail->_next = chain._head;
		}

		bufferList->_tail = chain._tail;
		if ( !bufferList->_head )
		{
			bufferList->_head = chain._head;
			signalNeeded = true;
		}

		int	chainBufferCount = (chain._head->_length+DataLog_BufferSize-1)/DataLog_BufferSize;
		bufferList->_currentBufferCount += chainBufferCount;

		if ( bufferList != &freeList )
		{
			bufferList->_bytesWritten += chain._head->_length;
		}

#ifdef DATALOG_BUFFER_STATISTICS
		if ( bufferList->_currentBufferCount > bufferList->_maxBufferCount ) bufferList->_maxBufferCount = bufferList->_currentBufferCount;
		bufferList->_numBufferCountSamples += 1;
		bufferList->_sumBufferCountSamples += bufferList->_currentBufferCount;
#endif /* ifdef DATALOG_BUFFER_STATISTICS */

		intUnlock(level); 

		if ( signalNeeded && list != FreeList )
		{
			datalog_SendSignal("DataLog_Output");
	   }
	}
}

unsigned long DataLog_BufferManager::currentBufferCount(DataLog_BufferManager::BufferList list)
{
	DataLog_BufferList * bufferList = getListPtr(list);
	unsigned long result = bufferList->_currentBufferCount;

	while ( result != bufferList->_currentBufferCount )
	{
		result = bufferList->_currentBufferCount;
	}

	return result;
}

unsigned long DataLog_BufferManager::bytesWritten(DataLog_BufferManager::BufferList list)
{
	DataLog_BufferList * bufferList = getListPtr(list);
	unsigned long result = bufferList->_bytesWritten;

	while ( result != bufferList->_bytesWritten )
	{
		result = bufferList->_bytesWritten;
	}

	return result;
}

unsigned long DataLog_BufferManager::bytesMissed(DataLog_BufferManager::BufferList list)
{
	DataLog_BufferList * bufferList = getListPtr(list);
	unsigned long result = bufferList->_bytesMissed;

	while ( result != bufferList->_bytesMissed )
	{
		result = bufferList->_bytesMissed;
	}

	return result;
}

#ifdef DATALOG_BUFFER_STATISTICS
unsigned long DataLog_BufferManager::minBufferCount(DataLog_BufferManager::BufferList list)
{
	DataLog_BufferList * bufferList = getListPtr(list);
	unsigned long result = bufferList->_minBufferCount;

	while ( result != bufferList->_minBufferCount )
	{
		result = bufferList->_minBufferCount;
	}

	return result;
}
#endif /* ifdef DATALOG_BUFFER_STATISTICS */

#ifdef DATALOG_BUFFER_STATISTICS
unsigned long DataLog_BufferManager::maxBufferCount(DataLog_BufferManager::BufferList list)
{
	DataLog_BufferList * bufferList = getListPtr(list);
	unsigned long result = bufferList->_maxBufferCount;

	while ( result != bufferList->_maxBufferCount )
	{
		result = bufferList->_maxBufferCount;
	}

	return result;
}
#endif /* ifdef DATALOG_BUFFER_STATISTICS */

#ifdef DATALOG_BUFFER_STATISTICS
unsigned long DataLog_BufferManager::avgBufferCount(DataLog_BufferManager::BufferList list)
{
	DataLog_BufferList * bufferList = getListPtr(list);

	unsigned long sumBufferCountSamples = bufferList->_sumBufferCountSamples;
	unsigned long numBufferCountSamples = bufferList->_numBufferCountSamples;

	while ( sumBufferCountSamples != bufferList->_sumBufferCountSamples ||
			  numBufferCountSamples != bufferList->_numBufferCountSamples )
	{
		sumBufferCountSamples = bufferList->_sumBufferCountSamples;
		numBufferCountSamples = bufferList->_numBufferCountSamples;
	}

	return sumBufferCountSamples/numBufferCountSamples;
}
#endif /* ifdef DATALOG_BUFFER_STATISTICS */

