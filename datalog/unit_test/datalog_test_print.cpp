/*
 * $Header$
 * $Log$
 *
 * Test is started with test_print().  Other task which perform
 * data logging must not be active when this test is executed.
 *
 */

#include <vxWorks.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <tickLib.h>

#include "datalog_levels.h"
#include "datalog/datalog_internal.h"
#include "datalog/datalog_records.h"

class PacketCapture
{
public:
	enum { PacketSize = 2048 };

	static void startCapture(void);
	static void stopCapture(void);
	static DataLog_BufferData * getPacket(size_t & packetLen) { assert(!_packetCapture); packetLen = _packetLen; return _packet; }

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

class SinglePrintMonitor
{
public:
	enum { OT_Char = 'C', OT_String = 'S', OT_Int = 'I', OT_Float = 'F', OT_Double = 'D' };
	static void checkOutput(DataLog_Handle handle, const char * format, DataLog_Result expectedResult, size_t outputCount, char * type);
};

void SinglePrintMonitor::checkOutput(DataLog_Handle handle, const char * format, DataLog_Result expectedResult, size_t outputCount, char * type)
{
	//
	// Building the arguments for the print call depends on the following
	// (all of which hold under vxWorks - this may not work properly under
	// other systems):
	//  1) C++ casts chars, shorts, etc. to ints for var args (i.e. function declared with ...)
	//  2) C++ casts floats to doubles for var args (i.e. function declared with ...)
	//  3) long ints are the same size as ints
	//  4) doubles are the size of two ints
	//  5) char pointers are the same size as ints
	//
	assert(sizeof(long int) == sizeof(int));
	assert(sizeof(double) == 2*sizeof(int));
	assert(sizeof(char *) == sizeof(int));

	//
	// Arguments are modified in a deterministic fashion from initial values so
	// they can be checked upon output.
	//
	double	dArgBase = rand()/(double)RAND_MAX * pow(10.0, (rand() % 21)-10.0);
	int		iArgBase = (rand() << 16) | rand();
	size_t	sArgBaseLen = (rand() % 256) + 1;
	char * 	sArgBase = new char[sArgBaseLen+1];

	sArgBase[sArgBaseLen] = '\0';
	for ( int i=0; i<sArgBaseLen; i++ )
	{
		sArgBase[i] = (rand() % 26) + 'A';
	}

	//
	// Build argument list
	//
	enum { MaxIntArgs = 10 };
	int	args[MaxIntArgs];
	int	intArgIdx = 0;

	double	dArg = dArgBase;
	int		iArg = iArgBase;

	int		argIdx;
	for ( argIdx = 0; argIdx < outputCount; argIdx += 1 )
	{
		switch ( type[argIdx] )
		{
		case OT_Char:
			assert(intArgIdx <= MaxIntArgs-1);
			args[intArgIdx++] = iArg & 0x7f;
			iArg = ( iArg ^ 0x12345678 ) + 0x87654321;
			break;

		case OT_Int:
			assert(intArgIdx <= MaxIntArgs-1);
			args[intArgIdx++] = iArg;
			iArg = ( iArg ^ 0x12345678 ) + 0x87654321;
			break;

		case OT_String:
			assert(intArgIdx <= MaxIntArgs-1);
			args[intArgIdx++] = (int)sArgBase;
			break;

		case OT_Float:
		case OT_Double:
			assert(intArgIdx <= MaxIntArgs-2);
			memcpy(&args[intArgIdx], &dArg, sizeof(double));
			intArgIdx += 2;

			dArg = dArg/1.2345678;
			break;
		}
	}

	DataLog_Result	result;
	DataLog_UINT16	lineNum = iArg&0xffff;

	PacketCapture::startCapture();
	result = datalog_Print(handle, __FILE__, lineNum, format, args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8], args[9]);
	assert(result == expectedResult);
	PacketCapture::stopCapture();

	DataLog_BufferData * buffer;
	size_t	bufferLen;
	buffer = PacketCapture::getPacket(bufferLen);

	DataLog_WriteTimeRecord * writeRecord = (DataLog_WriteTimeRecord *)&buffer[0];
	DataLog_PrintOutputRecord * printRecord = (DataLog_PrintOutputRecord * )&buffer[sizeof(DataLog_WriteTimeRecord)];

	assert(writeRecord->_recordType == DataLog_WriteTimeRecordID);
	assert(printRecord->_recordType == DataLog_PrintOutputRecordID);
	assert(printRecord->_levelID == handle->_id);
	assert(printRecord->_taskID == taskIdSelf());
	assert(printRecord->_formatLen == strlen(format));
	assert(printRecord->_fileNameLen == strlen(__FILE__));
	assert(printRecord->_lineNum == lineNum);

	size_t	bufferIdx = sizeof(DataLog_WriteTimeRecord) + sizeof(DataLog_PrintOutputRecord);

	assert(strncmp(format, (char *)&buffer[bufferIdx], strlen(format)) == 0);
	bufferIdx += strlen(format);

	assert(strncmp(__FILE__, (char *)&buffer[bufferIdx], strlen(__FILE__)) == 0);
	bufferIdx += strlen(__FILE__);

	dArg = dArgBase;
	iArg = iArgBase;

	for ( argIdx = 0; argIdx < outputCount; argIdx += 1 )
	{
		switch ( type[argIdx] )
		{
		case OT_Char:
			{
				char	ch = iArg & 0x7f;
				iArg = ( iArg ^ 0x12345678 ) + 0x87654321;

				assert(memcmp(&buffer[bufferIdx], &ch, 1) == 0);
				bufferIdx += 1;
			}

			break;

		case OT_Int:
			assert(memcmp(&buffer[bufferIdx], &iArg, sizeof(int)) == 0);
			iArg = ( iArg ^ 0x12345678 ) + 0x87654321;
			bufferIdx += sizeof(int);
			break;

		case OT_String:
			{
				DataLog_UINT16	stringLen = sArgBaseLen;
				assert(memcmp(&buffer[bufferIdx], &stringLen, 2) == 0);
				bufferIdx += 2;				
			}

			assert(memcmp(&buffer[bufferIdx], sArgBase, sArgBaseLen) == 0);
			bufferIdx += sArgBaseLen;
			break;

		case OT_Float:
			{
				float	val = (float)dArg;
				assert(memcmp(&buffer[bufferIdx], &val, sizeof(float)) == 0);
				bufferIdx += sizeof(float);
			}

			dArg = dArg/1.2345678;
			break;

		case OT_Double:
			assert(memcmp(&buffer[bufferIdx], &dArg, sizeof(double)) == 0);
			bufferIdx += sizeof(double);
			dArg = dArg/1.2345678;
			break;
		}
	}

	assert(bufferIdx == bufferLen);
	printf("print format \"%s\" OK\n", format);
}

extern "C" void test_print(void)
{
	DataLog_Result	result;

	result = datalog_Init(100, 10, "host:c:/taos_sim/print_test.dlog", "platform", "node", "info");
	assert(result == DataLog_OK);

	DataLog_Handle	commonHandle;
	result = datalog_CreateLevel("common", &commonHandle);
	assert(result == DataLog_OK);

	// Create a dummy handle to allow suppressing datalog error messages
	DataLog_Handle	silentHandle;
	result = datalog_CreateLevel("silent", &silentHandle);
	assert(result == DataLog_OK);

	log_level_datalog_error.setHandle(silentHandle);
	log_level_datalog_error.logOutput(DataLog_LogDisabled);
	log_level_datalog_error.consoleOutput(DataLog_ConsoleDisabled);

	SinglePrintMonitor::checkOutput(commonHandle, "%", DataLog_Error, 0, NULL);
	SinglePrintMonitor::checkOutput(commonHandle, "% %", DataLog_Error, 0, NULL);
	SinglePrintMonitor::checkOutput(commonHandle, "%%", DataLog_OK, 0, NULL);
	SinglePrintMonitor::checkOutput(commonHandle, "%%%", DataLog_Error, 0, NULL);
	SinglePrintMonitor::checkOutput(commonHandle, "%%%%", DataLog_OK, 0, NULL);

	SinglePrintMonitor::checkOutput(commonHandle, "%C", DataLog_Error, 0, NULL);
	SinglePrintMonitor::checkOutput(commonHandle, "%S", DataLog_Error, 0, NULL);
	SinglePrintMonitor::checkOutput(commonHandle, "%D", DataLog_Error, 0, NULL);
	SinglePrintMonitor::checkOutput(commonHandle, "%LD", DataLog_Error, 0, NULL);
	SinglePrintMonitor::checkOutput(commonHandle, "%lD", DataLog_Error, 0, NULL);

	SinglePrintMonitor::checkOutput(commonHandle, "%c", DataLog_OK, 1, "C");
	SinglePrintMonitor::checkOutput(commonHandle, "%1c", DataLog_OK, 1, "C");
	SinglePrintMonitor::checkOutput(commonHandle, "%*c", DataLog_OK, 2, "IC");

	SinglePrintMonitor::checkOutput(commonHandle, "%s", DataLog_OK, 1, "S");
	SinglePrintMonitor::checkOutput(commonHandle, "%1s", DataLog_OK, 1, "S");
	SinglePrintMonitor::checkOutput(commonHandle, "%*s", DataLog_OK, 2, "IS");

	SinglePrintMonitor::checkOutput(commonHandle, "%d", DataLog_OK, 1, "I");
	SinglePrintMonitor::checkOutput(commonHandle, "%1d", DataLog_OK, 1, "I");
	SinglePrintMonitor::checkOutput(commonHandle, "%*d", DataLog_OK, 2, "II");

	SinglePrintMonitor::checkOutput(commonHandle, "%ld", DataLog_OK, 1, "I");
	SinglePrintMonitor::checkOutput(commonHandle, "%1ld", DataLog_OK, 1, "I");
	SinglePrintMonitor::checkOutput(commonHandle, "%*ld", DataLog_OK, 2, "II");

	SinglePrintMonitor::checkOutput(commonHandle, "%x", DataLog_OK, 1, "I");
	SinglePrintMonitor::checkOutput(commonHandle, "%1x", DataLog_OK, 1, "I");
	SinglePrintMonitor::checkOutput(commonHandle, "%*x", DataLog_OK, 2, "II");

	SinglePrintMonitor::checkOutput(commonHandle, "%lx", DataLog_OK, 1, "I");
	SinglePrintMonitor::checkOutput(commonHandle, "%1lx", DataLog_OK, 1, "I");
	SinglePrintMonitor::checkOutput(commonHandle, "%*lx", DataLog_OK, 2, "II");

	SinglePrintMonitor::checkOutput(commonHandle, "%u", DataLog_OK, 1, "I");
	SinglePrintMonitor::checkOutput(commonHandle, "%1u", DataLog_OK, 1, "I");
	SinglePrintMonitor::checkOutput(commonHandle, "%*u", DataLog_OK, 2, "II");

	SinglePrintMonitor::checkOutput(commonHandle, "%lu", DataLog_OK, 1, "I");
	SinglePrintMonitor::checkOutput(commonHandle, "%1lu", DataLog_OK, 1, "I");
	SinglePrintMonitor::checkOutput(commonHandle, "%*lu", DataLog_OK, 2, "II");

	SinglePrintMonitor::checkOutput(commonHandle, "%f", DataLog_OK, 1, "F");
	SinglePrintMonitor::checkOutput(commonHandle, "%1f", DataLog_OK, 1, "F");
	SinglePrintMonitor::checkOutput(commonHandle, "%.1f", DataLog_OK, 1, "F");
	SinglePrintMonitor::checkOutput(commonHandle, "%2.1f", DataLog_OK, 1, "F");
	SinglePrintMonitor::checkOutput(commonHandle, "%2.*f", DataLog_OK, 2, "IF");
	SinglePrintMonitor::checkOutput(commonHandle, "%*.1f", DataLog_OK, 2, "IF");
	SinglePrintMonitor::checkOutput(commonHandle, "%*.*f", DataLog_OK, 3, "IIF");

	SinglePrintMonitor::checkOutput(commonHandle, "%lf", DataLog_OK, 1, "D");
	SinglePrintMonitor::checkOutput(commonHandle, "%1lf", DataLog_OK, 1, "D");
	SinglePrintMonitor::checkOutput(commonHandle, "%.1lf", DataLog_OK, 1, "D");
	SinglePrintMonitor::checkOutput(commonHandle, "%2.1lf", DataLog_OK, 1, "D");
	SinglePrintMonitor::checkOutput(commonHandle, "%2.*lf", DataLog_OK, 2, "ID");
	SinglePrintMonitor::checkOutput(commonHandle, "%*.1lf", DataLog_OK, 2, "ID");
	SinglePrintMonitor::checkOutput(commonHandle, "%*.*lf", DataLog_OK, 3, "IID");

	printf("tests complete\n");
}

