/*
 * $Header$
 * $Log$
 *
 * Test is started with test_stream().  Other task which perform
 * data logging must not be active when this test is executed.
 *
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

void test_stream_args(void)
{
	DataLog_Level	level("common");

	DataLog_UINT16	lineNum = rand();
	char	c1 = rand();
	unsigned char c2 = rand();
	short s1 = rand();
	unsigned short s2 = rand();
	int i1 = rand();
	unsigned int i2 = rand();
	long l1 = rand();
	unsigned long l2 = rand();
	bool	b1 = (rand() > RAND_MAX/2) ? true : false;
	bool	b2 = !b1;
	float	f1 = rand()/(double)RAND_MAX;
	float	f2 = rand()/(double)RAND_MAX;

	double d1 = rand()/(double)RAND_MAX;
	double d2 = rand()/(double)RAND_MAX;

	PacketCapture::startCapture();
	level(__FILE__, lineNum) << "test" << c1 << c2 << s1 << s2 << i1 << i2 << l1 << l2 << b1 << b2 << f1 << f2 << d1 << d2 << endmsg;
	PacketCapture::stopCapture();

	DataLog_BufferData * buffer;
	size_t	bufferLen;
	buffer = PacketCapture::getPacket(bufferLen);

	DataLog_WriteTimeRecord * writeRecord = (DataLog_WriteTimeRecord *)&buffer[0];
	DataLog_StreamOutputRecord * streamRecord = (DataLog_StreamOutputRecord * )&buffer[sizeof(DataLog_WriteTimeRecord)];

	assert(writeRecord->_recordType == DataLog_WriteTimeRecordID);
	assert(streamRecord->_recordType == DataLog_StreamOutputRecordID);
	assert(streamRecord->_levelID == level.getHandle()->_id);
	assert(streamRecord->_taskID == taskIdSelf());
	assert(streamRecord->_fileNameLen == strlen(__FILE__));
	assert(streamRecord->_lineNum == lineNum);

	size_t	bufferIdx = sizeof(DataLog_WriteTimeRecord) + sizeof(DataLog_StreamOutputRecord);

	assert(strncmp(__FILE__, (char *)&buffer[bufferIdx], strlen(__FILE__)) == 0);
	bufferIdx += strlen(__FILE__) + 3;

	DataLog_UINT16 * totalLength = (DataLog_UINT16 *)&buffer[bufferIdx];
	assert(bufferLen == bufferIdx+2 + (*totalLength));
	bufferIdx += 2;

	assert(buffer[bufferIdx] == 7);
	assert(*(DataLog_UINT16 *)&buffer[bufferIdx+1] == 4);
	assert(strncmp((char *)&buffer[bufferIdx+3], "test", 4) == 0);
	bufferIdx += 7;

	assert(buffer[bufferIdx] == 1);
	assert(buffer[bufferIdx+1] == c1);
	bufferIdx += 2;

	assert(buffer[bufferIdx] == 2);
	assert((unsigned char)buffer[bufferIdx+1] == c2);
	bufferIdx += 2;

	assert(buffer[bufferIdx] == 3);
	assert(*(int *)&buffer[bufferIdx+1] == s1);
	bufferIdx += 5;

	assert(buffer[bufferIdx] == 4);
	assert(*(unsigned int *)&buffer[bufferIdx+1] == s2);
	bufferIdx += 5;

	assert(buffer[bufferIdx] == 3);
	assert(*(int *)&buffer[bufferIdx+1] == i1);
	bufferIdx += 5;

	assert(buffer[bufferIdx] == 4);
	assert(*(unsigned int *)&buffer[bufferIdx+1] == i2);
	bufferIdx += 5;

	assert(buffer[bufferIdx] == 5);
	assert(*(long *)&buffer[bufferIdx+1] == l1);
	bufferIdx += 5;
	
	assert(buffer[bufferIdx] == 6);
	assert(*(unsigned long *)&buffer[bufferIdx+1] == l2);
	bufferIdx += 5;
	
	assert(buffer[bufferIdx] == 10);
	assert( (buffer[bufferIdx+1] && b1) || (!buffer[bufferIdx+1] && !b1) );
	bufferIdx += 2;

	assert(buffer[bufferIdx] == 10);
	assert( (buffer[bufferIdx+1] && b2) || (!buffer[bufferIdx+1] && !b2) );
	bufferIdx += 2;

	assert(buffer[bufferIdx] == 8);
	assert(*(float *)&buffer[bufferIdx+1] == f1);
	bufferIdx += 5;
	
	assert(buffer[bufferIdx] == 8);
	assert(*(float *)&buffer[bufferIdx+1] == f2);
	bufferIdx += 5;
	
	assert(buffer[bufferIdx] == 9);
	assert(*(double *)&buffer[bufferIdx+1] == d1);
	bufferIdx += 9;
	
	assert(buffer[bufferIdx] == 9);
	assert(*(double *)&buffer[bufferIdx+1] == d2);
	bufferIdx += 9;

	assert(bufferIdx == bufferLen);
	printf("stream argument test OK\n");
}

void test_stream_manip(void)
{
	DataLog_Level	level("common");

	DataLog_UINT16	lineNum = rand();
	int i1 = rand();
	int i2 = rand();
	int i3 = rand();
	int i4 = rand();
	int i5 = rand();

	PacketCapture::startCapture();
	level(__FILE__, lineNum) <<
     i1 << hex << i2 << precision(3) << dec << precision(10) << dec << i3  <<
     setflags(DataLog_Stream::f_showpos | DataLog_Stream::f_hex | DataLog_Stream::f_showbase | DataLog_Stream::f_showpoint | DataLog_Stream::f_scientific) <<
     i4 << endmsg;

	PacketCapture::stopCapture();

	DataLog_BufferData * buffer;
	size_t	bufferLen;
	buffer = PacketCapture::getPacket(bufferLen);

	DataLog_WriteTimeRecord * writeRecord = (DataLog_WriteTimeRecord *)&buffer[0];
	DataLog_StreamOutputRecord * streamRecord = (DataLog_StreamOutputRecord * )&buffer[sizeof(DataLog_WriteTimeRecord)];

	assert(writeRecord->_recordType == DataLog_WriteTimeRecordID);
	assert(streamRecord->_recordType == DataLog_StreamOutputRecordID);
	assert(streamRecord->_levelID == level.getHandle()->_id);
	assert(streamRecord->_taskID == taskIdSelf());
	assert(streamRecord->_fileNameLen == strlen(__FILE__));
	assert(streamRecord->_lineNum == lineNum);

	size_t	bufferIdx = sizeof(DataLog_WriteTimeRecord) + sizeof(DataLog_StreamOutputRecord);

	assert(strncmp(__FILE__, (char *)&buffer[bufferIdx], strlen(__FILE__)) == 0);
	bufferIdx += strlen(__FILE__);

	assert(*(DataLog_UINT16 *)&buffer[bufferIdx] == DataLog_Stream::f_dec | DataLog_Stream::f_fixed);
	bufferIdx += 3;

	DataLog_UINT16 * totalLength = (DataLog_UINT16 *)&buffer[bufferIdx];
	assert(bufferLen == bufferIdx+2 + (*totalLength));
	bufferIdx += 2;

	assert(buffer[bufferIdx] == 3);
	assert(*(int *)&buffer[bufferIdx+1] == i1);
	bufferIdx += 5;

	assert(buffer[bufferIdx] == 100);
	assert(*(DataLog_UINT16 *)&buffer[bufferIdx+1] == DataLog_Stream::f_hex | DataLog_Stream::f_fixed);
	bufferIdx += 3;

	assert(buffer[bufferIdx] == 3);
	assert(*(int *)&buffer[bufferIdx+1] == i2);
	bufferIdx += 5;

	assert(buffer[bufferIdx] == 101);
	assert(buffer[bufferIdx+1] == 10);
	bufferIdx += 2;

	assert(buffer[bufferIdx] == 100);
	assert(*(DataLog_UINT16 *)&buffer[bufferIdx+1] == DataLog_Stream::f_dec | DataLog_Stream::f_fixed);
	bufferIdx += 3;

	assert(buffer[bufferIdx] == 3);
	assert(*(int *)&buffer[bufferIdx+1] == i3);
	bufferIdx += 5;

	assert(buffer[bufferIdx] == 100);
	assert(*(DataLog_UINT16 *)&buffer[bufferIdx+1] == DataLog_Stream::f_showpos | DataLog_Stream::f_hex | DataLog_Stream::f_showbase | DataLog_Stream::f_showpoint | DataLog_Stream::f_scientific);
	bufferIdx += 3;

	assert(buffer[bufferIdx] == 3);
	assert(*(int *)&buffer[bufferIdx+1] == i4);
	bufferIdx += 5;

	assert(bufferIdx == bufferLen);


	PacketCapture::startCapture();
	level(__FILE__, lineNum) <<
     resetflags(DataLog_Stream::f_showpos | DataLog_Stream::f_hex | DataLog_Stream::f_showbase | DataLog_Stream::f_showpoint | DataLog_Stream::f_scientific) <<
     i5 << endmsg;

	PacketCapture::stopCapture();

	buffer = PacketCapture::getPacket(bufferLen);

	writeRecord = (DataLog_WriteTimeRecord *)&buffer[0];
	streamRecord = (DataLog_StreamOutputRecord * )&buffer[sizeof(DataLog_WriteTimeRecord)];

	assert(writeRecord->_recordType == DataLog_WriteTimeRecordID);
	assert(streamRecord->_recordType == DataLog_StreamOutputRecordID);
	assert(streamRecord->_levelID == level.getHandle()->_id);
	assert(streamRecord->_taskID == taskIdSelf());
	assert(streamRecord->_fileNameLen == strlen(__FILE__));
	assert(streamRecord->_lineNum == lineNum);

	bufferIdx = sizeof(DataLog_WriteTimeRecord) + sizeof(DataLog_StreamOutputRecord);

	assert(strncmp(__FILE__, (char *)&buffer[bufferIdx], strlen(__FILE__)) == 0);
	bufferIdx += strlen(__FILE__);

	assert(*(DataLog_UINT16 *)&buffer[bufferIdx] == DataLog_Stream::f_showpos | DataLog_Stream::f_hex | DataLog_Stream::f_showbase | DataLog_Stream::f_showpoint | DataLog_Stream::f_scientific);
	assert(buffer[bufferIdx+2] == 10);
	bufferIdx += 3;

	totalLength = (DataLog_UINT16 *)&buffer[bufferIdx];
	assert(bufferLen == bufferIdx+2 + (*totalLength));
	bufferIdx += 2;

	assert(buffer[bufferIdx] == 100);
	assert(*(DataLog_UINT16 *)&buffer[bufferIdx+1] == DataLog_Stream::f_dec | DataLog_Stream::f_fixed);
	bufferIdx += 3;

	assert(buffer[bufferIdx] == 3);
	assert(*(int *)&buffer[bufferIdx+1] == i5);
	bufferIdx += 5;

	assert(bufferIdx == bufferLen);
	printf("stream manipulator test OK\n");
}

extern "C" void test_stream(void)
{
	DataLog_Result	result;

	result = datalog_Init(100, 10, "host:c:/taos_sim/stream_test.dlog", "platform", "node", "info");
	assert(result == DataLog_OK);

	test_stream_args();
	test_stream_manip();
	printf("tests complete\n");
}

