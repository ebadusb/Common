/*
 * Copyright (C) 2002 Gambro BCT, Inc.  All rights reserved.
 *
 *	vxWorks utilities for logging debug information after a system failure is detected
 *
 * $Header: K:/BCT_Development/vxWorks/Common/debug/rcs/failure_debug.cpp 1.12 2004/03/24 19:47:47Z ms10234 Exp jl11312 $
 * $Log: failure_debug.cpp $
 * Revision 1.11  2004/02/24 22:31:40Z  jl11312
 * - more debug logging (see IT 6880)
 * Revision 1.10  2003/12/16 21:57:10Z  jl11312
 * - replaced binary semaphore with mutex semaphore to avoid potential priority inversion
 * Revision 1.9  2003/10/03 14:31:05Z  jl11312
 * - added switch back from hex to decimal after logging stack traces
 * Revision 1.8  2003/10/03 12:37:20Z  jl11312
 * - increased maximum stack levels displayed for debugging
 * Revision 1.7  2003/06/25 17:10:09Z  jl11312
 * - added logging for safety side CPU idle time and memory availability
 * Revision 1.6  2003/05/29 16:26:11Z  jl11312
 * - added logging of trace back information for all tasks
 * Revision 1.5  2003/05/23 16:09:32Z  jl11312
 * - additional debug logging
 * Revision 1.4  2003/05/21 20:13:23Z  jl11312
 * - added missing include file
 * Revision 1.3  2003/05/21 20:02:42Z  jl11312
 * - enhanced memory protection (IT 6091)
 * Revision 1.2  2003/05/05 14:14:53Z  jl11312
 * - enabled message debug logging (IT 5915)
 * Revision 1.1  2003/02/28 22:07:52Z  jl11312
 * Initial revision
 *
 */

#include <vxWorks.h>
#include <private/memPartLibP.h>
#include <sysLib.h>
#include <tickLib.h>
#include <taskHookLib.h>

#include "failure_debug.h"
#include "task_start.h"

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

static volatile unsigned int	maxWordsAllocated = 0;

void DBG_EnableTaskSwitchLogging(unsigned int recordCount)
{
	if ( recordCount > 2 && taskSwitchInfo.record == NULL )
	{
		taskSwitchInfo.record = new DBG_TaskSwitchRecord[recordCount];
		taskSwitchInfo.recordCount = recordCount;
		memset(taskSwitchInfo.record, 0, recordCount*sizeof(DBG_TaskSwitchRecord));

		taskStart("idle", 255, 2000, (FUNCPTR)idleTask, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
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

		messageInfo.updateLock = semMCreate(SEM_Q_PRIORITY | SEM_INVERSION_SAFE);
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
		recordPtr->msgID = msgID;
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
		recordPtr->msgID = msgID;
		datalog_GetTimeStamp(&recordPtr->timeStamp);
		messageInfo.recordIndex = (messageInfo.recordIndex+1) % messageInfo.recordCount;

		semGive(messageInfo.updateLock);
	}
}

void DBG_LogSentNetworkMessage(int taskID, int netaddr, unsigned long msgID)
{
	if ( messageInfo.record )
	{
		semTake(messageInfo.updateLock, WAIT_FOREVER);

		DBG_MessageRecord * recordPtr = &messageInfo.record[messageInfo.recordIndex];
		recordPtr->sendTID = taskID;
		recordPtr->receiveTID = netaddr;
		recordPtr->msgID = msgID;
		datalog_GetTimeStamp(&recordPtr->timeStamp);
		messageInfo.recordIndex = (messageInfo.recordIndex+1) % messageInfo.recordCount;

		semGive(messageInfo.updateLock);
	}
}

void DBG_DumpData(void)
{
	enum { MaxTasks = 100 };
	static WIND_TCB * taskList[MaxTasks];
	static bool dumpDone = false;

	if ( dumpDone ) return;
	dumpDone = true;

	int numTasks = taskIdListGet((int *)taskList, MaxTasks);
	DataLog_Stream & outStream = log_level_critical(__FILE__, __LINE__);
	outStream << "Tasks:" << hex;

	for ( int i=0; i<numTasks; i++ )
	{
		outStream << " " << (unsigned int)taskList[i] << ":" << (unsigned int)taskList[i]->regs.pc;
		unsigned int * frame = (unsigned int *)taskList[i]->regs.ebp;
		int	stackCount = 0;
		while ( (char *)frame < taskList[i]->pStackBase && (char *)frame > taskList[i]->pStackLimit && stackCount < 10 )
		{
			outStream << " " << frame[1];
			frame = (unsigned int *)(*frame);
			stackCount += 1;
	   }
	}
	outStream << dec << endmsg;

	outStream << "Task state:" << hex;
	for ( int i=0; i<numTasks; i++ )
	{
		outStream << (unsigned int)taskList[i] << ":" << taskList[i]->status << " " <<
                    taskList[i]->priority << " " << taskList[i]->priNormal << " ";
      if ( taskList[i]->pPriMutex )
		{
			outStream << (unsigned int)taskList[i]->pPriMutex << " " << (unsigned int)taskList[i]->pPriMutex->state.owner << " ";
		}
	}
	outStream << dec << endmsg;

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

unsigned int DBG_GetMaxWordsAllocated(void)
{
	return maxWordsAllocated;
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

			if ( memSysPartId->curWordsAllocated > maxWordsAllocated )
			{
				maxWordsAllocated = memSysPartId->curWordsAllocated;
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

				if ( memSysPartId->curWordsAllocated > maxWordsAllocated )
				{
					maxWordsAllocated = memSysPartId->curWordsAllocated;
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

