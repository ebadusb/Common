/*
 * Copyright (C) 2002 Gambro BCT, Inc.  All rights reserved.
 *
 */


#include "datalog.h"


DataLog_Result datalog_SetTaskErrorHandler(DataLog_TaskID , DataLog_TaskErrorHandler * ) { return DataLog_OK; }

DataLog_Level::DataLog_Level() { }

DataLog_Level::DataLog_Level(const char * levelName) { }

DataLog_Level::~DataLog_Level() { }

DataLog_Stream & DataLog_Level::operator()(const char * fileName, int lineNumber)
{
   return (DataLog_Stream&)(cout << "File: " << fileName << " Line: " << lineNumber << " ");
}

DataLog_Critical::DataLog_Critical(void) { }

DataLog_Critical::~DataLog_Critical() { }

ostream & endmsg(ostream & stream) { return endl( stream ); }

