/*
 * Copyright (C) 2002 Gambro BCT, Inc.  All rights reserved.
 *
 *	vxWorks utilities for logging debug information after a system failure is detected
 *
 * $Header: //bctquad3/home/BCT_Development/vxWorks/Common/debug/rcs/failure_debug.cpp 1.11 2004/02/24 22:31:40Z jl11312 Exp ms10234 $
 * $Log: failure_debug.cpp $
 * Revision 1.1  2003/02/28 22:07:52Z  jl11312
 * Initial revision
 *
 */

#include <vxWorks.h>
#include <sysLib.h>
#include <tickLib.h>
#include <taskHookLib.h>

#include "failure_debug.h"

// Local functions
static void idleTask(void);
static void taskSwitchHook(int oldTID, int newTID);

// Local data
static DBG_TaskSwitchInfo taskSwitchInfo = { 0, 0, NULL };
static DBG_MessageInfo messageInfo = { 0, 0, NULL, NULL };

static volatile unsigned long	idleCounter;
static unsigned long	maxCountsPerTick;
static volatile bool resetIdleCount = false;
static bool idleTaskStarted = false;

void DBG_EnableTaskSwitchLogging(unsigned int recordCount)
{
	if ( recordCount > 2 && taskSwitchInfo.record == NULL )
	{
		taskSwitchInfo.record = new DBG_TaskSwitchRecord[recordCount];
		taskSwitchInfo.recordCount = recordCount;
		memset(taskSwitchInfo.record, 0, recordCount*sizeof(DBG_TaskSwitchRecord));

		taskSpawn("idle", 255, 0, 2000, (FUNCPTR)idleTask, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
		idleTaskStarted = true;

		taskSwitchHookAdd((FUNCPTR)taskSwitchHook);
	}
}

void DBG_EnableMessageLogging(unsigned int recordCount)
{
	if ( recordCount > 2 && messageInfo.record == NULL )
	{
		messageInfo.record = new DBG_MessageRecord[recordCount];
		messageInfo.recordCount = recordCount;
		memset(messageInfo.record, 0, recordCount*sizeof(DBG_MessageRecord));

		semBCreate(SEM_Q_PRIORITY, SEM_FULL);
	}
}

void DBG_LogReceivedMessage(int taskID, int sendTaskID, unsigned long msgID)
{
	if ( messageInfo.record )
	{
		semTake(messageInfo.updateLock, WAIT_FOREVER);

		DBG_MessageRecord * recordPtr = &messageInfo.record[messageInfo.recordIndex];
		recordPtr->sendTID = sendTaskID;
		recordPtr->receiveTID = taskID;
		datalog_GetTimeStamp(&recordPtr->timeStamp);
		messageInfo.recordIndex = (messageInfo.recordIndex+1) % messageInfo.recordCount;

		semGive(messageInfo.updateLock);
	}
}

void DBG_LogSentMessage(int taskID, int op, unsigned long msgID)
{
	if ( messageInfo.record )
	{
		semTake(messageInfo.updateLock, WAIT_FOREVER);

		DBG_MessageRecord * recordPtr = &messageInfo.record[messageInfo.recordIndex];
		recordPtr->sendTID = taskID;
		recordPtr->receiveTID = 0xf0000000 | op;
		datalog_GetTimeStamp(&recordPtr->timeStamp);
		messageInfo.recordIndex = (messageInfo.recordIndex+1) % messageInfo.recordCount;

		semGive(messageInfo.updateLock);
	}
}

void DBG_DumpData(void)
{
	if ( taskSwitchInfo.record )
	{
		datalog_WriteBinaryRecord(log_handle_critical, DBG_RecordType, DBG_TaskSwitchInfoSubType,
                                 taskSwitchInfo.record, taskSwitchInfo.recordCount*sizeof(DBG_TaskSwitchRecord));
	}

	if ( messageInfo.record )
	{
		datalog_WriteBinaryRecord(log_handle_critical, DBG_RecordType, DBG_MessageInfoSubType,
                                 messageInfo.record, messageInfo.recordCount*sizeof(DBG_MessageRecord));
	}
}

int DBG_GetIdlePercent(void)
{
	static unsigned long	lastQueryTick = 0;
	static int result = 100;

	if ( idleTaskStarted )
	{
		unsigned long	tick = tickGet();
		if ( tick < lastQueryTick ||
			  !lastQueryTick )
		{
			// tick counter wrapped or first call
			lastQueryTick = tick;
			resetIdleCount = true;
		}
		else if ( tick >= lastQueryTick + sysClkRateGet()/2 )
		{
			unsigned long	countsPerTick = idleCounter/(tick - lastQueryTick);
			result = (100*countsPerTick)/maxCountsPerTick;
			if ( result > 100 ) result = 100;

			lastQueryTick = tick;
			resetIdleCount = true;
		}
	}

	return result;
}

static void idleTask(void)
{
	enum { CalibrationTicks = 5 };

	unsigned long startTick, currentTick, endTick;

	// wait until we are at a clock tick boundary
	taskLock();
	startTick = tickGet();

	do
	{
		currentTick = tickGet();
	} while ( currentTick == startTick );

	// calibration phase
	endTick = currentTick+CalibrationTicks;
	do
	{
		for ( int delay=0; delay<100; delay++ )
		{
			if ( resetIdleCount )
			{
				idleCounter = 0;
				resetIdleCount = false; 
			}
		}

		idleCounter += 1;
		currentTick = tickGet();
	} while ( currentTick < endTick );
	
	taskUnlock();

	// compute expected max count per tick (if 100% idle)
	maxCountsPerTick = idleCounter/CalibrationTicks;

	// update loop
	while ( 1 )
	{
		endTick = currentTick + CalibrationTicks;
		do
		{
			for ( int delay=0; delay<100; delay++ )
			{
				if ( resetIdleCount )
				{
					idleCounter = 0;
					resetIdleCount = false; 
				}
			}

			idleCounter += 1;
			currentTick = tickGet();
		} while ( currentTick < endTick );
	}
}

static void taskSwitchHook(int oldTID, int newTID)
{
	DBG_TaskSwitchRecord * recordPtr = &taskSwitchInfo.record[taskSwitchInfo.recordIndex];
	recordPtr->oldTID = oldTID;
	recordPtr->newTID = newTID;
	datalog_GetTimeStamp(&recordPtr->timeStamp);

	taskSwitchInfo.recordIndex = (taskSwitchInfo.recordIndex+1) % taskSwitchInfo.recordCount;
}

