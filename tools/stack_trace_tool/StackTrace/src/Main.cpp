/*****************************************************************************\
**
** Main.cpp
**
** Application entry point.
**
\*****************************************************************************/

/* Includes ******************************************************************/

#include <iostream>
#include <fstream>
#include <process.h>
#include "LogReader.h"
#include "StackTrace.h"

/* Constants *****************************************************************/

static const char *kStrErrorHelp = 
		"StackTrace version 1.9\n"
		"Creates a crash report from the data found in a given datalog.\n"
		"StackTrace [-location buildpath] [-version (2.02 | 2.20)] [-osfile filename] [-ospath pathnames] [-pathalias old[:new]] [-address values] filename\n";
static const char *kStrErrorFailed = "Error: Failed to open file.";
static const char *kStrErrorInvalid = "Error: Invalid header type.";
static const char *kStrErrorUnsupported = "Error: Log file version is unsupported.";

//From StackTrace.cpp
static const char *kEditorPath = "C:\\Windows\\Notepad.exe";

//All stack trace files will be stored in this location.
static const char *kTraceFilePath = "C:\\temp\\";
static const char *kTraceFileExtn = ".trace";

int main(int argc, char* argv[])
{
	std::ostringstream output;
	DECODER::Result results = DECODER::LogReader::kResultNone;
	TRACE::StackTrace::Arguments arguments;
	std::string filename;
	if(argc > 1)
		filename = (argv[argc - 1]);

	if(filename.find(".dlog") == std::string::npos)
	{
		//filename could be part of stdin pipe. 
		int count = 0;
		for(;;) {
			count++;
			if(count > 3)
			{
				std::cerr << kStrErrorHelp << std::endl;
				exit(0);
			}
			std::cerr << "Enter Dlog name: " << std::endl;
			std::cin >> filename;
			if(filename.empty()) {
				std::cerr << kStrErrorHelp << std::endl;
				exit(0);
			}
			
			//Check file exists
			std::ifstream inputfile(filename);
			if(!inputfile) {
				std::cerr << "File " << filename << " not found." << std::endl;
				continue;
			}
			else
				break;

		}
	}

	for (int i = 1; i <= argc - 1; ++i) {
		if (((i != argc - 1) && *argv[i] == '-') && (*argv[i + 1] != '-')) {
			arguments[argv[i]] = argv[++i];
		}
		else if (*argv[i] == '-') {
			arguments[argv[i]] = "";
		}
	}
	
	// Launch dlog file if already decoded.	
	std::string dlogpath(filename);
	std::string dlogname( dlogpath.substr( dlogpath.find_last_of('\\') + 1) );
	std::string tracefilename(kTraceFilePath + dlogname + kTraceFileExtn);
	std::ifstream itf(tracefilename);
	std::string editorCmd(kEditorPath);
	//if(itf)
	//{
	//	//Launch
	//	
	//	_execl(editorCmd.c_str(), editorCmd.c_str(), tracefilename.c_str(), NULL);
	//}

	std::cerr << "Processing " << filename << std::endl;
	results = TRACE::StackTrace::CreateCrashReport(filename, arguments, output);

	switch (results) {
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
	
	return results;
}
