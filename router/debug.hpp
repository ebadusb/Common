/*************************************************************************************
 * Copyright (c) 1997 Cobe BCT, Inc.
 *
 * FILENAME: Debug.hpp
 * PURPOSE: public header for Debug msg control
 * CHANGELOG:
 * $Header: I:/trima_project/source/ROUTER/rcs/Debug.hpp 1.2 1998/09/30 20:43:53 TD07711 Exp $
 * $Log: Debug.hpp $
 * Revision 1.2  1998/09/30 20:43:53  TD07711
 *   LOG_DEBUG fixed to pass callers TRACE_CODE info correctly to
 *   the tracelog.
 * Revision 1.1  1998/09/04 23:03:24  TD07711
 * Initial revision
 *   8/28/98 - dyes - initial verison
 *************************************************************************************/

#ifndef _DEBUG_HEADER
#define _DEBUG_HEADER

#include "error.h" // to get def for trace_codes_t

#define DEBUG_MAX_BUF_SIZE 500 // maximum size of formatted debug msg

// MACRO: LOG_DEBUG
// PURPOSE: macro that provides filename and line_number and variable printf style argument list
//   to the Debug::log() function.
// ARGUMENTS:
//   char* - printf format string
//   ... - variable number of things to be formatted in the debug msg
// RETURNS: void
// USAGE:
//   Example: LOG_DEBUG("current errno=%d", errno);
//   Example: LOG_DEBUG("%s: ratio=%f  rpm=%d", function_name, ratio, rpm);
//   1. trailing linefeed is provided.
//   2. max formatted message size is DEBUG_MAX_BUF_SIZE
#define LOG_DEBUG   Debug::Filename=__FILE__, Debug::Line=__LINE__, Debug::Domain=TRACE_CODE, Debug::log


// CLASS:  Debug
// PURPOSE: module encapsulating debug message control functions.
//   All functions are static.  Creating a Debug instance is not required.  Attempting to do
//   so will result in a compile error.
class Debug
{
    public:

        // FUNCTION: scan_argv
        // PURPOSE: scans command line for debug option and enables debug messages appropriately.
        //   Debug options are:
        //     -debug - debug msgs to stderr,
        //     -debugtime - debug msgs to stderr with timestamps,
        //     -debugtrace - debug msgs to tracelog
        //   Only first debug option found is processed, i.e. multiple debug options are ignored.
        //   First debug option found is stripped from the command line so that subsequent command line
        //   processing by the client is not affected.
        //   If no debug option is present, debug messages are disabled.
        //   If debug_init() is not called, -debug behavior is presumed.
        // USAGE:
        //   Ex: Debug::scan_argv(&argc, argv);
        //   1. scan_argv should be called once near top of main() prior to normal command line processing.
        //   2. calls to LOG_DEBUG prior to calling scan_argv will presume -debug behavior.
        //   3. calling scan_argv more than once results in an error msg to stderr and an exit.
        static void scan_argv(int* pArgc, char** argv);

        static int is_enabled(); // returns non-zero (true) if enabled, zero (false) if not enabled.

        static void log(char* format, ...); // called by LOG_DEBUG, not called directly by client but must
        // be public for use by macro expansion.

        // Following functions are provided for fine grained control, but typicall client will
        // just call scan_argv() once for debug setup.
        static void enable(); // enables debug messages
        static void disable(); // disables debug messages
        static void enable_timestamps(); // debug messages will have timestamp prefix
        static void disable_timestamps(); // debug messages will not have timestamp prefix [Default]
        static void use_tracelog(); // send debug messages to tracelog
        static void use_stderr(); // send debug messages to stderr [Default]

        // Following data is public so LOG_DEBUG macro can fill them in.
        static char* Filename; // filename where LOG_DEBUG is called from
        static int Line; // line number where LOG_DEBUG is called from
        static trace_codes_t Domain; // TRACE_CODES enum of caller

    private:

        // Debug msg control variables
        static int Enabled; // true for debug msg output
        static int Use_timestamps; // true for timestamps
        static int Use_tracelog; // true for tracelog, false for stderr
        static int Argv_scanned; // true if scan_argv has been called

        static char Buf[DEBUG_MAX_BUF_SIZE]; // where the debug msg is formatted

        // following declared private so that client cannot create an instance of this class
        // Do not implement these functions
        Debug(); // default constructor
        ~Debug(); // destructor
        Debug(const Debug&); // copy constructor
        Debug& operator=(const Debug&); // assignment op
};

#endif

