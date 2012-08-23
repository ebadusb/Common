/*****************************************************************************\
**
** LogExecute.h
**
** Executes a set of data procedures on the given datalog file. 
**
\*****************************************************************************/

#ifndef LOGEXECUTE_H
#define LOGEXECUTE_H

/* Includes ******************************************************************/

#include <algorithm>

#include "LogReader.h"
#include "LogExtractor.h"

namespace DECODER {

/* Types *********************************************************************/

typedef std::vector<LogExtractor*> LogProcessList;

/* Functions *****************************************************************/

class Register {
public:
	Register(LogReader &reader) : mLogReader(reader) {}

	void operator()(LogExtractor *process)
	{
		process->Register(mLogReader);
	}

private:
	LogReader &mLogReader;
};

class Unregister {
public:
	Unregister(LogReader &reader) : mLogReader(reader) {}

	void operator()(LogExtractor *process)
	{
		process->Unregister(mLogReader);
	}

private:
	LogReader &mLogReader;
};

class Preprocess {
public:
	void operator()(LogExtractor *process)
	{
		process->PreProcess();
	}
};

class Postprocess {
public:
	void operator()(LogExtractor *process)
	{
		process->PostProcess();
	}
};

/**
 *
 * ProcessLogData
 *
 * Processes the datalog file.
 *
 * @param filename The datalog file.
 * @param process The process to perform.
 *
 * @return The results.
 *
 */
static Result ProcessLogData(const char *filename, LogExtractor &process)
{
	Result results = LogReader::kResultNone;
	LogReader reader;

	process.Register(reader);
	process.PreProcess();

	results = reader.Open(filename);
	while (results == DECODER::LogReader::kResultNone && !reader.IsEndOfFile()) {
		results = reader.Read(1);
	}
	reader.Close();
	
	process.PostProcess();
	process.Unregister(reader);

	return results;
}

/**
 *
 * ProcessLogData
 *
 * Processes the datalog file.
 *
 * @param filename The datalog file.
 * @param processList A set of precesses to perform.
 *
 * @return The results.
 *
 */
static Result ProcessLogData(const char *filename, LogProcessList &processList)
{
	Result results = LogReader::kResultNone;
	LogReader reader;

	for_each(processList.begin(), processList.end(), Register(reader));
	for_each(processList.begin(), processList.end(), Preprocess());

	results = reader.Open(filename);
	while (results == DECODER::LogReader::kResultNone && !reader.IsEndOfFile()) {
		results = reader.Read(1);
	}
	reader.Close();
	
	for_each(processList.begin(), processList.end(), Postprocess());
	for_each(processList.begin(), processList.end(), Unregister(reader));

	return results;
}

} // namespace DECODER

#endif // LOGEXECUTE_H

