/*************************************************************************************
 * Copyright (c) 1997 Cobe BCT, Inc.
 *
 * FILENAME: debug.h
 * PURPOSE: public header for using LOG_DEBUG() to log varargs debug msg to stderr
 * CHANGELOG:
 * $Header: //bctquad3/home/BCT_Development/TrimaVx/Common/include/rcs/debug.h 1.1 1999/05/24 23:26:25 TD10216 Exp $
 * $Log: debug.h $
 * Revision 1.1  1999/05/24 23:26:25  TD10216
 * Initial revision
 * Revision 1.2  1998/09/30 20:42:36  TD07711
 *   log_debug_set_va_variables() fixed to pass caller's TRACE_CODE info 
 *   correctly to the tracelog.
 * Revision 1.1  1998/09/04 23:03:23  TD07711
 * Initial revision
 *   8/28/98 - dyes - initial verison
 *************************************************************************************/

#ifndef _DEBUG_HEADER
#define _DEBUG_HEADER

#include "error.h" /* // to get trace_codes_t */


#ifdef __cplusplus
extern "C" {
#endif

#define DEBUG_MAX_BUF_SIZE 500 /* // maximum size of formatted debug msg */

/*
// FUNCTION: LOG_DEBUG
// PURPOSE: macro that provides filename and line_number and variable printf style argument list
//   to the log_debug() function.
// ARGUMENTS:
//   char* - printf format string
//   ... - variable number of things to be formatted in the debug msg
// RETURNS: void
// USAGE:
//   Example: LOG_DEBUG("current errno=%d", errno);
//   Example: LOG_DEBUG("%s: ratio=%f  rpm=%d", function_name, ratio, rpm);
//   1. trailing linefeed is provided.
//   2. max formatted message size is DEBUG_MAX_BUF_SIZE
*/
#define LOG_DEBUG   log_debug_set_va_variables(__FILE__, __LINE__, TRACE_CODE), log_debug


/*
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
//   If debug_scan_argv() is not called, -debug behavior is presumed.
// ARGUMENTS:
//    int* pArgc - client should pass &argc.  argc will be decremented if debug option is found.
//    char** argv - client should pass argv
// RETURNS: void
// USAGE:
//    Ex: debug_scan_argv(&argc, argv);
//    1. debug_scan_argv should be called once near top of main() prior to normal command line processing.
//    2. calls to LOG_DEBUG prior to calling debug_scan_argv will presume -debug behavior.
//    3. debug_scan_argv will fatal out if called more than once.
*/
void debug_scan_argv(int* pArgc, char** argv);


/*
// FUNCTION: log_debug_set_va_variables
// PURPOSE: saves filename and linenumber for use in the debug msg.
// USAGE: called by the LOG_DEBUG macro.
*/
void log_debug_set_va_variables(char* filename, int line, int domain);


/*
// FUNCTION: log_debug
// PURPOSE: generates the debug message
// USAGE: called by the LOG_DEBUG macro.
*/
void log_debug(char* format, ...);


/*
// FUNCTION: is_debug
// PURPOSE: returns true/false indication of whether debug message are enabled.
// RETURNS: non-zero if enabled, 0 if not enabled
*/
int is_debug();


#ifdef __cplusplus
}
#endif

#endif

