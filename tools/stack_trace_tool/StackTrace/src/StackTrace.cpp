/*****************************************************************************\
**
** StackTrace.h
**
** Stack trace implementation.
**
\*****************************************************************************/

/* Includes ******************************************************************/

#include <algorithm>
#include <iostream>
#include <string>
#include <sstream>
#include <iterator>
#include <fstream>
#include <cstdio>
#include <cstdlib>

#include <process.h>
#include <conio.h>

#include <sys/stat.h>

#include "StackTrace.h"
#include "LogExecute.h"


namespace TRACE {

/* Constants *****************************************************************/

// Valid tool arguments
static const char *kFlagLocation = "-location";
static const char *kFlagVersion = "-version";
static const char *kFlagOSFile = "-osfile";
static const char *kFlagOSPath = "-ospath";
static const char *kFlagPathalias = "-pathalias";
static const char *kFlagAddress = "-address";

// Valid environmental variables
static const char *kEnvLocation = "STACKTRACE_location";
static const char *kEnvVersion = "STACKTRACE_version";
static const char *kEnvOSFile = "STACKTRACE_osfile";
static const char *kEnvOSPath = "STACKTRACE_ospath";
static const char *kEnvPathalias = "STACKTRACE_pathalias";
static const char *kEnvAddress  = "STACKTRACE_address";

//Program Strings
static const char* splitArgs = " \t";

/* Constants - Used in main.cpp  *********************************************/

static const char *kStrErrorHelp = 
		"StackTrace version 1.9\n"
		"Creates a crash report from the data found in a given datalog.\n"
		"StackTrace [-location buildpath] [-version (2.02 | 2.20)] [-osfile filename] [-ospath pathnames] [-pathalias old[:new]] [-address values] filename\n";
static const char *kStrErrorFailed = "Error: Failed to open file.";
static const char *kStrErrorInvalid = "Error: Invalid header type.";
static const char *kStrErrorUnsupported = "Error: Log file version is unsupported.";

static const char *kEditorPath = "C:\\Windows\\Notepad.exe";

//All stack trace files will be stored in this location.
static const char *kTraceFilePath = "C:\\Temp\\";
static const char *kTraceFileExtn = ".trace";

	/* Functions *****************************************************************/

/**
 *
 * StackTrace
 *
 * Constructor
 *
 * @param stream The report stream.
 *
 */
StackTrace::StackTrace(Stream &stream) : 
		mStream(stream),
		mProcessGetVXWorksInfo("vxworks_info", mLevelVXWorksInfo),
		mProcessGetCriticalOutput("CriticalOutput", mLevelCriticalOutput),
		mProcessGetStartupDetails("startup_detail", mLevelStartupDetails),
		mProcessGetMessageInfo("router_info", mLevelRouterInfo),
		mProcessGetLogInfo(mDatalogInfo, mRecordLogInfo),
		mProcessGetBuildPath(mBuildPath),
		mProcessGetSystemName(mSystemName),
		mProcessGetTasksNames(mTasksNames),
		mProcessGetTasksNodes(mTasksNodes),
		mProcessTaskState(mTaskState),
		mProcessLoadedComponent(mLoadedComponents),
		mProcessPageFault(mPagefaults),
		mProcessMessageID(mMessageIDMap),
		mProcessStackTrace(mStackTraceMap),
		mProcessTaskSwitch(mTaskSwitchRecords),
		mProcessSendMsg(mSendMsgRecords),
		mProcessNetworkStat(mNetworkStatRecords),
		mFilterGetSystemName(mLevelStartupDetails, mProcessGetSystemName),
		mFilterLoadedComponent1(mLevelVXWorksInfo, mProcessLoadedComponent),
		mFilterLoadedComponent2(mLevelStartupDetails, mProcessLoadedComponent),
		mFilterPageFault(mLevelVXWorksInfo, mProcessPageFault),
		mFilterMessageID(mLevelRouterInfo, mProcessMessageID),
		mFilterStackTrace(mLevelCriticalOutput, mProcessStackTrace),
		mFilterTaskState(mLevelCriticalOutput, mProcessTaskState)
{
}

/**
 *
 * Register
 *
 * Registers the necessary handlers with the reader.
 *
 * @param reader The file reader.
 *
 */
void StackTrace::Register(LogReader &reader)
{
	reader.RegisterLogLevel(mProcessGetVXWorksInfo);
	reader.RegisterLogLevel(mProcessGetCriticalOutput);
	reader.RegisterLogLevel(mProcessGetStartupDetails);
	reader.RegisterLogLevel(mProcessGetMessageInfo);
	reader.RegisterLogInfo(mProcessGetLogInfo);
	reader.RegisterLogInfo(mProcessGetBuildPath);
	reader.RegisterTaskCreated(mProcessGetTasksNames);
	reader.RegisterTaskCreated(mProcessGetTasksNodes);
	reader.RegisterStreamOutput(mFilterGetSystemName);
	reader.RegisterStreamOutput(mFilterPageFault);
	reader.RegisterStreamOutput(mFilterMessageID);
	reader.RegisterStreamOutput(mFilterTaskState);
	reader.RegisterStreamOutput(mFilterLoadedComponent1);
	reader.RegisterStreamOutput(mFilterLoadedComponent2);
	reader.RegisterStreamOutput(mFilterPageFault);
	reader.RegisterStreamOutput(mFilterStackTrace);
	reader.RegisterPrintOutput(mFilterLoadedComponent1);
	reader.RegisterPrintOutput(mFilterLoadedComponent2);
	reader.RegisterBinary(mProcessTaskSwitch);
	reader.RegisterBinary(mProcessSendMsg);
	reader.RegisterBinary(mProcessNetworkStat);
}

/**
 *
 * Unregister
 *
 * Unregisters the handlers from the reader.
 *
 * @param reader The file reader.
 *
 */
void StackTrace::Unregister(LogReader &reader)
{
	reader.UnregisterLogLevel(mProcessGetVXWorksInfo);
	reader.UnregisterLogLevel(mProcessGetCriticalOutput);
	reader.UnregisterLogLevel(mProcessGetStartupDetails);
	reader.UnregisterLogLevel(mProcessGetMessageInfo);
	reader.UnregisterLogInfo(mProcessGetLogInfo);
	reader.UnregisterLogInfo(mProcessGetBuildPath);
	reader.UnregisterTaskCreated(mProcessGetTasksNames);
	reader.UnregisterTaskCreated(mProcessGetTasksNodes);
	reader.UnregisterStreamOutput(mFilterGetSystemName);
	reader.UnregisterStreamOutput(mFilterPageFault);
	reader.UnregisterStreamOutput(mFilterMessageID);
	reader.UnregisterStreamOutput(mFilterTaskState);
	reader.UnregisterStreamOutput(mFilterLoadedComponent1);
	reader.UnregisterStreamOutput(mFilterLoadedComponent2);
	reader.UnregisterStreamOutput(mFilterPageFault);
	reader.UnregisterStreamOutput(mFilterStackTrace);
	reader.UnregisterPrintOutput(mFilterLoadedComponent1);
	reader.UnregisterPrintOutput(mFilterLoadedComponent2);
	reader.UnregisterBinary(mProcessTaskSwitch);
	reader.UnregisterBinary(mProcessSendMsg);
	reader.UnregisterBinary(mProcessNetworkStat);
}

/**
 *
 * PreProcess
 *
 * Perform the procedures before the import process.
 *
 */
void StackTrace::PreProcess(void)
{
	mDatalogInfo = "";
	mBuildPath = "";
	mSystemName = "";
	mSystemPaths = "";
	mAddress = "";
	mTasksNames.clear();
	mTasksNodes.clear();
	mTaskState.clear();
	mLoadedComponents.clear();
	mPagefaults.clear();
	mMessageIDMap.clear();
	mStackTraceMap.clear();
	mTaskSwitchRecords.clear();
	mSendMsgRecords.clear();
	mNetworkStatRecords.clear();
}

/**
 *
 * PostProcess
 *
 * Perform the procedures after the import process.
 *
 */
void StackTrace::PostProcess(void)
{
	GuessCommandLineSettings();

	DECODER::UTIL::SymbolsTable symbolTable;
	DECODER::UTIL::TaskIDList taskIDList;
	DECODER::UTIL::NodeIDList nodeIDList;
	DECODER::String pathname(this->GetBuildPath());
	DECODER::String osVersion(this->GetOsVersion());
	DECODER::String system(this->GetSystemFile());
	DECODER::String command(this->GetSymbolCommand());
	DECODER::String subfolders(this->GetSystemPaths());
	DECODER::String alias(this->GetPathAlias());
	DECODER::String addresses(this->GetAddresses());
	DECODER::UTIL::IncludeSystem includeSystem(0, system, subfolders);
	DECODER::UTIL::IncludeTaskID includeTaskID(taskIDList);
	DECODER::UTIL::IncludeNodeID includeNodeID(nodeIDList);

	for_each(mLoadedComponents.rbegin(), mLoadedComponents.rend(), includeSystem);
	for_each(mStackTraceMap.rbegin(), mStackTraceMap.rend(), includeTaskID);
	for_each(mTasksNodes.rbegin(), mTasksNodes.rend(), includeNodeID);

	DECODER::UTIL::PrintModules printModules(mStream);
	DECODER::UTIL::PrintAddresses printMessages(mStream);
	DECODER::UTIL::PrintPagefault printPagefault(
			pathname, osVersion, mTasksNodes, mLoadedComponents, 
			command, alias, symbolTable, mStream);
	DECODER::UTIL::PrintStackTrace printStackTrace(
			pathname, osVersion, mLoadedComponents, mTasksNames, mTasksNodes,
			mStackTraceMap, mTaskState, command, alias, symbolTable, mStream);
	DECODER::UTIL::PrintAddressFunction printAddressFunction(
			pathname, osVersion, mTasksNodes, mLoadedComponents, 
			command, alias, symbolTable, mStream);
	DECODER::UTIL::PrintTaskSwitch printTaskSwitch(mTasksNames, mStream);
	DECODER::UTIL::PrintMsgData printMsgData(mTasksNames, mMessageIDMap, mStream);
	DECODER::UTIL::PrintNetworkStat printNetworkStat(mStream);

	mStream << mDatalogInfo << std::endl << std::endl;

	if (!mLoadedComponents.empty()) {
		for_each(mLoadedComponents.rbegin(), mLoadedComponents.rend(), printModules);
	}


	if (!mPagefaults.empty()) {
		for_each(mPagefaults.rbegin(), mPagefaults.rend(), printPagefault);
		mStream << std::endl;
	}

	if (!taskIDList.empty()) {
		taskIDList.sort(DECODER::UTIL::SortTaskID(mTasksNodes, mTaskState));
		for_each(taskIDList.begin(), taskIDList.end(), printStackTrace);
		mStream << std::endl;
	}

	if (!mTaskSwitchRecords.empty()) {
		DECODER::UTIL::NodeIDList::reverse_iterator riter = nodeIDList.rbegin();
		DECODER::UTIL::NodeIDList::reverse_iterator rend = nodeIDList.rend();

		for (;riter != rend; ++riter) {
			DECODER::UTIL::FilterTaskSwitch filter(*riter, mTasksNodes, printTaskSwitch);

			mStream << "Task switch data for Node: 0x:";
			mStream.flags(std::ios::hex);
			mStream.width(8);
			mStream.fill('0');
			mStream << *riter;
			mStream << std::endl;
			mStream << "\tTIME            OLD TASK        NEW TASK" << std::endl;
			for_each(mTaskSwitchRecords.rbegin(), mTaskSwitchRecords.rend(), filter);
			mStream << std::endl;
		}
		mStream << std::endl;
	}

	if (!mMessageIDMap.empty()) {
		mStream << "Registered Messages:" << std::endl;
		for_each(mMessageIDMap.begin(), mMessageIDMap.end(), printMessages);
		mStream << std::endl;
	}

	if (!mSendMsgRecords.empty()) {
		DECODER::UTIL::NodeIDList::reverse_iterator riter = nodeIDList.rbegin();
		DECODER::UTIL::NodeIDList::reverse_iterator rend = nodeIDList.rend();

		for (;riter != rend; ++riter) {
			DECODER::UTIL::FilterSendMsgData filter(*riter, mTasksNodes, printMsgData);

			mStream << "Message data for Node: 0x:";
			mStream.flags(std::ios::hex);
			mStream.width(8);
			mStream.fill('0');
			mStream << *riter;
			mStream << std::endl;
			mStream << "\tTIME            SEND TASK       RCV TASK        MESSAGE ID" << std::endl;
			for_each(mSendMsgRecords.rbegin(), mSendMsgRecords.rend(), filter);
			mStream << std::endl;
		}
		mStream << std::endl;
	}

	if (!mNetworkStatRecords.empty()) {
		mStream << std::endl;
		for_each(mNetworkStatRecords.rbegin(), mNetworkStatRecords.rend(), printNetworkStat);
		mStream << std::endl;
	}

	if (!addresses.empty()) {
		DECODER::UTIL::ValueList values(DECODER::UTIL::ParseValuesFromText(addresses, ','));
		DECODER::UTIL::ValueList::const_iterator iter = values.begin();
		DECODER::UTIL::ValueList::const_iterator end = values.end();

		for_each(iter, end, printAddressFunction);
	}
}

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
StackTrace::Result StackTrace::CreateCrashReport(const String &filename, Stream &stream)
{
	return StackTrace::CreateCrashReport(filename, Arguments(), stream);
}

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
StackTrace::Result StackTrace::CreateCrashReport(const String &filename, const Arguments &arguments, Stream &stream)
{
	
	StackTrace extractor(stream);
	Arguments::const_iterator flagLocationIter = arguments.find(kFlagLocation);
	Arguments::const_iterator flagOSFileIter = arguments.find(kFlagOSFile);
	Arguments::const_iterator flagOSPathIter = arguments.find(kFlagOSPath);
	Arguments::const_iterator flagVersionIter = arguments.find(kFlagVersion);
	Arguments::const_iterator flagAliasIter = arguments.find(kFlagPathalias);
	Arguments::const_iterator flagAddress = arguments.find(kFlagAddress);
	Arguments::const_iterator flagEnd = arguments.end();

	const char *envLocation = std::getenv(kEnvLocation);
	const char *envOSFile = std::getenv(kEnvOSFile);
	const char *envOSPath = std::getenv(kEnvOSPath);
	const char *envVersion = std::getenv(kEnvVersion);
	const char *envAlias = std::getenv(kEnvPathalias);
	const char *envAddress = std::getenv(kEnvAddress);

	extractor.mAltBuildPath = "";
	extractor.mAltSystemName = "";
	extractor.mAltSystemPaths = "";
	extractor.mAltSymbolCommand = "";
	extractor.mAltPathAlias = "";
	extractor.mAltAddress = "";
	
	// Set build path setting
	if (flagLocationIter != flagEnd){
		extractor.mAltBuildPath = flagLocationIter->second;
	}
	else if (envLocation != NULL) {
		extractor.mAltBuildPath = envLocation;
	}

	// Set OS binary name
	if (flagOSFileIter != flagEnd){
		extractor.mAltSystemName = flagOSFileIter->second;
	}
	else if (envOSFile != NULL) {
		extractor.mAltSystemName = envOSFile;
	}

	// Set OS path names
	if (flagOSPathIter != flagEnd) {
		extractor.mAltSystemPaths = flagOSPathIter->second;
	}
	else if (envOSPath != NULL) {
		extractor.mAltSystemPaths = envOSPath;
	}

	// Set path alias
	if (flagAliasIter != flagEnd){
		extractor.mAltPathAlias = flagAliasIter->second;
	}
	else if (envAlias != NULL) {
		extractor.mAltPathAlias = envAlias;
	}

	// Set build address setting
	if (flagAddress != flagEnd){
		extractor.mAltAddress = flagAddress->second;
	}
	else if (envLocation != NULL) {
		extractor.mAltAddress = envAddress;
	}

	if (flagVersionIter != flagEnd) {
		if (flagVersionIter->second == DECODER::UTIL::kVerTornado2_20) {
			extractor.mAltSymbolCommand = DECODER::UTIL::kStrCommand2_20;
			extractor.mOsVersion = DECODER::UTIL::kVerTornado2_20;
		}
		else if (flagVersionIter->second == DECODER::UTIL::kVerTornado2_02) {
			extractor.mAltSymbolCommand = DECODER::UTIL::kStrCommand2_02;
			extractor.mOsVersion = DECODER::UTIL::kVerTornado2_02;
		}
	}
	else if (envVersion != NULL) {
		if (envVersion == DECODER::UTIL::kVerTornado2_20) {
			extractor.mAltSymbolCommand = DECODER::UTIL::kStrCommand2_20;
			extractor.mOsVersion = DECODER::UTIL::kVerTornado2_20;
		}
		else if (envVersion == DECODER::UTIL::kVerTornado2_02) {
			extractor.mAltSymbolCommand = DECODER::UTIL::kStrCommand2_02;
			extractor.mOsVersion = DECODER::UTIL::kVerTornado2_02;
		}
	}

	
		
	//Set pathnames if not provided. For eg. Trima pathnames are always /vxboot,/trima/safety/boot

			
	return DECODER::ProcessLogData(filename.c_str(), extractor);
}

/**
 *
 * GetOsVersion
 *
 * Gets the Tornado Version
 *
 * @return The Tornado Version.
 *
 */
void StackTrace::GuessCommandLineSettings(void) 
{
	const DECODER::RecordLogInfo& info = mRecordLogInfo;
	const String& log = info.mLogFilename;

	//OsVersion
	if(mAltSymbolCommand.empty())
	{
		
		const String& rev = info.mBuildRevision;
		
		//Is Trima?
		if(info.mDeviceType == DECODER::TRIMA)
		{
			 //Trima versions 5.1 or 5.2
			if( (rev.find("6.") != rev.npos) || (rev.find("7.") != rev.npos) ) 
				mAltSymbolCommand = DECODER::UTIL::kStrCommand2_02;
			
			else 
				mAltSymbolCommand = DECODER::UTIL::kStrCommand2_20;
			
		}
		else 
		{
			mAltSymbolCommand = DECODER::UTIL::kStrCommand2_20;
		}
		
	}
	

	//OsPath - Trima - ospath is /vxboot
	if(mAltSystemPaths.empty())
	{
		if(info.mDeviceType == DECODER::TRIMA)
		{
			mAltSystemPaths.append(DECODER::UTIL::kAltSysPathTrima);
		}
		else if(info.mDeviceType == DECODER::OPTIA)
		{
			mAltSystemPaths.append(DECODER::UTIL::kAltSysPathOptia);
		}
		else if(info.mDeviceType == DECODER::CES)
		{
			mAltSystemPaths.append(DECODER::UTIL::kAltSysPathCes);
		}
	}
}

/**
 *
 * GetBuildPath
 *
 * Gets the build path.
 *
 * @return The path.
 *
 */
DECODER::String StackTrace::GetBuildPath(void) const
{
	String& pathname = (mAltBuildPath.empty() ? mBuildPath : mAltBuildPath) + DECODER::UTIL::kStrCurrent;

	if (mAltBuildPath.empty() && (mRecordLogInfo.mDeviceType == DECODER::TRIMA) && (pathname.find("AutomatedBuilds") != pathname.npos))
	{
		
		if(pathname.find("I80486.current_build") == pathname.npos) {
			String insertStr("I80486.");
			pathname.insert(pathname.find("current_build"), insertStr);
		}
	}

	return pathname;
}

/**
 *
 * GetOsVersion
 *
 * Gets the Tornado version
 *
 * @return The tornado version.
 *
 */
DECODER::String StackTrace::GetOsVersion(void) const
{
	return mOsVersion;
}

/**
 *
 * GetSystemPath
 *
 * Gets the system path.
 *
 * @return The path.
 *
 */
DECODER::String StackTrace::GetSystemFile(void) const
{
	DECODER::String system(mAltSystemName.empty() ? mSystemName : mAltSystemName);

	return system.empty() ? DECODER::UTIL::kPathSystemCommon : system;
}

/**
 *
 * GetSymbolCommand
 *
 * Gets the command for extracting symbols names.
 *
 * @return The command.
 *
 */
DECODER::String StackTrace::GetSymbolCommand(void) const
{
	//Look at mDatalogInfo. Find the BUILD REVISION. Find out if Build uses T22 or T202. 
	return mAltSymbolCommand.empty() ? DECODER::UTIL::kStrCommand2_20 : mAltSymbolCommand;
}

/**
 *
 * GetSystemPaths
 *
 * Gets the folder names for the operating systems.
 *
 * @return The paths.
 *
 */
DECODER::String StackTrace::GetSystemPaths(void) const
{
	return mAltSystemPaths.empty() ? mSystemPaths : mAltSystemPaths;
}

/**
 *
 * GetPathAlias
 *
 * Gets the path alias.
 *
 * @return The alias.
 *
 */
DECODER::String StackTrace::GetPathAlias(void) const
{
	return mAltPathAlias.empty() ? mPathAlias : mAltPathAlias;
}

/**
 *
 * GetPathAlias
 *
 * Gets the path alias.
 *
 * @return The alias.
 *
 */
DECODER::String StackTrace::GetAddresses(void) const
{
	return mAltAddress.empty() ? mAddress : mAltAddress;
}

/**
 *
 * Main
 *
 * To be invoked from Managed code.
 *
 * @return The alias.
 *
 */
void StackTrace::Main(std::string args)
{

	using namespace std;
	std::ostringstream output;
	DECODER::Result results = DECODER::LogReader::kResultNone;
	TRACE::StackTrace::Arguments arguments;
	vector<string> argv;
	istringstream iss(args);
	copy(istream_iterator<string>(iss),
         istream_iterator<string>(),
         back_inserter<vector<string> >(argv));
	int argc = argv.size();

	for (int i = 1; i < argc - 1; ++i) {
		if (((i != argc - 2) && argv[i][0] == '-') && (argv[i + 1][0] != '-')) {
			arguments[argv[i]] = argv[i+1];
			i++;
		}
		else if (argv[i][0] == '-') {
			arguments[argv[i]] = "";
		}
		else {
			std::cerr << kStrErrorHelp << std::endl;
			exit(0);
		}
	}
	// Launch dlog file if already decoded.	
	std::string dlogpath(argv[argc - 1]);
	std::string dlogname( dlogpath.substr( dlogpath.find_last_of('\\') + 1) );
	std::string tracefilename(kTraceFilePath + dlogname + kTraceFileExtn);
	std::ifstream itf(tracefilename);
	std::string editorCmd(kEditorPath);
	if(itf)
	{
		//Launch
		
		_execl(editorCmd.c_str(), editorCmd.c_str(), tracefilename.c_str(), NULL);
	}

	results = TRACE::StackTrace::CreateCrashReport(dlogpath, arguments, output);

	switch(results)
	{
	case DECODER::LogReader::kResultNone:
	case DECODER::LogReader::kResultUnexpectedEOF:
	case DECODER::LogReader::kResultUnknownRecordType:
		{
			std::ofstream tracefile(tracefilename.c_str(), std::ios_base::out);
			tracefile << output.str() << std::endl;
			tracefile.close();
			//Launch
			_execl(editorCmd.c_str(), editorCmd.c_str(), tracefilename.c_str(), NULL);
		break;
		}
	case DECODER::LogReader::kResultFileOpenFailed:
		std::cerr << kStrErrorFailed << std::endl;
		break;
	case DECODER::LogReader::kResultInvalidHeaderType:
		std::cerr << kStrErrorInvalid << std::endl;
		break;
	case DECODER::LogReader::kResultInvalidVersion:
		std::cerr << kStrErrorUnsupported << std::endl;
		break;
	}
}
} // namespace TRACE

