/*
 * Copyright (C) 2002 Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: K:/BCT_Development/vxWorks/Common/datalog/rcs/datalog_internal.h 1.10 2003/10/03 12:35:02Z jl11312 Exp jl11312 $
 * $Log: datalog_internal.h $
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

struct DataLog_StreamOutputRecord;
class DataLog_OutputBuffer
{
public:
	DataLog_OutputBuffer(size_t bufferSize);
	virtual ~DataLog_OutputBuffer();

	size_t size(void) { return _bufferSize; }
	virtual size_t read(DataLog_BufferData * ptr, size_t maxSize);
	virtual size_t write(DataLog_BufferData * ptr, size_t size);

	virtual void partialWriteStart(void);
	virtual void partialWriteComplete(void);
	virtual size_t partialWrite(DataLog_BufferData * ptr, size_t size);

	size_t bytesWritten(void) { return _bytesWritten; }
	size_t bytesMissed(void) { return _bytesMissed; }
	size_t bytesBuffered(void);

	DataLog_Stream & lockStreamBuffer(const DataLog_StreamOutputRecord * streamOutputRecord, const char * fileName, DataLog_EnabledType logOutput, DataLog_ConsoleEnabledType consoleOutput);
	void releaseStreamBuffer(void);

protected:
	virtual void doWrite(DataLog_BufferData * ptr, size_t bytesWritten);
	virtual void lockWriteAccess(void);
	virtual void releaseWriteAccess(void);

private:
	typedef DataLog_SharedPtr(DataLog_BufferData) SharedBufferPtr;

	SharedBufferPtr _startBufferPtr;
	SharedBufferPtr _endBufferPtr;
	SharedBufferPtr _readPtr;
	SharedBufferPtr _writePtr;

	bool	_partialWriteInProgress;
	bool	_partialWriteOverflow;
	SharedBufferPtr _partialWriteStartPtr;
	size_t _partialWriteBytesWritten;

	size_t _bufferSize;
	size_t _bytesWritten;
	size_t _bytesMissed;

	DataLog_Stream * _stream;
	DataLog_BufferData * _streamBuffer;
	size_t _streamBufferSize;
	bool _streamBufferInUse;
	DataLog_EnabledType _streamLogOutput;
	DataLog_ConsoleEnabledType _streamConsoleOutput;

#ifdef DATALOG_MULTITHREADED
	DataLog_Lock _lock;
	DataLog_Lock _streamLock;
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

	virtual size_t write(DataLog_BufferData * ptr, size_t size);
	virtual void partialWriteComplete(void);
};

struct DataLog_HandleInfo
{
  enum HandleType { TraceHandle, IntHandle, CriticalHandle, InvalidHandle };
  HandleType _type;

  struct TraceHandleData
  {
    unsigned int _id;
    DataLog_EnabledType _logOutput;
    DataLog_ConsoleEnabledType _consoleOutput;
  };

  struct IntHandleData
  {
    unsigned int _id;
    DataLog_EnabledType _logOutput;
    DataLog_SharedPtr(DataLog_IntBuffer) _buffer;
  };

  struct CriticalHandleData
  {
    DataLog_SharedPtr(DataLog_CriticalBuffer) _buffer;
    DataLog_TaskID _id;
  };

  union
  {
    TraceHandleData    _traceData;
    IntHandleData      _intData;
    CriticalHandleData _criticalData;
  };
};

struct DataLog_BufferInfo
{
  DataLog_SharedPtr(DataLog_OutputBuffer)   _buffer;
  bool _deleteOnEmpty;
};

struct DataLog_TaskInfo
{
  DataLog_TaskID                _id;
  DataLog_SharedPtr(const char) _name;
  DataLog_ErrorType             _error;
  DataLog_Lock                  _lock;

  DataLog_SharedPtr(DataLog_BufferInfo)  _trace;
  DataLog_EnabledType                    _logOutput;
  DataLog_ConsoleEnabledType             _consoleOutput;

  DataLog_SharedPtr(DataLog_BufferInfo)  _critical;
  DataLog_Handle                         _criticalHandle;

  DataLog_Level   _defaultLevel;
  DataLog_Handle 	_defaultHandle;
 
  DataLog_TaskErrorHandler * _errorHandler;
  bool _errorHandlerActive;
};

struct DataLog_LevelInfo
{
  unsigned int                  _id;
  DataLog_SharedPtr(const char) _name;
  DataLog_Handle                _handle;
};

class DataLog_PeriodicItemBase
{
public:
   DataLog_PeriodicItemBase(
     size_t size,
     const char * key,
     const char * description
     );
   virtual ~DataLog_PeriodicItemBase();
   virtual void updateItem(void) = 0;
   virtual bool itemChanged(void);

protected:
   enum { BUFFER_SIZE_INC = 32 };
   void * _data;         // current value
   void * _oldData;      // previous value

   size_t _size;         // current size (in bytes)
   size_t _oldSize;      // previous size (in bytes)

   size_t _allocSize;    // allocated size of _data
   size_t _oldAllocSize; // allocated size of _oldData

private:
   int    _keyCode;      // key code used for this log item
};

struct DataLog_SetInfo
{
  enum { BUFFER_SIZE_INC = 256 };

  DataLog_SharedPtr(const char) _name;
  unsigned int _id;

  double _logInterval;
  DataLog_List<DataLog_SharedPtr(DataLog_PeriodicItemBase)> _items;
  DataLog_SharedPtr(DataLog_PeriodicBuffer) _buffer;
  DataLog_SharedPtr(const char) _signalName;
  DataLog_Lock _lock;
};

class DataLog_CommonData
{
public:
  typedef DataLog_SharedPtr(DataLog_BufferInfo) BufferInfoPtr;

  DataLog_CommonData(void);
  virtual ~DataLog_CommonData();

  enum ConnectType { NotConnected, LogToFile, LogToNetwork };
  void setConnect(ConnectType type, const char * name);
  ConnectType connectType(void) { return _connectType; }
  const char * connectName(void) { return _connectName; }

  typedef DataLog_SharedPtr(DataLog_TaskInfo) TaskInfoPtr;
  TaskInfoPtr findTask(DataLog_TaskID task);
  void addTask(TaskInfoPtr taskInfo);
  void deleteTask(DataLog_TaskID task);
  BufferInfoPtr getTaskTraceBuffer(DataLog_TaskID task);

  void addBuffer(BufferInfoPtr bufferInfo);
  void deleteBuffer(BufferInfoPtr bufferInfo);
  BufferInfoPtr findFirstBuffer(void);
  BufferInfoPtr findNextBuffer(void);

  DataLog_SetHandle findSet(const char * name);
  void addSet(DataLog_SetHandle setInfo);

  typedef DataLog_SharedPtr(DataLog_LevelInfo) LevelInfoPtr;
  LevelInfoPtr findLevel(const char * name);
  void addLevel(LevelInfoPtr levelInfo);

  size_t& defaultTraceBufferSize() { return _defaultTraceBufferSize; }
  size_t& defaultIntBufferSize() { return _defaultIntBufferSize; }
  size_t& defaultCriticalBufferSize() { return _defaultCriticalBufferSize; }
  size_t currentMaxBufferSize(void);

  void setTaskError(DataLog_ErrorType error, const char * file, int line);
  DataLog_TaskErrorHandler * getTaskErrorHandler(DataLog_TaskID task);
  void setTaskErrorHandler(DataLog_TaskID task, DataLog_TaskErrorHandler * func);

  size_t savedBytesWritten(void) { return _savedBytesWritten; }
  size_t savedBytesMissed(void) { return _savedBytesMissed; }

  DataLog_OutputBuffer * internalBuffer(void) { return _internalBuffer->_buffer; }

protected:
  void createInternalBuffer(void);
 
private:
  ConnectType  _connectType;
  const char * _connectName;

  DataLog_Map<DataLog_TaskID, TaskInfoPtr> _tasks;
  DataLog_Lock _tasksLock;

  DataLog_List<BufferInfoPtr> _buffers;
  DataLog_List<BufferInfoPtr>::iterator _buffersIter;
  DataLog_Lock _buffersLock;
  size_t _savedBytesWritten;
  size_t _savedBytesMissed;

  DataLog_Map<string, DataLog_SetHandle> _sets;
  DataLog_Lock _setsLock;
  unsigned int _setNumber;

  DataLog_Map<string, LevelInfoPtr> _levels;
  DataLog_Lock _levelsLock;
  unsigned int _levelNumber;

  BufferInfoPtr _internalBuffer;

  size_t _defaultTraceBufferSize;
  size_t _defaultIntBufferSize;
  size_t _defaultCriticalBufferSize;
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
	DataLog_UINT16	_levelID __attribute__ ((packed));
	
#ifndef DATALOG_NO_NETWORK_SUPPORT
	DataLog_NodeID _nodeID __attribute__ ((packed));
#endif /* ifndef DATALOG_NO_NETWORK_SUPPORT */

	DataLog_UINT16	_nameLen __attribute__ ((packed));
};

struct DataLog_PrintOutputRecord
{
	DataLog_UINT16	_recordType __attribute__ ((packed));
	DataLog_TimeStamp _timeStamp __attribute__ ((packed));
	DataLog_UINT16	_levelID __attribute__ ((packed));
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
	DataLog_UINT16	_levelID __attribute__ ((packed));
	DataLog_TaskID	_taskID __attribute__ ((packed));
	
#ifndef DATALOG_NO_NETWORK_SUPPORT
	DataLog_NodeID _nodeID __attribute__ ((packed));
#endif /* ifndef DATALOG_NO_NETWORK_SUPPORT */

	DataLog_UINT16	_fileNameLen __attribute__ ((packed));
	DataLog_UINT16 _lineNum __attribute__ ((packed));
};

struct DataLog_TaskCreateRecord
{
	DataLog_UINT16	_recordType __attribute__ ((packed));
	DataLog_TimeStamp _timeStamp __attribute__ ((packed));
	DataLog_UINT16	_levelID __attribute__ ((packed));  
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
	DataLog_UINT16	_levelID __attribute__ ((packed));
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

