/*
 * $Header$
 * $Log$
 *
 * Test is started with test_general().  Other task which perform
 * data logging must not be active when this test is executed.
 *
 */

#include <vxWorks.h>
#include <ioLib.h>
#include <math.h>
#include <memDrv.h>
#include <stdio.h>
#include <stdlib.h>
#include <tickLib.h>

#include "datalog/datalog_internal.h"
#include "datalog/datalog_records.h"

class ConsoleCapture
{
public:
	enum { CaptureSize = 128*1024 };
	enum { MarkSize = 15 };

	static void startCapture(void);

	static void generateMark(char * mark);
	static const char * search(const char * mark);

private:
	static char * data;
	static FILE * fp;
};

char * ConsoleCapture::data = NULL;
FILE * ConsoleCapture::fp = NULL;

extern int consoleFd;
void ConsoleCapture::startCapture(void)
{
	assert(data == NULL);
	data = new char[CaptureSize];

	memDevCreate("/mem/datalog_console", data, CaptureSize);

	int consoleFdSave = consoleFd;
	consoleFd = open("/mem/datalog_console", O_RDWR, DEFAULT_FILE_PERM);
	fp = datalog_ConsoleFile();
	consoleFd = consoleFdSave;

	assert(fp != NULL);

	char mark[MarkSize+1];
	generateMark(mark);

	fprintf(fp, "%s", mark);
	fflush(fp);
	assert(strncmp(data, mark, strlen(mark)) == 0);
}

void ConsoleCapture::generateMark(char * mark)
{
	static int	markNumber = 1;

   taskLock();
	sprintf(mark, "XYZZY%05dYZZYX", markNumber);
	markNumber += 1;
	taskUnlock();
}

const char * ConsoleCapture::search(const char * mark)
{
	fflush(fp);
	return strstr(data, mark);
}

class PacketCapture
{
public:
	enum { PacketSize = 2048 };

	static void startCapture(void);
	static void stopCapture(void);
	static DataLog_BufferData * getPacket(size_t & packetLen) { assert(!_packetCapture); packetLen = _packetLen; return _packet; }
	static size_t packetLen(void) { return _packetLen; }

private:
	static DataLog_BufferData * capture(DataLog_BufferData * input, size_t inputLength, size_t * outputLength);
	static DataLog_BufferData	_packet[PacketSize];
	static size_t	_packetLen;
	static bool		_packetCapture;
	static bool		_initialized;
};

DataLog_BufferData PacketCapture::_packet[PacketCapture::PacketSize];
size_t PacketCapture::_packetLen = 0;
bool PacketCapture::_packetCapture = false;
bool PacketCapture::_initialized = false;

void PacketCapture::startCapture(void)
{
	assert(!_packetCapture);
	if ( !_initialized )
	{
		DataLog_Result	result;
		result = datalog_SetEncryptFunc(PacketCapture::capture);
		assert(result == DataLog_OK);
	}

	do
	{
		taskDelay(1);
	} while ( datalog_GetTraceBufferBytes() + datalog_GetCriticalBufferBytes() > 0 );

	_packetLen = 0;
	_packetCapture = true;
}

void PacketCapture::stopCapture(void)
{
	do
	{
		taskDelay(1);
	} while ( datalog_GetTraceBufferBytes() + datalog_GetCriticalBufferBytes() > 0 );

	_packetCapture = false;
}

DataLog_BufferData * PacketCapture::capture(DataLog_BufferData * input, size_t inputLength, size_t * outputLength)
{
	if ( _packetCapture )
	{
		size_t	maxSize = PacketSize-_packetLen;
		size_t	copySize = (inputLength > maxSize) ? maxSize : inputLength;

		memcpy(&_packet[_packetLen], input, copySize);
		_packetLen += copySize;
	}

	*outputLength = inputLength;
	return input;
}

static void test_trace_output(void)
{
	DataLog_Result result;
	DataLog_EnabledType	logEnabled;
	DataLog_ConsoleEnabledType consoleEnabled;

	DataLog_Handle	commonHandle;
	result = datalog_CreateLevel("common", &commonHandle);
	assert(result == DataLog_OK);

	DataLog_Level commonLevel;
	commonLevel.setHandle(commonHandle);

	char	mark[ConsoleCapture::MarkSize + 1];
	result = datalog_SetTaskOutputOptions(DATALOG_CURRENT_TASK, DataLog_LogDisabled, DataLog_ConsoleDisabled);
	assert(result == DataLog_OK);
	result = datalog_GetTaskOutputOptions(DATALOG_CURRENT_TASK, &logEnabled, &consoleEnabled);
	assert(result == DataLog_OK);
	assert(logEnabled == DataLog_LogDisabled);
	assert(consoleEnabled == DataLog_ConsoleDisabled);

	result = datalog_SetLevelOutputOptions(commonHandle, DataLog_LogDisabled, DataLog_ConsoleDisabled);
	assert(result == DataLog_OK);
	result = datalog_GetLevelOutputOptions(commonHandle, &logEnabled, &consoleEnabled);
	assert(result == DataLog_OK);
	assert(logEnabled == DataLog_LogDisabled);
	assert(consoleEnabled == DataLog_ConsoleDisabled);

	// All output disabled
	ConsoleCapture::generateMark(mark);
	PacketCapture::startCapture();
	result = datalog_Print(commonHandle, __FILE__, __LINE__, "%s", mark);
	assert(result == DataLog_OK);
	PacketCapture::stopCapture();
 	assert(ConsoleCapture::search(mark) == NULL);
	assert(PacketCapture::packetLen() == 0);

	ConsoleCapture::generateMark(mark);
	PacketCapture::startCapture();
	DataLog(commonLevel) << mark << endmsg;
	PacketCapture::stopCapture();
 	assert(ConsoleCapture::search(mark) == NULL);
	assert(PacketCapture::packetLen() == 0);

	// Console enabled by level
	result = datalog_SetLevelOutputOptions(commonHandle, DataLog_LogDisabled, DataLog_ConsoleEnabled);
	assert(result == DataLog_OK);
	result = datalog_GetLevelOutputOptions(commonHandle, &logEnabled, &consoleEnabled);
	assert(result == DataLog_OK);
	assert(logEnabled == DataLog_LogDisabled);
	assert(consoleEnabled == DataLog_ConsoleEnabled);
	ConsoleCapture::generateMark(mark);
	PacketCapture::startCapture();
	result = datalog_Print(commonHandle, __FILE__, __LINE__, "%s", mark);
	assert(result == DataLog_OK);
	PacketCapture::stopCapture();
 	assert(ConsoleCapture::search(mark) != NULL);
	assert(PacketCapture::packetLen() == 0);

	ConsoleCapture::generateMark(mark);
	PacketCapture::startCapture();
	DataLog(commonLevel) << mark << endmsg;
	PacketCapture::stopCapture();
 	assert(ConsoleCapture::search(mark) != NULL);
	assert(PacketCapture::packetLen() == 0);

	// Log enabled by level
	result = datalog_SetLevelOutputOptions(commonHandle, DataLog_LogEnabled, DataLog_ConsoleDisabled);
	assert(result == DataLog_OK);
	result = datalog_GetLevelOutputOptions(commonHandle, &logEnabled, &consoleEnabled);
	assert(result == DataLog_OK);
	assert(logEnabled == DataLog_LogEnabled);
	assert(consoleEnabled == DataLog_ConsoleDisabled);
	ConsoleCapture::generateMark(mark);
	PacketCapture::startCapture();
	result = datalog_Print(commonHandle, __FILE__, __LINE__, "%s", mark);
	assert(result == DataLog_OK);
	PacketCapture::stopCapture();
 	assert(ConsoleCapture::search(mark) == NULL);
	assert(PacketCapture::packetLen() != 0);

	ConsoleCapture::generateMark(mark);
	PacketCapture::startCapture();
	DataLog(commonLevel) << mark << endmsg;
	PacketCapture::stopCapture();
 	assert(ConsoleCapture::search(mark) == NULL);
	assert(PacketCapture::packetLen() != 0);

	// Log and console enabled by level
	result = datalog_SetLevelOutputOptions(commonHandle, DataLog_LogEnabled, DataLog_ConsoleEnabled);
	assert(result == DataLog_OK);
	result = datalog_GetLevelOutputOptions(commonHandle, &logEnabled, &consoleEnabled);
	assert(result == DataLog_OK);
	assert(logEnabled == DataLog_LogEnabled);
	assert(consoleEnabled == DataLog_ConsoleEnabled);
	ConsoleCapture::generateMark(mark);
	PacketCapture::startCapture();
	result = datalog_Print(commonHandle, __FILE__, __LINE__, "%s", mark);
	assert(result == DataLog_OK);
	PacketCapture::stopCapture();
 	assert(ConsoleCapture::search(mark) != NULL);
	assert(PacketCapture::packetLen() != 0);

	ConsoleCapture::generateMark(mark);
	PacketCapture::startCapture();
	DataLog(commonLevel) << mark << endmsg;
	PacketCapture::stopCapture();
 	assert(ConsoleCapture::search(mark) != NULL);
	assert(PacketCapture::packetLen() != 0);

	// Console enabled by task
	result = datalog_SetLevelOutputOptions(commonHandle, DataLog_LogDisabled, DataLog_ConsoleDisabled);
	assert(result == DataLog_OK);
	result = datalog_SetTaskOutputOptions(DATALOG_CURRENT_TASK, DataLog_LogDisabled, DataLog_ConsoleEnabled);
	assert(result == DataLog_OK);
	result = datalog_GetTaskOutputOptions(DATALOG_CURRENT_TASK, &logEnabled, &consoleEnabled);
	assert(result == DataLog_OK);
	assert(logEnabled == DataLog_LogDisabled);
	assert(consoleEnabled == DataLog_ConsoleEnabled);
	ConsoleCapture::generateMark(mark);
	PacketCapture::startCapture();
	result = datalog_Print(commonHandle, __FILE__, __LINE__, "%s", mark);
	assert(result == DataLog_OK);
	PacketCapture::stopCapture();
 	assert(ConsoleCapture::search(mark) != NULL);
	assert(PacketCapture::packetLen() == 0);

	ConsoleCapture::generateMark(mark);
	PacketCapture::startCapture();
	DataLog(commonLevel) << mark << endmsg;
	PacketCapture::stopCapture();
 	assert(ConsoleCapture::search(mark) != NULL);
	assert(PacketCapture::packetLen() == 0);

	// Log enabled by task
	result = datalog_SetTaskOutputOptions(DATALOG_CURRENT_TASK, DataLog_LogEnabled, DataLog_ConsoleDisabled);
	assert(result == DataLog_OK);
	result = datalog_GetTaskOutputOptions(DATALOG_CURRENT_TASK, &logEnabled, &consoleEnabled);
	assert(result == DataLog_OK);
	assert(logEnabled == DataLog_LogEnabled);
	assert(consoleEnabled == DataLog_ConsoleDisabled);
	ConsoleCapture::generateMark(mark);
	PacketCapture::startCapture();
	result = datalog_Print(commonHandle, __FILE__, __LINE__, "%s", mark);
	assert(result == DataLog_OK);
	PacketCapture::stopCapture();
 	assert(ConsoleCapture::search(mark) == NULL);
	assert(PacketCapture::packetLen() != 0);

	ConsoleCapture::generateMark(mark);
	PacketCapture::startCapture();
	DataLog(commonLevel) << mark << endmsg;
	PacketCapture::stopCapture();
 	assert(ConsoleCapture::search(mark) == NULL);
	assert(PacketCapture::packetLen() != 0);

	// Log and console enabled by task
	result = datalog_SetTaskOutputOptions(DATALOG_CURRENT_TASK, DataLog_LogEnabled, DataLog_ConsoleEnabled);
	assert(result == DataLog_OK);
	result = datalog_GetTaskOutputOptions(DATALOG_CURRENT_TASK, &logEnabled, &consoleEnabled);
	assert(result == DataLog_OK);
	assert(logEnabled == DataLog_LogEnabled);
	assert(consoleEnabled == DataLog_ConsoleEnabled);
	ConsoleCapture::generateMark(mark);
	PacketCapture::startCapture();
	result = datalog_Print(commonHandle, __FILE__, __LINE__, "%s", mark);
	assert(result == DataLog_OK);
	PacketCapture::stopCapture();
 	assert(ConsoleCapture::search(mark) != NULL);
	assert(PacketCapture::packetLen() != 0);

	ConsoleCapture::generateMark(mark);
	PacketCapture::startCapture();
	DataLog(commonLevel) << mark << endmsg;
	PacketCapture::stopCapture();
 	assert(ConsoleCapture::search(mark) != NULL);
	assert(PacketCapture::packetLen() != 0);

	// Back to default
	result = datalog_SetTaskOutputOptions(DATALOG_CURRENT_TASK, DataLog_LogDisabled, DataLog_ConsoleDisabled);
	assert(result == DataLog_OK);
	result = datalog_SetLevelOutputOptions(commonHandle, DataLog_LogEnabled, DataLog_ConsoleDisabled);
	assert(result == DataLog_OK);
	ConsoleCapture::generateMark(mark);
	PacketCapture::startCapture();
	result = datalog_Print(commonHandle, __FILE__, __LINE__, "%s", mark);
	assert(result == DataLog_OK);
	PacketCapture::stopCapture();
 	assert(ConsoleCapture::search(mark) == NULL);
	assert(PacketCapture::packetLen() != 0);

	ConsoleCapture::generateMark(mark);
	PacketCapture::startCapture();
	DataLog(commonLevel) << mark << endmsg;
	PacketCapture::stopCapture();
 	assert(ConsoleCapture::search(mark) == NULL);
	assert(PacketCapture::packetLen() != 0);
}

static void test_critical_output(void)
{
	DataLog_Result result;
	DataLog_EnabledType	logEnabled;
	DataLog_ConsoleEnabledType consoleEnabled;

	DataLog_Handle	criticalHandle;
	result = datalog_CreateCriticalLevel(&criticalHandle);
	assert(result == DataLog_OK);

	DataLog_Level criticalLevel;
	criticalLevel.setHandle(criticalHandle);

	char	mark[ConsoleCapture::MarkSize + 1];
	result = datalog_SetTaskOutputOptions(DATALOG_CURRENT_TASK, DataLog_LogDisabled, DataLog_ConsoleDisabled);
	assert(result == DataLog_OK);
	result = datalog_GetTaskOutputOptions(DATALOG_CURRENT_TASK, &logEnabled, &consoleEnabled);
	assert(result == DataLog_OK);
	assert(logEnabled == DataLog_LogDisabled);
	assert(consoleEnabled == DataLog_ConsoleDisabled);

	// Should not be able to disable critical output
	result = datalog_SetLevelOutputOptions(criticalHandle, DataLog_LogDisabled, DataLog_ConsoleDisabled);
	assert(result == DataLog_OK);
	result = datalog_GetLevelOutputOptions(criticalHandle, &logEnabled, &consoleEnabled);
	assert(result == DataLog_OK);
	assert(logEnabled == DataLog_LogEnabled);
	assert(consoleEnabled == DataLog_ConsoleEnabled);

	ConsoleCapture::generateMark(mark);
	PacketCapture::startCapture();
	result = datalog_Print(criticalHandle, __FILE__, __LINE__, "%s", mark);
	assert(result == DataLog_OK);
	PacketCapture::stopCapture();
 	assert(ConsoleCapture::search(mark) != NULL);
	assert(PacketCapture::packetLen() != 0);

	ConsoleCapture::generateMark(mark);
	PacketCapture::startCapture();
	DataLog(criticalLevel) << mark << endmsg;
	PacketCapture::stopCapture();
 	assert(ConsoleCapture::search(mark) != NULL);
	assert(PacketCapture::packetLen() != 0);
}

static void test_int_output(void)
{
	DataLog_Result result;
	DataLog_EnabledType	logEnabled;
	DataLog_ConsoleEnabledType consoleEnabled;

	DataLog_Handle	intHandle;
	result = datalog_CreateIntLevel("int", &intHandle);
	assert(result == DataLog_OK);

	DataLog_Level intLevel;
	intLevel.setHandle(intHandle);

	char	mark[ConsoleCapture::MarkSize + 1];
	result = datalog_SetTaskOutputOptions(DATALOG_CURRENT_TASK, DataLog_LogDisabled, DataLog_ConsoleDisabled);
	assert(result == DataLog_OK);
	result = datalog_GetTaskOutputOptions(DATALOG_CURRENT_TASK, &logEnabled, &consoleEnabled);
	assert(result == DataLog_OK);
	assert(logEnabled == DataLog_LogDisabled);
	assert(consoleEnabled == DataLog_ConsoleDisabled);

	// All output disabled
	result = datalog_SetLevelOutputOptions(intHandle, DataLog_LogDisabled, DataLog_ConsoleDisabled);
	assert(result == DataLog_OK);
	result = datalog_GetLevelOutputOptions(intHandle, &logEnabled, &consoleEnabled);
	assert(result == DataLog_OK);
	assert(logEnabled == DataLog_LogDisabled);
	assert(consoleEnabled == DataLog_ConsoleDisabled);

	ConsoleCapture::generateMark(mark);
	PacketCapture::startCapture();
	result = datalog_Print(intHandle, __FILE__, __LINE__, "%s", mark);
	assert(result == DataLog_OK);
	PacketCapture::stopCapture();
 	assert(ConsoleCapture::search(mark) == NULL);
	assert(PacketCapture::packetLen() == 0);

	ConsoleCapture::generateMark(mark);
	PacketCapture::startCapture();
	DataLog(intLevel) << mark << endmsg;
	PacketCapture::stopCapture();
 	assert(ConsoleCapture::search(mark) == NULL);
	assert(PacketCapture::packetLen() == 0);

	// Task options should not apply to int levels
	result = datalog_SetTaskOutputOptions(DATALOG_CURRENT_TASK, DataLog_LogEnabled, DataLog_ConsoleEnabled);
	assert(result == DataLog_OK);
	result = datalog_GetTaskOutputOptions(DATALOG_CURRENT_TASK, &logEnabled, &consoleEnabled);
	assert(result == DataLog_OK);
	assert(logEnabled == DataLog_LogEnabled);
	assert(consoleEnabled == DataLog_ConsoleEnabled);

	ConsoleCapture::generateMark(mark);
	PacketCapture::startCapture();
	result = datalog_Print(intHandle, __FILE__, __LINE__, "%s", mark);
	assert(result == DataLog_OK);
	PacketCapture::stopCapture();
 	assert(ConsoleCapture::search(mark) == NULL);
	assert(PacketCapture::packetLen() == 0);

	ConsoleCapture::generateMark(mark);
	PacketCapture::startCapture();
	DataLog(intLevel) << mark << endmsg;
	PacketCapture::stopCapture();
 	assert(ConsoleCapture::search(mark) == NULL);
	assert(PacketCapture::packetLen() == 0);

	result = datalog_SetTaskOutputOptions(DATALOG_CURRENT_TASK, DataLog_LogDisabled, DataLog_ConsoleDisabled);
	assert(result == DataLog_OK);
	result = datalog_GetTaskOutputOptions(DATALOG_CURRENT_TASK, &logEnabled, &consoleEnabled);
	assert(result == DataLog_OK);
	assert(logEnabled == DataLog_LogDisabled);
	assert(consoleEnabled == DataLog_ConsoleDisabled);

	// Should not be able to enable console output for int levels
	result = datalog_SetLevelOutputOptions(intHandle, DataLog_LogDisabled, DataLog_ConsoleEnabled);
	assert(result == DataLog_OK);
	result = datalog_GetLevelOutputOptions(intHandle, &logEnabled, &consoleEnabled);
	assert(result == DataLog_OK);
	assert(logEnabled == DataLog_LogDisabled);
	assert(consoleEnabled == DataLog_ConsoleDisabled);
	ConsoleCapture::generateMark(mark);
	PacketCapture::startCapture();
	result = datalog_Print(intHandle, __FILE__, __LINE__, "%s", mark);
	assert(result == DataLog_OK);
	PacketCapture::stopCapture();
 	assert(ConsoleCapture::search(mark) == NULL);
	assert(PacketCapture::packetLen() == 0);

	ConsoleCapture::generateMark(mark);
	PacketCapture::startCapture();
	DataLog(intLevel) << mark << endmsg;
	PacketCapture::stopCapture();
 	assert(ConsoleCapture::search(mark) == NULL);
	assert(PacketCapture::packetLen() == 0);

	// Log enabled by level
	result = datalog_SetLevelOutputOptions(intHandle, DataLog_LogEnabled, DataLog_ConsoleDisabled);
	assert(result == DataLog_OK);
	result = datalog_GetLevelOutputOptions(intHandle, &logEnabled, &consoleEnabled);
	assert(result == DataLog_OK);
	assert(logEnabled == DataLog_LogEnabled);
	assert(consoleEnabled == DataLog_ConsoleDisabled);
	ConsoleCapture::generateMark(mark);
	PacketCapture::startCapture();
	result = datalog_Print(intHandle, __FILE__, __LINE__, "%s", mark);
	assert(result == DataLog_OK);
	PacketCapture::stopCapture();
 	assert(ConsoleCapture::search(mark) == NULL);
	assert(PacketCapture::packetLen() != 0);

	// Stream output always disabled for int 
	ConsoleCapture::generateMark(mark);
	PacketCapture::startCapture();
	DataLog(intLevel) << mark << endmsg;
	PacketCapture::stopCapture();
 	assert(ConsoleCapture::search(mark) == NULL);
	assert(PacketCapture::packetLen() == 0);
}

static void test_default_output(void)
{
	DataLog_Result result;
	DataLog_EnabledType	logEnabled;
	DataLog_ConsoleEnabledType consoleEnabled;

	DataLog_Handle	handle1;
	result = datalog_CreateLevel("handle1", &handle1);
	assert(result == DataLog_OK);

	DataLog_Handle	handle2;
	result = datalog_CreateLevel("handle2", &handle2);
	assert(result == DataLog_OK);

	DataLog_Level level1;
	level1.setHandle(handle1);

	DataLog_Level level2;
	level2.setHandle(handle2);


	char	mark[ConsoleCapture::MarkSize + 1];

	result = datalog_SetLevelOutputOptions(handle1, DataLog_LogEnabled, DataLog_ConsoleDisabled);
	assert(result == DataLog_OK);

	result = datalog_SetLevelOutputOptions(handle2, DataLog_LogDisabled, DataLog_ConsoleEnabled);
	assert(result == DataLog_OK);

	// Test with no default - should go to critical output
	ConsoleCapture::generateMark(mark);
	PacketCapture::startCapture();
	result = datalog_PrintToDefault(__FILE__, __LINE__, "%s", mark);
	assert(result == DataLog_OK);
	PacketCapture::stopCapture();
 	assert(ConsoleCapture::search(mark) != NULL);
	assert(PacketCapture::packetLen() != 0);

	ConsoleCapture::generateMark(mark);
	PacketCapture::startCapture();
	DataLog_Default << mark << endmsg;
	PacketCapture::stopCapture();
 	assert(ConsoleCapture::search(mark) != NULL);
	assert(PacketCapture::packetLen() != 0);

	// Test with default set to handle 1
	result = datalog_SetDefaultLevel(handle1);
	assert(result == DataLog_OK);

	ConsoleCapture::generateMark(mark);
	PacketCapture::startCapture();
	result = datalog_PrintToDefault(__FILE__, __LINE__, "%s", mark);
	assert(result == DataLog_OK);
	PacketCapture::stopCapture();
 	assert(ConsoleCapture::search(mark) == NULL);
	assert(PacketCapture::packetLen() != 0);

	ConsoleCapture::generateMark(mark);
	PacketCapture::startCapture();
	DataLog_Default << mark << endmsg;
	PacketCapture::stopCapture();
 	assert(ConsoleCapture::search(mark) == NULL);
	assert(PacketCapture::packetLen() != 0);

	// Test with default set to handle 2
	result = datalog_SetDefaultLevel(handle2);
	assert(result == DataLog_OK);

	ConsoleCapture::generateMark(mark);
	PacketCapture::startCapture();
	result = datalog_PrintToDefault(__FILE__, __LINE__, "%s", mark);
	assert(result == DataLog_OK);
	PacketCapture::stopCapture();
 	assert(ConsoleCapture::search(mark) != NULL);
	assert(PacketCapture::packetLen() == 0);

	ConsoleCapture::generateMark(mark);
	PacketCapture::startCapture();
	DataLog_Default << mark << endmsg;
	PacketCapture::stopCapture();
 	assert(ConsoleCapture::search(mark) != NULL);
	assert(PacketCapture::packetLen() == 0);

	// Test with default set to handle 1
	result = level1.setAsDefault();
	assert(result == DataLog_OK);

	ConsoleCapture::generateMark(mark);
	PacketCapture::startCapture();
	result = datalog_PrintToDefault(__FILE__, __LINE__, "%s", mark);
	assert(result == DataLog_OK);
	PacketCapture::stopCapture();
 	assert(ConsoleCapture::search(mark) == NULL);
	assert(PacketCapture::packetLen() != 0);

	ConsoleCapture::generateMark(mark);
	PacketCapture::startCapture();
	DataLog_Default << mark << endmsg;
	PacketCapture::stopCapture();
 	assert(ConsoleCapture::search(mark) == NULL);
	assert(PacketCapture::packetLen() != 0);

	// Test with default set to handle 2
	result = level2.setAsDefault();
	assert(result == DataLog_OK);

	ConsoleCapture::generateMark(mark);
	PacketCapture::startCapture();
	result = datalog_PrintToDefault(__FILE__, __LINE__, "%s", mark);
	assert(result == DataLog_OK);
	PacketCapture::stopCapture();
 	assert(ConsoleCapture::search(mark) != NULL);
	assert(PacketCapture::packetLen() == 0);

	ConsoleCapture::generateMark(mark);
	PacketCapture::startCapture();
	DataLog_Default << mark << endmsg;
	PacketCapture::stopCapture();
 	assert(ConsoleCapture::search(mark) != NULL);
	assert(PacketCapture::packetLen() == 0);
}

static void test_datalog_level(void)
{
	DataLog_Result result;
	DataLog_Level	* level1 = new DataLog_Level;
	DataLog_Level	* level2 = new DataLog_Level("Snoopy");
	DataLog_Level	* level3 = new DataLog_Level("Woodstock");

	result = level1->initialize("Snoopy");
	assert(result == DataLog_OK);
	assert(level1->getHandle() == level2->getHandle());
	assert(level1->getHandle() != level3->getHandle());

	DataLog_EnabledType	logEnabled;
	DataLog_ConsoleEnabledType consoleEnabled;

	logEnabled = level1->logOutput(DataLog_LogDisabled);
	assert(logEnabled == DataLog_LogEnabled);
	consoleEnabled = level1->consoleOutput(DataLog_ConsoleEnabled);
	assert(consoleEnabled == DataLog_ConsoleDisabled);

	logEnabled = level3->logOutput(DataLog_LogEnabled);
	assert(logEnabled == DataLog_LogEnabled);
	consoleEnabled = level3->consoleOutput(DataLog_ConsoleDisabled);
	assert(consoleEnabled == DataLog_ConsoleDisabled);

	assert(level1->logOutput() == DataLog_LogDisabled);
	assert(level1->consoleOutput() == DataLog_ConsoleEnabled);

	assert(level2->logOutput() == DataLog_LogDisabled);
	assert(level2->consoleOutput() == DataLog_ConsoleEnabled);

	assert(level3->logOutput() == DataLog_LogEnabled);
	assert(level3->consoleOutput() == DataLog_ConsoleDisabled);

	result = level2->setHandle(level3->getHandle());
	assert(result == DataLog_OK);

	logEnabled = level1->logOutput(DataLog_LogEnabled);
	assert(logEnabled == DataLog_LogDisabled);
	consoleEnabled = level1->consoleOutput(DataLog_ConsoleDisabled);
	assert(consoleEnabled == DataLog_ConsoleEnabled);

	logEnabled = level3->logOutput(DataLog_LogDisabled);
	assert(logEnabled == DataLog_LogEnabled);
	consoleEnabled = level3->consoleOutput(DataLog_ConsoleEnabled);
	assert(consoleEnabled == DataLog_ConsoleDisabled);

	assert(level1->logOutput() == DataLog_LogEnabled);
	assert(level1->consoleOutput() == DataLog_ConsoleDisabled);

	assert(level2->logOutput() == DataLog_LogDisabled);
	assert(level2->consoleOutput() == DataLog_ConsoleEnabled);

	assert(level3->logOutput() == DataLog_LogDisabled);
	assert(level3->consoleOutput() == DataLog_ConsoleEnabled);

	DataLog_Critical	critical;
	DataLog_Handle		handle;

	result = datalog_CreateCriticalLevel(&handle);
	assert(result == DataLog_OK);
	assert(handle == critical.getHandle());
}

extern "C" void test_general(void)
{
	// Force the data log to use a memory-mapped file for console writes
	ConsoleCapture::startCapture();

	DataLog_Result	result;
	result = datalog_Init(100, 10, "host:c:/taos_sim/general_test.dlog", "platform", "node", "info");
	assert(result == DataLog_OK);

	char	buffer[64];
	result = datalog_GetCurrentLogFileName(NULL, 0);
	assert(result == DataLog_Error);

	result = datalog_GetCurrentLogFileName(buffer, 64);
	assert(result == DataLog_OK);
	assert(strcmp(buffer, "host:c:/taos_sim/general_test.dlog") == 0);
	printf("datalog initialization test OK\n");

	test_trace_output();
	printf("datalog trace output test OK\n");

	test_critical_output();
	printf("datalog critical output test OK\n");

	test_int_output();
	printf("datalog interrupt output test OK\n");

	test_default_output();
	printf("datalog default output test OK\n");

	test_datalog_level();
	printf("datalog level test OK\n");
	printf("tests complete\n");
}

