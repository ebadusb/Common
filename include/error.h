/*
 * Copyright (c) 1996 by Cobe BCT, Inc.  All rights reserved.
 *
 * $Header: M:/BCT_Development/TrimaVx/Common/include/rcs/error.h 1.7 2001/08/30 17:22:53 rm70006 Exp sb07663 $
 * $Log: error.h $
 * Revision 1.3  1999/08/31 17:49:59  BS04481
 * Change _log_error to not display to screen unless the env var
 * DISPLAYSTATUS=DISPLAY.
 * Revision 1.2  1999/08/06 14:33:41  BS04481
 * New logging function to add entry to log but not display on the 
 * screen
 * Revision 1.1  1999/05/24 23:26:30  TD10216
 * Initial revision
 * Revision 1.10  1998/09/30 20:38:43  TD07711
 *   set_log_va_variables() fixed to pass caller's TRACE_CODE info 
 *   correctly to tracelog.
 * Revision 1.9  1998/09/23 22:40:35  TD07711
 *   VA_ERROR was calling log_error_set_va_variables() which is not defined,
 *   changed to call log_set_va_variables().
 * Revision 1.8  1998/09/18 21:18:46  TD07711
 *   add VA_ERROR, VA_INFO, and VA_FATAL macros to support printf
 *   style arg list.
 * Revision 1.7  1998/05/12 19:42:17  bs04481
 * Change all fatal error calls from the low-level drivers to the
 * fatal_error_drv function which does not exit.  The router  shutdown
 * sequence will cause the driver to exit in its shutdown sequence.
 * Revision 1.6  1997/11/21 00:44:00  TD07711
 *   1. _FATAL_ERROR and _LOG_ERROR functions prototypes - changed
 *      TRACE_CODE arg from int to trace_code_t to correspond to usage (and
 *      avoid lint errors).
 *   2. added ASSERT support
 * Revision 1.5  1996/11/25 20:17:01  SS03309
 * Spectra Mods
 * Revision 1.4  1996/07/24 19:49:38  SS03309
 * fix MKS
 * Revision 1.3  1996/06/27 16:34:02  SS03309
 * Added comments
 *
 * TITLE:      error.h, Focussed System standard error processing.
 *
 * ABSTRACT:   Defines two error handling routines:
 *             FATAL_ERROR - logs error and terminates program via SIGHUP.
 *             LOG_ERROR - logs error, allows program to continue
 *
 * DOCUMENTS
 * Requirements:
 * Test:
 */

#ifndef ERROR_HPP
#define ERROR_HPP


// define TRACE_CODE before including error.h otherwise otherwise it
// will default to TRACE_TEST1.  You can define it in your makefile as
// a compile flag i.e. -DTRACE_CODE=whatever or define it in a heder
// file that you include prior to including error.h.
#ifndef TRACE_CODE
#define TRACE_CODE  TRACE_TEST1
#endif


// typedefs

enum TRACE_CODES
{
   TRACE_PGATE =       (910<<12 | 1),     // pearly gate
   TRACE_ROUTER =      (910<<12 | 2),     // router
   TRACE_GATEWAY =     (910<<12 | 3),     // gateway
   TRACE_DISPATCHER =  (910<<12 | 4),     // dispatcher
   TRACE_PHDISPAT =    (910<<12 | 5),     // photon dispatcher
   TRACE_STARTER =     (910<<12 | 6),     // starter
   TRACE_PROC =        (910<<12 | 7),     // procedure
   TRACE_ULTRASONICS = (910<<12 | 8),     // ultrasonics
   TRACE_GUI =         (910<<12 | 9),     // gui
   TRACE_SAFETY =      (910<<12 | 10),    // safety
   TRACE_SAFE_DRV =    (910<<12 | 11),    // safety driver
   TRACE_CTL_DRV =     (910<<12 | 12),    // control driver
   TRACE_DISKCRC =     (910<<12 | 13),    // disk crc
   TRACE_TEST1 =       (910<<12 | 14),    // test modes
   TRACE_TEST2 =       (910<<12 | 15),
   TRACE_TEST3 =       (910<<12 | 16),
   TRACE_TEST4 =       (910<<12 | 17),
   TRACE_TEST5 =       (910<<12 | 18),
   TRACE_SPECTRA1 =    (910<<12 | 19),    // Spectra ProcII
   TRACE_SPECTRA2 =    (910<<12 | 20),
   TRACE_SPECTRA3 =    (910<<12 | 21),
   TRACE_SPECTRA4 =    (910<<12 | 22),
   TRACE_SPECTRA5 =    (910<<12 | 23)
};
typedef enum TRACE_CODES trace_codes_t;

#ifdef __cplusplus
extern "C" {
#endif


// SPECIFICATION:    FATAL_ERROR
//                      logs error and terminates program
//                   Parameters:
//                   file - file name string, typically __FILE__
//                   line - file line number, __LINE__
//                   code - trace code from above
//                   usercode - user data
//                   eString - error string
//                   logged message is prefixed with "FATAL "
//
// ERROR HANDLING:   none.
void
_FATAL_ERROR( char* file, int line, trace_codes_t code, int usercode, char* eString);


// SPECIFICATION:    FATAL_ERROR
//                      logs error and terminates program
//                   Parameters:
//                   file - file name string, typically __FILE__
//                   line - file line number, __LINE__
//                   code - trace code from above
//                   usercode - user data
//                   eString - error string
//                   Differs from normal fatal error call in that it does
//                   not exit through this function.  This is done in order
//                   to keep the hard watchdogs happy until the router is
//                   shutdown.
//                   logged message is prefixed with "FATAL "
//
// ERROR HANDLING:   none.
void
_FATAL_ERROR_DRV( char* file, int line, trace_codes_t code, int usercode, char* eString);


// SPECIFICATION:    LOG_ERROR_WITH_DISPLAY
//                      logs error and displays to the screen
//                   Parameters:
//                   file - file name string, typically __FILE__
//                   line - file line number, __LINE__
//                   code - trace code from above
//                   usercode - user data
//                   eString - error string
//
// ERROR HANDLING:   none.
void
_LOG_ERROR_WITH_DISPLAY( char* file, int line, trace_codes_t code, int usercode, char* eString);

// SPECIFICATION:    LOG_ERROR
//                      logs error but does not display to screen
//                   Parameters:
//                   file - file name string, typically __FILE__
//                   line - file line number, __LINE__
//                   code - trace code from above
//                   usercode - user data
//                   eString - error string
//
// ERROR HANDLING:   none.
void
_LOG_ERROR( char* file, int line, trace_codes_t code, int usercode, char* eString);

enum ASSERT_FLAGS
{
   FATAL_FLAG,
   LOG_FLAG
};
typedef enum ASSERT_FLAGS assert_flag_t;


// SPECIFICATION:  assert_failure()
//   handles assertion failure via _LOG_ERROR() or _FATAL_ERROR()
// ERROR HANDLING: none
void assert_failure(char* expr, char* file, int line, assert_flag_t flag, trace_codes_t code);


// SPECIFICATION:  DEBUG_ASSERT
//   ASSERT if USE_DEBUG_ASSERT defined
//   used for enabling expensive asserts in debug mode
// ERROR HANDLING:  calls assert_failure() to fatal error
#ifdef USE_DEBUG_ASSERT
#define DEBUG_ASSERT(expr)      ASSERT(expr)
#define DEBUG_LOG_ASSERT(expr)  LOG_ASSERT(expr)
#else
#define DEBUG_ASSERT(expr)      ((void)0)
#define DEBUG_LOG_ASSERT(expr)  ((void)0)
#endif


// SPECIFICATION:  ASSERT
//   if expr is false, do failure handling
// ERROR HANDLING:  calls assert_failure() to fatal error
#define ASSERT(expr)  ((expr)?(void)0:assert_failure(#expr, __FILE__, __LINE__, FATAL_FLAG, TRACE_CODE))


// SPECIFICATION:  LOG_ASSERT
//   if expr is false, do failure handling
// ERROR HANDLING:  calls assert_failure() to log error
#define LOG_ASSERT(expr)  ((expr)?(void)0:assert_failure(#expr, __FILE__, __LINE__, LOG_FLAG, TRACE_CODE))


// FUNCTION: VA_ERROR
// PURPOSE: macro that provides filename and line_number and variable printf style argument list
//   to the log_error() function.
// ARGUMENTS:
//   char* - printf format string
//   ... - variable number of things to be formatted in the error msg
// RETURNS: void
// USAGE:
//   Example: VA_ERROR("current errno=%d", errno);
//   Example: VA_ERROR("%s: ratio=%f  rpm=%d", function_name, ratio, rpm);
//   1. trailing linefeed is provided.
//   2. max formatted message size is ERROR_SIZE defined in error.c
//   3. error msg is prefixed with "ERROR "
#define VA_ERROR   log_set_va_variables(__FILE__, __LINE__, TRACE_CODE), log_error


// FUNCTION: VA_INFO - like VA_ERROR, only does not prefix msg with "ERROR "
#define VA_INFO   log_set_va_variables(__FILE__, __LINE__, TRACE_CODE), log_info


// FUNCTION: VA_FATAL - like VA_ERROR, except prefixes message with "FATAL " and calls
// fatal error handling to shutdown system.
#define VA_FATAL   log_set_va_variables(__FILE__, __LINE__, TRACE_CODE), fatal_error


// FUNCTION: log_set_va_variables
// PURPOSE: saves filename and linenumber for use in the log msg.
// USAGE: called by the LOG_ERROR and LOG_INFO macro.
void log_set_va_variables(char* filename, int line, trace_codes_t domain);


// FUNCTION: log_error
// PURPOSE: formats an error message and passes it to _LOG_ERROR
// USAGE: called by the LOG_ERROR macro.
void log_error(char* format, ...);


// FUNCTION: log_info
// PURPOSE: formats an informative message and passes it to _LOG_ERROR
// USAGE: called by the LOG_INFO macro.
void log_info(char* format, ...);


// FUNCTION: fatal_error
// PURPOSE: formats a fatal error message and passes it to _FATAL_ERROR
// USAGE: called by the FATAL_ERROR macro.
void fatal_error(char* format, ...);

// shortcut macros for specific domains
#define PROC_SLOGERROR(x)    _LOG_ERROR( __FILE__, __LINE__, TRACE_PROC, 0, x )
#define PROC_ULOGERROR(u,x)  _LOG_ERROR( __FILE__, __LINE__, TRACE_PROC, u, x )


#ifdef __cplusplus
};
#endif

#endif


