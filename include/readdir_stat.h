/*
 * Copyright (C) 2003 Gambro BCT, Inc.  All rights reserved.
 *
 * $Header$
 * $Log$
 *
 */

#ifndef _READDIR_STAT_INCLUDE
#define _READDIR_STAT_INCLUDE

#include <dirent.h>
#include <sys/stat.h>

#ifdef __cplusplus
extern "C" {
#endif /* ifdef __cplusplus */

struct dirent * readdir_stat(DIR * pDir, struct stat * pStat);

#ifdef __cplusplus
}; // extern "C"
#endif /* ifdef __cplusplus */

#endif /* ifndef _READDIR_STAT_INCLUDE */

