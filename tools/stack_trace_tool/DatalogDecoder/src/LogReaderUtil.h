/*****************************************************************************\
**
** LogReaderUtil.h
**
** Reader utility class.
**
\*****************************************************************************/

#ifndef LOGREADERUTIL_H
#define LOGREADERUTIL_H

/* Includes ******************************************************************/

#include "LogRecord.h"

namespace DECODER {

/* Forward Declarations ******************************************************/

/* Constants *****************************************************************/

/* Globals *******************************************************************/

/* Macros ********************************************************************/

/* Prototypes ****************************************************************/

/* Classes *******************************************************************/

template <class _TYPE, class _COLLECTION>
class ProcessInclude {
public:
	typedef _TYPE Type;
	typedef _COLLECTION Collection;

	ProcessInclude(Collection &collection) 
			: mCollection(collection) {}

	void operator()(const Type &data) 
	{
		mCollection.push_back(data);
	}

private:
	Collection &mCollection;	
};

template <class _TYPE>
class ProcessSet {
public:
	typedef _TYPE Type;

	ProcessSet(Type &type) 
			: mRecord(type) {}

	void operator()(const Type &data) 
	{ 	
		mRecord = data;
	}

private:
	Type &mRecord;
};

template <class _TYPE, class _HANDLER1, class _HANDLER2>
class ProcessSplit {
public:
	typedef _TYPE Type;
	typedef _HANDLER1 FirstHandler;
	typedef _HANDLER2 SecondHandler;

	ProcessSplit(FirstHandler &firstHandler, SecondHandler &secondHandler) 
			: mFirstHandler(firstHandler), mSecondHandler(secondHandler) {}

	void operator()(const Type &data) 
	{ 	
		mFirstHandler(data);
		mSecondHandler(data);
	}

private:
	FirstHandler mFirstHandler;
	SecondHandler mSecondHandler;
};

template <class _RECORD, class _HANDLER>
class ProcessFilterLog {
public:
	typedef _RECORD Record;
	typedef _HANDLER Handler;

	ProcessFilterLog(const RecordLogLevel &level, Handler &handler) 
			: mLevel(level), mHandler(handler) {}

	void operator()(const Record &data) 
	{ 	
		if (data.mLevelID == mLevel.mLevelID) {
			mHandler(data);
		}
	}

private:
	const RecordLogLevel &mLevel;
	Handler &mHandler;
};

class ProcessGetLevelID {
public:
	ProcessGetLevelID(const DECODER::String &name, DECODER::RecordLogLevel &record) 
			: mName(name), mRecord(record) {}

	void operator()(const DECODER::RecordLogLevel &data) 
	{ 	
		if (data.mLevelName == mName) {
			mRecord = data;
		}
	}

private:
	DECODER::String mName;
	DECODER::RecordLogLevel &mRecord;
};

} // namespace DECODER

#endif // LOGREADERUTIL_H

