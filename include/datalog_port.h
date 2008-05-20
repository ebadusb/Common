/*
 * Copyright (C) 2002 Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: //bctquad3/home/BCT_Development/vxWorks/Common/include/rcs/datalog_port.h 1.20 2005/05/31 20:27:14Z jheiusb Exp $
 * $Log: datalog_port.h $
 * Revision 1.20  2005/05/31 20:27:14Z  jheiusb
 * IT32 Make changes to common to accommodate the Trima 5.2 vxWorks 5.5 port
 * Revision 1.19  2005/05/31 19:58:25Z  jl11312
 * - temporarily removed previous changes since they broke the common build
 * Revision 1.17  2004/10/26 20:49:02Z  rm70006
 * Revision 1.16  2003/10/03 12:32:57Z  jl11312
 * - improved DataLog_Handle lookup time
 * - modified datalog signal handling to eliminate requirement for a name lookup and the semaphore lock/unlock that went with it
 * Revision 1.15  2003/04/29 17:06:56Z  jl11312
 * - added function to get FILE pointer for console output
 * Revision 1.14  2003/03/27 15:47:36Z  jl11312
 * - added support for new data log levels
 * Revision 1.13  2003/02/25 20:40:11Z  jl11312
 * - added support for logging platform specific information in log header
 * Revision 1.12  2003/02/25 16:12:01  jl11312
 * - modified buffering scheme to help prevent buffer overruns
 * Revision 1.11  2003/02/06 20:42:04  jl11312
 * - added support for binary record type
 * - added support for symbolic node names in networked configurations
 * - enabled compression/encryption of log files
 * Revision 1.10  2002/09/23 13:54:46  jl11312
 * - added access function for current log file name
 * Revision 1.9  2002/08/28 14:36:40  jl11312
 * - changed handling of critical output to avoid problem with handles referencing deleted tasks
 * Revision 1.8  2002/08/22 20:18:27  jl11312
 * - added network support
 * Revision 1.7  2002/08/15 20:54:52  jl11312
 * - added support for periodic logging
 * Revision 1.6  2002/07/18 21:20:05  jl11312
 * - added support for default log levels
 * Revision 1.5  2002/07/17 20:31:51  jl11312
 * - initial datalog implementation (no support for periodic logging)
 * Revision 1.4  2002/06/04 20:23:48  jl11312
 * - added default level related functions
 * - modified to compile correctly with C-source files
 * Revision 1.3  2002/04/29 21:48:42  jl11312
 * - updated to reflect API changes
 * Revision 1.2  2002/03/27 16:43:25  jl11312
 * - updated to allow multiple data sets for periodic logging
 * - updated error handling interface
 * Revision 1.1  2002/03/19 16:11:35  jl11312
 * Initial revision
 *
 */

#ifndef _DATALOG_PORT_INCLUDE
#define _DATALOG_PORT_INCLUDE

/*
 * Error type is defined here to allow for platform specific errors.  Most
 * errors however are common to all platforms.
 */
typedef enum
{
	DataLog_NoError = 0,			/* must be first entry */

	DataLog_BadNetworkClientData,
	DataLog_MultipleInitialization,
	DataLog_NetworkConnectionFailed,
	DataLog_InvalidHandle,
	DataLog_LevelNotInitialized,
	DataLog_CriticalBufferMissing,
	DataLog_LevelConstructorFailed,
	DataLog_OpenOutputFileFailed,
	DataLog_LevelRecordWriteFailed,
	DataLog_PeriodicSetRecordWriteFailed,
	DataLog_PeriodicItemRecordWriteFailed,
	DataLog_PrintFormatError,
	DataLog_InternalWriteError,
	DataLog_PeriodicWriteError,
	DataLog_NotLogToFile,
	DataLog_BufferTooSmall,
	DataLog_StartNewLogNotAllowed,

	DataLog_LastError				/* must be last entry */
} DataLog_ErrorType;

#ifdef DATALOG_DECLARE_ERROR_INFORMATION

typedef struct
{
	DataLog_ErrorType	_error;
	const char * _message;
	int _continuable;
} DataLog_ErrorInformation;

//
// Error messages and continuable flag for each datalog error type
//
DataLog_ErrorInformation	datalog_ErrorInformation[DataLog_LastError] =
{
	{ DataLog_NoError, "no error", 1 },
	{ DataLog_BadNetworkClientData, "bad network client data", 1 },
	{ DataLog_MultipleInitialization, "multiple initialization", 1 },
	{ DataLog_NetworkConnectionFailed, "network connection failed", 0 },
	{ DataLog_InvalidHandle, "invalid handle", 1 },
	{ DataLog_LevelNotInitialized, "level not initialized", 1 },
	{ DataLog_CriticalBufferMissing, "critical buffer missing", 0 },
	{ DataLog_LevelConstructorFailed, "level constructor failed", 0 },
	{ DataLog_OpenOutputFileFailed, "open output file failed", 0 },
	{ DataLog_LevelRecordWriteFailed, "level record write failed", 1 },
	{ DataLog_PeriodicSetRecordWriteFailed, "periodic set record write failed", 1 },
	{ DataLog_PeriodicItemRecordWriteFailed, "periodic item record write failed", 1 },
	{ DataLog_PrintFormatError, "print format error", 1 },
	{ DataLog_InternalWriteError, "internal write error", 1 },
	{ DataLog_PeriodicWriteError, "periodic write error", 1 },
	{ DataLog_NotLogToFile, "connection type not log to file", 1 },
	{ DataLog_BufferTooSmall, "supplied buffer too small", 1 },
	{ DataLog_StartNewLogNotAllowed, "not configured for created multiple logs without rebooting", 0 }
};

#endif /* ifdef DATALOG_DECLARE_ERROR_INFORMATION */

/*
 * Platform specific configuration
 */
#ifdef VXWORKS

#include <vxWorks.h>

#include <ioLib.h>      /* for common file IO */
#include <errnoLib.h>   /* for error functions */
#include <sysLib.h>
#include <sockLib.h>
#include <symLib.h>
#include <sysSymTbl.h>

/*
 *	Network related definitions (needed only if platform supports network
 * connections to data log system).
 */
#define DATALOG_NETWORK_SUPPORT
typedef unsigned long	DataLog_NodeID;

/*
 * Support initialization of global datalog levels/handles
 */
#define DATALOG_LEVELS_INIT_SUPPORT

/*
 *	Platform specific data types
 */
typedef unsigned char DataLog_UINT8;
typedef unsigned short DataLog_UINT16;
typedef unsigned long DataLog_UINT32;

/*
 *	Internal ID related definitions (used for ID information in log files)
 */
typedef unsigned short DataLog_InternalID;
#define DATALOG_NULL_ID 0

/*
 *	Task related definitions
 */
typedef int DataLog_TaskID;
#define DATALOG_CURRENT_TASK 0

/*
 *	Shared memory related definitions
 */
#define DataLog_SharedPtr(type) type *
#define DATALOG_NULL_SHARED_PTR NULL

/*
 *	Output buffer related definitions
 */
typedef unsigned char DataLog_BufferData;

/*
 *	Access lock related definitions
 */
#include <semLib.h>
typedef SEM_ID	DataLog_Lock;

/*
 *	STL list type
 */
#ifdef __cplusplus
#include <list.h>
#define	DataLog_List	list
#endif /* ifdef __cplusplus */

/*
 *	STL map type
 */
#ifdef __cplusplus
#include <map.h>
#define	DataLog_Map		map
#endif /* ifdef __cplusplus */

/*
 * Packing directives
 * VxWorks Supports the packing of individual elements so:
 *    Packing_Structure_Directive is blank
 *    Packing_Element_Directive is set appropriately
 */
#define Packing_Structure_Directive
#define Packing_Element_Directive __attribute__ ((packed))

#define DATALOG_OUTPUT_FILE_OPT (O_WRONLY | O_CREAT)
#include <inetLib.h>

#elif defined (_WIN32) /************************************************************************************/

#include <io.h>      /* Used to get access to open() call. */
#include <winsock2.h>  /* used for socket calls */
#include <assert.h>

#define OK 1
#define WAIT_FOREVER INFINITE

/*
 *	Network related definitions (needed only if platform supports network
 * connections to data log system).
 */
#define DATALOG_NETWORK_SUPPORT
typedef unsigned long	DataLog_NodeID;

/*
 * Support initialization of global datalog levels/handles
 */
#define DATALOG_LEVELS_INIT_SUPPORT

/*
 *	Platform specific data types
 */
typedef unsigned char DataLog_UINT8;
typedef unsigned short DataLog_UINT16;
typedef unsigned long DataLog_UINT32;

/*
 *	Internal ID related definitions (used for ID information in log files)
 */
typedef unsigned short DataLog_InternalID;
#define DATALOG_NULL_ID 0

/*
 *	Task related definitions
 */
typedef int DataLog_TaskID;
#define DATALOG_CURRENT_TASK 0

/*
 *	Shared memory related definitions
 */
#define DataLog_SharedPtr(type) type *
#define DATALOG_NULL_SHARED_PTR NULL

/*
 *	Output buffer related definitions
 */
typedef unsigned char DataLog_BufferData;

/*
 *	Access lock related definitions
 */
typedef HANDLE	DataLog_Lock;

/*
 *	STL list type
 */
#ifdef __cplusplus
#include <list>
#define	DataLog_List std::list
#endif /* ifdef __cplusplus */

/*
 *	STL map type
 */
#ifdef __cplusplus
#include <map>
#define	DataLog_Map	std::map
#endif /* ifdef __cplusplus */

/*
 * Packing directives
 * .NET DOES NOT support the packing of individual elements so:
 *    Packing_Structure_Directive is set appropriately
 *    Packing_Element_Directive is blank
 */
#define Packing_Structure_Directive __declspec(align(1))  /* pack to the closest byte */
#define Packing_Element_Directive

#define INET_ADDR_LEN 18

inline int errnoGet (void) { return errno; };
inline int taskDelay (int ticks) { Sleep (ticks); return 1; };


/* required function definitions */

extern int connectWithTimeout (int sock, struct sockaddr *adrs, int adrsLen, struct timeval *timeVal);
extern void inet_ntoa_b (struct in_addr inetAddress, char *pString);
inline int  sysClkRateGet (void) { return 1; };
extern int 	taskLock (void);
extern int 	taskIdListGet (int idList [ ], int maxTasks);
extern int 	taskUnlock (void);
extern int  intLock(void);
extern int  intUnlock(int level);
// We must define our output file to be opened with the O_RAW flag in order
// to assure that extra carriage returns aren't inserted into our output
#define DATALOG_OUTPUT_FILE_OPT (O_WRONLY | O_CREAT | O_RAW)


#else /* ifdef VXWORKS */
#error "Unknown platform"
#endif /* ifdef VXWORKS */

#ifdef __cplusplus
extern "C" {
#endif /* ifdef __cplusplus */

/*
 *	Console logging support
 */
#include <stdio.h>
FILE * datalog_ConsoleFile(void);

/*
 *	Task related functions
 */
DataLog_TaskID datalog_CurrentTask(void);
const char * datalog_TaskName(DataLog_TaskID taskID);

/*
 *	Shared memory related functions
 */
DataLog_SharedPtr(void) datalog_AllocSharedMem(size_t size);
void datalog_FreeSharedMem(DataLog_SharedPtr(void) ptr);

/*
 *	Access lock related functions
 */
DataLog_Lock datalog_CreateMLock(void);
DataLog_Lock datalog_CreateBLock(void);

void datalog_DeleteLock(DataLog_Lock lock);
int datalog_LockAccess(DataLog_Lock lock, int delay);
void datalog_ReleaseAccess(DataLog_Lock lock);

/*
 *	Signal related functions
 */
#ifdef __cplusplus
struct DataLog_SignalInfo;
DataLog_SignalInfo * datalog_CreateSignal(const char * signalName);
bool datalog_WaitSignal(DataLog_SignalInfo * signal, long milliSeconds);
void datalog_SendSignal(DataLog_SignalInfo * signal);
void datalog_SetupPeriodicSignal(DataLog_SignalInfo * signal, long milliSeconds);
#endif /* ifdef __cplusplus */

/*
 *	Data log task related functions
 */
void datalog_StartLocalOutputTask(const char * platformName, const char * nodeName, const char * platformInfo);
void datalog_StartNetworkOutputTask(long connectTimeout, const char * nodeName);
void datalog_StartPeriodicLogTask(DataLog_SetHandle set);

#ifdef DATALOG_NETWORK_SUPPORT
void datalog_StartNetworkTask(void);
void datalog_StartNetworkClientTask(int clientSocket, struct sockaddr_in * clientAddr);
#endif /* ifdef DATALOG_NETWORK_SUPPORT */

/*
 *	Time stamp related functions.  Note that the related structures
 * are written to the log file and so must be packed.
 */

Packing_Structure_Directive typedef struct
{
	DataLog_UINT8	_day Packing_Element_Directive;		/* 1-31 */
	DataLog_UINT8	_month Packing_Element_Directive;		/* 1-12 */
	DataLog_UINT16	_year Packing_Element_Directive;
	DataLog_UINT8	_hour Packing_Element_Directive;
	DataLog_UINT8	_minute Packing_Element_Directive;
	DataLog_UINT8	_second Packing_Element_Directive;
} DataLog_TimeStampStart;

Packing_Structure_Directive typedef struct
{
	DataLog_UINT32	_seconds Packing_Element_Directive;
	DataLog_UINT32	_nanoseconds Packing_Element_Directive;
} DataLog_TimeStamp;

void datalog_GetTimeStampStart(DataLog_TimeStampStart * start);
void datalog_GetTimeStamp(DataLog_TimeStamp * stamp);

void datalog_TaskCreated(DataLog_TaskID taskID);
void datalog_TaskDeleted(DataLog_TaskID taskID);

//#ifdef DATALOG_NETWORK_SUPPORT
DataLog_NodeID datalog_NodeID(void);
//#endif /* ifdef DATALOG_NETWORK_SUPPORT */

#ifdef __cplusplus
}; // extern "C"
#endif /* ifdef __cplusplus */

#endif /* ifndef _DATALOG_PORT_INCLUDE */
