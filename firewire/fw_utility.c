/*
 *  Copyright(c) 2006 by Gambro BCT, Inc. All rights reserved.
 *
 * $Header: H:/BCT_Development/vxWorks/Common/firewire/rcs/fw_utility.c 1.2 2007/02/13 22:46:48Z wms10235 Exp wms10235 $
 *
 * This file contains firewire utility routines.
 *
 * $Log: fw_utility.c $
 * Revision 1.1  2007/02/07 15:22:41Z  wms10235
 * Initial revision
 *
 */

#include <vxWorks.h>
#include <stdio.h>
#include <stdlib.h>
#include <semLib.h>
#include "fw_utility.h"

static long fwStandardMemAllocationCount = 0;
static long fwStandardMemAllocationFailed = 0;
static FILE *fwMsgLoggingFile = NULL;
static SEM_ID fwMsgLoggingSem = NULL;
int fwMsgLoggingLevel = 0;

void *fwMalloc(size_t numBytes)
{
	void *memPtr = malloc( numBytes );

	if( memPtr )
	{
		fwStandardMemAllocationCount++;
	}
	else
	{
		fwStandardMemAllocationFailed++;
		FWLOGLEVEL1("memory allocation failed!\n");
	}

	return memPtr;
}

void fwFree(void *ptr)
{
	if( ptr )
	{
		free( ptr );
		fwStandardMemAllocationCount--;
	}
}

void fwMemShow(void)
{
	printf("Standard memory. Allocations:%d Failures:%d\n", fwStandardMemAllocationCount, fwStandardMemAllocationFailed);
}

int fwOpenLog(const char *logfile, int level)
{
	int retVal = -1;

	fwMsgLoggingFile = fopen( logfile, "w" );

	if( fwMsgLoggingFile )
	{
		fwMsgLoggingSem = semMCreate( SEM_Q_PRIORITY | SEM_INVERSION_SAFE );
		fwMsgLoggingLevel = level;
		retVal = 0;
	}
	else
		fwMsgLoggingLevel = 0;

	return retVal;
}

void fwCloseLog(void)
{
	semTake( fwMsgLoggingSem, WAIT_FOREVER );

	if( fwMsgLoggingFile )
	{
		fclose( fwMsgLoggingFile );
		fwMsgLoggingFile = NULL;
	}
	fwMsgLoggingLevel = 0;

	semGive( fwMsgLoggingSem );
}

void fwFlushLog(void)
{
	semTake( fwMsgLoggingSem, WAIT_FOREVER );
	if( fwMsgLoggingFile )
	{
		fflush( fwMsgLoggingFile );
	}
	semGive( fwMsgLoggingSem );
}

void fwSetLoggingLevel(int level)
{
	semTake( fwMsgLoggingSem, WAIT_FOREVER );
	if( fwMsgLoggingFile )
	{
		fwMsgLoggingLevel = level;
	}
	semGive( fwMsgLoggingSem );
}

void fwLogMsg(const char *format, ...)
{
	va_list args;
	va_start(args, format);

	semTake( fwMsgLoggingSem, WAIT_FOREVER );

	if( fwMsgLoggingLevel >= 1 && fwMsgLoggingFile )
	{
		vfprintf( fwMsgLoggingFile, format, args );
		fflush( fwMsgLoggingFile );
	}

	semGive( fwMsgLoggingSem );
	va_end(args);
}

UINT16 fwComputeCRC16(UINT32 crcValue, UINT32 data)
{
	int shift;
	INT32 sum;

	for( shift = 28; shift >= 0; shift -= 4 )
	{
		sum = (INT32)(((crcValue >> 12) ^ (data >> shift)) & 0x0000000F);
		crcValue = (crcValue << 4) ^ (sum << 12) ^ (sum << 5) ^ sum;
	}

	return (UINT16)(crcValue & 0x0000FFFF);
}

UINT32 fwByteSwap32(const UINT32 value)
{
	return (((value<<24) & 0xFF000000) | ((value<<8) & 0x00FF0000) | ((value>>8) & 0x0000FF00) | ((value>>24) & 0x000000FF));
}

