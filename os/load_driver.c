/*
 * Copyright (C) 2003 Gambro BCT, Inc.  All rights reserved.
 *
 * $Header$
 * $Log$
 *
 */

#include <loadLib.h>
#include <symLib.h>
#include <sysSymTbl.h>
#include "load_driver.h"

void parseDriverCommandLine(const char * driverCmd, char ** driverFileName, char ** initFuncName)
{
	int moduleNameStartIdx = strspn(driverCmd, " \t");
	int moduleNameSize = strcspn(&driverCmd[moduleNameStartIdx], " \t");

	*driverFileName = (char *)malloc(moduleNameSize+1);

	strncpy(*driverFileName, &driverCmd[moduleNameStartIdx], moduleNameSize);
	(*driverFileName)[moduleNameSize] = '\0';

	if ( strlen(*driverFileName) > 0 )
	{
		char * moduleRootStart = strrchr(*driverFileName, '/');
		char * moduleRootEnd = strrchr(*driverFileName, '.');
		int moduleRootLen;
	
		if ( !moduleRootStart ) moduleRootStart = *driverFileName;
		if ( !moduleRootEnd ) moduleRootEnd = *driverFileName + strlen(*driverFileName);
	
		moduleRootLen = moduleRootEnd-moduleRootStart-1;
		*initFuncName = (char *)malloc(moduleRootLen+7);
		strcpy(*initFuncName, "_");
		strncat(*initFuncName, moduleRootStart+1, moduleRootLen);
		strcat(*initFuncName, "_init");
	}
	else
	{
		*initFuncName = NULL;
		fprintf(stderr, "invalid driver command line \"%s\"\n", driverCmd);
	}
}

const char * initializeDriver(const char * driverCmd, const char * driverFileName, const char * initFuncName)
{
	const char * deviceName = NULL;
	int	fd;

	fd = open(driverFileName, O_RDONLY, DEFAULT_FILE_PERM);
	if ( fd >= 0 )
	{
		typedef const char * DriverInitFunc(const char * cmdLine);
		DriverInitFunc * initFunc;
		MODULE_ID id = loadModule(fd, LOAD_GLOBAL_SYMBOLS);
		SYM_TYPE symType;

		close(fd);
		if ( symFindByName(sysSymTbl, (char *)initFuncName, (char **)&initFunc, &symType) == OK )
		{
			deviceName = (*initFunc)(driverCmd);
		}
		else if ( !id )
		{
			fprintf(stderr, "failed to load driver module \"%s\"\n", driverFileName);
			perror(driverCmd);
		}
		else
		{
			fprintf(stderr, "missing initialization function \"%s\" for \"%s\"\n", initFuncName, driverFileName);
	   }
	}

	return deviceName;
}

STATUS loadDriverFromMemory(const char * fileName, char * memStart, size_t memLength)
{
	static tmpFileIdx = 0;
	char	tmpFileName[20];
	int	loadFd;
	STATUS	result = ERROR;

	sprintf(tmpFileName, "/mem/drv%d", tmpFileIdx);
	tmpFileIdx += 1;

	memDevCreate(tmpFileName, memStart, memLength);
	loadFd = open(tmpFileName, O_RDONLY, DEFAULT_FILE_PERM);
	if ( loadFd >= 0 )
	{
		loadModule(loadFd, LOAD_GLOBAL_SYMBOLS);
		close(loadFd);
//		memDevDelete(tmpFileName);
		result = OK;
	}
	else
	{
		fprintf(stderr, "%s: load failed %s %x %x\n", fileName, tmpFileName, (unsigned int)memStart, memLength);
	}

	return result;
}
