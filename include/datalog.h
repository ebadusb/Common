/*
 * Copyright (C) 2002 Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: K:/BCT_Development/vxWorks/Common/include/rcs/datalog.h 1.21 2003/02/25 20:40:08Z jl11312 Exp jl11312 $
 * $Log: datalog.h $
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
enum DataLog_Result { DataLog_OK, DataLog_Error };

DataLog_Result datalog_Init(const char * logPath);
DataLog_Result datalog_InitNet(const char * ipAddress, double seconds);

/*
 * Data log level creation
 */
typedef void * DataLog_Handle;
#define NULL_DATALOG_HANDLE   NULL

DataLog_Result datalog_CreateLevel(const char * levelName, DataLog_Handle * handle);

/*
 * Data log option control
 */
enum DataLog_EnabledType { LogEnabled, LogDisabled };
enum DataLog_ConsoleEnabledType { ConsoleEnabled, ConsoleDisabled };

DataLog_Result datalog_GetTaskLevelOptions(DataLog_TaskID task, DataLog_Handle handle, DataLog_EnabledType * log, DataLog_ConsoleEnabledType * console);
DataLog_Result datalog_SetTaskLevelOptions(DataLog_TaskID task, DataLog_Handle handle, DataLog_EnabledType log, DataLog_ConsoleEnabledType console);
DataLog_Result datalog_GetGlobalLevelOptions(DataLog_Handle handle, DataLog_EnabledType * log, DataLog_ConsoleEnabledType * console);
DataLog_Result datalog_SetGlobalLevelOptions(DataLog_Handle handle, DataLog_EnabledType log, DataLog_ConsoleEnabledType console);
DataLog_Result datalog_EnableIntPrint(DataLog_Handle handle);

/*
 * printf-like interface for data log output
 */
DataLog_Result datalog_Print(DataLog_Handle handle, const char * file, int line, const char * format, ...);
DataLog_Result datalog_IntPrint(DataLog_Handle handle, const char * file, int line, const char * msg);

/*
 * periodic logging routines
 */
void datalog_AddPtr(void * ptr, size_t size, const char * key, const char * description);
void datalog_AddCharPtr(const char * const * ptr, const char * key, const char * description);
void datalog_AddIntFunc(int (* func)(void *), void * arg, const char * key, const char * description);
void datalog_AddLongFunc(long (* func)(void *), void * arg, const char * key, const char * description);
void datalog_AddDoubleFunc(double (* func)(void *), void * arg, const char * key, const char * description);
void datalog_AddCharPtrFunc(const char * (* func)(void *), void * arg, const char * key, const char * description);

/*
 * error interface
 */
enum DataLog_ErrorType
{
	DataLog_NoError
};

DataLog_ErrorType datalog_Error(void);
const char * datalog_ErrorMessage(void);
void datalog_ClearError(void);

#ifdef __cplusplus
}; // extern "C"
#endif /* ifdef __cplusplus */

/*
 * C++ only functions
 */
#ifdef __cplusplus

#include "logstream.h"

class DataLog_Level
{
public:
   DataLog_Level(void);
   virtual ~DataLog_Level();
   DataLog_Result initialize(const char * levelName);

	DataLog_EnabledType taskLogFlag(DataLog_TaskID task);
	DataLog_EnabledType taskLogFlag(DataLog_TaskID task, DataLog_EnabledType flag);

	DataLog_ConsoleEnabledType taskConsoleFlag(DataLog_TaskID task);
	DataLog_ConsoleEnabledType taskConsoleFlag(DataLog_TaskID task, DataLog_ConsoleEnabledType flag);

	DataLog_EnabledType globalLogFlag(void);
	DataLog_EnabledType globalLogFlag(DataLog_EnabledType flag);

	DataLog_ConsoleEnabledType globalConsoleFlag(void);
	DataLog_ConsoleEnabledType globalConsoleFlag(DataLog_ConsoleEnabledType flag);

   ologstream & operator()(const char * fileName, int lineNumber);
};

#define DataLog(instance) (instance)(__FILE__, __LINE__)

template<class Value> DataLog_Result datalog_AddRef(const Value& ref, const char * key, const char * description);
template<class Value, class Arg> DataLog_Result datalog_AddFunc(Value (* func)(const Arg& arg), const Arg& arg, const char * key, const char * description);

template<class Value> DataLog_Result datalog_AddRef(const Value& ref, const char * key, const char * description)
   {
      printf("datalog_AddRef key = %s, description = %s\n", key, description);
      return DataLog_OK;
   }

template<class Value, class Arg> DataLog_Result datalog_AddFunc(Value (* func)(const Arg& arg), const Arg& arg, const char * key, const char * description)
   {
      printf("datalog_AddFunc key = %s, description = %s\n", key, description);
      return DataLog_OK;
   }

/*
 * Specialized templates are defined for char pointers to allow them to be
 * handled correctly
 */
typedef char * DataLog_CharPtr;
template<> DataLog_Result datalog_AddRef<DataLog_CharPtr>(const DataLog_CharPtr& ref, const char * key, const char * description)
   {
      printf("datalog_AddRef(char *) key = %s, description = %s\n", key, description);
      return DataLog_OK;
   }

template<class Arg> DataLog_Result datalog_AddFunc(DataLog_CharPtr (* func)(const Arg& arg), const Arg& arg, const char * key, const char * description)
   {
      printf("datalog_AddFunc(char *) key = %s, description = %s\n", key, description);
      return DataLog_OK;
   }

typedef const char * DataLog_ConstCharPtr;
template<> DataLog_Result datalog_AddRef<DataLog_ConstCharPtr>(const DataLog_ConstCharPtr& ref, const char * key, const char * description)
   {
      printf("datalog_AddRef(const char *) key = %s, description = %s\n", key, description);
      return DataLog_OK;
   }

template<class Arg> DataLog_Result datalog_AddFunc(DataLog_ConstCharPtr (* func)(const Arg& arg), const Arg& arg, const char * key, const char * description)
   {
      printf("datalog_AddFunc(const char *) key = %s, description = %s\n", key, description);
      return DataLog_OK;
   }

#endif /* ifdef __cplusplus */
#endif /* ifndef _DATALOG_INCLUDE */

