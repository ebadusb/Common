/*
 * Copyright (c) 1996 by Cobe BCT, Inc.  All rights reserved.
 *
 * $Header: K:/BCT_Development/Common/router/rcs/error.c 1.7 2001/05/11 19:56:17 jl11312 Exp jl11312 $
 * $Log: error.c $
 * Revision 1.6  2000/05/05 21:41:29  BS04481
 * New _log_error function for drivers
 * Revision 1.5  2000/05/03 16:28:35  BD10648
 * Added Process ID to LOG_ERROR string.
 * Revision 1.4  1999/09/29 18:07:55  TD10216
 * IT4333 - changes for microsoft compiler
 * Revision 1.3  1999/08/31 17:50:03  BS04481
 * Change _log_error to not display to screen unless the env var
 * DISPLAYSTATUS=DISPLAY.
 * Revision 1.2  1999/08/06 14:33:45  BS04481
 * New logging function to add entry to log but not display on the 
 * screen
 * Revision 1.1  1999/05/24 23:29:33  TD10216
 * Initial revision
 * Revision 1.19  1998/10/23 21:04:22  bs04481
 * Lower the priority of the drivers after they call fatal error.  This
 * allows the router time to get a hold of the drivers and kill them.
 * Revision 1.18  1998/09/30 20:40:05  TD07711
 *   set_log_va_variables() fixed to pass caller's TRACE_CODE info correctly 
 *   to the tracelog.
 * Revision 1.17  1998/09/22 18:41:20  bs04481
 * Remove Photon slay from fatal error function because it is done
 * earlier in router shutdown.
 * Revision 1.16  1998/09/18 21:27:41  TD07711
 * 1.  add support functions for VA_ERROR, VA_INFO, and VA_FATAL macros
 * 2. prefix _FATAL_ERROR msgs with "FATAL "
 * 3. prefix VA_ERROR msgs with "ERROR "
 * 4. provide common fatal_handling() for _FATAL_ERROR
 *     and _FATAL_ERROR_DRV
 * Revision 1.15  1998/07/14 18:02:42  bs04481
 * Improve logging when shutting down
 * Revision 1.14  1998/05/12 19:42:18  bs04481
 * Change all fatal error calls from the low-level drivers to the
 * fatal_error_drv function which does not exit.  The router  shutdown
 * sequence will cause the driver to exit in its shutdown sequence.
 * Revision 1.13  1998/02/04 21:26:30  bs04481
 * Remove SIGKILL of pfsave in Fatal Error function as it was moved
 * to router shutdown instead.
 * Make slay of Photon dependent on the Photon version.
 * Revision 1.12  1997/12/02 16:57:53  SB07663
 * SIGKILL pfsave task to prevent PFR in case of a fatal error
 * Revision 1.11  1997/12/02 15:43:58  bs04481
 * Removed delay in fatal error functions which will cause the
 * hardware watchdog to go off before the logs are flushed
 * Revision 1.10  1997/11/21 00:40:26  TD07711
 *   1. add assert_failure()
 *   2. call abort() from _FATAL_ERROR to get process dump
 * Revision 1.9  1997/05/21 17:57:31  bs04481
 * Changed fatal error message to match what I told the tech writer.
 * Revision 1.8  1997/03/31 16:56:59  SS03309
 * change photon to Photon
 * Revision 1.7  1997/03/27 15:00:07  SS03309
 * Added slay router and error message to FATAL_ERROR.
 * Revision 1.6  1996/07/24 19:49:37  SS03309
 * fix MKS
 * Revision 1.5  1996/07/19 14:22:43  SS03309
 * lint
 *
 * TITLE:      Focussed System standard error processing.
 *
 * ABSTRACT:   Defines two error handling routines:
 *             FATAL_ERROR - logs error and terminates program via SIGHUP.
 *             LOG_ERROR - logs error, allows program to continue
 *
 * DOCUMENTS
 * Requirements:
 * Test:
 *
 * PUBLIC FUNCTIONS:
 *  FATAL_ERROR()
 *  LOG_ERROR()
 */

// #includes
#include <errno.h>
#include <malloc.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/trace.h>
#include <sys/tracecod.h>
#include <sys/types.h>
#include <sys/psinfo.h>
#include <sys/sched.h>
#include <unistd.h>
#include <stdarg.h>

#include "error.h"

// NOTE - don't change ERROR_SIZE without also updating associated sprintf format strings.
// for example:  sprintf(buf, "%.270s", errmsg);  the 270 may need to change.
#define ERROR_SIZE 300        // error buffer length

static char* Filename = "";
static int Line = 0;
static trace_codes_t Domain = TRACE_TEST1;
static char FormattedBuf[ERROR_SIZE] = {'E', 'R', 'R', 'O', 'R', ' '};
static int displayStatusKnown = 0;
static char* displayStatus;
static char displayDefault[10] = "NODISPLAY";
// used to format varargs info, ERROR prefix only used by log_error()


// FUNCTION: fatal_handling
// PURPOSE: common handling called by _FATAL_ERROR and _FATAL_ERROR_DRV
// 1. logs FATAL msg to tracelog
// 2. logs fatal error info to console
// 3. shutsdown router and Photon
// RETURNS: void
static void
fatal_handling(char* file, int line, trace_codes_t code, int usercode, char* eString)
{

    static char rev[] = "$ProjectRevision: 5.33 $";     // rev code
    static char buf[ERROR_SIZE]; // static to avoid stack overflow
    static struct _psinfo psdata; // this big struct is static to avoid stack overflow
    char *name;
    pid_t id;

    sprintf(buf, "FATAL %.290s", eString);
    _LOG_ERROR_WITH_DISPLAY( file, line, code, usercode, buf);
    printf("\nBuild %s. \nAn internal software error has occured.\n\n", rev);
    printf("Wait 1 minute then turn off power.  Wait 5 seconds,\n");
    printf("and turn power back on. Follow the disconnect procedure.\n\n");
    printf("Call service.  Thank you. \n ");

    // shutdown router and Photon
    // NOTE: we don't use system("slay ...") since that requires new process, disk I/O, etc.
    for (id = 1;  (id = qnx_psinfo(0, id, &psdata, 0, 0)) != -1; id++)
    {
        // MID and VID entries are not applicable
        if (psdata.flags & (_PPF_MID | _PPF_VID)) {
            continue;
        }

       // NOTE: except for the "idle" entry, proc.name is a full pathname and always contains /
       name = strrchr(psdata.un.proc.name,'/');
       if ( name != NULL ) {
          name++; // inc past '/'

          if ( (strcmp(name,"router")) == 0 ) {
             kill(id, SIGHUP);
          }
       } // end if
    } // end for
} // end fatal_handling


// SPECIFICATION:    Logs to trace buffer and terminates.  The printf()
//                   statement facilitates debugging.
//                   Parameters:
//                   line - source code line number
//                   error - error string
//
// ERROR HANDLING:   Terminates program by generating signal to self, causing
//                   graceful exit and closing of queues.
void
_FATAL_ERROR( char* file, int line, trace_codes_t code, int usercode, char* eString)
{
   fatal_handling(file, line, code, usercode, eString);
   abort(); // generates dump file if dumper is running and exits non zero
   exit(1); // should never get here
}


// SPECIFICATION:    Logs to trace buffer and terminates.  The printf()
//                   statement facilitates debugging.
//                   Parameters:
//                   line - source code line number
//                   error - error string
//                   Differs from normal fatal error call in that it does
//                   not exit through this function.  This is done in order
//                   to keep the hard watchdogs happy until the router is
//                   shutdown.
//
// ERROR HANDLING:   Terminates program by generating signal to self, causing
//                   graceful exit and closing of queues.
void
_FATAL_ERROR_DRV( char* file, int line, trace_codes_t code, int usercode, char* eString)
{
    fatal_handling(file, line, code, usercode, eString);
    // lower the priority of the driver as it is dying
    qnx_scheduler(0,0,-1,10,0);
}


void DoLog( char* file, int line, trace_codes_t code, int usercode, char* eString )
{
   static char eBuff[ERROR_SIZE];                             // fixed length error buffer

   sprintf(eBuff, "pid(%d) %.25s %.250s", getpid(), file, eString);
   eBuff[ERROR_SIZE-1] = 0;

   // send to trace log
   Trace3b( code, _TRACE_SEVERE, line, errno, usercode, strlen( eBuff) + 1, eBuff);
}

// SPECIFICATION:    Logs to trace buffer.  Display on screen
//                   Parameters:
//                   line - source code line number
//                   error - error string
// NOTE: this function is really a misnomer.  It logs any string to tracelog, not just errors.
//
// ERROR HANDLING:   none.
void
_LOG_ERROR_WITH_DISPLAY( char* file, int line, trace_codes_t code, int usercode, char* eString)
{
    DoLog( file, line, code, usercode, eString );

    // display on console for debug
    printf("%s @ %d, errno=%d, usercode=%d, %s\n", file, line, errno, usercode, eString);
}




// SPECIFICATION:    Logs to trace buffer.  No display to screen.
//                   Parameters:
//                   line - source code line number
//                   error - error string
// NOTE: this function is really a misnomer.  It logs any string to tracelog, not just errors.
//
// ERROR HANDLING:   none.
void
_LOG_ERROR( char* file, int line, trace_codes_t code, int usercode, char* eString)
{
    DoLog( file, line, code, usercode, eString );

   if (displayStatusKnown == 0)
   {
      displayStatus = getenv( "DISPLAYSTATUS" );
      if (displayStatus == NULL)
         displayStatus = (char *) &displayDefault;
      displayStatusKnown = 1;
   }
   else
   {
      if (strcmp(displayStatus,"DISPLAY") == 0)
         // display on console for debug
         printf("%s @ %d, errno=%d, usercode=%d, %s\n", file, line, errno, usercode, eString);
   }

}


// SPECIFICATION:    Logs to trace buffer for drivers.  No display to screen. 
//                   No kernel calls after the first pass (getenv) unless DISPLAY is set
//                   Parameters:
//                   line - source code line number
//                   error - error string
// NOTE: this function is really a misnomer.  It logs any string to tracelog, not just errors.
//
// ERROR HANDLING:   none.
void
_LOG_ERROR_DRV( char* file, int line, trace_codes_t code, int usercode, char* eString)
{
   static char eBuff[ERROR_SIZE];                             // fixed length error buffer

   sprintf(eBuff, "%.25s %.270s", file, eString);

   // send to trace log
   Trace3b( code, _TRACE_SEVERE, line, errno, usercode, strlen( eBuff) + 1, eBuff);

   if (displayStatusKnown == 0)
   {
      displayStatus = getenv( "DISPLAYSTATUS" );
      if (displayStatus == NULL)
         displayStatus = (char *) &displayDefault;
      displayStatusKnown = 1;
   }
   else
   {
      if (strcmp(displayStatus,"DISPLAY") == 0)
         // display on console for debug
         printf("%s @ %d, errno=%d, usercode=%d, %s\n", file, line, errno, usercode, eString);
   }

}


// SPECIFICATION:  assert_failure()
//   handles assertion failure via _LOG_ERROR() or _FATAL_ERROR()
// ERROR HANDLING: none
void assert_failure(char* expr, char* file, int line, assert_flag_t flag, trace_codes_t code)
{
   static char buf[80]; // use static to avoid using stack space

   sprintf(buf, "ASSERT (%.50s)", expr);
   if (flag == LOG_FLAG) {
       _LOG_ERROR(file, line, code, getpid(), buf);
   }
   else {
       _FATAL_ERROR(file, line, code, getpid(), buf);
   }
}


// FUNCTION: log_set_va_variables
// PURPOSE: saves filename and linenumber for use in the log msg.
// USAGE: called by the VA_ERROR, VA_INFO, and VA_FATAL macros.
void log_set_va_variables(char* filename, int line, trace_codes_t domain)
{
    Filename = filename;
    Line = line;
    Domain = domain;
}


// FUNCTION: log_error
// PURPOSE: formats an error message and passes it to _LOG_ERROR
// USAGE: called by the VA_ERROR macro.
void log_error(char* format, ...)
{
    va_list arglist;

    va_start(arglist, format);
    vsprintf(FormattedBuf+6, format, arglist); // skip "ERROR " prefix
    va_end(arglist);

    _LOG_ERROR(Filename, Line, Domain, getpid(), FormattedBuf);
}


// FUNCTION: log_info
// PURPOSE: formats an informative message and passes it to _LOG_ERROR
// USAGE: called by the VA_INFO macro.
void log_info(char* format, ...)
{
    va_list arglist;

    va_start(arglist, format);
    vsprintf(FormattedBuf+6, format, arglist); // skips "ERROR " prefix
    va_end(arglist);

    // don't log ERROR prefix in FormattedBuf
    _LOG_ERROR(Filename, Line, Domain, getpid(), FormattedBuf+6);
}


// FUNCTION: fatal_error
// PURPOSE: formats a fatal error message and passes it to _FATAL_ERROR
// USAGE: called by the VA_FATAL macro.
void fatal_error(char* format, ...)
{
    va_list arglist;

    va_start(arglist, format);
    vsprintf(FormattedBuf+6, format, arglist);
    va_end(arglist);

    // don't log ERROR prefix in FormattedBuf
    // _FATAL_ERROR will add a "FATAL " prefix
    _FATAL_ERROR(Filename, Line, Domain, getpid(), FormattedBuf+6);
}

