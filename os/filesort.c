/*
 * Copyright (C) 2002 Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: K:/BCT_Development/vxWorks/Common/os/rcs/filesort.c 1.1 2002/09/19 22:15:43 jl11312 Exp jl11312 $
 * $Log: filesort.c $
 *
 */

#include <vxworks.h>
#include <dirent.h>
#include <stdio.h>
#include "filesort.h"

typedef struct
{
	size_t	fileNameIndex;
	TIME		lastModifyTime;
} FileSortEntry;

static int fileCmpByDateAscending(const FileSortEntry * e1, const FileSortEntry * e2)
{
	int	retVal;

	if ( e1->lastModifyTime > e2->lastModifyTime ) retVal = 1;
	else if ( e1->lastModifyTime < e2->lastModifyTime ) retVal = -1;
	else retVal = 0;

	return retVal;
}

static int fileCmpByDateDescending(const FileSortEntry * e1, const FileSortEntry * e2)
{
	int	retVal;

	if ( e1->lastModifyTime < e2->lastModifyTime ) retVal = 1;
	else if ( e1->lastModifyTime > e2->lastModifyTime ) retVal = -1;
	else retVal = 0;

	return retVal;
}

static void buildFullPathName(char * result, const char * dirName, const char * fileName)
{
	strcpy(result, dirName);
	if ( dirName[strlen(dirName)-1] != '/' )
	{
		strcat(result, "/");
	}

	strcat(result, fileName);
}

STATUS fileSort(const char * dirName, FileSortType sortType, fileSortCallBack * callBack)
{
	STATUS	retVal = ERROR;
	DIR * sortDir = NULL;

	typedef int FileSortCmp(const void *e1, const void *e2);
	FileSortCmp	* fileSortCmp;

	switch ( sortType )
	{
	case FILE_SORT_BY_DATE_ASCENDING:
		fileSortCmp = (FileSortCmp *)fileCmpByDateAscending;
		break;

	case FILE_SORT_BY_DATE_DESCENDING:
		fileSortCmp = (FileSortCmp *)fileCmpByDateDescending;
		break;
	
	default:
		fileSortCmp = NULL;
		break;
	}

	if ( fileSortCmp )
	{
		sortDir = opendir((char *)dirName);
	}

	if ( sortDir )
	{
		struct dirent	* dirEntry;
		char 	fullPathName[NAME_MAX+1];

		size_t	fileCount = 0;
		size_t	fileListSize = 512;
		FileSortEntry * fileList = malloc(fileListSize*sizeof(FileSortEntry));

		size_t	fileNameBufferSize = fileListSize*32;
		size_t	fileNameBufferPos = 0;
		char * fileNameBuffer = malloc(fileNameBufferSize*sizeof(char));

		FileCallBackStatus	callBackStatus = FILE_CALLBACK_CONTINUE;

		/*
		 *	Scan directory
		 */
		while ( (dirEntry = readdir(sortDir)) != NULL )
		{
			/*
			 *	Skip . and .. directory entries
			 */
			if ( strcmp(dirEntry->d_name, ".") != 0 &&
				  strcmp(dirEntry->d_name, "..") != 0 )
			{
				int	fileNameLen = strlen(dirEntry->d_name);
				struct stat	fileStat;

				if ( fileCount >= fileListSize )
				{
					/*
					 *	Double file list size or increase by 4096 entries, whichever
					 * is smaller.
					 */
					fileListSize += (fileListSize < 4096) ? fileListSize : 4096;
					fileList = realloc(fileList, fileListSize*sizeof(FileSortEntry));
				}

				if ( fileNameBufferPos+fileNameLen+1 >= fileNameBufferSize )
				{
					/*
					 *	Look at average file name size so far and increase buffer
					 * size according to current file list size.
					 */
					size_t	avgFileNameLen = fileNameBufferPos/fileCount + 1;
					fileNameBufferSize = avgFileNameLen*fileListSize + fileNameLen + 1;
					fileNameBuffer = realloc(fileNameBuffer, fileNameBufferSize*sizeof(char));
				}

				/*
				 *	Get file information and add it to the file list
				 */
				buildFullPathName(fullPathName, dirName, dirEntry->d_name);
				if ( stat(fullPathName, &fileStat) == OK )
				{
					strcpy(&fileNameBuffer[fileNameBufferPos], dirEntry->d_name);
					fileNameBufferPos += fileNameLen+1;

					fileList[fileCount].fileNameIndex = fileNameBufferPos;
					fileList[fileCount].lastModifyTime = fileStat.st_mtime;
					fileCount += 1;
				}
			}
		}

		/*
		 *	Send sorted list of files to call back function
		 */
		if ( fileCount > 0 )
		{
			int	file = 0;

			/*
			 *	Sort file entries
			 */
			qsort(fileList, fileCount, sizeof(FileSortEntry), fileSortCmp);
			while ( file < fileCount &&
					  callBackStatus == FILE_CALLBACK_CONTINUE )
			{
				buildFullPathName(fullPathName, dirName, &fileNameBuffer[fileList[file].fileNameIndex]);
				callBackStatus = (*callBack)(fullPathName);
				file += 1;
			}
		}

		if ( callBackStatus == FILE_CALLBACK_CONTINUE )
		{
			/*
			 *	Perform final call back to notify application of end of file list
			 */
			(*callBack)(NULL);
		}

		/*
		 *	Clean up
		 */
		closedir(sortDir);
		free(fileNameBuffer);
		free(fileList);
		retVal = OK;
	}

	return retVal;
}

