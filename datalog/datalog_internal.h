/*
 * Copyright (C) 2002 Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: K:/BCT_Development/vxWorks/Common/datalog/rcs/datalog_internal.h 1.10 2003/10/03 12:35:02Z jl11312 Exp jl11312 $
 * $Log: datalog_internal.h $
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
#include <stdlib.h>
#include <string>
#include <strstream.h>

//
// level ID used for internal datalog records
//
#define	DATALOG_SYSTEM_LEVEL_ID		0xffff
#define	DATALOG_SYSTEM_LEVEL_NAME	"datalog_system"

class DataLog_Buffer
{
public:
	//
	// The data items in this struct must be shared between tasks.  They are
	// encapsulated in a single struct to make managing the shared memory for
	// them simpler.
	//
	struct SharedBufferData
	{
		DataLog_SharedPtr(DataLog_BufferData) _startBufferPtr;
		DataLog_SharedPtr(DataLog_BufferData) _endBufferPtr;
		DataLog_SharedPtr(DataLog_BufferData) _readPtr;
		DataLog_SharedPtr(DataLog_BufferData) _writePtr;

		size_t _bufferSize;
		size_t _bytesWritten;
		size_t _bytesMissed;

		DataLog_SharedPtr(SharedBufferData)	_prev;
		DataLog_SharedPtr(SharedBufferData)	_next;

		short _refCount;
   };

	DataLog_Buffer(void);
	virtual ~DataLog_Buffer();

	size_t size(void) { return (_data != DATALOG_NULL_SHARED_PTR) ? _data->_bufferSize : 0; }

	size_t bytesWritten(void) { return (_data != DATALOG_NULL_SHARED_PTR) ? _data->_bytesWritten : 0; }
	size_t bytesMissed(void) { return (_data != DATALOG_NULL_SHARED_PTR) ? _data->_bytesMissed : 0; }
	size_t bytesBuffered(void);

protected:
	//
	// These functions are platform specific, since they rely on the underlying
	// operating system to provide the means for sharing the associated data
	// items across all tasks.
	//
	static void lockBufferList(void);
	static void releaseBufferList(void);

	static DataLog_SharedPtr(SharedBufferData) getBufferListHead(void);
	static void setBufferListHead(DataLog_SharedPtr(SharedBufferData) head);

	static DataLog_SharedPtr(SharedBufferData) getBufferListTail(void);
	static void setBufferListTail(DataLog_SharedPtr(SharedBufferData) tail);

protected:
	DataLog_SharedPtr(SharedBufferData)	_data;
};

class DataLog_InputBuffer : public DataLog_Buffer
{
public:
	DataLog_InputBuffer(void) : DataLog_Buffer() { }
	virtual ~DataLog_InputBuffer() { }

	bool attachToFirstBuffer(void);
	bool attachToNextBuffer(void);

   size_t read(DataLog_BufferData * ptr, size_t maxSize);

private:
	void reattach(DataLog_SharedPtr(SharedBufferData) data);
};

class DataLog_OutputBuffer : public DataLog_Buffer
{
public:
	DataLog_OutputBuffer(size_t bufferSize);
	virtual ~DataLog_OutputBuffer();

	virtual size_t write(const DataLog_BufferData * ptr, size_t size);

	typedef DataLog_EnabledType NotifyStreamWriteComplete(const DataLog_BufferData * ptr, size_t size);
	virtual DataLog_Stream & streamWriteStart(NotifyStreamWriteComplete * callBack = NULL, size_t callBackArgSize = 0);
	virtual DataLog_Stream & streamWriteStartOrContinue(NotifyStreamWriteComplete * callBack, size_t callBackArgSize, bool & firstWrite);
	virtual void streamWriteReleaseToApp(void);
	virtual size_t streamWriteComplete(void);

protected:
	virtual void lockWriteAccess(void);
	virtual void releaseWriteAccess(void);
	virtual void addBufferToList(void);

protected:
	//
	// For normal output buffers, _writeStream is allocated on demand as a
	// dynamic string stream.  For critical output buffers, a static stream
	// is created by the constructor, since we don't want critical output
	// to require memory allocation.
	//
	DataLog_Stream * _writeStream;
	DataLog_BufferData * _streamWriteBuffer;
	NotifyStreamWriteComplete * _streamWriteCompleteCallBack;
	size_t _streamWriteCompleteCallBackArgSize;

private:
	bool	_streamWriteInProgress;
	bool	_streamWriteReleasedToApp;
	
#ifdef DATALOG_MULTITHREADED
	DataLog_Lock _writeLock;
#endif /* ifdef DATALOG_MULTITHREADED */
};

class DataLog_TraceBuffer : public DataLog_OutputBuffer
{
public:
	DataLog_TraceBuffer(size_t bufferSize);
	virtual ~DataLog_TraceBuffer() {}
};

#ifndef DATALOG_NO_NETWORK_SUPPORT
class DataLog_ClientBuffer : public DataLog_OutputBuffer
{
public:
	DataLog_ClientBuffer(size_t bufferSize);
	virtual ~DataLog_ClientBuffer() {}
};
#endif /* ifndef DATALOG_NO_NETWORK_SUPPORT */

class DataLog_PeriodicBuffer : public DataLog_OutputBuffer
{
public:
	DataLog_PeriodicBuffer(size_t bufferSize);
	virtual ~DataLog_PeriodicBuffer() {}
};

class DataLog_IntBuffer : public DataLog_OutputBuffer
{
public:
	DataLog_IntBuffer(size_t bufferSize);
	virtual ~DataLog_IntBuffer() {}

	virtual void lockWriteAccess(void) {}
	virtual void releaseWriteAccess(void) {}
};

class DataLog_CriticalBuffer : public DataLog_OutputBuffer
{
public:
	DataLog_CriticalBuffer(size_t bufferSize);
	virtual ~DataLog_CriticalBuffer() {}

	virtual size_t write(const DataLog_BufferData * ptr, size_t size);
};

struct DataLog_HandleInfo
{
	DataLog_InternalID _id;

	enum HandleType { TraceHandle, IntHandle, CriticalHandle, InvalidHandle };
	HandleType _type;

	struct TraceHandleData
	{
		DataLog_EnabledType _logOutput;
		DataLog_ConsoleEnabledType _consoleOutput;
	};

	struct IntHandleData
	{
		DataLog_EnabledType _logOutput;
		DataLog_IntBuffer * _buffer;
	};

	union
	{
		TraceHandleData    _traceData;
		IntHandleData      _intData;
	};
};

struct DataLog_TaskInfo
{
	DataLog_TaskID       		_id;
	DataLog_ErrorType          _error;

	DataLog_TraceBuffer *  		_trace;
	DataLog_EnabledType        _logOutput;
	DataLog_ConsoleEnabledType _consoleOutput;

	DataLog_CriticalBuffer *	_critical;

	DataLog_Level  _defaultLevel;
	DataLog_Handle	_defaultHandle;

	enum { MaxErrorRecursionLevel = 5 };
	DataLog_TaskErrorHandler * _errorHandler;
	short _errorActiveCount;
};

struct DataLog_SetInfo
{
  enum { BUFFER_SIZE_INC = 512 };
  unsigned int _id;

  double _logInterval;
  DataLog_List<DataLog_PeriodicItemBase *> _items;
  const char * _writeSignalName;
  const char * _modifiedSignalName;
  const char * _periodUpdateSignalName;
  
  DataLog_Lock _lock;
  DataLog_Lock _outputLock;
};

class DataLog_CommonData
{
public:
	DataLog_CommonData(void);
	virtual ~DataLog_CommonData() { }

	DataLog_TaskInfo * findTask(DataLog_TaskID task);
	void addTask(DataLog_TaskInfo * taskInfo);
	void deleteTask(DataLog_TaskID task);

	DataLog_Handle findHandle(const char * levelName);
	void addHandle(const char * levelName, DataLog_Handle handle);

	DataLog_TraceBuffer * getTaskTraceBuffer(DataLog_TaskID task);
	DataLog_CriticalBuffer * getTaskCriticalBuffer(DataLog_TaskID task);

	void setTaskError(DataLog_ErrorType error, const char * file, int line);
	DataLog_TaskErrorHandler * getTaskErrorHandler(DataLog_TaskID task);
	void setTaskErrorHandler(DataLog_TaskID task, DataLog_TaskErrorHandler * func);

	size_t getDefaultTraceBufferSize() { return _commonData->_defaultTraceBufferSize; }
	void setDefaultTraceBufferSize(size_t size) { _commonData->_defaultTraceBufferSize = size; }

	size_t getDefaultIntBufferSize() { return _commonData->_defaultIntBufferSize; }
	void setDefaultIntBufferSize(size_t size) { _commonData->_defaultIntBufferSize = size; }

	size_t getDefaultCriticalBufferSize() { return _commonData->_defaultCriticalBufferSize; }
	void setDefaultCriticalBufferSize(size_t size) { _commonData->_defaultCriticalBufferSize = size; }

	size_t getCurrentMaxBufferSize(void);

	//
	// All critical handles use this common handle information structure
	//
	static const struct DataLog_HandleInfo _criticalHandleInfo;

public:
	enum ConnectType { NotConnected, LogToFile, LogToNetwork };

	void setLocalConnect(const char * fileName);
	void setNetworkConnect(const char * ipAddress, int port);

	ConnectType connectType(void) { return _commonData->_connectType; }
	DataLog_SharedPtr(const char) connectName(void) { return _commonData->_connectName; }
	int connectPort(void) { return _commonData->_connectPort; }

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

	static DataLog_InternalID lookupLevelID(const char * levelName);
	static void registerLevelID(const char * levelName, DataLog_InternalID id);

private:
	struct CommonData
	{
		size_t	_defaultTraceBufferSize;
		size_t	_defaultIntBufferSize;
		size_t	_defaultCriticalBufferSize;

		ConnectType _connectType;
		DataLog_SharedPtr(const char) _connectName;
		int _connectPort;
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

	static DataLog_Map<DataLog_InternalID, DataLog_Handle> _handles;
	static DataLog_Lock _handlesLock;
};

enum DataLog_NetworkPacketType
{
	DataLog_NotifyBufferSize,
	DataLog_StartOutputRecord,
	DataLog_OutputRecordData,
	DataLog_EndOutputRecord,
	DataLog_EndConnection
};

struct DataLog_NetworkPacket
{
	DataLog_NetworkPacketType	_type __attribute__ ((packed));
	DataLog_UINT16					_length __attribute__ ((packed));
};

enum
{
	DataLog_HeaderRecordID = 0x5500,
	DataLog_LogLevelRecordID = 0x5501,
	DataLog_PrintOutputRecordID = 0x5502,
	DataLog_StreamOutputRecordID = 0x5503,
	DataLog_PeriodicOutputRecordID = 0x5504,
	DataLog_PeriodicSetRecordID = 0x5505,
	DataLog_PeriodicItemRecordID = 0x5506,
	DataLog_TaskCreateRecordID = 0x5507,
	DataLog_TaskDeleteRecordID = 0x5508,

	DataLog_EndOfNetworkOutputRecordID = 0x55fc,
	DataLog_FileCloseRecordID = 0x55fd,
	DataLog_WriteTimeRecordID = 0x55fe,
	DataLog_MissedDataRecordID = 0x55ff
};

/*
 *	All structures for log output must be packed to insure correct format
 */
struct DataLog_HeaderRecord
{
	DataLog_UINT16	_recordType __attribute__ ((packed));
	DataLog_UINT16	_charSize __attribute__ ((packed));
	DataLog_UINT16	_intSize __attribute__ ((packed));
	DataLog_UINT16	_longSize __attribute__ ((packed));
	DataLog_UINT16	_floatSize __attribute__ ((packed));
	DataLog_UINT16	_doubleSize __attribute__ ((packed));
	DataLog_UINT16	_taskIDSize __attribute__ ((packed));
	DataLog_TaskID	_currentTaskID __attribute__ ((packed));
	DataLog_UINT16	_nodeIDSize __attribute__ ((packed));
	DataLog_UINT16	_version __attribute__ ((packed));
};

struct DataLog_LogLevelRecord
{
	DataLog_UINT16	_recordType __attribute__ ((packed));
	DataLog_TimeStamp _timeStamp __attribute__ ((packed));
	DataLog_InternalID _levelID __attribute__ ((packed));
	
#ifndef DATALOG_NO_NETWORK_SUPPORT
	DataLog_NodeID _nodeID __attribute__ ((packed));
#endif /* ifndef DATALOG_NO_NETWORK_SUPPORT */

	DataLog_UINT16	_nameLen __attribute__ ((packed));
};

struct DataLog_PrintOutputRecord
{
	DataLog_UINT16	_recordType __attribute__ ((packed));
	DataLog_TimeStamp _timeStamp __attribute__ ((packed));
	DataLog_InternalID _levelID __attribute__ ((packed));
	DataLog_TaskID	_taskID __attribute__ ((packed));
	
#ifndef DATALOG_NO_NETWORK_SUPPORT
	DataLog_NodeID _nodeID __attribute__ ((packed));
#endif /* ifndef DATALOG_NO_NETWORK_SUPPORT */

	DataLog_UINT16	_formatLen __attribute__ ((packed));
	DataLog_UINT16	_fileNameLen __attribute__ ((packed));
	DataLog_UINT16 _lineNum __attribute__ ((packed));
};

struct DataLog_StreamOutputRecord
{
	DataLog_UINT16	_recordType __attribute__ ((packed));
	DataLog_TimeStamp _timeStamp __attribute__ ((packed));
	DataLog_InternalID _levelID __attribute__ ((packed));
	DataLog_TaskID	_taskID __attribute__ ((packed));
	
#ifndef DATALOG_NO_NETWORK_SUPPORT
	DataLog_NodeID _nodeID __attribute__ ((packed));
#endif /* ifndef DATALOG_NO_NETWORK_SUPPORT */

	DataLog_UINT16	_fileNameLen __attribute__ ((packed));
	DataLog_UINT16 _lineNum __attribute__ ((packed));
};

struct DataLog_PeriodicOutputRecord
{
	DataLog_UINT16	_recordType __attribute__ ((packed));
	DataLog_TimeStamp _timeStamp __attribute__ ((packed));
	DataLog_InternalID _setID __attribute__ ((packed));
	
#ifndef DATALOG_NO_NETWORK_SUPPORT
	DataLog_NodeID _nodeID __attribute__ ((packed));
#endif /* ifndef DATALOG_NO_NETWORK_SUPPORT */

	DataLog_UINT16 _itemCount __attribute__ ((packed));
};

struct DataLog_PeriodicSetRecord
{
	DataLog_UINT16	_recordType __attribute__ ((packed));
	DataLog_TimeStamp _timeStamp __attribute__ ((packed));
	DataLog_InternalID _setID __attribute__ ((packed));
	
#ifndef DATALOG_NO_NETWORK_SUPPORT
	DataLog_NodeID _nodeID __attribute__ ((packed));
#endif /* ifndef DATALOG_NO_NETWORK_SUPPORT */

	DataLog_UINT16	_nameLen __attribute__ ((packed));
};

struct DataLog_PeriodicItemRecord
{
	DataLog_UINT16	_recordType __attribute__ ((packed));
	DataLog_TimeStamp _timeStamp __attribute__ ((packed));
	DataLog_InternalID _keyCode __attribute__ ((packed));
	
#ifndef DATALOG_NO_NETWORK_SUPPORT
	DataLog_NodeID _nodeID __attribute__ ((packed));
#endif /* ifndef DATALOG_NO_NETWORK_SUPPORT */

	DataLog_UINT16	_keyLen __attribute__ ((packed));
	DataLog_UINT16	_descLen __attribute__ ((packed));
	DataLog_UINT16	_formatLen __attribute__ ((packed));
};

struct DataLog_TaskCreateRecord
{
	DataLog_UINT16	_recordType __attribute__ ((packed));
	DataLog_TimeStamp _timeStamp __attribute__ ((packed));
	DataLog_InternalID	_levelID __attribute__ ((packed));  
	DataLog_TaskID	_taskID __attribute__ ((packed));

#ifndef DATALOG_NO_NETWORK_SUPPORT
	DataLog_NodeID _nodeID __attribute__ ((packed));
#endif /* ifndef DATALOG_NO_NETWORK_SUPPORT */

	DataLog_UINT16	_nameLen __attribute__ ((packed));
};

struct DataLog_TaskDeleteRecord
{
	DataLog_UINT16	_recordType __attribute__ ((packed));
	DataLog_TimeStamp _timeStamp __attribute__ ((packed));
	DataLog_InternalID _levelID __attribute__ ((packed));
	DataLog_TaskID	_taskID __attribute__ ((packed));

#ifndef DATALOG_NO_NETWORK_SUPPORT
	DataLog_NodeID _nodeID __attribute__ ((packed));
#endif /* ifndef DATALOG_NO_NETWORK_SUPPORT */
};

struct DataLog_FileCloseRecord
{
	DataLog_UINT16	_recordType __attribute__ ((packed));
	DataLog_TimeStamp _timeStamp __attribute__ ((packed));	
};

struct DataLog_WriteTimeRecord
{
	DataLog_UINT16	_recordType __attribute__ ((packed));
	DataLog_TimeStamp _timeStamp __attribute__ ((packed));	
};

#endif /* ifndef _DATALOG_INTERNAL_INCLUDE */

