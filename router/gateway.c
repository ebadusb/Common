/*
 * Copyright (c) 1995, 1996 by Cobe BCT, Inc.  All rights reserved.
 *
 * $Header: K:/BCT_Development/Common/router/rcs/gateway.c 1.5 2001/05/24 22:41:38 jl11312 Exp jl11312 $
 * $Log: gateway.c $
 * Revision 1.5  2001/05/24 22:41:38  jl11312
 * - initialize msg buffer before Receive() to correct operation of sin ver command (IT 5135)
 * Revision 1.4  2001/05/11 19:56:38  jl11312
 * - removed "COBE" name from fatal error message
 * Revision 1.3  2000/07/07 20:53:36  bs04481
 * Bump priorities up 1
 * Revision 1.2  1999/08/31 17:51:01  BS04481
 * Change to use new _log_error_with_display in fatalerror function
 * Revision 1.1  1999/05/24 23:29:37  TD10216
 * Initial revision
 * Revision 1.6  1998/09/23 18:27:56  bs04481
 * Add "Call Cobe etc" message to fatal error
 * Revision 1.5  1998/07/14 18:02:41  bs04481
 * Improve logging when shutting down
 * Revision 1.4  1998/07/10 23:36:10  bs04481
 * Add error type when logging mqueue failures
 * Revision 1.3  1996/09/27 17:43:45  SS03309
 * Indirection problem with Watcom 10.6 line 264
 * Revision 1.2  1996/07/24 19:49:48  SS03309
 * fix MKS
 * Revision 1.2  1996/07/22 14:43:18  SS03309
 * Revision 1.17  1996/07/19 14:39:51  SS03309
 * lint
 *
 * TITLE:      Focussed System message gateway.
 *
 * ABSTRACT:   The gateways send messages from one node to the other.
 * The local router will send messages to the gateway,
 * which will then send the message to the other router.
 * The gateway will set the PID field to zero, and update
 * the message CRC, before sending the message to the
 * other router.  This will tell the other router not
 * to send it back.  The following signals casuse the gateway to
 * gracefully (closing resources) terminate:
 *    SIGHUP
 *    SIGINT
 *    SIGQUIT
 *    SIGTERM
 *    SIGPWR
 *
 * DOCUMENTS
 * Requirements:     I:\ieee1498\SSS2.DOC
 * Test:             I:\ieee1498\STD2.DOC
 *
 * PUBLIC FUNCTIONS:
 * main() - main entry point
 */

// #includes

#include <env.h>
#include <errno.h>
#include <mqueue.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/kernel.h>
#include <sys/name.h>
#include <sys/osinfo.h>
#include <sys/proxy.h>
#include <sys/sched.h>
#include <sys/types.h>
#include <sys/vc.h>
#include <time.h>
#include <unistd.h>

#include "crc.h"
#include "error.h"
#include "hal_notify.h"
#include "msghdr.h"
#include "sinver.h"

#define Q_LENGTH 80           // queue name length
#define MAX_MSGS 64           // max msgs in queue
#define ERROR_SIZE 300        // error buffer length

static volatile unsigned char taskRunning=1; // task running
static mqd_t    mq=-1;                       // router input q
static mqd_t    gq;                          // gateway queue
static char     gatewayQueueName[Q_LENGTH];  // gateway q name

// forward ref
static void openGatewayQueue(  char* qname,
                              mqd_t* mq,
                              pid_t* qproxy,
                              struct sigevent* qnotify);
static void processGatewayQueue( mqd_t gq, mqd_t *mq, struct sigevent* qnotify);


// SPECIFICATION:    signal handler, causes program to stop, called by QNX
//                   Parameters:
//                   signum - signal number, see signal.h
//
// ERROR HANDLING:   none.

void signalHandler( int signum)
{
   signum = signum;              // remove Compiler warning
   taskRunning = 0;
}

// SPECIFICATION:    signal handler, causes program to stop, called by QNX
//                   Parameter:
//                   signum - signal number being processed, see signal.h
//
// ERROR HANDLING:   none

static
void fatalError( int line, int code, char* err)
{
   static char rev[] = "$ProjectRevision: 5.33 $";     // rev code
   static char buf[ERROR_SIZE]; // static to avoid stack overflow
   
   //
   // Let HAL know about problem in case we can't get the information to the log
   //
   hal_notify_fatal(line, code, __FILE__);

   mq_close( mq);                                  // close remote router queue
   mq_close( gq);                                  // close gateway queue
   mq_unlink( gatewayQueueName);                   // remove it
   sprintf(buf, "FATAL %.290s", err);
   _LOG_ERROR_WITH_DISPLAY( __FILE__, line, TRACE_GATEWAY, code, buf);
   printf("\nBuild %s. \nAn internal software error has occured.\n\n", rev);
   printf("Wait 1 minute then turn off power.  Wait 5 seconds,\n"); 
   printf("and turn power back on. Follow the disconnect procedure.\n\n");
   printf("Call service.  Thank you. \n ");
   kill( getppid(), SIGHUP);                       // kill router
}


// SPECIFICATION:    main entry point
//                   argc and argv are used to access command line parameters
//                   parameter 1 = local queue name
//                             2 = remote node number
//                             3 = remote queue name
//
// ERROR HANDLING:   Calls fatalError().

void
main(int argc, char** argv)
{
   pid_t          pid;                    // msg received from pid
   char           msg[BSIZE];             // receive message buffer
   time_t         lastConnect=0;          // last connect time
   struct sigevent qnotify;               // q notify
   pid_t          qproxy;                 // q proxy
   struct sched_param param;

// set priority and scheduling method as round robin

   setprio( 0, 12);                       // set priority
   sched_getparam( 0, &param);
   sched_setscheduler( 0, SCHED_RR, &param);

// check parameter list

   if (argc < 4)
   {
      fatalError( __LINE__, argc, "Not enough parameters");
   }

// signal handling

   signal( SIGHUP, signalHandler);
   signal( SIGINT, signalHandler);
   signal( SIGQUIT, signalHandler);                // used by procedure
   signal( SIGTERM, signalHandler);
   signal( SIGPWR, signalHandler);

// open Q

   strcpy( gatewayQueueName, argv[1]);
   strcat( gatewayQueueName, "Gateway");
   openGatewayQueue(gatewayQueueName, &gq, &qproxy, &qnotify);

// initialize version detection

   sinVerInitialize();

// loop processing messages

   while(taskRunning)
   {

// open remote router input queue

      if ((mq == -1) &&                            // not yet open
          ((time(NULL) - lastConnect) > 10))       // and ten seconds
      {
         int flag;                                 // return flag
         struct _osinfo osdata;                    // info struct
         struct mq_attr attr;                      // message queue attributes
         attr.mq_flags = MQ_NONBLOCK;
         lastConnect = time( NULL);                // reset timer

// try to hook to other node

         flag = qnx_osinfo( atoi( argv[2]), &osdata);
         if (flag != -1)
         {

// set environment variable to other node

            if (setenv( "MQ_NODE", argv[2], 1) != 0)  // set remote node
            {
               fatalError( __LINE__, 0, "setenv()");
            }
            mq = mq_open( argv[3], O_WRONLY, 0, &attr);   // open queue
         }
      }

// wait for messages

      memset(msg, 0xff, sizeof(struct _sysmsg_hdr));
      pid = Receive( 0, msg, sizeof( msg));
      if (pid == -1)
      {
         if (errno == EINTR)                       // signal
         {
            continue;
         }
         fatalError( __LINE__, errno, "Receive()");
      }

// process messages

      if (!sinVerMessage( pid, (SINVERMSG*) msg))
      {

// check for q messages

         if (pid != qproxy)
         {
            Reply( pid, 0, 0);                     // prevent lock-ups
         }
         else
         {
            processGatewayQueue( gq, &mq, &qnotify);
         }
      }
   }

// graceful exit (via signal)

   mq_close( mq);                                  // close remote router queue
   mq_close( gq);                                  // close gateway queue
   mq_unlink( gatewayQueueName);                   // remove it
   kill( getppid(), SIGHUP);                      // kill router
   exit(1);
}


// SPECIFICATION:    Open gateway queue.
//                   Parameters:
//                   *qname - queuue name string
//                   *mqd - message queue descriptor
//                   *qproxy - proxy used for this queue
//                   *qnotify - message queue notify structure
//
// ERROR HANDLING:   Terminates program.

static void openGatewayQueue( char* qname,
                              mqd_t* mqd,
                              pid_t* qproxy,
                              struct sigevent* qnotify)
{
   struct mq_attr attr;             // message queue attributes

// open queue

   attr.mq_maxmsg = MAX_MSGS;       // max number of messages in queue
   attr.mq_msgsize = BSIZE;         // message size
   attr.mq_flags = MQ_NONBLOCK;     // set non-block communication
   *mqd = mq_open( qname, O_RDWR | O_CREAT, 0666, &attr);
   if (*mqd == -1)
   {
      fatalError( __LINE__, 0, "mq_open()");
   }

// create proxy for message queue

   *qproxy = qnx_proxy_attach( 0, 0, 0, -1);
   if (*qproxy == -1)
   {
      fatalError( __LINE__, 0, "qnx_proxy_attach()");
   }

// setup queue notify

   qnotify->sigev_signo = -(*qproxy);
   if (mq_notify( *mqd, qnotify) == -1)
   {
      fatalError( __LINE__, 0, "mq_notify()");
   }
   Trigger( *qproxy);                // avoid race condition by checking queue
}


// SPECIFICATION:    Process gateway queue.
//                   Parameters:
//                   gqd - gateway queue
//                   *mqd - remote router queue descriptor
//                   *qnotify - queue notify structure
//
// ERROR HANDLING:   Terminates program.

static void processGatewayQueue( mqd_t gqd, mqd_t *mqd, struct sigevent* qnotify)
{
   char           msg[BSIZE];             // receive message buffer

// re-arm Q

   if (mq_notify( gqd, qnotify) == -1)
   {
      if (errno!=EBUSY)
      {
         fatalError( __LINE__, errno, "mq_notify()");
      }
   }

// loop getting messages

   while ((mq_receive( gqd, msg, BSIZE, 0) != -1) && (taskRunning))
   {

// place message in other router's input queue

      if (*mqd != -1)
      {

// send via while to process errors

         unsigned short k = 0;                  // signal count

         while((mq_send( *mqd, msg, ((MSGHEADER*)msg)->length, 0) == -1) &&
              (taskRunning))
         {
            if( ((errno == EINTR) || (errno == EAGAIN)) &&
                 (k < 3))                       // signals and network problems
            {
               k++;
               delay (10);                      // short delay
            }
            else
            {
               fatalError( __LINE__, errno, "mq_send()");
            }
         }
      }
   }
}
