/*****************************************************************************\
**
** StackTraceUtil.h
**
** Stack trace utility implementation.
**
\*****************************************************************************/

#ifndef STACKTRACEUTIL_H
#define STACKTRACEUTIL_H

/* Includes ******************************************************************/

#include <Windows.h>
#include <string>
#include <sstream>
#include <set>
#include <list>
#include <algorithm>
#include <cstdlib>

namespace DECODER {
namespace UTIL {

/* Constants *****************************************************************/

enum {
	kDebugRecordType = 0x0001,
	kDebugTaskData = 0x0001,
	kDebugMsgData = 0x0002,
	kDebugInputPacketData = 0x0003,
	kDebugOutputPacketData = 0x0004,
	kDebugNetworkStatData = 0x0005,
};

enum {
	ETHERTYPE_IP = 0x0800, // IP protocol
	ETHERTYPE_ARP = 0x0806	// Address resolution protocol
};

// Tags for parsing string output
static const char *kTabModule = ".out";
static const char *kTabModule1 = ".o";
static const char *kTabAddress1 = "textAddr=";
static const char *kTabAddress2 = "D=";
static const char *kTabDelim = ":";
static const char *kTabSpace = " ";
static const char *kTabSlash = "\\";
static const char *kTabFault = "Page Fault";
static const char *kTabFile = "Log file";
static const char *kTabBuild = "BUILD";
static const char *kTabBuildDir1 = "BUILD DIR: ";
static const char *kTabBuildDir2 = "directory=";
static const char *kTabTasks = "Tasks: ";
static const char *kTabTaskState = "Task state:";
static const char *kTabMsgID = "Mid ";
static const char *kTabPC = "Program Counter : ";
static const char *kTabTask = "Task: ";
static const char *kTabMachine = "/machine";

// Commands for extracting debug sysbols
static const char *kStrCommand2_20 = "nmpentium.exe --numeric-sort --demangle ";
//static const char *kStrCommand2_20 = "nmsimpc.exe --numeric-sort --demangle ";
static const char *kStrCommand2_02 = "objdump386.exe --syms ";
//static const char *kStrCommand2_02 = "nm386.exe --numeric-sort --demangle ";

// Tornado tool versions
static const char *kVerTornado2_20 = "2.2";
static const char *kVerTornado2_02 = "2.02";
static const char *kVerTornado6_4  = "6.4";

// Hardware manufacturer names. 
static const char *kSysAmpro = "Ampro Computers Inc.";
static const char *kSysVersalogic = "VersaLogic Corp.";

// The name of the operating systems
static const char *kPathSystemAmpro = "vxWorks_ampro";
static const char *kPathSystemVersalogic = "vxWorks_versalogic";
static const char *kPathSystemCommon = "vxWorks";

// Default build paths
static const char *kStrCurrent = "\\current_build";
static const char *kStrPathname = "\\\\bctquad3\\home";


static const char *kAltSysPathTrima = "/vxboot,/trima/safety/boot"; /* control, safety */
static const char *kAltSysPathOptia = "/../base/vxworks_target/control,/../base/vxworks_target/apc,/../base/vxworks_target/safety"; /* control, apc, safety */
static const char *kAltSysPathCes	= "/vxboot,/ces";

/* Types *********************************************************************/

struct ipstat {
	Uint32	ips_total;		/* total packets received */
	Uint32	ips_badsum;		/* checksum bad */
	Uint32	ips_tooshort;		/* packet too short */
	Uint32	ips_toosmall;		/* not enough data */
	Uint32	ips_badhlen;		/* ip header length < data size */
	Uint32	ips_badlen;		/* ip length < ip header length */
	Uint32	ips_fragments;		/* fragments received */
	Uint32	ips_fragdropped;	/* frags dropped (dups, out of space) */
	Uint32	ips_fragtimeout;	/* fragments timed out */
	Uint32	ips_forward;		/* packets forwarded */
	Uint32	ips_cantforward;	/* packets rcvd for unreachable dest */
	Uint32	ips_redirectsent;	/* packets forwarded on same net */
	Uint32	ips_noproto;		/* unknown or unsupported protocol */
	Uint32	ips_delivered;		/* datagrams delivered to upper level*/
	Uint32	ips_localout;		/* total ip packets generated here */
	Uint32	ips_odropped;		/* lost packets due to nobufs, etc. */
	Uint32	ips_reassembled;	/* total packets reassembled ok */
	Uint32	ips_fragmented;		/* datagrams sucessfully fragmented */
	Uint32	ips_ofragments;		/* output fragments created */
	Uint32	ips_cantfrag;		/* don't fragment flag was set, etc. */
	Uint32	ips_badoptions;		/* error in option processing */
	Uint32	ips_noroute;		/* packets discarded due to no route */
	Uint32	ips_badvers;		/* ip version != 4 */
	Uint32	ips_rawout;		/* total raw ip packets generated */
};

struct tcpstat {
	Uint32	tcps_connattempt;	/* connections initiated */
	Uint32	tcps_accepts;		/* connections accepted */
	Uint32	tcps_connects;		/* connections established */
	Uint32	tcps_drops;		/* connections dropped */
	Uint32	tcps_conndrops;		/* embryonic connections dropped */
	Uint32	tcps_closed;		/* conn. closed (includes drops) */
	Uint32	tcps_segstimed;		/* segs where we tried to get rtt */
	Uint32	tcps_rttupdated;	/* times we succeeded */
	Uint32	tcps_delack;		/* delayed acks sent */
	Uint32	tcps_timeoutdrop;	/* conn. dropped in rxmt timeout */
	Uint32	tcps_rexmttimeo;	/* retransmit timeouts */
	Uint32	tcps_persisttimeo;	/* persist timeouts */
	Uint32	tcps_keeptimeo;		/* keepalive timeouts */
	Uint32	tcps_keepprobe;		/* keepalive probes sent */
	Uint32	tcps_keepdrops;		/* connections dropped in keepalive */

	Uint32	tcps_sndtotal;		/* total packets sent */
	Uint32	tcps_sndpack;		/* data packets sent */
	Uint32	tcps_sndbyte;		/* data bytes sent */
	Uint32	tcps_sndrexmitpack;	/* data packets retransmitted */
	Uint32	tcps_sndrexmitbyte;	/* data bytes retransmitted */
	Uint32	tcps_sndacks;		/* ack-only packets sent */
	Uint32	tcps_sndprobe;		/* window probes sent */
	Uint32	tcps_sndurg;		/* packets sent with URG only */
	Uint32	tcps_sndwinup;		/* window update-only packets sent */
	Uint32	tcps_sndctrl;		/* control (SYN|FIN|RST) packets sent */

	Uint32	tcps_rcvtotal;		/* total packets received */
	Uint32	tcps_rcvpack;		/* packets received in sequence */
	Uint32	tcps_rcvbyte;		/* bytes received in sequence */
	Uint32	tcps_rcvbadsum;		/* packets received with ccksum errs */
	Uint32	tcps_rcvbadoff;		/* packets received with bad offset */
	Uint32	tcps_rcvshort;		/* packets received too short */
	Uint32	tcps_rcvduppack;	/* duplicate-only packets received */
	Uint32	tcps_rcvdupbyte;	/* duplicate-only bytes received */
	Uint32	tcps_rcvpartduppack;	/* packets with some duplicate data */
	Uint32	tcps_rcvpartdupbyte;	/* dup. bytes in part-dup. packets */
	Uint32	tcps_rcvoopack;		/* out-of-order packets received */
	Uint32	tcps_rcvoobyte;		/* out-of-order bytes received */
	Uint32	tcps_rcvpackafterwin;	/* packets with data after window */
	Uint32	tcps_rcvbyteafterwin;	/* bytes rcvd after window */
	Uint32	tcps_rcvafterclose;	/* packets rcvd after "close" */
	Uint32	tcps_rcvwinprobe;	/* rcvd window probe packets */
	Uint32	tcps_rcvdupack;		/* rcvd duplicate acks */
	Uint32	tcps_rcvacktoomuch;	/* rcvd acks for unsent data */
	Uint32	tcps_rcvackpack;	/* rcvd ack packets */
	Uint32	tcps_rcvackbyte;	/* bytes acked by rcvd acks */
	Uint32	tcps_rcvwinupd;		/* rcvd window update packets */
	Uint32	tcps_pawsdrop;		/* segments dropped due to PAWS */
	Uint32	tcps_predack;		/* times hdr predict ok for acks */
	Uint32	tcps_preddat;		/* times hdr predict ok for data pkts */
	Uint32	tcps_pcbcachemiss;
	Uint32	tcps_persistdrop;		/* timeout in persist state */
	Uint32	tcps_badsyn;		/* bogus SYN, e.g. premature ACK */
};

struct mbstat {
	Uint32	mNum;			/* mBlks obtained from page pool */
	Uint32	mDrops;			/* times failed to find space */
	Uint32	mWait;			/* times waited for space */
	Uint32	mDrain;			/* times drained protocols for space */
	Uint32	mTypes[256];		/* type specific mBlk allocations */
};

struct ether_header {
   Uchar ether_dhost[6];
   Uchar ether_shost[6];
   Uchar ether_type;
};

struct BinaryTaskSwitchRecord {
	Uint32 oldTask;
	Uint32 newTask;
	Uint32 seconds;
	Uint32 nanoSeconds;
};

struct BinarySendMsgRecord {
	Uint32 sendTask;
	Uint32 rcvTask;
	Uint32 msgID;
	Uint32 seconds;
	Uint32 nanoSeconds;
};

struct BinaryNetworkStatRecord {
	Uint32	seconds;
	Uint32	nanoSeconds;

	ipstat ipstat;
	tcpstat tcpstat;
	mbstat data_mbstat;
	mbstat sys_mbstat;
} ;

struct BinaryPacketDataRecord {
	Uint32 seconds;
	Uint32 nanoSeconds;
	Uint16 len;
	Uint16 index;
};

template<class _TYPE>
struct BinaryDataLess : public std::binary_function<_TYPE, _TYPE, bool> {
	bool operator()(const _TYPE& _Left, const _TYPE& _Right) const
	{
		return (_Left.seconds < _Right.seconds) 
				|| ((_Left.seconds == _Right.seconds) 
					&& (_Left.nanoSeconds < _Right.nanoSeconds));
	}
};

typedef std::ostream Stream;
typedef std::vector<RecordStreamOutput> RecordVector;
typedef std::vector<Uint32> TraceVector;
typedef std::vector<Uint32> StateVector;
typedef std::map<Uint32, String> AddressMap;
typedef std::map<Uint32, AddressMap> NodeAddressMap;
typedef std::vector<String> PageFaultInfo;
typedef std::vector<PageFaultInfo> FaultVector;
typedef ProcessSet<RecordLogInfo> ProcessSetInfo;
typedef ProcessSet<RecordLogHeader> ProcessSetHeader;
typedef std::map<Uint32, NodeAddressMap> SymbolsTable;
typedef std::map<Uint32, String> TaskNameMap;
typedef std::map<Uint32, Uint32> TaskNodeMap;
typedef std::map<Uint32, TraceVector> TaskValues;
typedef TaskValues StackTraceMap;
typedef TaskValues TaskStateMap;
typedef std::vector<Uint8> BinaryData;
typedef std::multiset<BinaryTaskSwitchRecord, BinaryDataLess<BinaryTaskSwitchRecord>> TaskSwitchRecords;
typedef std::multiset<BinarySendMsgRecord, BinaryDataLess<BinarySendMsgRecord>> SendMsgRecords;
typedef std::multiset<BinaryNetworkStatRecord, BinaryDataLess<BinaryNetworkStatRecord>> NetworkStatRecords;
typedef std::map<Uint32, String> MessageIDMap;
typedef std::list<Uint32> TaskIDList;
typedef std::set<Uint32> NodeIDList;
typedef std::vector<std::string> ValueList;

/* Utility Functions *********************************************************/

static ValueList ParseValuesFromText(const String &text, char delim)
{
	ValueList values;

	if (!text.empty()) {
		int loc = 0;
		int end = 0;

		end = text.find(delim);
		while (end != String::npos) {
			values.push_back(text.substr(loc, end - loc));
			loc = end + 1;
			end = text.find(delim, loc);
		}
		values.push_back(text.substr(loc));
	}

	return values;
}

/* Datalog Functors **********************************************************/

class ProcessGetLogInfo {
public:
	ProcessGetLogInfo(String &info, RecordLogInfo& logInfo) 
		: mInfo(info), mlogInfo(logInfo) {}

	void operator()(const RecordLogInfo &data) 
	{ 	
		mInfo = data.mMessage.substr(data.mMessage.find(kTabFile));
		mInfo = mInfo.substr(0, mInfo.length() - 2);

		std::string::size_type loc = mInfo.find(kTabBuild);
		
		while (loc != std::string::npos) {
			mInfo.insert(loc++, "\n");
			loc = mInfo.find(kTabBuild, ++loc);
		}
		
		mlogInfo = data;
	}

private:
	String &mInfo;	
	RecordLogInfo& mlogInfo;
};

class ProcessGetBuildPath {
public:
	ProcessGetBuildPath(String &path) : mPath(path) {}

	void operator()(const RecordLogInfo &data) 
	{ 	

		std::string pathname = data.mBuildDir;
#ifndef __CLR_MANAGED__
		Uint32 attrib = GetFileAttributes(pathname.c_str());
#else
		std::wstring pathnameW(pathname.length(), L'');
		std::copy(pathname.begin(), pathname.end(), pathnameW.begin());
		Uint32 attrib = GetFileAttributes(pathnameW.c_str());
#endif

		if(attrib == 0xffffffff) {
			pathname = pathname.replace(0, pathname.find(kTabSlash), kStrPathname);
		}

		mPath = pathname;
	}
private:
	String &mPath;
};

class ProcessGetSystemName {
public:
	ProcessGetSystemName(String &path) : mSystemPath(path)
	{
		mSystemPath = kPathSystemCommon;
	}

	void operator()(const DECODER::RecordStreamOutput &data) 
	{ 	
		if (data.mMessage.find(kSysVersalogic) != std::string::npos) {
			mSystemPath = kPathSystemVersalogic;
		}
		else if (data.mMessage.find(kSysAmpro) != std::string::npos) {
			mSystemPath = kPathSystemAmpro;
		}
	}

private:
	String &mSystemPath;
};

class ProcessGetTasksNames {
public:
	ProcessGetTasksNames(TaskNameMap &map) : mTaskNameMap(map) {}

	void operator()(const DECODER::RecordTaskCreated &data) 
	{ 	
		mTaskNameMap[data.mTaskID] = data.mTaskName;
	}

private:
	TaskNameMap &mTaskNameMap;
};

class ProcessGetTasksNodes {
public:
	ProcessGetTasksNodes(TaskNodeMap &map) : mTaskNodeMap(map) {}

	void operator()(const DECODER::RecordTaskCreated &data) 
	{ 	
		mTaskNodeMap[data.mTaskID] = data.mNodeID;
	}

private:
	TaskNodeMap &mTaskNodeMap;
};

class ProcessTaskBase {
public:
	ProcessTaskBase(const String &tag, TaskValues &map) 
			: mTaskTag(tag), mTaskValues(map) {}

	void operator()(const RecordStreamOutput &record)
	{
		const std::string &message = record.mMessage;
		std::string::size_type loc = message.find(mTaskTag, 0);

		if (loc != std::string::npos) {
			loc += mTaskTag.length();

			while (loc < message.size()) {
				std::string::size_type taskBegin = loc;
				std::string::size_type taskEnd = message.find_first_of(kTabDelim, taskBegin);
				std::string::size_type stackBegin = taskEnd + 1;
				std::string::size_type stackEnd = stackBegin;
				std::string::size_type next = message.find_first_of(kTabDelim, taskEnd + 1);
				
				if (next == std::string::npos) {
					stackEnd = message.size();
				}
				else {
					stackEnd = message.find_last_of(kTabSpace, next);
				}
				
				std::string task = message.substr(taskBegin, taskEnd - taskBegin);
				std::string value = message.substr(stackBegin, stackEnd - stackBegin);
				std::istringstream taskInStream(task);
				std::istringstream pointerInStream(value);
				std::vector<Uint32> vector;
				Uint32 address = 0;
				Char space = 0;

				taskInStream.flags(std::ios::hex);
				pointerInStream.flags(std::ios::hex);

				while (pointerInStream.good()) {
					pointerInStream >> address >> space;
					vector.push_back(address);
				}

				taskInStream >> address;

				mTaskValues[address] = vector;
				loc = stackEnd + 1;
			}
		}
	}

private:
	const String mTaskTag;
	TaskValues &mTaskValues;
};

class ProcessTaskState : public ProcessTaskBase {
public:
	ProcessTaskState(TaskStateMap &map) : ProcessTaskBase(kTabTaskState, map) {}
};

class ProcessStackTrace : public ProcessTaskBase {
public:
	ProcessStackTrace(StackTraceMap &map) : ProcessTaskBase(kTabTasks, map) {}
};

class ProcessLoadedComponent {
public:
	ProcessLoadedComponent(NodeAddressMap &map) : mAddressMap(map) {}

	void operator()(const RecordTextOutput &record)
	{
		std::string::size_type tabModuleLoc = record.mMessage.find(kTabModule);
		if(tabModuleLoc == std::string::npos)
			tabModuleLoc = record.mMessage.find(kTabModule1);
		std::string::size_type tabModuleEnd = std::string::npos;
		std::string::size_type tabAddress1Loc = record.mMessage.find(kTabAddress1);
		std::string::size_type tabAddress2Loc = record.mMessage.find(kTabAddress2);
		std::string::size_type tabAddressBegin = std::string::npos;
		std::string::size_type tabAddressLen = std::string::npos;

		if (tabAddress1Loc != std::string::npos) {
			tabModuleEnd = record.mMessage.find(kTabDelim, 0);
			tabAddressBegin = tabAddress1Loc;
			tabAddressLen = std::strlen(kTabAddress1);
		}
		else if (tabAddress2Loc != std::string::npos) {
			tabModuleEnd = record.mMessage.find(kTabSpace, 0);
			tabAddressBegin = tabAddress2Loc;
			tabAddressLen = std::strlen(kTabAddress2);
		}

		if ((tabModuleLoc != std::string::npos) && (tabAddressBegin != std::string::npos)) {
			std::string::size_type tabAddressEnd = record.mMessage.find(kTabSpace, tabAddressBegin);
			std::string pathname = record.mMessage.substr(0, tabModuleEnd);
			std::string address = record.mMessage.substr(tabAddressBegin + tabAddressLen, tabAddressEnd - tabAddressBegin);
			unsigned int value = 0;
			sscanf(address.c_str(), "%x", &value);
			mAddressMap[record.mNodeID][value] = pathname;
		}
	}

private:
	NodeAddressMap &mAddressMap;
};

class ProcessPageFault {
public:
	enum {
		kPageFaultPageDirBase = 1,
		kPageFaultEsp0First,
		kPageFaultEsp0Second,
		kPageFaultProgramCounter,
		kPageFaultCodeSelector,
		kPageFaultEFlagsReg,
		kPageFaultErrorCode,
		kPageFaultTaskID, 
		kPageFaultSize = 9
	};

	ProcessPageFault(FaultVector &vector) : mFaultVector(vector), mFile(""), mLine(0) {}

	void operator()(const RecordStreamOutput &record)
	{
		if (record.mMessage == kTabFault) {
			mTaskID = record.mTaskID;
			mNodeID = record.mNodeID;
			mFile = record.mFile;
			mLine = record.mLine;
			mFaultVector.push_back(PageFaultInfo());
		}
		else if (!mFaultVector.empty() 
				&& (mFaultVector.back().size() < kPageFaultSize)
				&& (record.mTaskID == mTaskID) 
				&& (record.mNodeID == mNodeID) 
				&& (record.mFile == mFile) 
				&& (record.mLine == mLine)) {
			if (mFaultVector.back().size() == kPageFaultTaskID) {
				mFaultVector.back().push_back(record.mMessage.substr(0, record.mMessage.length() - 1));
			}
			else {
				mFaultVector.back().push_back(record.mMessage);
			}
		}
	}

private:
	FaultVector &mFaultVector;

	Uint32 mTaskID;
	Uint32 mNodeID;
	String mFile;
	Uint16 mLine;
};

class ProcessMessageID {
public:
	ProcessMessageID(MessageIDMap &map) : mMessageIDMap(map) {}

	void operator()(const RecordStreamOutput &record)
	{
		const std::string &message = record.mMessage;
		std::string::size_type loc = message.find(kTabMsgID, 0);
		std::string::size_type cnt = std::strlen(kTabMsgID);

		while (loc != std::string::npos) {
			std::string::size_type space = record.mMessage.find(kTabSpace, loc + cnt);
			std::string::size_type end = message.find(kTabMsgID, space);
			std::string::size_type len = end != std::string::npos 
					? end - space - 3 : std::string::npos;

			std::string id = record.mMessage.substr(loc + cnt, space - loc - cnt);
			std::string name = record.mMessage.substr(space + 1, len);
			std::istringstream stream(id);
			Uint32 value = 0;

			stream.flags(std::ios::hex);
			stream >> value;
			mMessageIDMap[value] = name;
			loc = end;
		}
	}

private:
	MessageIDMap &mMessageIDMap;
};

class ProcessTaskSwitch {
public:
	ProcessTaskSwitch(TaskSwitchRecords &vector) : mRecords(vector) {}

	void operator()(const RecordBinary &record)
	{
		if (*reinterpret_cast<const Uint16*>(&record.mData[0]) == kDebugRecordType
				&& *reinterpret_cast<const Uint16*>(&record.mData[2]) == kDebugTaskData) {
			Int32 count = (record.mData.size() - sizeof(Uint16) * 2) / sizeof(BinaryTaskSwitchRecord);
			Int32 index = sizeof(Uint16) * 2;

			for (Int32 i = 0; i < count; ++i, index += sizeof(BinaryTaskSwitchRecord)) {
				mRecords.insert(*reinterpret_cast<const BinaryTaskSwitchRecord*>(&record.mData[index]));
			}
		}
	}

private:
	TaskSwitchRecords &mRecords;
};

class ProcessSendMsg {
public:
	ProcessSendMsg(SendMsgRecords &vector) : mRecords(vector) {}

	void operator()(const RecordBinary &record)
	{
		if (*reinterpret_cast<const Uint16*>(&record.mData[0]) == kDebugRecordType
				&& *reinterpret_cast<const Uint16*>(&record.mData[2]) == kDebugMsgData) {
			Int32 count = (record.mData.size() - sizeof(Uint16) * 2) / sizeof(BinarySendMsgRecord);
			Int32 index = sizeof(Uint16) * 2;

			for (Int32 i = 0; i < count; ++i, index += sizeof(BinarySendMsgRecord)) {
				mRecords.insert(*reinterpret_cast<const BinarySendMsgRecord*>(&record.mData[index]));
			}
		}
	}

private:
	SendMsgRecords &mRecords;
};

class ProcessNetworkStat {
public:
	ProcessNetworkStat(NetworkStatRecords &vector) : mRecords(vector) {}

	void operator()(const RecordBinary &record)
	{
		if (*reinterpret_cast<const Uint16*>(&record.mData[0]) == kDebugRecordType
				&& *reinterpret_cast<const Uint16*>(&record.mData[2]) == kDebugNetworkStatData) {
			Int32 count = (record.mData.size() - sizeof(Uint16) * 2) / sizeof(BinaryNetworkStatRecord);
			Int32 index = sizeof(Uint16) * 2;

			for (Int32 i = 0; i < count; ++i, index += sizeof(BinaryNetworkStatRecord)) {
				mRecords.insert(*reinterpret_cast<const BinaryNetworkStatRecord*>(&record.mData[index]));
			}
		}
	}

private:
	NetworkStatRecords &mRecords;
};

/* Print Functors ************************************************************/

class PrintAddresses {
public:
	PrintAddresses(Stream &stream) : mStream(stream) {}

	void operator()(const AddressMap::value_type &record)
	{
		mStream << "\t0x";
		mStream.flags(std::ios::hex);
		mStream.width(8);
		mStream.fill('0');
		mStream << record.first;
		mStream << " ";
		mStream << record.second;
		mStream << std::endl;
	}

private:
	Stream &mStream;
};

class PrintModules {
public:
	PrintModules(Stream &stream) : mStream(stream) {}

	void operator()(const NodeAddressMap::value_type &data)
	{
		mStream << "Loaded Modules for Node: 0x";
		mStream.flags(std::ios::hex);
		mStream.width(8);
		mStream.fill('0');
		mStream << data.first; 
		mStream << std::endl;
		for_each(data.second.begin(), data.second.end(), PrintAddresses(mStream));
		mStream << std::endl;
	}

private:
	Stream &mStream;
};

class PrintSymbolData {
public:
	PrintSymbolData(const String &pathname, 
			const String& osVersion, // not utilized
			const TaskNodeMap &taskNodes,
			const NodeAddressMap &components, 
			const String &command,
			const String &alias,
			SymbolsTable &table, 
			Stream &stream) 
			: mPathname(pathname), 
			mTaskNodes(taskNodes),
			mComponents(components), 
			mCommand(command), 
			mSymbols(table), 
			mStream(stream)
	{
		if (!alias.empty()) {
			std::string::size_type loc = alias.find(':');

			if (loc != std::string::npos) {
				mAliasOld = alias.substr(0, loc);
				mAliasNew = alias.substr(loc + 1);
			}
			else {
				mAliasOld = alias;
			}
		}

		NodeAddressMap::const_iterator iter = mComponents.begin();
		NodeAddressMap::const_iterator end = mComponents.end();

		for (; iter != end; ++iter) {
			Uint32 node = iter->first;
			Uint32 address = iter->second.begin()->first;
			String module = iter->second.begin()->second;

			if (mSymbols[node].find(address) == mSymbols[node].end()) {
				this->IncludeLoadedFunctions(node, address, mPathname + module);
			}
		}
	}

protected:
	void IncludeLoadedFunctions(Uint32 systemNode, Uint32 baseAddress, String objectFile)
	{
		AddressMap functions;

		if (!objectFile.empty()) {
			if (!mAliasOld.empty()) {
				std::string::size_type loc = objectFile.find(mAliasOld);

				if (loc != std::string::npos) {
					objectFile.replace(loc, mAliasOld.length(), mAliasNew);
				}
			}

			std::ostringstream commandStream;
			commandStream << mCommand << " " << objectFile << std::endl;
			char buffer[128] = { 0 };
			unsigned int address = 0;
			const char *function = buffer + 11;
			std::string command(commandStream.str());
			
			FILE *pipe = _popen(command.c_str(), "rt");

			while(fgets(buffer, 128, pipe)) {
				buffer[std::strlen(buffer) - 1] = NULL;
				buffer[8] = NULL;
				sscanf(buffer, "%x", &address);
				functions[address] = function;
			}

			_pclose(pipe);
			//delete []command;
		}

		mSymbols[systemNode].insert(std::make_pair(baseAddress, functions));	 
	}

	void PrintAddressInfoForTask(Uint32 task, Uint32 address) 
	{
		Uint32 base = 0;
		Uint32 offset = 0;
		String function;

		TaskNodeMap::const_iterator node = mTaskNodes.find(task);

		if (node != mTaskNodes.end()) {
			this->ExtractAddressInfo(node->second, address, function, base, offset);
		}

		this->PrintAddressInfo(function, address, base, offset);
	}

	void PrintAddressInfoForNode(Uint32 node, Uint32 address) 
	{
		Uint32 base = 0;
		Uint32 offset = 0;
		String function;

		this->ExtractAddressInfo(node, address, function, base, offset);
		this->PrintAddressInfo(function, address, base, offset);
	}

	void ExtractAddressInfo(Uint32 node, Uint32 address, String &function, Uint32 &base, Uint32 &offset)
	{
		function = "";
		base = 0;
		offset = 0;

		NodeAddressMap::const_iterator components = mComponents.find(node);

		if ((components != mComponents.end()) && (!components->second.empty())) {
			AddressMap::const_iterator loc = --((components->second).upper_bound(address));
			NodeAddressMap::iterator sym = mSymbols[node].find(loc->first);

			base = loc->first;
			offset = address - loc->first;

			if (sym == mSymbols[node].end()) {
            String module = loc->second;
            if (module.find(kTabMachine) == 0)
               module.replace(0, std::strlen(kTabMachine), "");
            this->IncludeLoadedFunctions(node, loc->first, mPathname + module);
				sym = mSymbols[node].find(loc->first);
			}

			AddressMap::const_iterator func = sym->second.upper_bound(offset);
			
			if (func == sym->second.end()) {
				loc = components->second.begin();
				sym = mSymbols[node].find(loc->first);
				func = sym->second.upper_bound(address);
			}

			if (func != sym->second.end() && func != sym->second.begin()) {
				--func;
				function = func->second;
			}
		}
	}

	void PrintAddressInfo(const String &function, Uint32 address, Uint32 base, Uint32 offset)
	{
		if (!function.empty()) {
			mStream << "0x";
			mStream.flags(std::ios::hex);
			mStream.width(8);
			mStream.fill('0');
			mStream << base;
			mStream << ":0x";
			mStream.flags(std::ios::hex);
			mStream.width(8);
			mStream.fill('0');
			mStream << offset;
			mStream << " ";
			mStream << function;
		}
		else {
			mStream << "0x";
			mStream.flags(std::ios::hex);
			mStream.width(8);
			mStream.fill('0');
			mStream << address;
		}
	}

protected:
	const String &mPathname;
	const TaskNodeMap &mTaskNodes;
	const NodeAddressMap &mComponents;
	const String &mCommand;
	
	SymbolsTable &mSymbols;
	Stream &mStream;

	String mAliasOld;
	String mAliasNew;
};

class PrintPagefault : public PrintSymbolData {
public:
	PrintPagefault(const String &pathname, 
			const String& osVersion,
			const TaskNodeMap &taskNodes,
			const NodeAddressMap &components, 
			const String &command,
			const String &alias,
			SymbolsTable &table, 
			Stream &stream) 
			: PrintSymbolData(pathname, osVersion, taskNodes, components, command, alias, table, stream)
	{
	}

	void operator()(const PageFaultInfo &record)
	{
		PageFaultInfo::const_iterator iter = record.begin();
		PageFaultInfo::const_iterator end = record.end();
		Uint32 task = 0;
		Uint32 address = 0;

		mStream <<  "Page Fault:" << std::endl;
		for (; iter != end; ++iter) {
			mStream << "\t" << *iter << std::endl;

			if ((*iter).find(kTabPC) != std::string::npos) {
				sscanf((*iter).c_str() + std::strlen(kTabPC), "%x", &address);
			}
			else if ((*iter).find(kTabTask) != std::string::npos) {
				sscanf((*iter).c_str() + std::strlen(kTabTask), "%x", &task);
			}
		}

		mStream << "\tFunction: ";
		this->PrintAddressInfoForTask(task, address); 
		mStream << std::endl;

		mStream << std::endl;
	}
};

class PrintStackTrace : public PrintSymbolData{
public:
	PrintStackTrace(const String &pathname, 
			const String& osVersion,
			const NodeAddressMap &components, 
			const TaskNameMap &taskNames, 
			const TaskNodeMap &taskNodes, 
			const StackTraceMap &stackTraceMap,
			const TaskStateMap &taskStates,
			const String &command,
			const String &alias,
			SymbolsTable &table, 
			Stream &stream) 
			: PrintSymbolData(pathname, osVersion, taskNodes, components, command, alias, table, stream),
			mTaskNames(taskNames), 
			mStackTraceMap(stackTraceMap),
			mTaskStates(taskStates)
	{
	}

	void operator()(const Uint32 &id)
	{
		TraceVector::const_iterator iter = mStackTraceMap.find(id)->second.begin();
		TraceVector::const_iterator end = mStackTraceMap.find(id)->second.end();
		TaskNameMap::const_iterator task = mTaskNames.find(id);
		TaskStateMap::const_iterator state = mTaskStates.find(id);
		TaskNodeMap::const_iterator node = mTaskNodes.find(id);
		NodeAddressMap::const_iterator components = mComponents.find(node->second);

		mStream << "Node: ";
		mStream << "0x";
		mStream.flags(std::ios::hex);
		mStream.width(8);
		mStream.fill('0');
		mStream << node->second; 
		mStream << " Task: ";
		mStream << "0x";
		mStream.flags(std::ios::hex);
		mStream.width(8);
		mStream.fill('0');
		mStream << id;  
		mStream	<< " Name: " ;
		mStream	<< task->second; 

		if (state != mTaskStates.end()) {
			mStream	<< " State: ";
			mStream	<< this->GetState(state->second[0]);
			mStream	<< " ";
			mStream	<< std::dec;
			mStream	<< state->second[1];
			mStream	<< " ";
			mStream	<< state->second[2];
		}

		mStream	<< std::endl;
		for (; iter != end; ++iter) {
			mStream << "\t";
			this->PrintAddressInfoForTask(id, *iter); 
			mStream << std::endl;
		}
		mStream << std::endl;
	}

	String GetState(Uint32 id)
	{
		String state;

		if (id & 0x01) {
			state += "SUSPEND ";
		}

		if (id & 0x02) {
			state += "PEND ";
		}

		if (id & 0x04) {
			state += "DELAY ";
		}

		if (id & 0x08) {
			state += "DEAD ";
		}

		if (id & 0x10) {
			state += "LOCKED ";
		}

		if (id & 0x20) {
			state += "INHERITED ";
		}

		if (id & 0x40) {
			state += "RUNNING ";
		}

		if (state.empty()) {
			state = "READY ";
		}

		return state;
	}

private:
	const StackTraceMap &mStackTraceMap;
	const TaskNameMap &mTaskNames;
	const TaskStateMap &mTaskStates;
};

class PrintAddressFunction : public PrintSymbolData {
public:
	PrintAddressFunction(const String &pathname, 
			const String& osVersion,
			const TaskNodeMap &taskNodes,
			const NodeAddressMap &components, 
			const String &command,
			const String &alias,
			SymbolsTable &table, 
			Stream &stream) 
			: PrintSymbolData(pathname, osVersion, taskNodes, components, command, alias, table, stream)
	{
	}

	void operator()(const String &addressString)
	{
		NodeAddressMap::const_iterator iter = mComponents.begin();
		NodeAddressMap::const_iterator end = mComponents.end();
		Uint32 address = 0;

		sscanf(addressString.c_str(), "%x", &address);

		mStream <<  "Address:" << addressString << std::endl;

		for (; iter != end; ++iter) {
			mStream << "\tNode: ";
			mStream << "0x";
			mStream.flags(std::ios::hex);
			mStream.width(8);
			mStream.fill('0');
			mStream << iter->first; 
			mStream << " Function: ";
			this->PrintAddressInfoForNode(iter->first, address); 
			mStream << std::endl;
		}

		mStream << std::endl;
	}
};

class PrintTaskSwitch {
public:
	PrintTaskSwitch(const TaskNameMap &tasks, Stream &stream) 
			: mTasks(tasks), mStream(stream) {}
	
	void operator()(const BinaryTaskSwitchRecord &record)
	{
		mStream.setf(std::ios::left);
		mStream.setf(std::ios::dec);
		mStream << "\t";
		mStream.width(15);
		mStream.precision(9);
		mStream.fill(' ');
		mStream << (double)record.seconds + (double)record.nanoSeconds / 1000000000.0;
		mStream << " ";
		mStream.width(15);
		mStream << this->GetTaskName(record.oldTask);
		mStream << " ";
		mStream.width(15);
		mStream << this->GetTaskName(record.newTask);
		mStream << std::endl;
	}

private:
	String GetTaskName(Uint32 id)
	{
		TaskNameMap::const_iterator iter = mTasks.find(id);
		TaskNameMap::const_iterator end = mTasks.end();
		std::ostringstream stream;

		if (iter != end) {
			stream.fill(' ');
			stream << iter->second;
		}
		else {
			stream << "0x";
			stream.flags(std::ios::hex);
			stream.width(8);
			stream.fill('0');
			stream << id;
		}

		return stream.str();
	}

	const TaskNameMap &mTasks;

	Stream &mStream;
};

class PrintMsgData {
public:
	PrintMsgData(const TaskNameMap &tasks, const MessageIDMap &messages, Stream &stream) 
			: mTasks(tasks), mMessages(messages), mStream(stream) {}
	
	void operator()(const BinarySendMsgRecord &record)
	{
		mStream.setf(std::ios::left);
		mStream.setf(std::ios::dec);
		mStream << "\t";
		mStream.width(15);
		mStream.precision(9);
		mStream.fill(' ');
		mStream << (double)record.seconds + (double)record.nanoSeconds / 1000000000.0;
		mStream << " ";
		mStream.width(15);
		mStream << this->GetTaskName(record.sendTask);
		mStream << " ";
		mStream.width(15);
		mStream << this->GetTaskName(record.rcvTask);
		mStream << " ";
		mStream << this->GetMeassgeName(record.msgID);
		mStream << std::endl;
	}

private:
	String GetTaskName(Uint32 id)
	{
		TaskNameMap::const_iterator iter = mTasks.find(id);
		TaskNameMap::const_iterator end = mTasks.end();
		std::ostringstream stream;

		if (iter != end) {
			stream.fill(' ');
			stream << iter->second;
		}
		else {
			stream << "0x";
			stream.flags(std::ios::hex);
			stream.width(8);
			stream.fill('0');
			stream << id;
		}

		return stream.str();
	}

	String GetMeassgeName(Uint32 id)
	{
		MessageIDMap::const_iterator iter = mMessages.find(id);
		MessageIDMap::const_iterator end = mMessages.end();
		std::ostringstream stream;

		if (iter != end) {
			stream.fill(' ');
			stream << iter->second;
		}
		else {
			stream << "0x";
			stream.flags(std::ios::hex);
			stream.width(8);
			stream.fill('0');
			stream << id;
		}

		return stream.str();
	}

	const TaskNameMap &mTasks;
	const MessageIDMap &mMessages;

	Stream &mStream;
};

class PrintNetworkStat {
public:
	PrintNetworkStat(Stream &stream) : mStream(stream) {}

	void operator()(const BinaryNetworkStatRecord &record)
	{
		mStream.fill(' ');
		mStream.setf(std::ios::left);
		mStream.setf(std::ios::dec);
		mStream << "Network stat data: ";
		mStream << (double)record.seconds + (double)record.nanoSeconds / 1000000000.0;
		mStream << std::endl;

		this->PrintAttrib("ipstat.ips_total", record.ipstat.ips_total, "total packets received");
		this->PrintAttrib("ipstat.ips_badsum", record.ipstat.ips_badsum, "checksum bad");
		this->PrintAttrib("ipstat.ips_tooshort", record.ipstat.ips_tooshort, "packet too short");
		this->PrintAttrib("ipstat.ips_toosmall", record.ipstat.ips_toosmall, "not enough data");
		this->PrintAttrib("ipstat.ips_badhlen", record.ipstat.ips_badhlen, "ip header length < data size");
		this->PrintAttrib("ipstat.ips_badlen", record.ipstat.ips_badlen, "ip length < ip header length");
		this->PrintAttrib("ipstat.ips_fragments", record.ipstat.ips_fragments, "fragments received");
		this->PrintAttrib("ipstat.ips_fragdropped", record.ipstat.ips_fragdropped, "frags dropped (dups, out of space)");
		this->PrintAttrib("ipstat.ips_fragtimeout", record.ipstat.ips_fragtimeout, "fragments timed out");
		this->PrintAttrib("ipstat.ips_forward", record.ipstat.ips_forward, "packets forwarded");
		this->PrintAttrib("ipstat.ips_cantforward", record.ipstat.ips_cantforward, "packets rcvd for unreachable dest");
		this->PrintAttrib("ipstat.ips_badsum", record.ipstat.ips_badsum, "packets rcvd for unreachable dest");
		this->PrintAttrib("ipstat.ips_redirectsent", record.ipstat.ips_redirectsent, "packets forwarded on same net");
		this->PrintAttrib("ipstat.ips_noproto", record.ipstat.ips_noproto, "unknown or unsupported protocol");
		this->PrintAttrib("ipstat.ips_delivered", record.ipstat.ips_delivered, "datagrams delivered to upper level");
		this->PrintAttrib("ipstat.ips_localout", record.ipstat.ips_localout, "total ip packets generated here");
		this->PrintAttrib("ipstat.ips_odropped", record.ipstat.ips_odropped, "lost packets due to nobufs, etc.");
		this->PrintAttrib("ipstat.ips_reassembled", record.ipstat.ips_reassembled, "total packets reassembled ok");
		this->PrintAttrib("ipstat.ips_fragmented", record.ipstat.ips_fragmented, "datagrams sucessfully fragmented");
		this->PrintAttrib("ipstat.ips_ofragments", record.ipstat.ips_ofragments, "output fragments created");
		this->PrintAttrib("ipstat.ips_cantfrag", record.ipstat.ips_cantfrag, "don't fragment flag was set, etc.");
		this->PrintAttrib("ipstat.ips_badoptions", record.ipstat.ips_badoptions, "error in option processing");
		this->PrintAttrib("ipstat.ips_noroute", record.ipstat.ips_noroute, "packets discarded due to no route");
		this->PrintAttrib("ipstat.ips_badvers", record.ipstat.ips_badvers, "ip version != 4");
		this->PrintAttrib("ipstat.ips_rawout", record.ipstat.ips_rawout, "total raw ip packets generated");

		this->PrintAttrib("tcpstat.tcps_connattempt", record.tcpstat.tcps_connattempt, "connections initiated");
		this->PrintAttrib("tcpstat.tcps_accepts", record.tcpstat.tcps_accepts, "connections accepted");
		this->PrintAttrib("tcpstat.tcps_connects", record.tcpstat.tcps_connects, "connections established");
		this->PrintAttrib("tcpstat.tcps_drops", record.tcpstat.tcps_drops, "connections dropped");
		this->PrintAttrib("tcpstat.tcps_conndrops", record.tcpstat.tcps_conndrops, "embryonic connections dropped");
		this->PrintAttrib("tcpstat.tcps_closed", record.tcpstat.tcps_closed, "conn. closed (includes drops)");
		this->PrintAttrib("tcpstat.tcps_segstimed", record.tcpstat.tcps_segstimed, "segs where we tried to get rtt");
		this->PrintAttrib("tcpstat.tcps_rttupdated", record.tcpstat.tcps_rttupdated, "times we succeeded");
		this->PrintAttrib("tcpstat.tcps_delack", record.tcpstat.tcps_delack, "delayed acks sent");
		this->PrintAttrib("tcpstat.tcps_timeoutdrop", record.tcpstat.tcps_timeoutdrop, "conn. dropped in rxmt timeout");
		this->PrintAttrib("tcpstat.tcps_rexmttimeo", record.tcpstat.tcps_rexmttimeo, "retransmit timeouts");
		this->PrintAttrib("tcpstat.tcps_persisttimeo", record.tcpstat.tcps_persisttimeo, "persist timeouts");
		this->PrintAttrib("tcpstat.tcps_keeptimeo", record.tcpstat.tcps_keeptimeo, "keepalive timeouts");
		this->PrintAttrib("tcpstat.tcps_keepprobe", record.tcpstat.tcps_keepprobe, "keepalive probes sent");
		this->PrintAttrib("tcpstat.tcps_keepdrops", record.tcpstat.tcps_keepdrops, "connections dropped in keepalive");
		this->PrintAttrib("tcpstat.tcps_sndtotal", record.tcpstat.tcps_sndtotal, "total packets sent");
		this->PrintAttrib("tcpstat.tcps_sndpack", record.tcpstat.tcps_sndpack, "data packets sent");
		this->PrintAttrib("tcpstat.tcps_sndbyte", record.tcpstat.tcps_sndbyte, "data bytes sent");
		this->PrintAttrib("tcpstat.tcps_sndrexmitpack", record.tcpstat.tcps_sndrexmitpack, "data packets retransmitted");
		this->PrintAttrib("tcpstat.tcps_sndrexmitbyte", record.tcpstat.tcps_sndrexmitbyte, "data bytes retransmitted");
		this->PrintAttrib("tcpstat.tcps_sndrexmitbyte", record.tcpstat.tcps_sndrexmitbyte, "ack-only packets sent");
		this->PrintAttrib("tcpstat.tcps_sndprobe", record.tcpstat.tcps_sndprobe, "window probes sent");
		this->PrintAttrib("tcpstat.tcps_sndprobe", record.tcpstat.tcps_sndprobe, "packets sent with URG only");
		this->PrintAttrib("tcpstat.tcps_sndwinup", record.tcpstat.tcps_sndwinup, "window update-only packets sent");
		this->PrintAttrib("tcpstat.tcps_sndctrl", record.tcpstat.tcps_sndctrl, "control (SYN|FIN|RST) packets sent");
		this->PrintAttrib("tcpstat.tcps_rcvtotal", record.tcpstat.tcps_rcvtotal, "total packets received");
		this->PrintAttrib("tcpstat.tcps_rcvpack", record.tcpstat.tcps_rcvpack, "packets received in sequence");
		this->PrintAttrib("tcpstat.tcps_rcvbyte", record.tcpstat.tcps_rcvbyte, "bytes received in sequence");
		this->PrintAttrib("tcpstat.tcps_rcvbadsum", record.tcpstat.tcps_rcvbadsum, "packets received with ccksum errs");
		this->PrintAttrib("tcpstat.tcps_rcvbadoff", record.tcpstat.tcps_rcvbadoff, "packets received with bad offset");
		this->PrintAttrib("tcpstat.tcps_rcvshort", record.tcpstat.tcps_rcvshort, "packets received too short");
		this->PrintAttrib("tcpstat.tcps_rcvduppack", record.tcpstat.tcps_rcvduppack, "duplicate-only packets received");
		this->PrintAttrib("tcpstat.tcps_rcvdupbyte", record.tcpstat.tcps_rcvdupbyte, "duplicate-only bytes received");
		this->PrintAttrib("tcpstat.tcps_rcvpartduppack", record.tcpstat.tcps_rcvpartduppack, "packets with some duplicate data");
		this->PrintAttrib("tcpstat.tcps_rcvpartdupbyte", record.tcpstat.tcps_rcvpartdupbyte, "dup. bytes in part-dup. packets");
		this->PrintAttrib("tcpstat.tcps_rcvoopack", record.tcpstat.tcps_rcvoopack, "out-of-order packets received");
		this->PrintAttrib("tcpstat.tcps_rcvoobyte", record.tcpstat.tcps_rcvoobyte, "out-of-order bytes received");
		this->PrintAttrib("tcpstat.tcps_rcvpackafterwin", record.tcpstat.tcps_rcvpackafterwin, "packets with data after window");
		this->PrintAttrib("tcpstat.tcps_rcvbyteafterwin", record.tcpstat.tcps_rcvbyteafterwin, "bytes rcvd after window");
		this->PrintAttrib("tcpstat.tcps_rcvafterclose", record.tcpstat.tcps_rcvafterclose, "packets rcvd after \"close\"");
		this->PrintAttrib("tcpstat.tcps_rcvwinprobe", record.tcpstat.tcps_rcvwinprobe, "rcvd window probe packets");
		this->PrintAttrib("tcpstat.tcps_rcvdupack", record.tcpstat.tcps_rcvdupack, "rcvd duplicate acks");
		this->PrintAttrib("tcpstat.tcps_rcvacktoomuch", record.tcpstat.tcps_rcvacktoomuch, "rcvd acks for unsent data");
		this->PrintAttrib("tcpstat.tcps_rcvackpack", record.tcpstat.tcps_rcvackpack, "rcvd ack packets");
		this->PrintAttrib("tcpstat.tcps_rcvackbyte", record.tcpstat.tcps_rcvackbyte, "bytes acked by rcvd acks");
		this->PrintAttrib("tcpstat.tcps_rcvwinupd", record.tcpstat.tcps_rcvwinupd, "rcvd window update packets");
		this->PrintAttrib("tcpstat.tcps_pawsdrop", record.tcpstat.tcps_pawsdrop, "segments dropped due to PAWS");
		this->PrintAttrib("tcpstat.tcps_predack", record.tcpstat.tcps_predack, "times hdr predict ok for acks");
		this->PrintAttrib("tcpstat.tcps_preddat", record.tcpstat.tcps_preddat, "times hdr predict ok for data pkts");
		this->PrintAttrib("tcpstat.tcps_pcbcachemiss", record.tcpstat.tcps_pcbcachemiss, "");
		this->PrintAttrib("tcpstat.tcps_persistdrop", record.tcpstat.tcps_persistdrop, "timeout in persist state");
		this->PrintAttrib("tcpstat.tcps_badsyn", record.tcpstat.tcps_badsyn, "bogus SYN, e.g. premature ACK");

		this->PrintAttrib("data_mbstat.mNum", record.data_mbstat.mNum, "mBlks obtained from page pool");
		this->PrintAttrib("data_mbstat.mDrops", record.data_mbstat.mDrops, "times failed to find space");
		this->PrintAttrib("data_mbstat.mWait", record.data_mbstat.mWait, "times waited for space");
		this->PrintAttrib("data_mbstat.mDrain", record.data_mbstat.mDrain, "times drained protocols for space");

		this->PrintAttrib("sys_mbstat.mNum", record.sys_mbstat.mNum, "mBlks obtained from page pool");
		this->PrintAttrib("sys_mbstat.mDrops", record.sys_mbstat.mDrops, "times failed to find space");
		this->PrintAttrib("sys_mbstat.mWait", record.sys_mbstat.mWait, "times waited for space");
		this->PrintAttrib("sys_mbstat.mDrain", record.sys_mbstat.mDrain, "times drained protocols for space");

		mStream << std::endl;
	}

private:
	void PrintAttrib(char *name, Uint32 value, char *info)
	{
		mStream << "\t";
		mStream.width(30);
		mStream << name;
		mStream.width(10);
		mStream << value;
		mStream << info;
		mStream << std::endl;
	}

	Stream &mStream;
};

/* Util Functors *************************************************************/

class IncludeModule {
public:
	IncludeModule(Uint32 address, String name) : mAddress(address), mName(name) {}

	void operator()(NodeAddressMap::value_type &data)
	{
		data.second[mAddress] = mName;
	}

private:
	Uint32 mAddress;
	String mName;
};

class IncludeSystem {
public:
	IncludeSystem(Uint32 address, String name, String paths) : 
			mAddress(address), 
			mName(name),
			mPaths(paths),
			mIndex(0),
			mPathnames(ParseValuesFromText(paths, ','))
	{
	}

	void operator()(NodeAddressMap::value_type &data)
	{
		if (mIndex < mPathnames.size()) {
			data.second[mAddress] = mPathnames[mIndex++] + "/" + mName;
		}
		else {
			data.second[mAddress] = "/" + mName;
		}
	}

private:
	Uint32 mAddress;
	String mName;
	String mPaths;

	ValueList mPathnames;

	Uint32 mIndex;
};

class IncludeTaskID {
public:
	IncludeTaskID(TaskIDList &taskIDList) : mTaskIDList(taskIDList) {}

	void operator()(const StackTraceMap::value_type &data)
	{
		mTaskIDList.push_back(data.first);
	}

private:
	TaskIDList &mTaskIDList;
};

class IncludeNodeID {
public:
	IncludeNodeID(NodeIDList &nodeIDList) : mNodeIDList(nodeIDList) {}

	void operator()(const TaskNodeMap::value_type &data)
	{
		mNodeIDList.insert(data.second);
	}

private:
	NodeIDList &mNodeIDList;
};

class SortTaskID {
public:
	SortTaskID(const TaskNodeMap &taskNodeMap, const TaskStateMap &taskStateMap)
			: mTaskNodeMap(taskNodeMap), mTaskStateMap(taskStateMap) {}

	bool operator()(const Uint32 &lhs, const Uint32 &rhs)
	{
		Uint32 leftNodeID = this->GetNodeID(lhs);
		Uint32 rightNodeID = this->GetNodeID(rhs);

		return leftNodeID == rightNodeID 
				? this->GetState(lhs) < this->GetState(rhs)
				: leftNodeID < rightNodeID;
	}

private:
	Uint32 GetNodeID(Uint32 task)
	{
		TaskNodeMap::const_iterator iter = mTaskNodeMap.find(task);

		return iter != mTaskNodeMap.end() ? iter->second : 0;
	}

	Uint32 GetState(Uint32 task)
	{
		TaskStateMap::const_iterator iter = mTaskStateMap.find(task);

		return iter != mTaskStateMap.end() ? iter->second[0] : 0;
	}

	const TaskNodeMap &mTaskNodeMap;
	const TaskStateMap &mTaskStateMap;
};

class FilterTaskSwitch {
public:
	FilterTaskSwitch(Uint32 node, TaskNodeMap maping, PrintTaskSwitch &functor) 
			: mNode(node), mMaping(maping),mFunctor(functor) {}

	void operator()(const BinaryTaskSwitchRecord &record)
	{
		TaskNodeMap::iterator iter = mMaping.find(record.newTask);
		TaskNodeMap::iterator end = mMaping.end();

		if ((iter != end) && ((*iter).second == mNode)) {
			mFunctor(record);
		}
	}

private:
	Uint32 mNode;
	TaskNodeMap mMaping;
	PrintTaskSwitch &mFunctor;
};

class FilterSendMsgData {
public:
	FilterSendMsgData(Uint32 node, TaskNodeMap maping, PrintMsgData &functor) 
			: mNode(node), mMaping(maping),mFunctor(functor) {}

	void operator()(const BinarySendMsgRecord &record)
	{
		TaskNodeMap::iterator iter = mMaping.find(record.sendTask);
		TaskNodeMap::iterator end = mMaping.end();

		if ((iter != end) && ((*iter).second == mNode)) {
			mFunctor(record);
		}
	}

private:
	Uint32 mNode;
	TaskNodeMap mMaping;
	PrintMsgData &mFunctor;
};

} // namespace UTIL
} // namespace TRACE

#endif // STACKTRACEUTIL_H

