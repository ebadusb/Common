/*
 * $Header$
 * $Log$
 *
 * Test is started with test_periodic().  Other task which perform
 * data logging must not be active when this test is executed.
 */

#include <vxWorks.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <tickLib.h>

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

void test_periodic_output(void)
{
	DataLog_SetHandle	handle;
	DataLog_Result result;

	result = datalog_CreatePeriodicSet("snoopy", &handle);
	assert(result == DataLog_OK);
	printf("datalog periodic set initialization OK\n");
 

	long	milliSeconds;
	result = datalog_SetPeriodicOutputInterval(handle, 500);
	assert(result == DataLog_OK);
	result = datalog_GetPeriodicOutputInterval(handle, &milliSeconds);
	assert(result == DataLog_OK);
	assert(milliSeconds == 500);
	
	PacketCapture::startCapture();
	taskDelay(60);
	PacketCapture::stopCapture();

	DataLog_BufferData * buffer;
	size_t	bufferLen;
	buffer = PacketCapture::getPacket(bufferLen);
	assert(bufferLen == 0);
	printf("datalog empty periodic set output OK\n");

	static int iVal = 0;
	result = datalog_SetPeriodicOutputInterval(handle, 0);
	assert(result == DataLog_OK);

	PacketCapture::startCapture();
	result = datalog_AddRef(handle, iVal, "key", "desc", "%d");
	assert(result == DataLog_OK);
	result = datalog_SetPeriodicOutputInterval(handle, 500);
	assert(result == DataLog_OK);
	taskDelay(60);
	PacketCapture::stopCapture();

	buffer = PacketCapture::getPacket(bufferLen);
	DataLog_WriteTimeRecord * writeRecord = (DataLog_WriteTimeRecord *)&buffer[0];
	DataLog_PeriodicItemRecord * itemRecord = (DataLog_PeriodicItemRecord * )&buffer[sizeof(DataLog_WriteTimeRecord)];
	assert(writeRecord->_recordType == DataLog_WriteTimeRecordID);
 	assert(itemRecord->_recordType == DataLog_PeriodicItemRecordID);
 	assert(itemRecord->_keyLen == 3);
 	assert(itemRecord->_descLen == 4);
 	assert(itemRecord->_formatLen == 2);

	size_t	bufferIdx = sizeof(DataLog_WriteTimeRecord) + sizeof(DataLog_PeriodicItemRecord);
 	DataLog_InternalID	keyCode = itemRecord->_keyCode;

	assert(strncmp((char *)&buffer[bufferIdx], "key", 3) == 0);
	assert(strncmp((char *)&buffer[bufferIdx+3], "desc", 4) == 0);
	assert(strncmp((char *)&buffer[bufferIdx+7], "%d", 2) == 0);
	bufferIdx += 9;

	writeRecord = (DataLog_WriteTimeRecord *)&buffer[bufferIdx];
	DataLog_PeriodicOutputRecord * periodicRecord = (DataLog_PeriodicOutputRecord * )&buffer[bufferIdx+sizeof(DataLog_WriteTimeRecord)];
	bufferIdx += sizeof(DataLog_PeriodicOutputRecord) + sizeof(DataLog_WriteTimeRecord);

	assert(writeRecord->_recordType == DataLog_WriteTimeRecordID);
	assert(periodicRecord->_recordType == DataLog_PeriodicOutputRecordID);
	assert(periodicRecord->_setID == handle->_id);
	assert(periodicRecord->_itemCount == 1);

	assert(*(DataLog_UINT16 *)&buffer[bufferIdx] == 4);
	assert(*(DataLog_UINT16 *)&buffer[bufferIdx+2] == keyCode);
	assert(*(int *)&buffer[bufferIdx+4] == iVal);
	bufferIdx += 8;

	assert(bufferIdx == bufferLen);
	printf("datalog single item periodic set output OK\n");

	PacketCapture::startCapture();
	taskDelay(60);
	PacketCapture::stopCapture();

	buffer = PacketCapture::getPacket(bufferLen);
	assert(bufferLen == 0);
	printf("datalog periodic set with no change output OK\n");

	PacketCapture::startCapture();
	iVal = 2;
	taskDelay(60);
	PacketCapture::stopCapture();

	buffer = PacketCapture::getPacket(bufferLen);
	writeRecord = (DataLog_WriteTimeRecord *)&buffer[0];
	periodicRecord = (DataLog_PeriodicOutputRecord * )&buffer[sizeof(DataLog_WriteTimeRecord)];

	assert(writeRecord->_recordType == DataLog_WriteTimeRecordID);
	assert(periodicRecord->_recordType == DataLog_PeriodicOutputRecordID);
	assert(periodicRecord->_setID == handle->_id);
	assert(periodicRecord->_itemCount == 1);

	bufferIdx = sizeof(DataLog_WriteTimeRecord) + sizeof(DataLog_PeriodicOutputRecord);

	assert(*(DataLog_UINT16 *)&buffer[bufferIdx] == 4);
	assert(*(int *)&buffer[bufferIdx+4] == iVal);
	bufferIdx += 8;

	assert(bufferIdx == bufferLen);
	printf("datalog periodic set with change output OK\n");

	PacketCapture::startCapture();
	result = datalog_DisablePeriodicOutput(handle);
	assert(result == DataLog_OK);
	iVal = 20;
	taskDelay(60);
	PacketCapture::stopCapture();

	buffer = PacketCapture::getPacket(bufferLen);
	assert(bufferLen == 0);
	printf("datalog periodic set disabled output OK\n");

	PacketCapture::startCapture();
	result = datalog_EnablePeriodicOutput(handle);
	assert(result == DataLog_OK);
	taskDelay(60);
	PacketCapture::stopCapture();

	buffer = PacketCapture::getPacket(bufferLen);
	writeRecord = (DataLog_WriteTimeRecord *)&buffer[0];
	periodicRecord = (DataLog_PeriodicOutputRecord * )&buffer[sizeof(DataLog_WriteTimeRecord)];

	assert(writeRecord->_recordType == DataLog_WriteTimeRecordID);
	assert(periodicRecord->_recordType == DataLog_PeriodicOutputRecordID);
	assert(periodicRecord->_setID == handle->_id);
	assert(periodicRecord->_itemCount == 1);

	bufferIdx = sizeof(DataLog_WriteTimeRecord) + sizeof(DataLog_PeriodicOutputRecord);

	assert(*(DataLog_UINT16 *)&buffer[bufferIdx] == 4);
	assert(*(int *)&buffer[bufferIdx+4] == iVal);
	bufferIdx += 8;

	assert(bufferIdx == bufferLen);
	printf("datalog periodic set re-enabled output OK\n");
}

static double test_func1(void)
{
	return 1.0;
}

static const char * test_func2(const int & arg)
{
	return (arg) ? "non-zero" : "zero";
}

class test_class1
{
public:
	test_class1(int i) { _i = i; }
	int func(void) const { return _i; }

private:
	int _i;
};

static void test_add_funcs(void)
{
	DataLog_SetHandle	handle;
	DataLog_Result result;

	result = datalog_CreatePeriodicSet("woodstock", &handle);
	assert(result == DataLog_OK);

	static int i1;
	result = datalog_AddRef(handle, i1, "i1", "i1 description", "%d");
	assert(result == DataLog_OK);

	result = datalog_AddFunc(handle, &test_func1, "test_func1", "test_func1 description", "%lf");
	assert(result == DataLog_OK);

	static int i2;
	result = datalog_AddArgFunc(handle, &test_func2, i2, "test_func2", "test_func2 description", "%s");
	assert(result == DataLog_OK);

	static test_class1 obj1(0);
	result = datalog_AddMemberFunc(handle, &obj1, &test_class1::func, "test_class1::func", "test_class1::func description", "%d");
	assert(result == DataLog_OK);

	printf("datalog periodic set adding functions OK\n");
}

extern "C" void test_periodic(void)
{
	DataLog_Result	result;

	result = datalog_Init(100, 10, "host:c:/taos_sim/periodic_test.dlog", "platform", "node", "info");
	assert(result == DataLog_OK);

	test_periodic_output();
	test_add_funcs();
	printf("tests complete\n");
}

