/*
 * Copyright (c) 1995, 1996 by Cobe BCT, Inc.  All rights reserved.
 *
 * $Header: Q:/home1/COMMON_PROJECT/Source/ROUTER/rcs/PEARLYG.C 1.2 1999/05/31 20:35:10 BS04481 Exp TD10216 $
 * $Log: pearlyg.c $
 * Revision 1.4  1998/08/21 19:44:11  MS10234
 * Removed tracelog deletion capabilities
 * Revision 1.3  1996/09/24 19:44:35  SS03309
 * priority and other changes for power fail warning
 * Revision 1.2  1996/07/24 19:49:53  SS03309
 * fix MKS
 * Revision 1.2  1996/07/22 14:43:32  SS03309
 * Revision 1.6  1996/07/19 15:23:09  SS03309
 * lint changes
 *
 * TITLE:   Focussed PearlyGate
 *
 * ABSTRACT: This standalone program monitors other tasks in the system and
 * logs abnormal terminations to the trace log.
 *
 * DOCUMENTS:
 * Requirements:     I:\ieee1498\SSS2.DOC
 * Test:             I:\ieee1498\STD2.DOC
 *
 * PUBLIC FUNCTIONS:
 *    main() - main entry point
 */

#include <dirent.h>
#include <errno.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/kernel.h>
#include <sys/psinfo.h>
#include <sys/sched.h>
#include <sys/seginfo.h>
#include <sys/stat.h>
#include <sys/sys_msg.h>
#include <sys/trace.h>
#include <sys/tracecod.h>
#include <sys/types.h>
#include <unistd.h>

#include "error.h"
#include "sinver.h"

// system message structure

typedef struct
{
   struct _sysmsg_hdr hdr;
   struct _sysmsg_death body;
} SYS_MSG;

// constants

#define BUFFER_SIZE 128                   // buffer size
static const int      QNX_ERROR=-1;       // qnx errors

// SPECIFICATION:  Logs to the trace buffer and terminates.  The
//                 printf() statement facilitates system debugging.
//
// ERROR HANDLING: Terminates program.

static void FATAL_ERROR( unsigned short line, int status, char* error)
{
   Trace3b( TRACE_PGATE, _TRACE_SEVERE, 0, line,
      status, strlen( error)+1, error);
   printf( "pearlygate: line%d, status=%d, %s\n", line, status, error);
   exit(1);
}


// SPECIFICATION:  Main program entry point, no command line parameters used
//                 never returns
//
// ERROR HANDLING: calls FATAL_ERROR()

void
main()
{
   long bits;                                // process flags
   pid_t pid;                                // process id
   char  msg[BUFFER_SIZE];                   // message buffer
   struct _sysmsg_hdr_reply replyMsg;        // reply message
   struct _psinfo psdata;                    // process data
   int    status;                            // status code
   short  signal;                            // signal code
   long   exitStatus;                        // exit status
   struct sched_param param;                 // scheduler parameters

// set priority and scheduling method to round robin
   setprio( 0, 11);
   sched_getparam( 0, &param);
   sched_setscheduler( 0, SCHED_RR, &param);

// set system flag to inform on task termination

   bits = _PPF_INFORM;
   if (qnx_pflags( bits, bits, 0, 0) == QNX_ERROR)
   {
      FATAL_ERROR( __LINE__, 0, "qnx_flags()");
   }
   sinVerInitialize();                       // init sin ver processing

// loop forever, processing messages

   while( 1)
   {
// get messages
      pid = Receive( 0, msg, sizeof( msg));
      if (pid == QNX_ERROR)
      {
         FATAL_ERROR( __LINE__, 0, "Receive()");
      }

// check for sin ver message, if not process task termination

      if (!sinVerMessage( pid, (SINVERMSG*) &msg[0]))
      {

         SYS_MSG* s = (SYS_MSG*) &msg[0];

// check for task messages

         if (( s->hdr.type == _SYSMSG) &&
             ( s->hdr.subtype == _SYSMSG_SUBTYPE_DEATH))
         {

// get task name

            status = qnx_psinfo( PROC_PID, pid, &psdata, 0, 0);
            replyMsg.status = EOK;

// reply to proc

            Reply( pid, &replyMsg, sizeof( replyMsg));

// enter trace data into log if abnormal termination

            signal = s->body.signum;
            exitStatus = s->body.xstatus;

            if ((signal != 0) || (exitStatus != 0))
            {
               if( status == QNX_ERROR)         // no name found
               {
                  Trace3b( TRACE_PGATE, _TRACE_SEVERE,
                     pid, signal, exitStatus, 1, "");
               }
               else                             // name in psdata struct
               {
                  Trace3b( TRACE_PGATE, _TRACE_SEVERE,
                     pid, signal, exitStatus,
                     strlen( psdata.un.proc.name) + 1, psdata.un.proc.name);
               }  // end of status check
            }  // end of signal test
         }  // end of message type check
      }  // end of sin ver check
   }  // end of while(1)
}



