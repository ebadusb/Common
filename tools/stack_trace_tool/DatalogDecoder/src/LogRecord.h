/*****************************************************************************\
**
** LogRecord.h
**
** DLog record subtype.
**
\*****************************************************************************/

#ifndef LOGRECORDBASE_H
#define LOGRECORDBASE_H

/* Includes ******************************************************************/

#include <vector>
#include <map>

#include "LogTypes.h"

namespace DECODER {

/* Constants *****************************************************************/
enum DeviceType
{
	TRIMA=0,
	OPTIA,
	ATREUS,
	MASSIVE,
	CES
};

/* Globals *******************************************************************/

/* Macros ********************************************************************/

/* Prototypes ****************************************************************/

/* Functions *****************************************************************/

/* Classes *******************************************************************/

class LogRecordBase {
public:
	LogRecordBase(void) {}
	virtual ~LogRecordBase(void) {}
};

class RecordLogInfo : public LogRecordBase {
public:
	RecordLogInfo(void) {}
	virtual ~RecordLogInfo(void) {}

	String mMessage;
	//Let's break up the message. 
	String mLogFilename;
	String mBuildRevision;
	String mCrc;
	String mBuildDate;
	String mBuildDir;
	String mBuildComputer;
	String mBuildUser;
	String mVersion; //Optia.
	DeviceType mDeviceType;
};

class RecordLogHeader : public LogRecordBase {
public:
	RecordLogHeader(void) :
			mSizeOfChar(0),
			mSizeOfInt(0),
			mSizeOfLong(0),
			mSizeOfFloat(0),
			mSizeOfDouble(0),
			mSizeOfTaskID(0),
			mSizeOfNodeID(0),
			mLogVersion(0),
			mLogTaskID(0),
			mLogNodeID(0),
			mDay(0),
			mMonth(0),
			mYear(0),
			mHour(0),
			mMin(0),
			mSec(0) {}
	virtual ~RecordLogHeader(void) {}

	Uint16 mSizeOfChar;
	Uint16 mSizeOfInt;
	Uint16 mSizeOfLong;
	Uint16 mSizeOfFloat;
	Uint16 mSizeOfDouble;
	Uint16 mSizeOfTaskID;
	Uint16 mSizeOfNodeID;

	Uint16 mLogVersion;
	String mPlatformName;
	String mNodeName;

	Uint32 mLogTaskID;
	Uint32 mLogNodeID;

	Int32 mDay;
	Int32 mMonth;
	Uint16 mYear;
	Int32 mHour;
	Int32 mMin;
	Int32 mSec;
};

class RecordTaskCreated : public LogRecordBase {
public:
	RecordTaskCreated(void) : 
			mTimeSeconds(0), 
			mTimeNanosec(0),
			mLevelID(0), 
			mTaskID(0), 
			mNodeID(0) {}
	virtual ~RecordTaskCreated(void) {}

	Uint32 mTimeSeconds;
	Uint32 mTimeNanosec;

	Uint16 mLevelID;
	Uint32 mTaskID;
	Uint32 mNodeID;

	String mTaskName;
};

class RecordTaskDeleted : public LogRecordBase {
public:
	RecordTaskDeleted(void) : 
			mTimeSeconds(0), 
			mTimeNanosec(0),
			mLevelID(0), 
			mTaskID(0), 
			mNodeID(0) {}
	virtual ~RecordTaskDeleted(void) {}

	Uint32 mTimeSeconds;
	Uint32 mTimeNanosec;

	Uint16 mLevelID;
	Uint32 mTaskID;
	Uint32 mNodeID;
};

class RecordTimeStamp : public LogRecordBase {
public:
	RecordTimeStamp(void) : 
			mTimeSeconds(0), 
			mTimeNanosec(0) {}
	virtual ~RecordTimeStamp(void) {}

	Uint32 mTimeSeconds;
	Uint32 mTimeNanosec;
};

class RecordEndOfFile : public LogRecordBase {
public:
	RecordEndOfFile(void) : 
			mTimeSeconds(0), 
			mTimeNanosec(0) {}
	virtual ~RecordEndOfFile(void) {}

	Uint32 mTimeSeconds;
	Uint32 mTimeNanosec;
};

class RecordPeriodicSet : public LogRecordBase {
public:
	RecordPeriodicSet(void) : 
			mTimeSeconds(0), 
			mTimeNanosec(0),
			mSetID(0),
			mNodeID(0) {}
	virtual ~RecordPeriodicSet(void) {}

	Uint32 mTimeSeconds;
	Uint32 mTimeNanosec;

	Uint16 mSetID;
	Uint32 mNodeID;

	String mSetName;
};

class RecordPeriodicLogOutput : public LogRecordBase {
public:
	typedef std::vector<Uint8> Vector;
	typedef std::map<Uint16, Vector> Map;

	RecordPeriodicLogOutput(void) : 
			mTimeSeconds(0), 
			mTimeNanosec(0),
			mSetID(0),
			mNodeID(0) {}
	virtual ~RecordPeriodicLogOutput(void) {}

	Uint32 mTimeSeconds;
	Uint32 mTimeNanosec;

	Uint16 mSetID;
	Uint32 mNodeID;

	Map mData;
};

class RecordBinary : public LogRecordBase {
public:
	typedef std::vector<Uint8> Vector;

	RecordBinary(void) {}
	virtual ~RecordBinary(void) {}

	Vector mData;
};

class RecordStartNetCon : public LogRecordBase {
public:
	RecordStartNetCon(void) :
			mNodeID(0),
			mDay(0),
			mMonth(0),
			mYear(0),
			mHour(0),
			mMin(0),
			mSec(0) {}
	virtual ~RecordStartNetCon(void) {}

	Uint32 mNodeID;

	Int32 mDay;
	Int32 mMonth;
	Uint16 mYear;
	Int32 mHour;
	Int32 mMin;
	Int32 mSec;

	String mName;
};

class RecordPeriodicSetItem : public LogRecordBase {
public:
	RecordPeriodicSetItem(void) :
			mTimeSeconds(0), 
			mTimeNanosec(0),
			mKeyCode(0),
			mNodeID(0) {}
	virtual ~RecordPeriodicSetItem(void) {}

	Uint32 mTimeSeconds;
	Uint32 mTimeNanosec;

	Uint16 mKeyCode;
	Uint32 mNodeID;

	String mName;
	String mDescription;
	String mFormat;
};

class RecordLogLevel : public LogRecordBase {
public:
	RecordLogLevel(void) : 
			mTimeSeconds(0), 
			mTimeNanosec(0),
			mLevelID(0),
			mNodeID(0) {}
	virtual ~RecordLogLevel(void) {}

	Uint32 mTimeSeconds;
	Uint32 mTimeNanosec;

	Uint16 mLevelID;
	Uint32 mNodeID;

	String mLevelName;
};

class RecordDataMissed : public LogRecordBase {
public:
	RecordDataMissed(void) {}
	virtual ~RecordDataMissed(void) {}
};

class RecordTextOutput : public LogRecordBase {
public:
	RecordTextOutput(void) :
			mTimeSeconds(0), 
			mTimeNanosec(0),
			mLevelID(0),
			mTaskID(0),
			mNodeID(0),
			mLine(0) {}
	virtual ~RecordTextOutput(void) {}

	Uint32 mTimeSeconds;
	Uint32 mTimeNanosec;

	Uint16 mLevelID;
	Uint32 mTaskID;
	Uint32 mNodeID;

	String mFile;
	Uint16 mLine;

	String mMessage;
};

class RecordPrintOutput : public RecordTextOutput {
public:
	RecordPrintOutput(void) {}
	virtual ~RecordPrintOutput(void) {}
};

class RecordStreamOutput : public RecordTextOutput {
public:
	RecordStreamOutput(void) {}
	virtual ~RecordStreamOutput(void) {}
};

} // namespace DECODER

#endif // LOGRECORDBASE_H

