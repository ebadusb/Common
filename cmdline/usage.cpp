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


// FUNCTION: Usage
// PURPOSE: constructs a Usage instance, comment should describes basic purpose
// of the application.
Usage::Usage(char* programName, char* comment)
: _full_usage(0), _usage_text(0), _comment(comment), _programName(programName)
{
    ASSERT(comment);
    _usage_text = new char[1]; // add_usage() expects _usage_text
    *_usage_text = 0;
//    LOG_DEBUG("Usage object constructed");
}


Usage::~Usage()
{
    delete [] _usage_text;
    _usage_text = 0;
    delete [] _full_usage;
    _full_usage = 0;
    _comment = 0;
}


// FUNCTION:  add_usage
// PURPOSE:  add to usage text
void Usage::add_usage(char* usage)
{
//    LOG_DEBUG("add_usage: %s", usage);
    char* newusage = new char[strlen(_usage_text)+strlen(usage)+10];
    sprintf(newusage,"%s\n  %s", _usage_text, usage);
    delete [] _usage_text;
    _usage_text = newusage;
}


// FUNCTION:  get_usage
// PURPOSE: build and return the full usage text
const char* Usage::get_usage()
{
    delete [] _full_usage;

    _full_usage = new char[2*strlen(_programName) + 30  
                           + strlen(_comment) + strlen(_usage_text)];
    sprintf(_full_usage, "%s - %s\n%s [options]\nOptions:%s", 
            _programName, _comment, 
            _programName, _usage_text);

    return _full_usage;
}
