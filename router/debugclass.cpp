/*************************************************************************************
 * Copyright (c) 1997 Cobe BCT, Inc.
 *
 * FILENAME: DebugClass.cpp - named DebugClass.cpp to avoid object file naming conflict
 *   with debug.c
 * PURPOSE: implementation of Debug class
 * CHANGELOG:
 * $Header: Z:/BCT_Development/Common/ROUTER/rcs/DEBUGCLASS.CPP 1.2 1999/09/14 16:51:29 TD10216 Exp MS10234 $
 * $Log: DEBUGCLASS.CPP $
 * Revision 1.1  1999/05/24 23:29:29  TD10216
 * Initial revision
 * Revision 1.2  1998/09/30 20:44:46  TD07711
 *   fixed to pass callers TRACE_CODE info correctly to the tracelog.
 * Revision 1.1  1998/09/04 23:03:26  TD07711
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

#include "Debug.hpp"
#include "error.h"


// initialize static data
// default control values
int Debug::Enabled = 1;
int Debug::Use_timestamps = 1;
int Debug::Use_tracelog = 0;
int Debug::Argv_scanned = 0;

// default filename and linenumber
char* Debug::Filename = "";
int Debug::Line = 0;
trace_codes_t Debug::Domain = TRACE_TEST1;

char Debug::Buf[] = {'D', 'E', 'B', 'U', 'G', ' '}; // prefix Buf with "DEBUG "



// FUNCTION:  scan_argv
// PURPOSE: scans command line for debug option and enables debug messages appropriately.
//   Debug options are:
//    -debug - debug msgs to stderr,
//    -debugtime - debug msgs to stderr with timestamps,
//    -debugtrace - debug msgs to tracelog
//   Only first debug option found is processed, i.e. multiple debug options are ignored.
//   First debug option found is stripped from the command line so that subsequent command line
//   processing by the client is not affected.
//   If no debug option is present, debug messages are disabled.
//   If scan_argv() is not called, -debug behavior is presumed.
// ARGUMENTS:
//    int* pArgc - client should pass &argc.  argc will be decremented if debug option is found.
//    char** argv - client should pass argv
// RETURNS: void
// USAGE:
//    Example: Debug::scan_argv(&argc, argv);
//    1. scan_argv should be called once near top of main() prior to normal command line processing.
//    2. calls to LOG_DEBUG prior to calling scan_argv will presume -debug behavior.
//    3. calling scan_argv more than once will result in error msg to stderr and exit
void Debug::scan_argv(int* pArgc, char** argv)
{
    if (Debug::Argv_scanned) {
        fprintf(stderr, "ERROR Debug::scan_argv: called more than once, exiting\n");
        exit(1);
    }
    Debug::Argv_scanned = 1;

    // scan argv for:
    //  -debug - msg to stderr
    //  -debugtime - timestamped msg to stderr
    //  -debugtrace - msg to tracelog
    disable();
    disable_timestamps();
    use_stderr();
    int i,j;
    for (i = 0; i < *pArgc; i++) {

        // end scan early if null terminated argv[]
        if (argv[i] == 0) {
            break;
        }

        // sets flags if -debug found
        if (strcmp("-debug", argv[i]) == 0) {
            enable();
            break;
        }

        // sets flags if -debugtime found
        if (strcmp("-debugtime", argv[i]) == 0) {
            enable();
            enable_timestamps();
            break;
        }

        // sets flags if -debugtrace found
        if (strcmp("-debugtrace", argv[i]) == 0) {
            enable();
            use_tracelog();
            break;
        }

    } // end for


    // remove from argv[] if found
    if (is_enabled()) {
        for (j = i+1; j < *pArgc; i++, j++) {
            argv[i] = argv[j];
        }

        argv[i] = 0;  // null terminate argv
        (*pArgc)--;     // decrement argc
    }
} // end scan_argv()


// FUNCTION: log
// PURPOSE: generates the debug message
// USAGE: called by the LOG_DEBUG macro.
void Debug::log(char* format, ...)
{
    va_list arglist;
    struct timeb timebuf;
    struct tm* pTm;
    char timestamp[20];

    if (Enabled) {
        timestamp[0] = 0; // start with empty string
        if (Use_timestamps) {
            ftime(&timebuf);
            pTm = localtime(&timebuf.time);
            sprintf(timestamp, "%02d:%02d:%02d.%03d ", pTm->tm_hour,
                    pTm->tm_min, pTm->tm_sec, timebuf.millitm);
        }

        va_start(arglist, format);
        vsprintf(Buf+6, format, arglist); // +6 skips "DEBUG " prefix
        va_end(arglist);

        if (Use_tracelog) {
            _LOG_ERROR(Filename, Line, Domain, getpid(), Buf);
        } else {
            fprintf(stderr, "DEBUG %s%s@%d: %s\n", timestamp, Filename, Line, Buf+6);
        }
    }
}


// FUNCTION: is_enabled
// PURPOSE: returns true/false indication of whether debug message are enabled.
// RETURNS: non-zero if enabled, 0 if not enabled
int
Debug::is_enabled()
{
    return Enabled;
}


void
Debug::enable()
{
    Enabled = 1;
}

void
Debug::disable()
{
    Enabled = 0;
}

void
Debug::enable_timestamps()
{
    Use_timestamps = 1;
}

void
Debug::disable_timestamps()
{
    Use_timestamps = 0;
}

void
Debug::use_tracelog()
{
    Use_tracelog = 1;
}

void
Debug::use_stderr()
{
    Use_tracelog = 0;
}

