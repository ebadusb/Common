/*
 * $Header$
 * $Log$
 *
 * Test is started with test_buffer().  Other task which perform
 * data logging must not be active when this test is executed.
 *
 */

#include <vxWorks.h>
#include <map.h>
#include <stdio.h>
#include <stdlib.h>
#include <tickLib.h>

#include "datalog.h"

// Include internal structures for testing purposes
#include "datalog/datalog_internal.h"

class Test_DataLog_BufferManager
{
public:
	static void main(void);

private:
	enum { TotalBuffers = 200 };
	enum { CriticalBuffers = 50 };

	static void stress_test(void); 
	static void check_buffer_consistency(void);
};

void Test_DataLog_BufferManager::main(void)
{
	DataLog_Result result;

	taskPrioritySet(0, 40);
	result = datalog_Init((TotalBuffers*DataLog_BufferSize)/1024, (CriticalBuffers*DataLog_BufferSize)/1024, "host:c:/buffer_test.dlog", "platform", "node", "info");
	assert(result == DataLog_OK);

	// Wait for datalog initialization to complete
	taskDelay(30);

	stress_test();
}

void Test_DataLog_BufferManager::stress_test(void)
{
	enum	{ TestTicks = 3600*60 };
	DataLog_BufferManager::DataLog_BufferList * freeList = DataLog_BufferManager::getInternalList(DataLog_BufferManager::FreeList);
	DataLog_BufferManager::DataLog_BufferList * traceList = DataLog_BufferManager::getInternalList(DataLog_BufferManager::TraceList);
	DataLog_BufferManager::DataLog_BufferList * criticalList = DataLog_BufferManager::getInternalList(DataLog_BufferManager::CriticalList);
	assert(freeList->_currentBufferCount == TotalBuffers);
	assert(freeList->_bytesWritten == 0);
	assert(freeList->_bytesMissed == 0);

	assert(traceList->_currentBufferCount == 0);
	assert(traceList->_bytesWritten == 0);
	assert(traceList->_bytesMissed == 0);

	assert(criticalList->_currentBufferCount == 0);
	assert(criticalList->_bytesMissed == 0);

	check_buffer_consistency();

	DataLog_Level		traceLevel("Trace");
	DataLog_Critical	criticalLevel;

	DataLog_Stream	& traceStream = traceLevel(__FILE__, __LINE__);
	DataLog_Stream & criticalStream = criticalLevel(__FILE__, __LINE__);

	unsigned int	startTick = tickGet();
	unsigned int	logTick = tickGet()+180;
	int	delayTicks = 1;

	while ( tickGet() - startTick < TestTicks )
	{
		if ( tickGet() >= logTick )
		{
			printf("%d: fb=%lu tb=%lu cb=%lu tWritten=%lu tMissed=%lu cWritten=%lu cMissed=%lu\n",
							(logTick-startTick+30)/60, freeList->_currentBufferCount, traceList->_currentBufferCount,
							criticalList->_currentBufferCount, traceList->_bytesWritten, traceList->_bytesMissed,
							criticalList->_bytesWritten, criticalList->_bytesMissed);

			logTick += 180;
		}
		switch (rand() % 10)
		{
		case 0:
		case 1:
			check_buffer_consistency();
			break;

		case 2:
			traceStream << endmsg;
			criticalStream << endmsg;
			check_buffer_consistency();
			assert(traceList->_currentBufferCount < TotalBuffers-CriticalBuffers);
			break;

		case 3:
			traceStream << "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890";
			break;

		case 4:
			criticalStream << "1234567890";
			break;

		case 5:
			traceStream << "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz";
			traceStream << "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz";
			traceStream << "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz";
			traceStream << "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz";
			traceStream << "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz";
			traceStream << "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz";
			traceStream << "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz";
			break;

		case 6:
			criticalStream << "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz";
			criticalStream << "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz";
			criticalStream << "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz";
			criticalStream << "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz";
			break;

		case 7:
			traceStream << "-" << endmsg;
			break;

		case 8:
			criticalStream << "-" << endmsg;
			break;

		case 9:
			if ( (rand() % 4) == 1)
			{
				if ( freeList->_currentBufferCount < TotalBuffers/4 ) delayTicks += 1;
				if ( freeList->_currentBufferCount > 3*TotalBuffers/4 && delayTicks > 0 ) delayTicks -= 1;
				taskDelay(delayTicks);
			}
			break;
		}
	}
}

enum BufferState { OnFree, OnTrace, OnCritical, Busy };
typedef map<DataLog_Buffer *, BufferState>	BufferMap;

void Test_DataLog_BufferManager::check_buffer_consistency(void)
{
	static BufferMap	bufferData;
	static bool			firstScan = true;

	DataLog_BufferManager::DataLog_BufferList * freeList = DataLog_BufferManager::getInternalList(DataLog_BufferManager::FreeList);
	DataLog_BufferManager::DataLog_BufferList * traceList = DataLog_BufferManager::getInternalList(DataLog_BufferManager::TraceList);
	DataLog_BufferManager::DataLog_BufferList * criticalList = DataLog_BufferManager::getInternalList(DataLog_BufferManager::CriticalList);

	taskSuspend(taskNameToId("dlog_out"));

	// If this isn't the first scan, then we should have all buffers
	// in the buffer map.  Mark all buffers as busy so we can check for
	// multipule linkages.
	BufferMap::iterator	iter;
	iter = bufferData.begin();
	while ( iter != bufferData.end() )
	{
		(*iter).second = Busy;
		++iter;
	}

	// Scan through buffers on each list
	DataLog_BufferPtr	ptr;
	unsigned int	bufferCount;

	ptr = freeList->_head;
	bufferCount = 0;
	while ( ptr )
	{
		if ( !firstScan) assert(bufferData[ptr] == Busy);
		bufferData[ptr] = OnFree;
		bufferCount += 1;
		if ( !ptr->_next) assert(freeList->_tail == ptr);
		ptr = ptr->_next;
	}
	assert(bufferCount == freeList->_currentBufferCount);
 
	ptr = traceList->_head;
	bufferCount = 0;
	int	chainSize = -1;
	DataLog_BufferPtr chainTail = NULL;

	while ( ptr )
	{
		if ( !firstScan) assert(bufferData[ptr] == Busy);
		bufferData[ptr] = OnTrace;
		bufferCount += 1;
		if ( !ptr->_next) assert(traceList->_tail == ptr);

		if ( chainSize <= 0 )
		{
			chainSize = ptr->_length;
			chainTail = ptr->_tail;
		}

		chainSize -= DataLog_BufferSize;
		if ( chainSize <= 0 ) assert(ptr == chainTail);

		ptr = ptr->_next;
	}

	assert(bufferCount == traceList->_currentBufferCount);
	assert(chainSize <= 0);

	ptr = criticalList->_head;
	bufferCount = 0;
	chainSize = -1;
	chainTail = NULL;

	while ( ptr )
	{
		if ( !firstScan) assert(bufferData[ptr] == Busy);
		bufferData[ptr] = OnCritical;
		bufferCount += 1;
		if ( !ptr->_next) assert(criticalList->_tail == ptr);

		if ( chainSize <= 0 )
		{
			chainSize = ptr->_length;
			chainTail = ptr->_tail;
		}

		chainSize -= DataLog_BufferSize;
		if ( chainSize <= 0 ) assert(ptr == chainTail);

		ptr = ptr->_next;
	}

	assert(bufferCount == criticalList->_currentBufferCount);
	assert(chainSize <= 0);

	taskResume(taskNameToId("dlog_out"));
	firstScan = false;
}

extern "C" void test_buffer(void)
{
	Test_DataLog_BufferManager::main();
	printf("tests complete\n");
}
