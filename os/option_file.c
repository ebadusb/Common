/*
 * Copyright (C) 2003 Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: K:/BCT_Development/vxWorks/Common/os/rcs/option_file.c 1.2 2003/11/05 19:28:49Z jl11312 Exp jl11312 $
 * $Log: option_file.c $
 * Revision 1.2  2003/11/05 19:28:49Z  jl11312
 * - close file at end of processing
 * Revision 1.1  2003/06/26 22:31:26Z  jl11312
 * Initial revision
 *
 */

#include <vxworks.h>
#include <stdio.h>
#include "option_file.h"

static STATUS processSectionHeader(const char * fileName, int lineNumber, char * token, const char * sectionName, int * foundSection)
{
	STATUS	result = OK;
	int tokenLen = strcspn(token, "]");

	if ( token[tokenLen] != ']' )
	{
		fprintf(stderr, "%s(%d): error parsing section header\n", fileName, lineNumber);
		result = ERROR;
	}
	else 
	{
		token[tokenLen] = '\0';
		*foundSection = (strcmp(token, sectionName) == 0) ? 1 : 0;
	}

	return result;
}

static STATUS processKeyword(const char * fileName, int lineNumber, char * token, OptionFileData ** data, int crcRequired)
{
	STATUS	result = OK;
	char * equals = strchr(token, '=');
	int	keywordLen = 0;
	int	settingLen = 0;

	if ( !equals )
	{
		fprintf(stderr, "%s(%d): missing \"=\"\n", fileName, lineNumber);
		result = ERROR;
	}
	else
	{
		keywordLen = equals-token;
		if ( crcRequired &&
			  equals[1] != 0 )
		{
			char * comma = strrchr(token, ',');
			unsigned long	checkValue = 0;
			unsigned long	verifyValue = 0;

			if ( !comma ||
				  sscanf(comma+1, "%lx", &verifyValue) != 1 )
			{
				fprintf(stderr, "%s(%d): missing CRC value\n", fileName, lineNumber);
				result = ERROR;
		   }

			settingLen = comma-equals-1; 
			crcgen32(&checkValue, (unsigned char *)token, keywordLen+settingLen+1);
         if ( checkValue != verifyValue )
			{
				fprintf(stderr, "%s(%d): incorrect CRC value\n", fileName, lineNumber);
				result = ERROR;
			}
		}
		else
		{
			settingLen = strlen(equals+1);
		}
   }

	if ( result == OK )
	{
		/*
		 *	Allocate one chunck of memory to store new data entry as well as the
		 * name and setting strings.
		 */
		char * newData = malloc(sizeof(OptionFileData) + (keywordLen+settingLen+2)*sizeof(char));

		OptionFileData * newEntry = (OptionFileData *)newData;
		newEntry->name = (char *)(&newEntry[1]);
		newEntry->setting = &(newEntry->name[keywordLen+1]);

		strncpy(newEntry->name, token, keywordLen);
		newEntry->name[keywordLen] = '\0';

		strncpy((char *)newEntry->setting, equals+1, settingLen);
		newEntry->setting[settingLen] = '\0';

		newEntry->next = *data;
		*data = newEntry;
	}

	return result;
}

STATUS readOptionFile(const char * fileName, const char * sectionName, OptionFileData ** data, int crcRequired)
{
	enum 	{ LineSize = 256 };
	char	line[LineSize];
	STATUS	result = OK;

	FILE * fp = fopen(fileName, "r");
	int	lineNumber = 0;
	int	foundSection = 0;
	if ( !fp )
	{
		perror(fileName);
		result = ERROR;
	}

	while ( result == OK &&
           fgets(line, LineSize, fp) )
	{
		char * token = line + strspn(line, " \t\r\n");
		token[strcspn(token, "\r\n")] = '\0';

		lineNumber += 1;
		switch ( token[0] )
		{
		case '#':	// comment line
			break;

		case '[':	// section header
			result = processSectionHeader(fileName, lineNumber, &token[1], sectionName, &foundSection);
			break;

		default:		// keyword
			if ( foundSection && token[0] )
			{
				result = processKeyword(fileName, lineNumber, token, data, crcRequired);
			}
			break;
		}
	}

	if ( fp )
	{
		fclose(fp);
	}

	return result;
}

STATUS getOption(const char * optionName, const char ** optionSetting, OptionFileData * data)
{
	OptionFileData * entry = data;

	*optionSetting = NULL;
	while ( entry && !*optionSetting )
	{
		if ( strcmp(entry->name, optionName) == 0 )
		{
			*optionSetting = entry->setting;
		}

		entry = entry->next;
	}

	return (*optionSetting) ? OK : ERROR;
}

