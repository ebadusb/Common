/*
 * Copyright (C) 2003 Gambro BCT, Inc.  All rights reserved.
 *
 * $Header$
 * $Log$
 *
 */

#ifndef _TASK_START_INCLUDE
#define _TASK_START_INCLUDE

#ifdef __cplusplus
extern "C" {
#endif /* ifdef __cplusplus */

int taskStart(const char * taskName, int priority, unsigned long stackSize, FUNCPTR entryPoint, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7, int arg8, int arg9, int arg10);

#ifdef __cplusplus
}; // extern "C"
#endif /* ifdef __cplusplus */

#endif /* ifndef _TASK_START_INCLUDE */

