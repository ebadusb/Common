/*
 * Copyright (C) 2002 Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: //bctquad3/home/BCT_Development/vxWorks/Common/datalog/rcs/datalog.cpp 1.13 2003/12/05 16:33:05Z jl11312 Exp rm70006 $
 * $Log: datalog.cpp $
 * Revision 1.2  2002/05/17 18:28:34  jl11312
 * - temporary changes for debug use under vxWorks
 * Revision 1.1  2002/05/17 14:51:29  jl11312
 * Initial revision
 *
 */

#include "datalog.h"

DataLog_Level::DataLog_Level(void)
{
}

DataLog_Level::DataLog_Level(const char * levelName)
{
}

DataLog_Result DataLog_Level::initialize(const char * levelName)
{
	return DataLog_OK;
}

DataLog_Level::~DataLog_Level()
{
}

ostream & DataLog_Level::operator()(const char * fileName, int lineNumber)
{
	return cout;
}

DataLog_Critical::DataLog_Critical(void)
					  : DataLog_Level()
{
}

DataLog_Critical::~DataLog_Critical()
{
}

DataLog_Result datalog_CreateLevel(const char * levelName, DataLog_Handle * handle)
{
	handle = NULL;
	return DataLog_OK;
}

DataLog_Result	datalog_SetDefaultLevel(DataLog_Handle handle)
{
	return DataLog_OK;
}

DataLog_Result datalog_Print(DataLog_Handle handle, const char * file, int line, const char * format, ...)
{
	cout << file << " " << line << " " << format << endl;
	return DataLog_OK;
}

DataLog_Result datalog_PrintToDefault(const char * file, int line, const char * format, ...)
{
	cout << file << " " << line << " " << format << endl;
	return DataLog_OK;
}

ostream & DataLog_DefaultLevel::getStream(const char * fileName, int lineNumber)
{
	return cout;
}
