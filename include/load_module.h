/*
 * Copyright (C) 2003 Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: K:/BCT_Development/vxWorks/Common/include/rcs/load_module.h 1.1 2003/05/21 20:02:00Z jl11312 Exp jl11312 $
 * $Log: load_module.h $
 * Revision 1.1  2003/05/21 20:02:00Z  jl11312
 * Initial revision
 *
 */

#ifndef _LOAD_MODULE_INCLUDE
#define _LOAD_MODULE_INCLUDE

#ifdef __cplusplus
extern "C" {
#endif /* ifdef __cplusplus */

typedef struct
{
	unsigned long	textAddr;
	unsigned long	textSize;
	unsigned long  dataAddr;
	unsigned long	dataSize;
	unsigned long	bssAddr;
	unsigned long	bssSize;
} LoadModuleInfo;

STATUS loadModuleFromFile(const char * fileName, LoadModuleInfo * info);

#ifdef __cplusplus
}; // extern "C"
#endif /* ifdef __cplusplus */

#endif /* ifndef _LOAD_MODULE_INCLUDE */

