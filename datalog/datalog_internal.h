/*
 * Copyright (C) 2002 Gambro BCT, Inc.  All rights reserved.
 *
 * $Header$
 * $Log: datalog_internal.h $
 * Revision 1.13  2004/10/26 20:19:01Z  rm70006
 * Ported datalog code to be compatible with windows compiler.  No functional changes made.  Re-ran unit test and it passed.
 * Revision 1.12  2003/12/09 14:14:23Z  jl11312
 * - corrected time stamp problem (IT 6668)
 * - removed obsolete code/data types (IT 6664)
 * Revision 1.11  2003/11/10 17:46:12Z  jl11312
 * - corrections from data log unit tests (see IT 6598)
 * Revision 1.10  2003/10/03 12:35:02Z  jl11312
 * - improved DataLog_Handle lookup time
 * - modified datalog signal handling to eliminate requirement for a name lookup and the semaphore lock/unlock that went with it
 * Revision 1.9  2003/02/25 16:10:13Z  jl11312
 * - modified buffering scheme to help prevent buffer overruns
 * Revision 1.8  2003/02/06 20:41:30  jl11312
 * - added support for binary record type
 * - added support for symbolic node names in networked configurations
 * - enabled compression/encryption of log files
 * Revision 1.7  2003/01/31 19:52:50  jl11312
 * - new stream format for datalog
 * Revision 1.6  2002/10/08 14:43:02  jl11312
 * - added code to handle case for application saving a reference to a data log stream and performing multiple message writes
 * Revision 1.5  2002/09/19 21:25:59  jl11312
 * - modified stream functions to not reset stream state when two stream writes occur without endmsg in between
 * - added errnoMsg manipulator function
 * Revision 1.4  2002/08/28 14:37:07  jl11312
 * - changed handling of critical output to avoid problem with handles referencing deleted tasks
 * Revision 1.3  2002/08/22 20:19:11  jl11312
 * - added network support
 * Revision 1.2  2002/08/15 20:53:55  jl11312
 * - added support for periodic logging
 * Revision 1.1  2002/07/18 21:20:53  jl11312
 * Initial revision
 *
 */

#ifndef _DATALOG_INTERNAL_INCLUDE
#define _DATALOG_INTERNAL_INCLUDE

#include "datalog.h"
#include "datalog_periodic.h"
#include <stdlib.h>
#include <string>

//
// level ID used for internal datalog records
//
#define	DATALOG_SYSTEM_LEVEL_ID		0xffff
#define	DATALOG_SYSTEM_LEVEL_NAME	"datalog_system"

enum { DataLog_BufferSize = 256 };

struct DataLog_Buffer
{
	DataLog_BufferPtr _next;		// pointer to next buffer in list

	DataLog_BufferPtr _tail;		// pointer to tail of chain (valid only for first buffer in chain)
	size_t _length;					// valid only for first buffer in chain

	DataLog_BufferData _data[DataLog_BufferSize];
};

class DataLog_BufferManager
{
	friend class Test_DataLog_BufferManager;

public:
	//
	// Platform specific routines
	//
	static void initialize(size_t bufferSizeKBytes);

	static DataLog_BufferPtr getFreeBuffer(unsigned long reserveBuffers);
	static bool getNextChain(DataLog_BufferChain & chain, bool * isCritical = NULL);

	enum BufferList { TraceList, CriticalList, FreeList };
	static void addChainToList(BufferList list, const DataLog_BufferChain & chain);

	static unsigned long currentBufferCount(BufferList list);
	static unsigned long	bytesWritten(BufferList list);
	static unsigned long bytesMissed(BufferList list);

#ifdef DATALOG_BUFFER_STATISTICS
	static unsigned long minBufferCount(BufferList list);
	static unsigned long maxBufferCount(BufferList list);
	static unsigned long avgBufferCount(BufferList list);
#endif /* ifdef DATALOG_BUFFER_STATISTICS */

public:
	//
	// Non platform-specific routines
	//
	static bool createChain(DataLog_BufferChain & chain, unsigned long reserveBuffers=0);
	static bool copyChain(DataLog_BufferChain & dest, const DataLog_BufferChain & source);
	static bool writeToChain(DataLog_BufferChain & chain, DataLog_BufferData * data, size_t size);

	static void modifyChainData(DataLog_BufferChain & chain, unsigned long offset, DataLog_BufferData * data, size_t size);

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

private:
	static DataLog_BufferList * getInternalList(BufferList list);
};

struct DataLog_HandleInfo
{
	DataLog_InternalID _id;

	enum HandleType { TraceHandle, IntHandle, CriticalHandle, InvalidHandle };
	HandleType _type;

	DataLog_EnabledType _logOutput;
	DataLog_ConsoleEnabledType _consoleOutput;
};

struct DataLog_TaskInfo
{
	DataLog_TaskID       		_id;
	DataLog_ErrorType          _error;

	DataLog_EnabledType        _logOutput;
	DataLog_ConsoleEnabledType _consoleOutput;

	DataLog_Level  _defaultLevel;
	DataLog_Handle	_defaultHandle;

	enum { MaxErrorRecursionLevel = 5 };
	DataLog_TaskErrorHandler * _errorHandler;
	short _errorActiveCount;

	DataLog_Map<DataLog_InternalID, DataLog_Stream *> _outputStream;
};


struct DataLog_SetInfo
{
  unsigned int _id;

  long _logIntervalMilliSec;
  DataLog_List<DataLog_PeriodicItemBase *> _items;
  DataLog_SignalInfo * _writeSignal;
  DataLog_SignalInfo * _modifiedSignal;
  DataLog_SignalInfo * _periodUpdateSignal;
  
  DataLog_Lock _lock;
  DataLog_Lock _outputLock;

  bool _writeAllItems;
};

class DataLog_CommonData
{
public:
	DataLog_CommonData(void);
	virtual ~DataLog_CommonData() { }

	DataLog_TaskInfo * findTask(DataLog_TaskID task);
	void addTask(DataLog_TaskInfo * taskInfo);
	void deleteTask(DataLog_TaskID task);

	DataLog_Handle findHandle(const char * handleName);
	void addHandle(const char * handleName, DataLog_Handle handle);

	void setTaskError(DataLog_ErrorType error, const char * file, int line);
	DataLog_TaskErrorHandler * getTaskErrorHandler(DataLog_TaskID task);
	void setTaskErrorHandler(DataLog_TaskID task, DataLog_TaskErrorHandler * func);

	static const DataLog_HandleInfo _criticalHandleInfo;

public:
	enum ConnectType { NotConnected, LogToFile, LogToNetwork };

	void setLocalConnect(const char * fileName);
	void setNetworkConnect(const char * ipAddress, int port);
	void setCriticalReserveBuffers(unsigned long criticalReserveBuffers) { _commonData->_criticalReserveBuffers = criticalReserveBuffers; }
	void setPersistSystemInfo(bool flag);
	void setPlatformName(const char * platformName);
	void setPlatformInfo(const char * platformInfo);
	void setNodeName(const char * nodeName);

	ConnectType connectType(void) { return _commonData->_connectType; }
	DataLog_SharedPtr(const char) connectName(void) { return _commonData->_connectName; }
	int connectPort(void) { return _commonData->_connectPort; }
	unsigned long criticalReserveBuffers(void) { return _commonData->_criticalReserveBuffers; }
	bool persistSystemInfo(void) { return _commonData->_persistSystemInfo; }
	DataLog_BufferChain &systemInfoChain(void) { return _commonData->_persistedSystemInfoChain; }
	DataLog_SharedPtr(const char) platformName(void) { return _commonData->_platformName; }
	DataLog_SharedPtr(const char) platformInfo(void) { return _commonData->_platformInfo; }
	DataLog_SharedPtr(const char) nodeName(void) { return _commonData->_nodeName; }

	void outputSystemInfo(void);
	
public:
	//
	// The following functions are platform dependent, generally because they
	// must provide protected access to shared data among all tasks.  In some
	// cases, they are functions which are called frequently, and are included
   // in this set of functions so that they can take advantage of platform
   // specific features to improve execution time.
   //
	static bool startInitialization(void);
	static bool isInitialized(void);

	static DataLog_InternalID getNextInternalID(void);

private:
	struct CommonData
	{
		ConnectType _connectType;
		DataLog_SharedPtr(const char) _connectName;
		int _connectPort;

		unsigned long _criticalReserveBuffers;

		DataLog_SharedPtr(const char) _platformName;
		DataLog_SharedPtr(const char) _platformInfo;
		DataLog_SharedPtr(const char) _nodeName;
		bool _persistSystemInfo;
		DataLog_BufferChain _persistedSystemInfoChain;
	};

	static void initializeCommonData(DataLog_SharedPtr(CommonData) data);
	DataLog_SharedPtr(CommonData)	_commonData;

private:
	//
	// This function is platform specific.  It needs to provide a pointer to a
	// single data area (see the definition for the struct CommonData) shared
	// among all tasks and set the member data _commonData appropriately.  On
	// the first call to this function, the data area should be allocated and
	// the member function initializeCommonData called to initialize it.  The
	// function must insure that access to the common data area is locked
	// appropriately (i.e. more than one task can not be in the
	// allocate/initialize code section).
	//
	void setCommonDataPtr(void);
 
private:
	static DataLog_Map<DataLog_TaskID, DataLog_TaskInfo *> _tasks;
	static DataLog_Lock _tasksLock;
};

enum DataLog_NetworkPacketType
{
	DataLog_StartTraceOutputRecord,
	DataLog_StartCriticalOutputRecord,
	DataLog_OutputRecordData,
	DataLog_EndOutputRecord,
	DataLog_EndConnection
};

Packing_Structure_Directive struct DataLog_NetworkPacket
{
	DataLog_NetworkPacketType	_type Packing_Element_Directive;
	DataLog_UINT16				_length Packing_Element_Directive;
};

#endif /* ifndef _DATALOG_INTERNAL_INCLUDE */

