/*
 * Copyright (C) 2002 Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: K:/BCT_Development/vxWorks/Common/include/rcs/datalog.h 1.21 2003/02/25 20:40:08Z jl11312 Exp jl11312 $
 * $Log: datalog.h $
 * Revision 1.7  2002/05/17 18:27:59  jl11312
 * - more temporary changes for debug use under vxWorks
 * Revision 1.6  2002/05/17 17:15:59  jl11312
 * - temporary change of operator() return type
 * Revision 1.5  2002/04/29 21:48:42  jl11312
 * - updated to reflect API changes
 * Revision 1.4  2002/03/27 16:43:25  jl11312
 * - updated to allow multiple data sets for periodic logging
 * - updated error handling interface
 * Revision 1.3  2002/03/20 16:37:27  jl11312
 * - redefined template functions as inline
 * Revision 1.2  2002/03/19 16:27:20  jl11312
 * - added the missing periodic logging functions
 * Revision 1.1  2002/03/19 16:11:36  jl11312
 * Initial revision
 *
 */

#ifndef _DATALOG_INCLUDE
#define _DATALOG_INCLUDE

#include <stdio.h>
#include <stdlib.h>
#include "datalog_port.h"

/*
 * C/C++ functions
 */
#ifdef __cplusplus
extern "C" {
#endif /* ifdef __cplusplus */

/*
 * Data log initialization routines
 */
typedef enum { DataLog_OK, DataLog_Error } DataLog_Result;

DataLog_Result datalog_Init(const char * logPath);
DataLog_Result datalog_InitNet(const char * ipAddress, double seconds);

DataLog_Result datalog_SetDefaultTraceBufferSize(size_t size);
DataLog_Result datalog_SetDefaultIntBufferSize(size_t size);
DataLog_Result datalog_SetDefaultCriticalBufferSize(size_t size);

typedef DataLog_SharedPtr(DataLog_BufferData) DataLog_EncryptFunc(DataLog_SharedPtr(DataLog_BufferData) input, size_t inputLength, size_t * outputLength);
DataLog_Result datalog_SetEncryptFunc(DataLog_EncryptFunc * func);

/*
 * Data log level creation
 */
typedef struct DataLog_HandleInfo * DataLog_Handle;
#define NULL_DATALOG_HANDLE   NULL

DataLog_Result datalog_CreateLevel(const char * levelName, DataLog_Handle * handle);
DataLog_Result	datalog_SetDefaultLevel(DataLog_Handle handle);

/*
 * Interrupt log level creation
 */
DataLog_Result datalog_CreateIntLevel(const char * levelName, DataLog_Handle * handle);

/*
 * Critical log level functions
 */
DataLog_Handle datalog_GetCriticalHandle(void);

/*
 * Data log option control
 */
typedef enum { LogEnabled, LogDisabled } DataLog_EnabledType;
typedef enum { ConsoleEnabled, ConsoleDisabled } DataLog_ConsoleEnabledType;

DataLog_Result datalog_GetTaskOutputOptions(DataLog_TaskID task, DataLog_EnabledType * log, DataLog_ConsoleEnabledType * console);
DataLog_Result datalog_SetTaskOutputOptions(DataLog_TaskID task, DataLog_EnabledType log, DataLog_ConsoleEnabledType console);
DataLog_Result datalog_GetLevelOptions(DataLog_Handle handle, DataLog_EnabledType * log, DataLog_ConsoleEnabledType * console);
DataLog_Result datalog_SetLevelOptions(DataLog_Handle handle, DataLog_EnabledType log, DataLog_ConsoleEnabledType console);

/*
 * printf-like interface for data log output
 */
DataLog_Result datalog_Print(DataLog_Handle handle, const char * file, int line, const char * format, ...);
DataLog_Result datalog_PrintToDefault(const char * file, int line, const char * format, ...);

/*
 * periodic logging routines
 */
typedef struct DataLog_SetInfo * DataLog_Set;
#define NULL_DATALOG_SET   NULL
DataLog_Result datalog_CreatePeriodicSet(const char * setName, DataLog_Set * handle);

void datalog_AddPtr(DataLog_Set handle, void * ptr, size_t size, const char * key, const char * description);
void datalog_AddCharPtr(DataLog_Set handle, const char * const * ptr, const char * key, const char * description);
void datalog_AddIntFunc(DataLog_Set handle, int (* func)(void *), void * arg, const char * key, const char * description);
void datalog_AddLongFunc(DataLog_Set handle, long (* func)(void *), void * arg, const char * key, const char * description);
void datalog_AddDoubleFunc(DataLog_Set handle, double (* func)(void *), void * arg, const char * key, const char * description);
void datalog_AddCharPtrFunc(DataLog_Set handle, const char * (* func)(void *), void * arg, const char * key, const char * description);

DataLog_Result datalog_GetPeriodicOutputInterval(DataLog_Set handle, double * seconds);
DataLog_Result datalog_SetPeriodicOutputInterval(DataLog_Set handle, double seconds);
DataLog_Result datalog_ForcePeriodicOutput(DataLog_Set handle);

/*
 * error interface
 */
DataLog_ErrorType datalog_Error(DataLog_TaskID task);
const char * datalog_ErrorMessage(DataLog_ErrorType error);
void datalog_ClearError(DataLog_TaskID task);

typedef void DataLog_AppErrorHandler(const char * file, int line, DataLog_ErrorType error, const char * msg, int continuable);
DataLog_Result datalog_GetAppErrorHandler(DataLog_TaskID task, DataLog_AppErrorHandler ** func);
DataLog_Result datalog_SetAppErrorHandler(DataLog_TaskID task, DataLog_AppErrorHandler * func);

DataLog_Result datalog_GetBytesBuffered(size_t * byteCount);
DataLog_Result datalog_GetBytesWritten(size_t * byteCount);
DataLog_Result datalog_GetBytesMissed(size_t * byteCount);

#ifdef __cplusplus
}; // extern "C"
#endif /* ifdef __cplusplus */

/*
 * C++ only functions
 */
#ifdef __cplusplus

// #include "logstream.h"
#include <iostream.h>

class DataLog_Level
{
public:
   DataLog_Level(void);
   DataLog_Result initialize(const char * levelName);

	DataLog_Level(const char * levelName);

   virtual ~DataLog_Level();

	DataLog_EnabledType taskLogFlag(DataLog_TaskID task);
	DataLog_EnabledType taskLogFlag(DataLog_TaskID task, DataLog_EnabledType flag);

	DataLog_ConsoleEnabledType taskConsoleFlag(DataLog_TaskID task);
	DataLog_ConsoleEnabledType taskConsoleFlag(DataLog_TaskID task, DataLog_ConsoleEnabledType flag);

	DataLog_EnabledType globalLogFlag(void);
	DataLog_EnabledType globalLogFlag(DataLog_EnabledType flag);

	DataLog_ConsoleEnabledType globalConsoleFlag(void);
	DataLog_ConsoleEnabledType globalConsoleFlag(DataLog_ConsoleEnabledType flag);

   ostream & operator()(const char * fileName, int lineNumber);
};

class DataLog_DefaultLevel
{
public:
   static ostream & getStream(const char * fileName, int lineNumber);
};

#define endmsg endl

class DataLog_Critical : public DataLog_Level
{
public:
	DataLog_Critical(void);
	virtual ~DataLog_Critical();
};

#define DataLog(instance) (instance)(__FILE__, __LINE__)
#define DataLog_Default DataLog_DefaultLevel::getStream(__FILE__, __LINE__)

template<class Value> inline DataLog_Result datalog_AddRef(DataLog_Set handle, const Value& ref, const char * key, const char * description)
   {
      printf("datalog_AddRef key = %s, description = %s\n", key, description);
      return DataLog_OK;
   }

template<class Value, class Arg> inline DataLog_Result datalog_AddFunc(DataLog_Set handle, Value (* func)(const Arg& arg), const Arg& arg, const char * key, const char * description)
   {
      printf("datalog_AddFunc key = %s, description = %s\n", key, description);
      return DataLog_OK;
   }

/*
 * Specialized templates are defined for char pointers to allow them to be
 * handled correctly
 */
typedef char * DataLog_CharPtr;
template<> inline DataLog_Result datalog_AddRef<DataLog_CharPtr>(DataLog_Set handle, const DataLog_CharPtr& ref, const char * key, const char * description)
   {
      printf("datalog_AddRef(char *) key = %s, description = %s\n", key, description);
      return DataLog_OK;
   }

template<class Arg> inline DataLog_Result datalog_AddFunc(DataLog_Set handle, DataLog_CharPtr (* func)(const Arg& arg), const Arg& arg, const char * key, const char * description)
   {
      printf("datalog_AddFunc(char *) key = %s, description = %s\n", key, description);
      return DataLog_OK;
   }

typedef const char * DataLog_ConstCharPtr;
template<> inline DataLog_Result datalog_AddRef<DataLog_ConstCharPtr>(DataLog_Set handle, const DataLog_ConstCharPtr& ref, const char * key, const char * description)
   {
      printf("datalog_AddRef(const char *) key = %s, description = %s\n", key, description);
      return DataLog_OK;
   }

template<class Arg> inline DataLog_Result datalog_AddFunc(DataLog_Set handle, DataLog_ConstCharPtr (* func)(const Arg& arg), const Arg& arg, const char * key, const char * description)
   {
      printf("datalog_AddFunc(const char *) key = %s, description = %s\n", key, description);
      return DataLog_OK;
   }

#endif /* ifdef __cplusplus */
#endif /* ifndef _DATALOG_INCLUDE */

