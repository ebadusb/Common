/* Copyright 1998 Cobe BCT, Inc.
 *
 * FILENAME: Usage.cpp
 * PURPOSE: provides application usage info
 * format of usage info is:
 * Usage:  <application name> - <what the application does>
 * <application name> [options]
 * Options:
 *   <info on each option>
 * CHANGELOG:
 *   11/04/99 - dyes - initial version, add_usage moved from Logger
 */


#include <string.h>
#include "usage.h"
// #include "logger.hpp"

#include "stdio.h"
#include "assert.h"

#define ASSERT assert


Usage::Usage(const char* programName, const char* comment)
: _usage_text(0), _comment(comment), _programName(programName)
{
    // PURPOSE: constructs a Usage instance, comment should describes basic purpose
    // of the application.
    
    ASSERT(programName);
    ASSERT(comment);

    // initialize usage test
    _usage_text = new char[ 2*strlen(programName) + strlen(comment) + 50 ];
    sprintf(_usage_text, "Usage: %s - %s\n%s [options]\nOptions:\n",
            programName, comment, programName);
}


Usage::~Usage()
{
    delete [] _usage_text;
    _usage_text = 0;
    
    _comment = 0;
    _programName = 0;
}


void Usage::add_usage(const char* usage)
{
    // PURPOSE:  add to usage text
 
    char* newusage = new char[ strlen(_usage_text) + strlen(usage) + 10];
    sprintf(newusage,"%s\n  %s", _usage_text, usage);
    delete [] _usage_text;
    _usage_text = newusage;
}

