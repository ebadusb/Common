/*
 * Copyright (C) 2002 Gambro BCT, Inc.  All rights reserved.
 *
 *	vxWorks utilities for logging debug information after a system failure is detected
 *
 * $Header: //bctquad3/home/BCT_Development/vxWorks/Common/include/rcs/failure_debug.h 1.3 2003/06/25 17:09:56Z jl11312 Exp ms10234 $
 * $Log: failure_debug.h $
 *
 */

#ifndef _FAILURE_DEBUG_INCLUDE
#define _FAILURE_DEBUG_INCLUDE

#include <vxWorks.h>
#include <semLib.h>
#include "datalog.h"

/*
 *	Types/subtypes used for binary data log records associated with
 * debug entries.
 */
#define DBG_RecordType 0x0001
#define DBG_TaskSwitchInfoSubType	0x0001
#define DBG_MessageInfoSubType		0x0002
#define DBG_MessageRegisterSubType	0x0003

typedef struct
{
	int	oldTID __attribute__ ((packed));
	int	newTID __attribute__ ((packed));
	DataLog_TimeStamp	timeStamp __attribute__ ((packed));
} DBG_TaskSwitchRecord;

typedef struct
{
	unsigned int	recordIndex;
	unsigned int	recordCount;
	DBG_TaskSwitchRecord * record;
} DBG_TaskSwitchInfo;

typedef struct
{
	int	sendTID __attribute__ ((packed));
	int	receiveTID __attribute__ ((packed));
	unsigned long msgID __attribute__ ((packed));
	DataLog_TimeStamp	timeStamp __attribute__ ((packed));
} DBG_MessageRecord;

typedef struct
{
	unsigned int	recordIndex;
	unsigned int	recordCount;
	DBG_MessageRecord * record;
	SEM_ID updateLock;
} DBG_MessageInfo;

#ifdef __cplusplus
extern "C" {
#endif /* ifdef __cplusplus */

void DBG_EnableTaskSwitchLogging(unsigned int recordCount);
DBG_MessageInfo * DBG_EnableMessageLogging(unsigned int recordCount);
void DBG_DumpData(void);
int DBG_GetIdlePercent(void);

#ifdef __cplusplus
}; // extern "C"
#endif /* ifdef __cplusplus */

#endif /* ifndef _FAILURE_DEBUG_INCLUDE */

