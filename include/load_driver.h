/*
 * Copyright (C) 2003 Gambro BCT, Inc.  All rights reserved.
 *
 * $Header$
 * $Log$
 *
 */

#ifndef _LOAD_DRIVER_INCLUDE
#define _LOAD_DRIVER_INCLUDE

#ifdef __cplusplus
extern "C" {
#endif /* ifdef __cplusplus */

void parseDriverCommandLine(const char * driverCmd, char ** driverFileName, char ** initFuncName);
const char * initializeDriver(const char * driverCmd, const char * driverFileName, const char * initFuncName);
STATUS loadDriverFromMemory(const char * fileName, char * memStart, size_t memLength);

#ifdef __cplusplus
}; // extern "C"
#endif /* ifdef __cplusplus */

#endif /* ifndef _LOAD_DRIVER_INCLUDE */

