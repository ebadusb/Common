/*
 * Copyright (C) 2003 Gambro BCT, Inc.  All rights reserved.
 *
 * $Header$
 * $Log$
 *
 */

#ifndef _LOG_TRAP_INCLUDE
#define _LOG_TRAP_INCLUDE

#ifdef __cplusplus
extern "C" {
#endif /* ifdef __cplusplus */

int logTrapInit(const char * pipeName, size_t pipeBufferSize, size_t logBufferSize);
void logTrapWaitData(void);
int logTrapRead(char * buffer, size_t bufferSize);

#ifdef __cplusplus
}; // extern "C"
#endif /* ifdef __cplusplus */

#endif /* ifndef _LOG_TRAP_INCLUDE */


