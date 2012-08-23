/*****************************************************************************\
**
** ColumnDataUtil.h
**
** Utility classes for extracting column data.
**
\*****************************************************************************/

/* Includes ******************************************************************/

#include <algorithm>
#include <iostream>
#include <list>
#include <set>
#include <map>

namespace DECODER {
namespace UTIL {

/* Types *********************************************************************/

struct PeriodicSetItem {
	Uint32 node;
	String set;
	String name;
	String info;
};

struct PeriodicSetItemLess : public std::binary_function<PeriodicSetItem, PeriodicSetItem, bool> {
	bool operator()(const PeriodicSetItem& left, const PeriodicSetItem& right) const
	{
		return (left.node < right.node) || ((left.node == right.node) && (left.name < right.name));
	}
};

typedef std::string String;
typedef std::set<String> ColumnNameSet;
typedef std::map<Uint32, String>	PeriodicSets;
typedef std::set<PeriodicSetItem, PeriodicSetItemLess> PeriodicItems;

/* Classes *******************************************************************/

template <class _HANDLER>
class FilterPeriodicSetByName {
public:
	typedef _HANDLER Handler;

	FilterPeriodicSetByName(Handler &handler, String name) 
			: mHandler(handler), mName(name) {}

	void operator()(const DECODER::RecordPeriodicSet &data) 
	{ 	
		if (data.mSetName == mName) {
			mHandler(data);
		}
	}

private:
	Handler &mHandler;
	String mName;
};

template <class _HANDLER>
class FilterPeriodicSetItemByID {
public:
	typedef _HANDLER Handler;

	FilterPeriodicSetItemByID(Handler &handler, DECODER::RecordPeriodicSet &data) 
			: mHandler(handler), mType(data) {}

	void operator()(const DECODER::RecordPeriodicSetItem &data) 
	{ 	
		if (data.mNodeID == mType.mNodeID) {
			mHandler(data);
		}
	}

private:
	Handler &mHandler;
	DECODER::RecordPeriodicSet &mType;
};

template <class _HANDLER>
class FilterPeriodicLogOutputByID {
public:
	typedef _HANDLER Handler;
	typedef std::list<DECODER::RecordPeriodicSetItem> PeriodicSetItemList;

	FilterPeriodicLogOutputByID(Handler &handler, PeriodicSetItemList &data) 
			: mHandler(handler), mList(data) {}

	void operator()(const DECODER::RecordPeriodicLogOutput &data) 
	{ 	
		PeriodicSetItemList::const_iterator iter = mList.begin();
		PeriodicSetItemList::const_iterator end = mList.end();

		for (; iter != end; ++iter) {
			if (data.mData.find((*iter).mKeyCode) != data.mData.end()) {
				mHandler(data);
			}
		}
	}

private:
	Handler &mHandler;
	PeriodicSetItemList &mList;
};

template <class _HANDLER>
class FilterPeriodicLogOutputBySetID {
public:
	typedef _HANDLER Handler;

	FilterPeriodicLogOutputBySetID(Handler &handler, RecordPeriodicSet &data) 
			: mHandler(handler), mSet(data) {}

	void operator()(const DECODER::RecordPeriodicLogOutput &data) 
	{ 	
		if (data.mSetID == mSet.mSetID) {
			mHandler(data);
		}
	}

private:
	Handler &mHandler;
	RecordPeriodicSet &mSet;
};

class ProcessGetColumns {
public:
	ProcessGetColumns(ColumnNameSet &set) : mColumnNameSet(set) {}

	void operator()(const DECODER::RecordPeriodicSetItem &data) 
	{ 	
		mColumnNameSet.insert(data.mName);
	}

private:
	ColumnNameSet &mColumnNameSet;
};

class ProcessGetPeriodicSets {
public:
	ProcessGetPeriodicSets(PeriodicSets &set) : mPeriodicSets(set) {}

	void operator()(const DECODER::RecordPeriodicSet &data) 
	{ 	
		mPeriodicSets[data.mNodeID] = data.mSetName;
	}

private:
	PeriodicSets &mPeriodicSets;
};

class ProcessGetPeriodicItems {
public:
	ProcessGetPeriodicItems(PeriodicItems &set) : mPeriodicItems(set) {}

	void operator()(const DECODER::RecordPeriodicSetItem &data) 
	{ 	
		PeriodicSetItem item = { data.mNodeID, "", data.mName, data.mDescription };

		mPeriodicItems.insert(item);
	}

private:
	PeriodicItems &mPeriodicItems;
};

} // namespace UTIL
} // namespace TRACE

