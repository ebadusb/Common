/*
 * Copyright (C) 2002 Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: //bctquad3/home/BCT_Development/vxWorks/Common/include/rcs/datalog_port.h 1.16 2003/10/03 12:32:57Z jl11312 Exp rm70006 $
 * $Log: datalog_port.h $
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
	DataLog_NoSuchTask,
	DataLog_InvalidHandle,
	DataLog_LevelNotInitialized,
	DataLog_CriticalBufferMissing,
	DataLog_LevelConstructorFailed,
	DataLog_OpenOutputFileFailed,
	DataLog_LevelRecordWriteFailed,
	DataLog_PrintFormatError,
	DataLog_InternalWriteError,

	DataLog_LastError				/* must be last entry */
} DataLog_ErrorType;

#ifdef DATALOG_DECLARE_ERROR_INFORMATION

typedef struct
{
	DataLog_ErrorType	_error;
	const char * _message;
	int _continuable;
} DataLog_ErrorInformation;

DataLog_ErrorInformation	datalog_ErrorInformation[DataLog_LastError] =
{
	{ DataLog_NoError, "no error", 1 },
	{ DataLog_BadNetworkClientData, "bad network client data", 1 },
	{ DataLog_MultipleInitialization, "multiple initialization", 1 },
	{ DataLog_NoSuchTask, "no such task", 1 },
	{ DataLog_InvalidHandle, "invalid handle", 1 },
	{ DataLog_LevelNotInitialized, "level not initialized", 1 },
	{ DataLog_CriticalBufferMissing, "critical buffer missing", 0 },
	{ DataLog_LevelConstructorFailed, "level constructor failed", 0 },
	{ DataLog_OpenOutputFileFailed, "open output file failed", 0 },
	{ DataLog_LevelRecordWriteFailed, "level record write failed", 1 },
	{ DataLog_PrintFormatError, "print format error", 1 },
	{ DataLog_InternalWriteError, "internal write error", 1 }
};

#endif /* ifdef DATALOG_DECLARE_ERROR_INFORMATION */

/*
 * Platform specific configuration
 */
#ifdef VXWORKS

/*
 *	Platform specific data types
 */
typedef unsigned char DataLog_UINT8;
typedef unsigned short DataLog_UINT16;
typedef unsigned long DataLog_UINT32;
 
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
 *	Network related definitions
 */
#ifndef DATALOG_NO_NETWORK_SUPPORT
typedef unsigned long	DataLog_NodeID;
#endif /* ifndef DATALOG_NO_NETWORK_SUPPORT */

#else /* ifdef VXWORKS */
#error "Unknown platform"
#endif /* ifdef VXWORKS */

#ifdef __cplusplus
extern "C" {
#endif /* ifdef __cplusplus */

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
DataLog_Lock datalog_CreateLock(void);
void datalog_DeleteLock(DataLog_Lock lock);
void datalog_LockAccess(DataLog_Lock lock);
void datalog_ReleaseAccess(DataLog_Lock lock);

/*
 *	Signal related functions
 */
#ifdef __cplusplus
bool datalog_WaitSignal(const char * signalName, double seconds);
void datalog_SendSignal(const char * signalName);
#endif /* ifdef __cplusplus */

/*
 *	Comman data area related functions
 */
#ifdef __cplusplus
class DataLog_CommonData;
DataLog_SharedPtr(DataLog_CommonData) datalog_GetCommonDataPtr(void);
void datalog_SetCommonDataPtr(DataLog_SharedPtr(DataLog_CommonData) ptr);
bool datalog_StartInitialization(void);
#endif /* ifdef __cplusplus */

/*
 *	Data log task related functions
 */
void datalog_StartOutputTask(const char * platformName);

/*
 *	Time stamp related functions.  Note that the related structures
 * are written to the log file and so must be packed.
 */
typedef struct
{
	DataLog_UINT8	_day __attribute__ ((packed));		/* 1-31 */
	DataLog_UINT8	_month __attribute__ ((packed));		/* 1-12 */
	DataLog_UINT16	_year __attribute__ ((packed));
	DataLog_UINT8	_hour __attribute__ ((packed));
	DataLog_UINT8	_minute __attribute__ ((packed));
	DataLog_UINT8	_second __attribute__ ((packed));
} DataLog_TimeStampStart;

typedef struct
{
	DataLog_UINT32	_seconds __attribute__ ((packed));
	DataLog_UINT32	_nanoseconds __attribute__ ((packed));
} DataLog_TimeStamp;

void datalog_GetTimeStampStart(DataLog_TimeStampStart * start);
void datalog_GetTimeStamp(DataLog_TimeStamp * stamp);

void datalog_TaskCreated(DataLog_TaskID taskID);
void datalog_TaskDeleted(DataLog_TaskID taskID);

#ifndef DATALOG_NO_NETWORK_SUPPORT
DataLog_NodeID datalog_NodeID(void);
#endif /* ifndef DATALOG_NO_NETWORK_SUPPORT */

#ifdef __cplusplus
}; // extern "C"
#endif /* ifdef __cplusplus */

#endif /* ifndef _DATALOG_PORT_INCLUDE */
