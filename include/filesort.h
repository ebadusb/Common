/*
 * Copyright (C) 2002 Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: //BCTquad3/home/BCT_Development/vxWorks/Common/include/rcs/filesort.h 1.2 2003/05/13 09:00:40 jl11312 Exp pn02526 $
 * $Log: filesort.h $
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
 * When the last file name has been processed, it will execute the call back
 * function a final time, with fullPathName set to NULL.
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

