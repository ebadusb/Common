/*
 * Copyright (C) 2002 Gambro BCT, Inc.  All rights reserved.
 *
 * $Header$
 * $Log: filesort.h $
 * Revision 1.1  2002/09/19 22:16:15Z  jl11312
 * Initial revision
 *
 */

#ifndef _FILESORT_INCLUDE
#define _FILESORT_INCLUDE

#ifdef __cplusplus
extern "C" {
#endif /* ifdef __cplusplus */

typedef enum
{
	FILE_CALLBACK_CONTINUE,		/* fileSort() should continue to next file */
	FILE_CALLBACK_END				/* fileSort() should stop with no further callbacks */
} FileCallBackStatus;

typedef enum
{
	/*
	 *	Other types can be added here if necessary.  The necessary support routine
	 * for the qsort call in fileSort() will also need to be added to filesort.c
	 */
	FILE_SORT_BY_DATE_ASCENDING,
	FILE_SORT_BY_DATE_DESCENDING
} FileSortType;

typedef FileCallBackStatus fileSortCallBack(const char * fullPathName);

/*
 * The fileSort function accepts a directory name, sort type, and application
 * supplied call back function.  It scans and sorts the directory, and executes
 * the call back function for each file in the directory, in the order specified.
 *
 * The call back function should normally return FILE_CALLBACK_CONTINUE. A
 * value of FILE_CALLBACK_END can be return to end the file sort operation
 * before the last file name has been processed.
 */
STATUS fileSort(const char * dirName, FileSortType sortType, fileSortCallBack * callBack);

#ifdef __cplusplus
}; // extern "C"
#endif /* ifdef __cplusplus */

#endif /* ifndef _FILESORT_INCLUDE */

