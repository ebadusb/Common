/*
 *  Copyright(c) 2006 by Gambro BCT, Inc. All rights reserved.
 *
 * $Header: H:/BCT_Development/vxWorks/Common/firewire/rcs/fw_utility.h 1.1 2007/02/07 15:22:42Z wms10235 Exp wms10235 $
 *
 * This file contains firewire utility routines.
 *
 * $Log: fw_utility.h $
 *
 */

#ifndef _FW_UTILITY_INCLUDE_
#define _FW_UTILITY_INCLUDE_

#include <vxWorks.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Function for allocating standard memory.
 */
void *fwMalloc(size_t numBytes);

/* Function to free standard memory.
 */
void fwFree(void *ptr);

/* Reports current memory allocations statistics for
 * standard memory.
 */
void fwMemShow(void);

/*
 * CRC-16 calculation, from the IEEE-1212 spec on page 28.
 * This function is initially called with a crcValue of 0.
 * A new CRC value is returned on each call for use in
 * the next iteration.
 */
UINT16 fwComputeCRC16(UINT32 crcValue, UINT32 data);

/* perform byte swapping on a 32-bit integer. Used
 * convert from/to little endian to big endian.
 */
UINT32 fwByteSwap32(const UINT32 value);

/* Global variable for log level. */
extern int fwMsgLoggingLevel;

/* Initialize and open a log file with at a given
 * log level. Returns 0 on success, -1 on failure.
 */
int fwOpenLog(const char *logfile, int level);

/* Close the log file.
 */
void fwCloseLog(void);

/* Flush the log file buffers to disk.
 */
void fwFlushLog(void);

/* Set logging level for debug logging. Valid
 * levels are 0 to 9. For level 0, nothing is
 * logged, and for level 9, everything is logged.
 */
void fwSetLoggingLevel(int level);

/* Functions for logging that work like printf
 * but only output when the log level is greater
 * than or equal to the functions number.
 */
void fwLogMsg(const char *format, ...);

#define FWLOGLEVEL1 if(fwMsgLoggingLevel >= 1) printf
#define FWLOGLEVEL2 if(fwMsgLoggingLevel >= 2) printf
#define FWLOGLEVEL3 if(fwMsgLoggingLevel >= 3) printf
#define FWLOGLEVEL4 if(fwMsgLoggingLevel >= 4) printf
#define FWLOGLEVEL5 if(fwMsgLoggingLevel >= 5) printf
#define FWLOGLEVEL6 if(fwMsgLoggingLevel >= 6) printf
#define FWLOGLEVEL7 if(fwMsgLoggingLevel >= 7) printf
#define FWLOGLEVEL8 if(fwMsgLoggingLevel >= 8) printf
#define FWLOGLEVEL9 if(fwMsgLoggingLevel >= 9) printf

#ifdef __cplusplus
}
#endif

#endif /* #ifndef _FW_UTILITY_INCLUDE_ */

