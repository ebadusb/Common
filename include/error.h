/*
 * Copyright (C) 2002 Gambro BCT, Inc.  All rights reserved.
 *
 * $Header$
 * $Log: error.h $
 * Revision 1.11  2002/10/29 21:19:03Z  jl11312
 * - change char * to const char * for _FATAL_ERROR arguments
 * Revision 1.10  2002/06/10 21:37:03  ms10234
 * Added default error logging levels
 * Revision 1.9  2002/06/07 21:42:04  jl11312
 * - added routine for chaining to fatal error handler
 * Revision 1.8  2002/03/20 15:16:21  sb07663
 * Removed QNX dependencies and assorted macros
 * Revision 1.7  2001/08/30 17:22:53  rm70006
 * IT 5252.
 * Change VIP logging to not use display option.
 * Revision 1.6  2001/07/25 20:46:35  rm70006
 * IT 4766
 * Added error logging codes for VIP interface.
 * Revision 1.5  2000/05/05 21:41:02  BS04481
 * New _log_error function for drivers
 * Revision 1.4  2000/05/03 16:27:30  BD10648
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
 * TITLE:      error.h, common fatal error handling routines
 *
 * 				Note that the prototypes for these routines are required in the
 *					common area, since common code uses these functions.  The
 *					implementations for these functions are platform-specific and
 *					are within the platform build area, defined as part of the
 *					vxWorks operating system image.
 */

#ifndef _ERROR_INCLUDE
#define _ERROR_INCLUDE

#ifdef __cplusplus
extern "C" {
#endif

/*
 * SPECIFICATION:    _FATAL_ERROR
 *                      logs error and terminates program
 *                   Parameters:
 *                   file - file name string, typically __FILE__
 *                   line - file line number, __LINE__
 *                   eString - error string
 *                   logged message is prefixed with "FATAL "
 *
 * ERROR HANDLING:   none.
 */
void
_FATAL_ERROR(const char * file, int line, const char * eString);

/*
 * SPECIFICATION:		CHAIN_FATAL_ERROR_HANDLER
 *								Specifies a function to be called for the current task
 *								after _FATAL_ERROR has completed any common fatal error
 *								handling, and can be used to implement task specific
 *								error handling requirements.
 *
 * ERROR HANDLING:   none.
 */
typedef void FatalErrorHandler(void * arg);

void
_CHAIN_FATAL_ERROR_HANDLER(FatalErrorHandler * func, void * arg);

#ifdef __cplusplus
};
#endif

#endif /* ifndef _ERROR_INCLUDE */


