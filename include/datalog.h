/*
 * Copyright (C) 2002 Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: K:/BCT_Development/vxWorks/Common/include/rcs/datalog.h 1.21 2003/02/25 20:40:08Z jl11312 Exp jl11312 $
 * $Log: datalog.h $
 * Revision 1.17  2002/10/28 14:33:07  jl11312
 * - moved include for iomanip.h to C++ only section
 * Revision 1.16  2002/10/25 16:56:13  jl11312
 * - added new form of errnoMsg stream manipulator which takes an argument for errno
 * Revision 1.15  2002/09/23 13:54:46  jl11312
 * - added access function for current log file name
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
#define	DATALOG_MAJOR_VERSION	(2)
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

#include <string>

/*
 * The DataLog_Stream class supports only two types of manipulators:
 * those taking no arguments and those taking a single integer argument.
 * More general support using templates similar to that provided by
 * iomanip.h for the C++ streams library could be added in the future if
 * necessary.
 */
class DataLog_Stream;
typedef DataLog_Stream & (* DataLog_StreamManip)(DataLog_Stream &);

class DataLog_StreamIManip
{
	int _value;
	DataLog_Stream & (* _func)(DataLog_Stream &, int);

public:
	DataLog_StreamIManip(DataLog_Stream & (* func)(DataLog_Stream &, int), int value)
	  : _func(func), _value(value) { }

	friend DataLog_Stream & operator << (DataLog_Stream & stream, const DataLog_StreamIManip & manip);
};

inline DataLog_Stream & operator << (DataLog_Stream & stream, const DataLog_StreamIManip & manip)
{
	return (*manip._func)(stream, manip._value); 
}

class DataLog_Stream
{
	friend class DataLog_OutputBuffer;
	friend class DataLog_CriticalBuffer;
	friend DataLog_Stream & endmsg(DataLog_Stream & stream);

public:
	virtual ~DataLog_Stream();

//	DataLog_Stream & write(const void * data, size_t size) 	{ writeStringArg(String, data, size); return *this; }

	DataLog_Stream & operator << (char c);
	DataLog_Stream & operator << (signed char c) { return operator<< ((char)c); }
	DataLog_Stream & operator << (unsigned char c) { return operator<< ((char)c); }
	DataLog_Stream & operator << (short val) { return operator<< ((int)val); }
	DataLog_Stream & operator << (unsigned short val) { return operator<< ((unsigned int)val); }
	DataLog_Stream & operator << (int val);
	DataLog_Stream & operator << (unsigned int val);
	DataLog_Stream & operator << (long val);
	DataLog_Stream & operator << (unsigned long val);
	DataLog_Stream & operator << (bool val);
	DataLog_Stream & operator << (const char * s);
	DataLog_Stream & operator << (const signed char * s) { return operator<< ((const char *)s); }
	DataLog_Stream & operator << (const unsigned char * s) { return operator<< ((const char *)s); }
	DataLog_Stream & operator << (const string & s) { return operator<< (s.c_str()); }
	DataLog_Stream & operator << (float val);
	DataLog_Stream & operator << (double val);
	DataLog_Stream & operator << (const void * ptr) { return operator<< ((unsigned long)ptr); }

	DataLog_Stream & operator << (DataLog_StreamManip func) { return (*func)(*this); }

	enum Flag
	{
		// General numeric format control
		f_showpos = 0x0001,		// print explicit '+' on output of positive values
 
		// Integer format control
		f_dec = 0x0010,			// output integers as decimal values
		f_hex = 0x0020,			// output integers as hexadecimal values
		f_oct = 0x0040,			// output integers as octal values
		f_basemask = 0x0070,		// mask for output base flags
		f_showbase = 0x0100,		// show integer output base (e.g. 0x for hex)

		// Floating point format control
		f_showpoint = 0x1000,	// show trailing zeroes after decimal point
		f_scientific = 0x2000,	// scientific notation (e.g. 1.234E02)
		f_fixed = 0x4000,			// fixed point notation (e.g. 123.4)
		f_floatmask = 0x6000,	// mask for floating point format flags

		// Default flag setting
		f_defaultflags = f_dec | f_fixed
	};

	void flags(unsigned int flagSetting) { _flags = flagSetting; _flagsChanged = true; }
	void setFlags(unsigned int flagSetting);
	void resetFlags(unsigned int flagSetting);
	void precision(unsigned int precSetting) { _precision = precSetting; _precisionChanged = true; }

protected:
	DataLog_Stream(DataLog_OutputBuffer * output);
	void setLogOutput(DataLog_EnabledType logOutput) { _logOutput = logOutput; }
	void setConsoleOutput(DataLog_ConsoleEnabledType consoleOutput) { _consoleOutput = consoleOutput; }

	void rawWrite(const void * data, size_t size);

	size_t pcount(void) { return _bufferPos; }
	void clear(void) { _bufferPos = 0; _fail = false; _flagsChanged = false; _precisionChanged = false; }
	void seekp(size_t pos) { _bufferPos = (pos < _bufferSize) ? pos : _bufferSize; }
	const void * data(void) { return _buffer; }
	bool fail(void) { bool retVal = _fail; _fail = false; return retVal; }

	DataLog_UINT16 getFlags(void) { return _flags; }
	DataLog_UINT8	getPrecision(void) { return _precision; }

private:
	enum ArgumentType
	{
		SignedChar = 1,
		UnsignedChar = 2,
		SignedInt = 3,
		UnsignedInt = 4,
		SignedLong = 5,
		UnsignedLong = 6,
		String = 7,
		Float = 8,
		Double = 9,
		Bool = 10,
		FlagSetting = 100,
		PrecisionSetting = 101
	};

	void writeArg(ArgumentType type, const void * data, DataLog_UINT16 size);
	void writeStringArg(ArgumentType type, const void * data, DataLog_UINT16 size);
	void printLong(long val);
	void printUnsignedLong(unsigned long val);
	void printDouble(double val);

private:
	DataLog_OutputBuffer * _output;
	DataLog_EnabledType _logOutput;
	DataLog_ConsoleEnabledType _consoleOutput;

	size_t _bufferSize;
	size_t _bufferPos;
	DataLog_UINT16 _flags;
	DataLog_UINT8  _precision; 
	bool _fail;
	bool _flagsChanged, _precisionChanged;
	
	DataLog_BufferData * _buffer;
};

/*
 *	DataLog_Stream manipulators
 */
DataLog_Stream & endmsg(DataLog_Stream & stream);

DataLog_Stream & manipfunc_setprecision(DataLog_Stream & stream, int param);
DataLog_Stream & manipfunc_setflags(DataLog_Stream & stream, int param);
DataLog_Stream & manipfunc_resetflags(DataLog_Stream & stream, int param);

inline DataLog_StreamIManip precision(int param) { return DataLog_StreamIManip(manipfunc_setprecision, param); }
inline DataLog_StreamIManip setflags(int param) { return DataLog_StreamIManip(manipfunc_setflags, param); }
inline DataLog_StreamIManip resetflags(int param) { return DataLog_StreamIManip(manipfunc_resetflags, param); }

inline DataLog_Stream & hex(DataLog_Stream & stream) { stream << setflags(DataLog_Stream::f_hex); return stream; }
inline DataLog_Stream & dec(DataLog_Stream & stream) { stream << setflags(DataLog_Stream::f_dec); return stream; }

DataLog_Stream & manipfunc_errnoMsg(DataLog_Stream & stream, int param);
inline DataLog_StreamIManip errnoMsg(int param) { return DataLog_StreamIManip(manipfunc_errnoMsg, param); }
inline DataLog_Stream & errnoMsg(DataLog_Stream & stream) { return manipfunc_errnoMsg(stream, errno); }

DataLog_Stream & datalog_GetDefaultStream(const char * file, int line);

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

	enum State { WaitStart, WaitEnd };
	State	_state;

	bool _isExiting;
	int  _exitCode;
};

#endif /* ifdef __cplusplus */

#include "datalog_private.h"

#endif /* ifndef _DATALOG_INCLUDE */

