/*
 * Copyright (C) 2002 Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: K:/BCT_Development/vxWorks/Common/datalog/rcs/datalog.cpp 1.1 2002/05/17 14:51:29 jl11312 Exp jl11312 $
 * $Log: datalog.cpp $
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

ologstream & DataLog_Level::operator()(const char * fileName, int lineNumber)
{
	return (ologstream &)cout;
}

DataLog_Critical::DataLog_Critical(void)
					  : DataLog_Level()
{
}

DataLog_Critical::~DataLog_Critical()
{
}

