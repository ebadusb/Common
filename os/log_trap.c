/*
 * Copyright (C) 2003 Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: K:/BCT_Development/vxWorks/Common/os/rcs/log_trap.c 1.3 2003/06/19 20:05:59Z jl11312 Exp jl11312 $
 * $Log: log_trap.c $
 * Revision 1.1  2003/04/30 14:54:39Z  jl11312
 * Initial revision
 */

#include "log_trap.h"

static char * logBuffer = NULL;
static int logBufferEndIdx;
static int logBufferReadIdx;
static int logBufferWriteIdx;
static SEM_ID	logDataAvailSem;

static void logTrapTask(const char * pipeName, size_t pipeBufferSize)
{
	int	logTrapFD = open(pipeName, O_RDONLY, DEFAULT_FILE_PERM);
	char	* buff = malloc(pipeBufferSize);

	while ( logTrapFD > 0 )
	{
		size_t  size;

		size = read(logTrapFD, buff, PIPE_LOGTRAP_BUFFER);
		if ( size >= 0 )
		{
			size_t sizeRemaining = size;
			size_t copyIdx = 0;
			size_t sizeAvail = ( logBufferWriteIdx < logBufferReadIdx ) ?
											logBufferReadIdx - logBufferWriteIdx - 1 :
											logBufferEndIdx - (logBufferWriteIdx-logBufferReadIdx) - 1;

			write(consoleFd, buff, size);

			/*
			 *	Because the log buffer is organized as a circular buffer, the copy may need to
			 * be done in two pieces - the first from write index to the end of the buffer, and
			 * the second from the beginning of the buffer.
			 */
			if ( sizeRemaining > 0 &&
				  sizeAvail >= sizeRemaining )
			{
				size_t	copySize = (sizeRemaining < logBufferEndIdx-logBufferWriteIdx) ?
							             sizeRemaining : logBufferEndIdx-logBufferWriteIdx;
				memcpy(&logBuffer[logBufferWriteIdx], buff, copySize);

				logBufferWriteIdx += copySize;
				sizeRemaining -= copySize;
				sizeAvail -= copySize;
				copyIdx = copySize;

				if ( logBufferWriteIdx >= logBufferEndIdx )
				{
					logBufferWriteIdx = 0;
				}
			}

			if ( sizeRemaining > 0 )
			{
				size_t	copySize = (sizeRemaining < sizeAvail) ? sizeRemaining : sizeAvail;

				memcpy(&logBuffer[logBufferWriteIdx], &buff[copyIdx], copySize);
				logBufferWriteIdx += copySize;
			}

			semGive(logDataAvailSem);
		}
		else
		{
			close(logTrapFD);
			logTrapFD = -1;
		}
	}

	free(buff);
}

void logTrapWaitData(void)
{
	semTake(logDataAvailSem, WAIT_FOREVER);
}

int logTrapRead(char * buffer, size_t bufferSize)
{
	int	result = 0;
	size_t sizeAvail = ( logBufferReadIdx <= logBufferWriteIdx ) ?
							logBufferWriteIdx - logBufferReadIdx :
							logBufferEndIdx - logBufferReadIdx;

	if ( sizeAvail > 0 )
	{
		size_t sizeCopy = (bufferSize < sizeAvail) ? bufferSize : sizeAvail;
		size_t newReadIdx = logBufferReadIdx + sizeCopy;

		memcpy(buffer, &logBuffer[logBufferReadIdx], sizeCopy);
		if ( newReadIdx >= logBufferEndIdx )
		{
			newReadIdx = logBufferEndIdx;
		}

		logBufferReadIdx = newReadIdx;
		result = sizeCopy;
	}

	return result;
}

int logTrapInit(const char * pipeName, size_t pipeBufferSize, size_t logBufferSize)
{
	int	stdoutTrap = -1;
	int	stderrTrap = -1;
	int	result = 1;

	//
	// Create a pipe device for trapping stdout/stderr output
	//
	pipeDevCreate(pipeName, 8, pipeBufferSize);
	stdoutTrap = open(pipeName, O_WRONLY, DEFAULT_FILE_PERM);
	stderrTrap = open(pipeName, O_WRONLY, DEFAULT_FILE_PERM);

	if ( stdoutTrap < 0 || stderrTrap < 0 )
	{
		fprintf(stderr, "failed to open pipe device for log trapping\n");
		perror(PIPE_LOGTRAP);
		result = 0;
	}

	if ( result )
	{
		ioGlobalStdSet(1, stdoutTrap);
		ioGlobalStdSet(2, stderrTrap);

		logBuffer = malloc(logBufferSize);
		logBufferEndIdx = logBufferSize;
		logBufferReadIdx = logBufferWriteIdx = 0;

		logDataAvailSem = semBCreate(SEM_Q_PRIORITY, SEM_EMPTY);
		taskStart("log_trap", 2, 4000, logTrapTask, (int)pipeName, (int)pipeBufferSize, 0, 0, 0, 0, 0, 0, 0, 0);
	}
	else
	{
		if ( stdoutTrap > 0 ) close(stdoutTrap);
		if ( stderrTrap > 0 ) close(stderrTrap);
	}

	return result;
}
