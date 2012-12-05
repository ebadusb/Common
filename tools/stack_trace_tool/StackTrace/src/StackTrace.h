/*****************************************************************************\
**
** StackTrace.h
**
** Stack trace implementation.
**
\*****************************************************************************/

#ifndef STACKTRACE_H
#define STACKTRACE_H

/* Includes ******************************************************************/

#include <string>
#include <sstream>

#include "LogReader.h"
#include "LogExtractor.h"
#include "StackTraceUtil.h"

namespace TRACE {

/* Types *********************************************************************/

typedef DECODER::ProcessFilterLog<DECODER::RecordStreamOutput, DECODER::UTIL::ProcessGetSystemName> FilterGetSystemPath;
typedef DECODER::ProcessFilterLog<DECODER::RecordTextOutput, DECODER::UTIL::ProcessLoadedComponent> FilterLoadedComponent;
typedef DECODER::ProcessFilterLog<DECODER::RecordStreamOutput, DECODER::UTIL::ProcessPageFault> FilterPageFault;
typedef DECODER::ProcessFilterLog<DECODER::RecordStreamOutput, DECODER::UTIL::ProcessMessageID> FilterMessageID;
typedef DECODER::ProcessFilterLog<DECODER::RecordStreamOutput, DECODER::UTIL::ProcessStackTrace> FilterStackTrace;
typedef DECODER::ProcessFilterLog<DECODER::RecordStreamOutput, DECODER::UTIL::ProcessTaskState> FilterTaskState;

/* Classes *******************************************************************/

class StackTrace : public DECODER::LogExtractor {
public:
	typedef std::string String;
	typedef std::ostream Stream;
	typedef std::map<std::string, std::string> Arguments;
	typedef DECODER::LogReader LogReader;
	typedef DECODER::Result Result;

	/**
	 *
	 * StackTrace
	 *
	 * Constructor
	 *
	 * @param stream The report stream.
	 *
	 */
	StackTrace(Stream &stream);

	/**
	 *
	 * Register
	 *
	 * Registers the necessary handlers with the reader.
	 *
	 * @param reader The file reader.
	 *
	 */
	virtual void Register(LogReader &reader);

	/**
	 *
	 * Unregister
	 *
	 * Unregisters the handlers from the reader.
	 *
	 * @param reader The file reader.
	 *
	 */
	virtual void Unregister(LogReader &reader);

	/**
	 *
	 * PreProcess
	 *
	 * Perform the procedures before the import process.
	 *
	 */
	virtual void PreProcess(void);

	/**
	 *
	 * PostProcess
	 *
	 * Perform the procedures after the import process.
	 *
	 */
	virtual void PostProcess(void);

	/**
	 *
	 * CreateCrashReport
	 *
	 * Utility function that creates a crash report for the given datalog.
	 *
	 * @param filename The filename of the file.
	 * @param stream The report stream.
	 *
	 * @return The results.
	 *
	 */
	static Result CreateCrashReport(const String &filename, Stream &stream);

	/**
	 *
	 * CreateCrashReport
	 *
	 * Utility function that creates a crash report for the given datalog.
	 *
	 * @param filename The filename of the file.
	 * @param buildfolder The buil folder.
	 * @param stream The report stream.
	 *
	 * @return The results.
	 *
	 */
	static Result CreateCrashReport(const String &filename, const Arguments &arguments, Stream &stream);

	static void StackTrace::Main(std::string args );

private:
	String GetBuildPath(void) const;
	String GetOsVersion(void) const;
	String GetSystemFile(void) const;
	String GetSymbolCommand(void);
	String GetSystemPaths(void);
	String GetPathAlias(void) const;
	String GetAddresses(void) const;
	
	//void GuessCommandLineSettings(void);
	
	Stream &mStream;

	// Alternate values
	String mAltBuildPath;
	String mAltSystemName;
	String mAltSymbolCommand;
	String mAltSystemPaths;
	String mAltPathAlias;
	String mAltAddress;

	String mBuildPath;
	String mSystemName;
	String mSystemPaths;
	String mPathAlias;
	String mAddress;
	String mOsVersion;

	// Data collected
	DECODER::RecordLogLevel mLevelVXWorksInfo;
	DECODER::RecordLogLevel mLevelCriticalOutput;
	DECODER::RecordLogLevel mLevelStartupDetails;
	DECODER::RecordLogLevel mLevelRouterInfo;
	DECODER::RecordLogInfo mRecordLogInfo;

	DECODER::String mDatalogInfo;
	DECODER::UTIL::TaskNameMap mTasksNames;
	DECODER::UTIL::TaskNodeMap mTasksNodes;
	DECODER::UTIL::TaskStateMap mTaskState;
	DECODER::UTIL::NodeAddressMap mLoadedComponents;
	DECODER::UTIL::FaultVector mPagefaults;
	DECODER::UTIL::MessageIDMap mMessageIDMap;
	DECODER::UTIL::StackTraceMap mStackTraceMap;
	DECODER::UTIL::TaskSwitchRecords mTaskSwitchRecords;
	DECODER::UTIL::SendMsgRecords mSendMsgRecords;
	DECODER::UTIL::NetworkStatRecords mNetworkStatRecords;

	// Process Functors
	DECODER::ProcessGetLevelID mProcessGetVXWorksInfo;
	DECODER::ProcessGetLevelID mProcessGetCriticalOutput;
	DECODER::ProcessGetLevelID mProcessGetStartupDetails;
	DECODER::ProcessGetLevelID mProcessGetMessageInfo;

	DECODER::UTIL::ProcessGetLogInfo mProcessGetLogInfo;
	DECODER::UTIL::ProcessGetBuildPath mProcessGetBuildPath;
	DECODER::UTIL::ProcessGetSystemName mProcessGetSystemName;
	DECODER::UTIL::ProcessGetTasksNames mProcessGetTasksNames;
	DECODER::UTIL::ProcessGetTasksNodes mProcessGetTasksNodes;
	DECODER::UTIL::ProcessTaskState mProcessTaskState;
	DECODER::UTIL::ProcessLoadedComponent mProcessLoadedComponent;
	DECODER::UTIL::ProcessPageFault mProcessPageFault;
	DECODER::UTIL::ProcessMessageID mProcessMessageID;
	DECODER::UTIL::ProcessStackTrace mProcessStackTrace;
	DECODER::UTIL::ProcessTaskSwitch mProcessTaskSwitch;
	DECODER::UTIL::ProcessSendMsg mProcessSendMsg;
	DECODER::UTIL::ProcessNetworkStat mProcessNetworkStat;

	// Filter Functors
	FilterGetSystemPath mFilterGetSystemName;
	FilterLoadedComponent mFilterLoadedComponent1;
	FilterLoadedComponent mFilterLoadedComponent2;
	FilterPageFault mFilterPageFault;
	FilterMessageID mFilterMessageID;
	FilterStackTrace mFilterStackTrace;
	FilterTaskState mFilterTaskState;
};


} // namespace TRACE

#endif // STACKTRACE_H

