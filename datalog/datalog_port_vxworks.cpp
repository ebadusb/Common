/*
 * Copyright (C) 2002 Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: K:/BCT_Development/vxWorks/Common/datalog/rcs/datalog_port_vxworks.cpp 1.9 2003/10/16 14:57:40Z jl11312 Exp jl11312 $
 * $Log: datalog_port_vxworks.cpp $
 * Revision 1.3  2003/01/08 15:37:48  jl11312
 * - corrected timestamp operation
 * Revision 1.2  2002/08/15 20:53:58  jl11312
 * - added support for periodic logging
 * Revision 1.1  2002/07/18 21:20:52  jl11312
 * Initial revision
 *
 */

#include <vxWorks.h>
#include <intLib.h>
#include <taskHookLib.h>
#include <string>
#include <sysLib.h>
#include <timers.h>

#include "datalog.h"
#include "datalog_internal.h"
#include "error.h"

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
		DataLog_Critical	errorLog;
		DataLog(errorLog) << "attempt to register duplicate level name ignored" << endmsg;
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
// Buffer list related functions
//
static SEM_ID	bufferListDataLock = semBCreate(SEM_Q_PRIORITY, SEM_FULL);
static DataLog_SharedPtr(DataLog_Buffer::SharedBufferData) bufferListHead = DATALOG_NULL_SHARED_PTR;
static DataLog_SharedPtr(DataLog_Buffer::SharedBufferData) bufferListTail = DATALOG_NULL_SHARED_PTR;

void DataLog_Buffer::lockBufferList(void)
{
	semTake(bufferListDataLock, WAIT_FOREVER);
}

void DataLog_Buffer::releaseBufferList(void)
{
	semGive(bufferListDataLock);
}

DataLog_SharedPtr(DataLog_Buffer::SharedBufferData) DataLog_Buffer::getBufferListHead(void)
{
	return bufferListHead;
}

void DataLog_Buffer::setBufferListHead(DataLog_SharedPtr(DataLog_Buffer::SharedBufferData) head)
{
	bufferListHead = head;
}

DataLog_SharedPtr(DataLog_Buffer::SharedBufferData) DataLog_Buffer::getBufferListTail(void)
{
	return bufferListTail;
}

void DataLog_Buffer::setBufferListTail(DataLog_SharedPtr(DataLog_Buffer::SharedBufferData) tail)
{
	bufferListTail = tail;
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

bool datalog_WaitSignal(const char * signalName, double seconds)
{
	SignalInfo * signalInfo = getSignalInfo(signalName);
	STATUS result = semTake(signalInfo->_semID, (seconds < 0) ? WAIT_FOREVER : (int)(seconds * sysClkRateGet()));

	return (result == OK) ? true : false;
}

void datalog_SendSignal(const char * signalName)
{
	SignalInfo * signalInfo = getSignalInfo(signalName);
	semGive(signalInfo->_semID);
}

void datalog_SetupPeriodicSignal(const char * signalName, double seconds)
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
	if ( seconds > 0 )
	{
		struct itimerspec	period;

		period.it_value.tv_sec = period.it_interval.tv_sec = (time_t)seconds;
		period.it_value.tv_nsec = period.it_interval.tv_nsec = (long)((seconds-period.it_interval.tv_sec) * 1e9);
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

