/*
 * Copyright (C) 2003 Gambro BCT, Inc.  All rights reserved.
 *
 * $Header$
 * $Log$
 *
 */

#ifndef _OPTION_FILE_INCLUDE
#define _OPTION_FILE_INCLUDE

#ifdef __cplusplus
extern "C" {
#endif /* ifdef __cplusplus */

typedef struct OptionFileDataStruct
{
	char * name;
	char * setting;

	struct OptionFileDataStruct * next;
} OptionFileData;

STATUS readOptionFile(const char * fileName, const char * sectionName, OptionFileData ** data, int crcRequired);
STATUS getOption(const char * optionName, const char ** option, OptionFileData * data);

#ifdef __cplusplus
}; // extern "C"
#endif /* ifdef __cplusplus */

#endif /* ifndef _OPTION_FILE_INCLUDE */

