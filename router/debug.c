/*************************************************************************************
 * Copyright (c) 1997 Cobe BCT, Inc.
 *
 * FILENAME: debug.c
 * PURPOSE: support functions for variable args LOG_DEBUG macro
 * CHANGELOG:
 * $Header: I:/trima_project/source/ROUTER/rcs/DEBUG.C 1.2 1998/09/30 20:42:09 TD07711 Exp $
 * $Log: DEBUG.C $
 * Revision 1.2  1998/09/30 20:42:09  TD07711
 *   log_debug_set_va_variables() fixed to pass caller's TRACE_CODE info
 *   correctly to the tracelog.
 * Revision 1.1  1998/09/04 23:03:21  TD07711
 * Initial revision
 *  8/28/98 - dyes
 *************************************************************************************/


#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/timeb.h>
#include <stdlib.h>

#include "debug.h"
#include "error.h"

static int Debug_enable = 1;
static int Timestamp_enable = 1;
static int Tracelog_enable = 0;

static int Line = 0;
static char* Filename = "";
static trace_codes_t Domain = TRACE_TEST1;
static char Buf[DEBUG_MAX_BUF_SIZE] = {'D', 'E', 'B', 'U', 'G', ' '}; // prefix for tracelog msgs



// FUNCTION:  debug_scan_argv
// PURPOSE: scans command line for debug option and enables debug messages appropriately.
//   Debug options are:
//    -debug - debug msgs to stderr,
//    -debugtime - debug msgs to stderr with timestamps,
//    -debugtrace - debug msgs to tracelog
//   Only first debug option found is processed, i.e. multiple debug options are ignored.
//   First debug option found is stripped from the command line so that subsequent command line
//   processing by the client is not affected.
//   If no debug option is present, debug messages are disabled.
//   If debug_scan_argv is not called, -debug behavior is presumed.
// ARGUMENTS:
//    int* pArgc - client should pass &argc.  argc will be decremented if debug option is found.
//    char** argv - client should pass argv
// RETURNS: void
// USAGE:
//    Ex: debug_scan_argv(&argc, argv);
//    1. debug_scan_argv should be called once near top of main() prior to normal command line processing.
//    2. calls to LOG_DEBUG prior to calling debug_scan_argv will presume -debug behavior.
//    3. debug_scan_argv will fatal out if called more than once.
void debug_scan_argv(int* pArgc, char** argv)
{
    int i,j;
    static int Called_once = 0;

    if (Called_once) {
        fprintf(stderr, "ERROR debug_init called more than once\n");
        exit(1);
    }
    Called_once = 1;

    // scan argv for:
    //  -debug - msg to stderr
    //  -debugtime - timestamped msg to stderr
    //  -debugtrace - msg to tracelog
    Debug_enable = 0;
    Timestamp_enable = 0;
    Tracelog_enable = 0;
    for (i = 0; i < *pArgc; i++) {

        // end scan early if null terminated argv[]
        if (argv[i] == 0) {
            break;
        }

        // sets flags if -debug found
        if (strcmp("-debug", argv[i]) == 0) {
            Debug_enable = 1;
            break;
        }

        // sets flags if -debugtime found
        if (strcmp("-debugtime", argv[i]) == 0) {
            Debug_enable = 1;
            Timestamp_enable = 1;
            break;
        }

        // sets flags if -debugtrace found
        if (strcmp("-debugtrace", argv[i]) == 0) {
            Debug_enable = 1;
            Tracelog_enable = 1;
            break;
        }

    } // end for


    // remove from argv[] if found
    if (Debug_enable) {
        for (j = i+1; j < *pArgc; i++, j++) {
            argv[i] = argv[j];
        }

        argv[i] = 0;  // null terminate argv
        (*pArgc)--;     // decrement argc
    }
} // end debug_init()


// FUNCTION: log_debug_set_va_variables
// PURPOSE: saves filename and linenumber for use in the debug msg.
// USAGE: called by the LOG_DEBUG macro.
void log_debug_set_va_variables(char* filename, int line, trace_codes_t domain)
{
    Line = line;
    Filename = filename;
    Domain = domain;
}


// FUNCTION: log_debug
// PURPOSE: generates the debug message
// USAGE: called by the LOG_DEBUG macro.
void log_debug(char* format, ...)
{
    va_list arglist;
    struct timeb timebuf;
    struct tm* pTm;
    char timestamp[20];

    if (Debug_enable) {
        timestamp[0] = 0; // start with empty string
        if (Timestamp_enable) {
            ftime(&timebuf);
            pTm = localtime(&timebuf.time);
            sprintf(timestamp, "%02d:%02d:%02d.%03d ", pTm->tm_hour,
                    pTm->tm_min, pTm->tm_sec, timebuf.millitm);
        }

        va_start(arglist, format);
        vsprintf(Buf+6, format, arglist);
        va_end(arglist);

        if (Tracelog_enable) {
            _LOG_ERROR(Filename, Line, Domain, getpid(), Buf);
        } else {
            fprintf(stderr, "DEBUG %s%s@%d: %s\n", timestamp, Filename, Line, Buf+6);
        }
    }
}


// FUNCTION: is_debug
// PURPOSE: returns true/false indication of whether debug message are enabled.
// RETURNS: non-zero if enabled, 0 if not enabled
int is_debug()
{
    return Debug_enable;
}

