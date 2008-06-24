/*
 * Copyright (C) 2002 Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: I:/BCT_Development/vxWorks/Common/datalog/rcs/datalog_port_vxworks.cpp 1.12 2003/12/16 21:56:59Z jl11312 Exp $
 * $Log: datalog_port_vxworks.cpp $
 * Revision 1.12  2003/12/16 21:56:59Z  jl11312
 * - replaced binary semaphore with mutex semaphore to avoid potential priority inversion
 * Revision 1.11  2003/12/09 14:14:40Z  jl11312
 * - corrected time stamp problem (IT 6668)
 * - removed obsolete code/data types (IT 6664)
 * Revision 1.10  2003/11/10 17:46:24Z  jl11312
 * - corrections from data log unit tests (see IT 6598)
 * Revision 1.9  2003/10/16 14:57:40Z  jl11312
 * - corrected time stamp problem when Trima time/date changes (IT 6516)
 * Revision 1.8  2003/10/03 12:35:13Z  jl11312
 * - improved DataLog_Handle lookup time
 * - modified datalog signal handling to eliminate requirement for a name lookup and the semaphore lock/unlock that went with it
 * Revision 1.7  2003/04/29 17:07:58Z  jl11312
 * - direct console output directly to console instead of stdout
 * Revision 1.6  2003/03/27 16:27:04Z  jl11312
 * - added support for new datalog levels
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


#define _WIN32_WINNT 0x0500

#include <stdio.h>
#include "datalog.h"
#include "error.h"
#include <string>
#include <time.h>
#include <sys/timeb.h>

#include "datalog_internal.h"

#ifdef DATALOG_LEVELS_INIT_SUPPORT
# include "datalog_levels.h"
#endif /* ifdef DATALOG_LEVELS_INIT_SUPPORT */


//
// Platform Specific functions.
// These functions have been sorted out for easier porting.
//
#define timer_t HANDLE


DataLog_Lock datalog_CreateMLock(void)
{
	return CreateMutex(0, FALSE, 0); /* semMCreate(SEM_Q_PRIORITY | SEM_INVERSION_SAFE); */
}

DataLog_Lock datalog_CreateBLock(void)
{
	return CreateSemaphoreW(0, 0, 1, 0); /* semBCreate(SEM_Q_PRIORITY, SEM_EMPTY); */
}

void datalog_DeleteLock(DataLog_Lock lock)
{
	CloseHandle(lock);  
}

int datalog_LockAccess(DataLog_Lock lock, int delay)
{
	return WaitForSingleObject(lock, delay);
}

void datalog_ReleaseAccess(DataLog_Lock lock)
{
	ReleaseMutex(lock);
}

void datalog_ReleaseSemaphore(DataLog_Lock lock)
{
  ReleaseSemaphore(lock, 1, NULL);
}


//
// Local functions
//
static __timeb64 markTimeStampStart(void);

//
// Initialization related functions
//
static DataLog_Lock initializationDataLock = datalog_CreateMLock();   /* semMCreate(SEM_Q_PRIORITY | SEM_INVERSION_SAFE); */
static bool initializationStarted;
static HANDLE timerQueue = NULL;

bool DataLog_CommonData::startInitialization(void)
{
	bool result = false;

	datalog_LockAccess(initializationDataLock, WAIT_FOREVER);
	if ( !initializationStarted )
	{
		initializationStarted = true;
		result = true;
	}

	datalog_ReleaseAccess(initializationDataLock);
	return result;
}

bool DataLog_CommonData::isInitialized(void)
{
	datalog_LockAccess(initializationDataLock, WAIT_FOREVER);
	bool result = initializationStarted;
	datalog_ReleaseAccess(initializationDataLock);

	return result;
}

//
// Internal ID related functions
//
static DataLog_Lock	internalIDLock = datalog_CreateMLock(); /* semMCreate(SEM_Q_PRIORITY | SEM_INVERSION_SAFE); */

DataLog_InternalID DataLog_CommonData::getNextInternalID(void)
{
	static DataLog_InternalID	id = DATALOG_NULL_ID;

	datalog_LockAccess(internalIDLock, WAIT_FOREVER);

	id += 1;
	DataLog_InternalID	result = id;
	datalog_ReleaseAccess(internalIDLock);

	return result;	
}

//
// DataLog_Handle related functions
//
static DataLog_Lock	handleLock = datalog_CreateMLock(); /* semMCreate(SEM_Q_PRIORITY | SEM_INVERSION_SAFE); */
static std::map<std::string, DataLog_Handle>	handleMap;

DataLog_Handle DataLog_CommonData::findHandle(const char * handleName)
{
	DataLog_Handle result = DATALOG_NULL_HANDLE;

	datalog_LockAccess(handleLock, WAIT_FOREVER);
	if ( handleMap.find(handleName) != handleMap.end() )
	{
		result = handleMap[handleName];
	}

	datalog_ReleaseAccess(handleLock);
	return result;
}

void DataLog_CommonData::addHandle(const char * handleName, DataLog_Handle handle)
{
	datalog_LockAccess(handleLock, WAIT_FOREVER);
	if ( handleMap.find(handleName) != handleMap.end() )
	{
#ifdef DATALOG_LEVELS_INIT_SUPPORT
		DataLog(log_level_datalog_error)
#else /* ifdef DATALOG_LEVELS_INIT_SUPPORT */
		DataLog_Default
#endif /* ifdef DATALOG_LEVELS_INIT_SUPPORT */
			<< "attempt to register duplicate handle name ignored" << endmsg;
	}
	else
	{
		handleMap[handleName] = handle;
	}

	datalog_ReleaseAccess(handleLock);
}

//
// Common data area related functions
//
static DataLog_Lock	commonDataLock = datalog_CreateMLock(); /* semMCreate(SEM_Q_PRIORITY | SEM_INVERSION_SAFE); */

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
		datalog_LockAccess(commonDataLock, WAIT_FOREVER);
		if ( commonData == DATALOG_NULL_SHARED_PTR )
		{
			commonData = (DataLog_SharedPtr(CommonData))datalog_AllocSharedMem(sizeof(CommonData));
			initializeCommonData(commonData);

#if 0
			taskCreateHookAdd((FUNCPTR)datalog_TaskCreated);
			taskDeleteHookAdd((FUNCPTR)datalog_TaskDeleted);
#endif
	   }

		datalog_ReleaseAccess(commonDataLock);
	}

	_commonData = commonData;
}

//
// Signal related functions
//
struct DataLog_SignalInfo
{
	DataLog_Lock	_semID;

	bool		_timerCreated;
	timer_t 	_timerID;
};

static DataLog_Lock	signalDataLock = datalog_CreateMLock(); /* semMCreate(SEM_Q_PRIORITY | SEM_INVERSION_SAFE); */
static std::map<std::string, DataLog_SignalInfo *>	signalMap;

VOID CALLBACK timerCallback(PVOID lpParameter, BOOLEAN)
{
  datalog_ReleaseSemaphore(static_cast<DataLog_SignalInfo*>(lpParameter)->_semID);
}

DataLog_SignalInfo * datalog_CreateSignal(const char * signalName)
{
	//
	//	Create signal if necessary
	//
	DataLog_SignalInfo *	result;

	datalog_LockAccess(signalDataLock, WAIT_FOREVER);
	if ( signalMap.find(signalName) == signalMap.end() )
	{
		result = new DataLog_SignalInfo;
		result->_semID = datalog_CreateBLock();  /* semBCreate(SEM_Q_PRIORITY, SEM_EMPTY); */
		result->_timerCreated = false;
		signalMap[signalName] = result;
	}

	result = signalMap[signalName];
	datalog_ReleaseAccess(signalDataLock);
	return result;
}

bool datalog_WaitSignal(DataLog_SignalInfo * signalInfo, long milliSeconds)
{
	int timeout = WAIT_FOREVER;
	if (milliSeconds >= 0)
	{
		timeout = milliSeconds * sysClkRateGet() / 1000;
	} 

	int result = datalog_LockAccess(signalInfo->_semID, timeout);
	return (result == WAIT_OBJECT_0) ? true : false;
}

void datalog_SendSignal(DataLog_SignalInfo * signalInfo)
{
	datalog_ReleaseSemaphore(signalInfo->_semID);
}

void datalog_SetupPeriodicSignal(DataLog_SignalInfo * signalInfo, long milliSeconds)
{
  if(timerQueue == NULL)
    timerQueue =  CreateTimerQueue();
	if ( !signalInfo->_timerCreated )
	{
    if ( CreateTimerQueueTimer(&(signalInfo->_timerID), timerQueue, (WAITORTIMERCALLBACKFUNC)timerCallback , 
			signalInfo, 0, milliSeconds, WT_EXECUTEINIOTHREAD) != OK )
		{
			_FATAL_ERROR(__FILE__, __LINE__, "timerCreate failed");
		}

		signalInfo->_timerCreated = true;
	}

	ChangeTimerQueueTimer(timerQueue, signalInfo->_timerID, 0, milliSeconds);

/*
	timer_cancel(signalInfo->_timerID);
	if ( milliSeconds > 0 )
	{
		struct itimerspec	period;

		period.it_value.tv_sec = period.it_interval.tv_sec = (time_t)(milliSeconds/1000);
		period.it_value.tv_nsec = period.it_interval.tv_nsec = (long)((milliSeconds%1000) * 1e6);
		timer_settime(signalInfo->_timerID, 0, &period, NULL);
	}
*/
}

//
// Time stamp related functions
//
static DataLog_Lock timeDataLock = datalog_CreateMLock(); /* semMCreate(SEM_Q_PRIORITY | SEM_INVERSION_SAFE); */

static __timeb64 timeStart = markTimeStampStart();

static __timeb64 markTimeStampStart(void)
{
	__timeb64 currentTime;

	//
	// Save start time for real-time clock.  All timestamp information in the
	// log file is relative to this start time.
	//
	datalog_LockAccess(timeDataLock, WAIT_FOREVER);

	_ftime64(&currentTime);

	datalog_ReleaseAccess(timeDataLock);

	return currentTime;
}

void datalog_GetTimeStampStart(DataLog_TimeStampStart * start)
{
	struct tm *tmVal;
	tmVal = _gmtime64(&timeStart.time);

	start->_day = tmVal->tm_mday;
	start->_month = tmVal->tm_mon + 1;
	start->_year = tmVal->tm_year + 1900;
	start->_hour = tmVal->tm_hour;
	start->_minute = tmVal->tm_min;
	start->_second = tmVal->tm_sec;
}

void datalog_GetTimeStamp(DataLog_TimeStamp * stamp)
{

#if (CPU != SIMNT)
	UINT32 timestampCurrent = sysTimestamp();
#endif /* if (CPU != SIMNT) */

	__timeb64 timeCurrent;
	_ftime64(&timeCurrent);

	stamp->_seconds     = (DataLog_UINT32)(timeCurrent.time - timeStart.time);
	stamp->_nanoseconds = (DataLog_UINT32)(timeCurrent.millitm - timeStart.millitm);
}

static FILE * consoleFile = NULL;
int consoleFd = _fileno(stdout);

FILE * datalog_ConsoleFile(void)
{
	if ( !consoleFile )
	{
		consoleFile = fdopen(consoleFd, "w");
		if ( !consoleFile )
		{
			consoleFile = stderr;
		}
	}
 
	return consoleFile;
}

DataLog_TaskID datalog_CurrentTask(void)
{
	/* TBD
	return taskIdSelf();
	*/
	return -1;
}

const char * datalog_TaskName(DataLog_TaskID taskID)
{
	/* TBD
	return taskName((taskID == DATALOG_CURRENT_TASK) ? taskIdSelf() : taskID);\
	*/
	return " ";
}

DataLog_SharedPtr(void) datalog_AllocSharedMem(size_t size)
{
	return malloc(size);
}

void datalog_FreeSharedMem(DataLog_SharedPtr(void) ptr)
{
	free(ptr);
}



//
// Buffer manager
//
static DataLog_SignalInfo *	outputSignal = NULL;
static DataLog_SignalInfo *   dataLostSignal = NULL;
static DataLog_BufferManager::DataLog_BufferList	traceList;
static DataLog_BufferManager::DataLog_BufferList	criticalList;
static DataLog_BufferManager::DataLog_BufferList	freeList;

void DataLog_BufferManager::initialize(size_t bufferSizeKBytes)
{
	outputSignal = datalog_CreateSignal("DataLog_Output");
	dataLostSignal = datalog_CreateSignal("DataLog_DataLost");

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
		if ( !freeList._head ) freeList._tail = NULL;
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

bool DataLog_BufferManager::getNextChain(DataLog_BufferChain & chain, bool * isCritical)
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

	if ( isCritical )
	{
		*isCritical = (list == &criticalList);
	}

	return (chain._head != NULL);
}

inline DataLog_BufferManager::DataLog_BufferList * getListPtr(DataLog_BufferManager::BufferList list)
{
	DataLog_BufferManager::DataLog_BufferList * bufferList = NULL;

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
			datalog_SendSignal(dataLostSignal);
		}

		int chainBufferCount = (chain._head->_length+DataLog_BufferSize-1)/DataLog_BufferSize;
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
			datalog_SendSignal(outputSignal);
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

DataLog_BufferManager::DataLog_BufferList * DataLog_BufferManager::getInternalList(BufferList list)
{
	return getListPtr(list);
}



/*****************************************************************************/

/* connectWithTimeout
 *    This function always assumes that the socket is blocking.  I couldn't find a way to determine the state of
 * the socket without using MFC.
 */
int connectWithTimeout (int sock, struct sockaddr *adrs, int adrsLen, struct timeval *timeVal)
{
    unsigned long on = 0;
    fd_set		writeFds;
    int			retVal = -1;
    int 		error;
    int			peerAdrsLen;
    struct sockaddr	peerAdrs;

    if (timeVal == NULL)
        return (connect (sock, adrs, adrsLen));

	{
		on = 1;	/* set NBIO to have the connect() return without pending */

        if ((ioctlsocket (sock, FIONBIO, &on)) == ERROR)
			return (ERROR);
	}

    if (connect (sock, adrs, adrsLen) < 0)
	{
	/*
         * When a TCP socket is set to non-blocking mode, the connect() 
         * routine might return EINPROGRESS (if the three-way handshake
         * is not completed immediately) or EALREADY (if a previous connect
         * attempt is still pending). All other errors (for any socket type)
         * indicate a fatal problem, such as insufficient memory to record
         * the remote address or (for TCP) an inability to transmit the
         * initial SYN segment.
	 */

    error = WSAGetLastError() & 0xffff;

	if (error == WSAEINPROGRESS || error == WSAEALREADY)
            {
	    /*
             * Use the select() routine to monitor the socket status. The
             * socket will be writable when the three-way handshake completes
             * or when the handshake timeout occurs.
	     */

	    FD_ZERO (&writeFds);
	    FD_SET (sock, &writeFds);

	    if (select (FD_SETSIZE, (fd_set *) NULL, &writeFds,(fd_set *) NULL, timeVal) > 0)
		{
		/* The FD_ISSET test is theoretically redundant, but safer. */

			if (FD_ISSET (sock, &writeFds))
			{
				/*
						* The connection attempt has completed. The getpeername()
						* routine retrieves the remote address if the three-way
						* handshake succeeded and fails if the attempt timed out.
				*/

				peerAdrsLen = sizeof (peerAdrs);

				if (getpeername (sock, &peerAdrs, &peerAdrsLen) != ERROR)
				retVal = OK;
			}
		}
	    else
			WSASetLastError(WSAETIMEDOUT);
	    }
	}
    else
		retVal = OK;

    if (on)
	{
		on = 0;		/* turn OFF the non-blocking I/O */

		if ((ioctlsocket (sock, FIONBIO, &on)) == ERROR)
			return (ERROR);
	}

    return retVal;
}

void inet_ntoa_b (struct in_addr inetAddress, char *pString)
{
#define	UC(b)	(((int)b)&0xff)
    register char *p = (char *)&inetAddress;

    sprintf (pString, "%d.%d.%d.%d", UC(p[0]), UC(p[1]), UC(p[2]), UC(p[3]));
}

void _FATAL_ERROR(const char * file, int line, const char * eString)
{
    fprintf(stderr, "%s(%d): %s\n", file, line, eString);
	exit(-1);
}



int taskLock (void) { return 0; };
int taskIdListGet (int idList [ ], int maxTasks) { return 0; };
int taskUnlock (void) { return 0; };
int intLock(void) { return 0; };
int intUnlock(int level) { return 0; };


