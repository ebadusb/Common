/*
 * Copyright (C) 2002 Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: K:/BCT_Development/vxWorks/Common/datalog/rcs/datalog_port_vxworks.cpp 1.9 2003/10/16 14:57:40Z jl11312 Exp jl11312 $
 * $Log: datalog_port_vxworks.cpp $
 *
 */

#include <vxWorks.h>
#include <intLib.h>
#include <taskHookLib.h>
#include <string>
#include <sysLib.h>
#include <timers.h>

#include "datalog.h"
#include "error.h"

class DataLog_PortData
{
public:
	DataLog_PortData(void);

	DataLog_SharedPtr(DataLog_CommonData) getCommonDataPtr(void) { return commonDataPtr; }
	void setCommonDataPtr(DataLog_SharedPtr(DataLog_CommonData) ptr) { commonDataPtr = ptr; }

	bool startInitialization(void);

	SEM_ID getSignalSem(const char * signalName);
	void sendSignal(const char * signalName);
	bool waitSignal(const char * signalName, double seconds);

	void getTimeStampStart(DataLog_TimeStampStart * start);
	void getTimeStamp(DataLog_TimeStamp * stamp);

protected:
	void markTimeStampStart(void);

private:
	bool initializationStarted;
	DataLog_SharedPtr(DataLog_CommonData) commonDataPtr; 

	map<string, SEM_ID>	signalMap;

#if (CPU != SIMNT)
	UINT32				timestampStart;
#endif

	struct timespec	clockStart;
	time_t				timeStart;

	SEM_ID portDataLock;
};

static DataLog_PortData	localPortData;

DataLog_PortData::DataLog_PortData(void)
{
	portDataLock = semBCreate(SEM_Q_PRIORITY, SEM_FULL);
	initializationStarted = false;
	commonDataPtr = NULL;

	markTimeStampStart();
}

bool DataLog_PortData::startInitialization(void)
{
	semTake(portDataLock, WAIT_FOREVER);
	bool result = initializationStarted;
	initializationStarted = true;
	semGive(portDataLock);

	return result;
}

SEM_ID DataLog_PortData::getSignalSem(const char * signalName)
{
	//
	//	Create signal if necessary
	//
	SEM_ID	result;

	semTake(portDataLock, WAIT_FOREVER);
	if ( signalMap.find(signalName) == signalMap.end() )
	{
		signalMap[signalName] = semBCreate(SEM_Q_PRIORITY, SEM_EMPTY);
	}

	result = signalMap[signalName];
	semGive(portDataLock);
	return result;
}

bool DataLog_PortData::waitSignal(const char * signalName, double seconds)
{
	SEM_ID signalSem = getSignalSem(signalName);
	STATUS result = semTake(signalSem, (seconds < 0) ? WAIT_FOREVER : (int)(seconds * sysClkRateGet()));

	return (result == OK) ? true : false;
}

void DataLog_PortData::sendSignal(const char * signalName)
{
	SEM_ID signalSem = getSignalSem(signalName);
	semGive(signalSem);
}

void DataLog_PortData::markTimeStampStart(void)
{
	int	oldLevel;

#if (CPU != SIMNT)
	//
	// Make sure time stamp clock is enabled and period set correctly (for the
	// 8253 driver - getTimeStampPeriod() also sets an internal period value
	// needed for proper operation).
	//
	sysTimestampEnable();
	sysTimestampPeriod();
#endif /* if (CPU != SIMNT) */

	//
	// Latch current values for all clocks needed for log timestamp
	//
	semTake(portDataLock, WAIT_FOREVER);
	oldLevel = intLock();

#if (CPU != SIMNT)
	timestampStart = sysTimestamp();
#endif /* if (CPU != SIMNT) */

	clock_gettime(CLOCK_REALTIME, &clockStart);
	timeStart = time(NULL);

	intUnlock(oldLevel);
	semGive(portDataLock);
}

void DataLog_PortData::getTimeStampStart(DataLog_TimeStampStart * start)
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

void DataLog_PortData::getTimeStamp(DataLog_TimeStamp * stamp)
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

	long			seconds;
	long long	nanoseconds;

	seconds = clockCurrent.tv_sec - clockStart.tv_sec;
	nanoseconds = clockCurrent.tv_nsec - clockStart.tv_nsec;

#if (CPU != SIMNT)
	nanoseconds += ((long long)timestampCurrent - (long long)timestampStart) * (long long)1000000000 / (long long)sysTimestampPeriod();
#endif /* if (CPU != SIMNT) */

	if ( nanoseconds < 0 )
	{
		long long multiple = ((long long)999999999-nanoseconds)/(long long)1000000000;
		nanoseconds += multiple*(long long)1000000000;
		seconds -= multiple;
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

bool datalog_WaitSignal(const char * signalName, double seconds)
{
	return localPortData.waitSignal(signalName, seconds);
}

void datalog_SendSignal(const char * signalName)
{
	localPortData.sendSignal(signalName);
}

DataLog_SharedPtr(DataLog_CommonData) datalog_GetCommonDataPtr(void)
{
	DataLog_SharedPtr(DataLog_CommonData) result = localPortData.getCommonDataPtr();
	if ( result == NULL )
	{
		_FATAL_ERROR(__FILE__, __LINE__, "data log common data pointer not set");
	}

	return result;
}

void datalog_SetCommonDataPtr(DataLog_SharedPtr(DataLog_CommonData) ptr)
{
	localPortData.setCommonDataPtr(ptr);

	/*
	 *	task hooks require the common pointer to be set to function correctly
	 */
	taskCreateHookAdd((FUNCPTR)datalog_TaskCreated);
	taskDeleteHookAdd((FUNCPTR)datalog_TaskDeleted);
}

bool datalog_StartInitialization(void)
{
	return localPortData.startInitialization();
}

void datalog_GetTimeStampStart(DataLog_TimeStampStart * start)
{
	localPortData.getTimeStampStart(start);
}

void datalog_GetTimeStamp(DataLog_TimeStamp * stamp)
{
	localPortData.getTimeStamp(stamp);
}

