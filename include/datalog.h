/*
 * Copyright (C) 2002 Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: K:/BCT_Development/vxWorks/Common/include/rcs/datalog.h 1.21 2003/02/25 20:40:08Z jl11312 Exp jl11312 $
 * $Log: datalog.h $
 * Revision 1.14  2002/09/19 21:26:37  jl11312
 * - added errnoMsg manipulator function
 * Revision 1.13  2002/08/28 14:36:40  jl11312
 * - changed handling of critical output to avoid problem with handles referencing deleted tasks
 * Revision 1.12  2002/08/22 20:18:26  jl11312
 * - added network support
 * Revision 1.11  2002/08/15 20:54:52  jl11312
 * - added support for periodic logging
 * Revision 1.10  2002/07/18 21:20:05  jl11312
 * - added support for default log levels
 * Revision 1.9  2002/07/17 20:31:51  jl11312
 * - initial datalog implementation (no support for periodic logging)
 * Revision 1.8  2002/06/04 20:23:48  jl11312
 * - added default level related functions
 * - modified to compile correctly with C-source files
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

#include <in.h>
#include <inetLib.h>
#include <sockLib.h>
#include <stdio.h>
#include <stdlib.h>

/*
 *	Version information
 */
#define	DATALOG_MAJOR_VERSION	(1)
#define	DATALOG_MINOR_VERSION	(1)

/*
 *	Datalog related data types
 */
typedef enum { DataLog_OK, DataLog_Error } DataLog_Result;
typedef enum { DataLog_LogEnabled, DataLog_LogDisabled } DataLog_EnabledType;
typedef enum { DataLog_ConsoleEnabled, DataLog_ConsoleDisabled } DataLog_ConsoleEnabledType;

typedef const struct DataLog_HandleInfo *	DataLog_Handle;
typedef struct DataLog_SetInfo *		DataLog_SetHandle;

#define DATALOG_NULL_HANDLE NULL
#define DATALOG_NULL_SET_HANDLE NULL
 
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
DataLog_Result datalog_Init(const char * logPath, const char * platformName);
DataLog_Result datalog_InitNet(const char * ipAddress, int port, long connectTimeout);

DataLog_Result datalog_SetDefaultTraceBufferSize(size_t size);
DataLog_Result datalog_SetDefaultIntBufferSize(size_t size);
DataLog_Result datalog_SetDefaultCriticalBufferSize(size_t size);

typedef DataLog_BufferData * DataLog_EncryptFunc(DataLog_BufferData input, size_t inputLength, size_t * outputLength);
DataLog_Result datalog_SetEncryptFunc(DataLog_EncryptFunc * func);

DataLog_Result datalog_GetCurrentLogFileName(char * fileName, int bufferLength);

/*
 * Data log handle routines
 */
DataLog_Result datalog_CreateLevel(const char * levelName, DataLog_Handle * handle);
DataLog_Result datalog_CreateIntLevel(const char * levelName, DataLog_Handle * handle);
DataLog_Handle datalog_GetCriticalHandle(void);
DataLog_Result	datalog_SetDefaultLevel(DataLog_Handle handle);

/*
 * Data log option control
 */
DataLog_Result datalog_GetTaskOutputOptions(DataLog_TaskID task, DataLog_EnabledType * log, DataLog_ConsoleEnabledType * console);
DataLog_Result datalog_SetTaskOutputOptions(DataLog_TaskID task, DataLog_EnabledType log, DataLog_ConsoleEnabledType console);
DataLog_Result datalog_GetLevelOutputOptions(DataLog_Handle handle, DataLog_EnabledType * log, DataLog_ConsoleEnabledType * console);
DataLog_Result datalog_SetLevelOutputOptions(DataLog_Handle handle, DataLog_EnabledType log, DataLog_ConsoleEnabledType console);

/*
 * printf-like interface for data log output
 */
DataLog_Result datalog_Print(DataLog_Handle handle, const char * file, int line, const char * format, ...);
DataLog_Result datalog_PrintToDefault(const char * file, int line, const char * format, ...);

/*
 * periodic logging routines
 */
DataLog_Result datalog_CreatePeriodicSet(const char * setName, DataLog_SetHandle * handle);

void datalog_AddPtr(DataLog_SetHandle handle, void * ptr, size_t size, const char * key, const char * description);
void datalog_AddCharPtr(DataLog_SetHandle handle, const char * const * ptr, const char * key, const char * description);
void datalog_AddIntFunc(DataLog_SetHandle handle, int (* func)(void *), void * arg, const char * key, const char * description);
void datalog_AddLongFunc(DataLog_SetHandle handle, long (* func)(void *), void * arg, const char * key, const char * description);
void datalog_AddDoubleFunc(DataLog_SetHandle handle, double (* func)(void *), void * arg, const char * key, const char * description);
void datalog_AddCharPtrFunc(DataLog_SetHandle handle, const char * (* func)(void *), void * arg, const char * key, const char * description);

DataLog_Result datalog_GetPeriodicOutputInterval(DataLog_SetHandle handle, double * seconds);
DataLog_Result datalog_SetPeriodicOutputInterval(DataLog_SetHandle handle, double seconds);
DataLog_Result datalog_ForcePeriodicOutput(DataLog_SetHandle handle);
DataLog_Result datalog_DisablePeriodicOutput(DataLog_SetHandle handle);
DataLog_Result datalog_EnablePeriodicOutput(DataLog_SetHandle handle);

/*
 * error interface
 */
DataLog_Result datalog_Error(DataLog_TaskID task, DataLog_ErrorType * error);
DataLog_Result datalog_ClearError(DataLog_TaskID task);
const char * datalog_ErrorMessage(DataLog_ErrorType error);
int datalog_ErrorContinuable(DataLog_ErrorType error);

typedef void DataLog_TaskErrorHandler(const char * file, int line, DataLog_ErrorType error, const char * msg, int continuable);
DataLog_Result datalog_GetTaskErrorHandler(DataLog_TaskID task, DataLog_TaskErrorHandler ** func);
DataLog_Result datalog_SetTaskErrorHandler(DataLog_TaskID task, DataLog_TaskErrorHandler * func);

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

#include <strstream.h>

class DataLog_OutputBuffer;
class DataLog_Stream : public ostrstream
{
	friend class DataLog_OutputBuffer;
	friend class DataLog_CriticalBuffer;
	friend ostream & endmsg(ostream & stream);

private:
	DataLog_Stream(DataLog_OutputBuffer * output);
	DataLog_Stream(DataLog_BufferData * buffer, size_t bufferSize, DataLog_OutputBuffer * output);
	virtual ~DataLog_Stream() {}

	DataLog_OutputBuffer * _output;
};

class DataLog_Level
{
public:
   DataLog_Level(void);
   DataLog_Result initialize(const char * levelName);

	DataLog_Level(const char * levelName);

   virtual ~DataLog_Level();

	DataLog_EnabledType logOutput(void);
	DataLog_EnabledType logOutput(DataLog_EnabledType flag);

	DataLog_ConsoleEnabledType consoleOutput(void);
	DataLog_ConsoleEnabledType consoleOutput(DataLog_ConsoleEnabledType flag);

	DataLog_Stream & operator()(const char * fileName, int lineNumber);

	DataLog_Handle getHandle(void) { return _handle; }
	DataLog_Result setHandle(DataLog_Handle handle) { _handle = handle; return DataLog_OK; }
	DataLog_Result setAsDefault(void);

protected:
	DataLog_Handle	_handle;
};

ostream & endmsg(ostream & stream);
ostream & errnoMsg(ostream & stream);
ostream & datalog_GetDefaultStream(const char * file, int line);

class DataLog_Critical : public DataLog_Level
{
public:
	DataLog_Critical(void);
	virtual ~DataLog_Critical();
};

#define DataLog(instance) (instance)(__FILE__, __LINE__)
#define DataLog_Default datalog_GetDefaultStream(__FILE__, __LINE__)

class DataLog_OutputTask
{
public:
	DataLog_OutputTask(void);
	virtual ~DataLog_OutputTask() {}

	int main(void);
	void pause(void) { _isRunning = false; }
	void resume(void) { _isRunning = true; }
	void exit(int code);

protected:
	virtual void handleBufferSizeChange(size_t size) = 0;
	virtual void writeOutputRecord(DataLog_BufferData * buffer, size_t size) = 0;
	virtual void shutdown(void) = 0;

	void writeSystemLevelRecord(void);
	void writeMissedLogDataRecord(void);
	void writeTimeStampRecord(void);

protected:
	bool _isRunning;
	bool _isExiting;
	int  _exitCode;
	int _outputFD;
};

class DataLog_LocalOutputTask : public DataLog_OutputTask
{
public:
	DataLog_LocalOutputTask(const char * platformName);
	virtual ~DataLog_LocalOutputTask() {}

protected:
	virtual void handleBufferSizeChange(size_t size);
	virtual void writeOutputRecord(DataLog_BufferData * buffer, size_t size);
	virtual void shutdown(void);

	void writeLogFileHeader(const char * platformName);
	void writeFileCloseRecord(void);
};

class DataLog_NetworkOutputTask : public DataLog_OutputTask
{
public:
	DataLog_NetworkOutputTask(long connectTimeout);
	virtual ~DataLog_NetworkOutputTask() {}

protected:
	virtual void handleBufferSizeChange(size_t size);
	virtual void writeOutputRecord(DataLog_BufferData * buffer, size_t size);
	virtual void shutdown(void);

	void writeConnectionEndRecord(void);
};

class DataLog_PeriodicTask
{
public:
	DataLog_PeriodicTask(DataLog_SetHandle set);
	virtual ~DataLog_PeriodicTask() {}

	int main(void);
	void pause(void) { _isRunning = false; }
	void resume(void) { _isRunning = true; }
	void exit(int code);

private:
	void shutdown(void);

private:
	bool _isRunning;
	bool _isExiting;
	int  _exitCode;

	DataLog_SetHandle	_set;
};

class DataLog_NetworkClientTask;
class DataLog_NetworkTask
{
public:
	DataLog_NetworkTask(int port);
	virtual ~DataLog_NetworkTask() {}

	int main(void);
	void exit(int code);

private:
	void createClientTask(int clientSocket, struct sockaddr_in * clientAddr);

	DataLog_List<DataLog_NetworkClientTask *> _clientTaskList;
	bool _isExiting;
	int  _exitCode;
	int  _port; 
};

class DataLog_ClientBuffer;
struct DataLog_NetworkPacket;
class DataLog_NetworkClientTask
{
public:
	DataLog_NetworkClientTask(int clientSocket, struct sockaddr_in * clientAddr);
	virtual ~DataLog_NetworkClientTask() {}

	int main(void);
	void exit(int code);

public:
	enum { MaxDataSize = 1024 };

private:
	void handlePacket(const DataLog_NetworkPacket & packet);
	void processBufferSizeRecord(DataLog_UINT16 packetLength);
	void processInvalidPacket(const DataLog_NetworkPacket & packet);
	bool readData(DataLog_BufferData * buffer, size_t size);

private:
	int _clientSocket;
	char	_asciiAddr[INET_ADDR_LEN];

	DataLog_ClientBuffer * _clientBuffer;
	DataLog_BufferData _tempBuffer[MaxDataSize];
	DataLog_Stream * _dataStream;

	enum State { WaitStart, WaitEnd };
	State	_state;

	bool _isExiting;
	int  _exitCode;
};

#endif /* ifdef __cplusplus */

#include "datalog_private.h"

#endif /* ifndef _DATALOG_INCLUDE */

