/*
 * Copyright (c) 1995, 1996 by Cobe BCT, Inc.  All rights reserved.
 *
 * $Header: K:/BCT_Development/Common/router/rcs/router.c 1.11 2001/05/11 19:57:01 jl11312 Exp jl11312 $
 * $Log: router.c $
 * Revision 1.4  1999/07/30 20:49:55  TD10216
 * IT4154
 * Revision 1.3  1999/07/14 22:44:21  BS04481
 * Arg!  No memory was allocated for name while searching the 
 * ps_info data.  Found by inspection.
 * Revision 1.2  1999/05/31 20:35:12  BS04481
 * Remove unused MSGHEADER structure from messages. 
 * Decrease maximum message size.  Add new version of 
 * focusBufferMsg and focusInt32Msg that do not bounce the message
 * back to the originator.  All changes to increase free memory.
 * Revision 1.1  1999/05/24 23:29:53  TD10216
 * Initial revision
 * Revision 1.26  1998/09/23 18:27:34  bs04481
 * Add Photon to the kill list when router is shutting down.  Add "Call
 * Cobe etc" message to fatal error
 * Revision 1.25  1998/09/22 18:35:23  bs04481
 * Clean-up commented out code
 * Revision 1.24  1998/08/11 01:16:52  bs04481
 * Shutdown the gateway tasks when shutting down the router
 * under error conditions to avoid repetitive sends from the gateway.
 * Revision 1.23  1998/08/03 20:32:06  TD07711
 *   use mq_check() to log when queue is almost full
 * Revision 1.22  1998/07/15 23:58:10  bs04481
 * Clean-up printfs in logging at shutdown
 * Revision 1.21  1998/07/14 19:54:35  bs04481
 * Clean up declarations of unused variables
 * Revision 1.20  1998/07/14 18:02:40  bs04481
 * Improve logging when shutting down
 * Revision 1.19  1998/07/10 23:36:35  bs04481
 * Changes to improve router performance and enhance error logging.
 * Revision 1.18  1998/06/02 17:59:01  bs04481
 * Changes to enhance logging during fatal errors and soft watchdog
 * shutdown.
 * Revision 1.17  1998/05/12 19:43:11  bs04481
 * During the router shutdown sequence, wait until last to
 * shutdown the drivers.
 * Revision 1.16  1998/03/10 19:39:46  bs04481
 * Code review of the shutdown code indicated the tracelogger was
 * not getting a SIGALRM and the first task in the tasklist was
 * Revision 1.15  1998/02/04 21:24:33  bs04481
 * Change shutdown function to SIGKILL pfsave and saf_exec
 * to prevent data from being saved during SW watchdog or
 * Fatal Errors.
 * Revision 1.14  1997/12/02 15:46:05  bs04481
 * Changed where tracelogger is flushed
 * Revision 1.13  1997/11/21 19:28:48  bs04481
 * Sync to disk after killing the tracelogger
 * Revision 1.12  1997/05/16 21:26:49  bs04481
 * Change to kill the tracelogger with SIGALRM to force a flush.
 * Revision 1.11  1997/05/09 21:52:31  bs04481
 * Remove fatal error log when router is slain.
 * Revision 1.10  1997/04/29 15:34:58  bs04481
 * Add slay of an2inetd and call to fatal error as the router shuts down.
 * Revision 1.9  1996/12/06 18:08:08  SS03309
 * Fix bug when tcp gateway died, would take out router
 * Revision 1.8  1996/12/06 15:47:55  SS03309
 * tcp gateway and spoofer
 * Revision 1.7  1996/11/15 23:35:24  SS03309
 * changes to make spoofer work
 * Revision 1.5  1996/11/14 17:18:24  SS03309
 * added spoofer hooks
 * Revision 1.4  1996/10/08 21:12:37  SS03309
 * tcp/ip gateway support
 * Revision 1.3  1996/09/27 20:47:38  SS03309
 * Watcom 10.6 warnings
 * Revision 1.2  1996/07/24 19:49:59  SS03309
 * fix MKS
 * Revision 1.2  1996/07/22 17:46:13  SS03309
 * lint change fix
 * Revision 1.2  1996/07/22 14:43:40  SS03309
 * Revision 1.24  1996/07/19 16:12:50  SS03309
 * lint changes
 * Revision 1.23  1996/06/26 19:42:25  SS03309
 * message trace features to add time stamp
 *
 * TITLE:      router.c, Focussed System message router.
 *
 * ABSTRACT:   This task routes messages to the various tasks on the local
 * node.  It spawns the gateway task to communicate with the router on the
 * other node.
 *
 * The router accepts messages from its message queue.  The message queue
 * is named based on the task parameters.  Each task communicates with the
 * router by first putting a TASK_REGISTER message into the router's input
 * queue.  This message is then added to a linked list.  Then, each task will
 * place MSG_MULTICAST messages into the router's input queue.  These messages
 * are then distributed to previously reqistered tasks by placing a copy of
 * the message into each task's input queue.  The router gracefully terminates
 * with the following signals:
 *    SIGHUP
 *    SIGINT
 *    SIGQUIT
 *    SIGTERM
 *    SIGPWR
 *
 * The router will also spawn the message spoofer if the
 * environment variable SPOOFER_ARGS is set.
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
#include <process.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/kernel.h>
#include <sys/name.h>
#include <sys/proxy.h>
#include <sys/psinfo.h>
#include <sys/qnx_glob.h>
#include <sys/sched.h>
#include <sys/trace.h>
#include <sys/tracecod.h>
#include <sys/types.h>
#include <sys/vc.h>
#include <unistd.h>

#include "crc.h"
#include "error.h"
#include "msghdr.h"
#include "sinver.h"
#include "mq_check.h"


// #defines, typedefs, enums, structs

#define NAME_LENGTH 80
#define ERROR_LENGTH 132
#define MAX_Q_MSGS  128
#define QNX_ERROR (-1)

typedef enum
{
   INACTIVE = 0,
   ACTIVE = 1,
} TASKSTATUS;

// linked list structure

typedef struct
{
   MSGHEADER      h;                         // message header
   mqd_t          mq;                        // messsage queue
   int            counter;                   // times pid is register for this msgid
   bounce_t       bounce;                    // bounce back to originator of message?
   void*          next;                      // next in chain
} TASKLIST;

typedef struct
{
   MSGHEADER      h;                         // message header
   mqd_t          mq;                        // messsage queue
   void*          next;                      // next in chain
} GATEWAYLIST;

typedef struct
{
   MSGHEADER      h;                         // message header
   mqd_t          mq;                        // messsage queue
   unsigned char  msgs[MAX_MESSAGES];        // registered messages
} SPOOFER;

typedef struct
{
   char           name[NAME_LENGTH];         // task name
   pid_t          pid;                       // pid
   mqd_t          mq;                        // message queue
   int            signal;                    // signal to use on kill
} TASKKILL;

enum KILLEDPROCS { 
	TRACELOG, SAFEDRV, CTLDRV, PFSAVE, SAFEXEC, DATALOG, METER, GUI, 
	PHOTON,EVERESTLOG,
	// leave this be the last entry - used to loop the list
	TASK_KILL_COUNT
};


// local routines

static void distributeMessage( MSGHEADER* msg);
static void processGateways( MSGHEADER* msg);
static void openRouterQueue( char*  qname,
                             mqd_t* mq,
                             pid_t* qproxy,
                             struct sigevent* qnotify);
static void messageDeregister( char* msg, TASKSTATUS status);
static void messageRegister( char* msg, bounce_t bounce);
static void shutdown(void);
static int  spoofMessage( MSGHEADER* msg);
static void gatewayRegister( char* msg);
static void spooferRegister( char* msg);
static void taskRegister( char* msg);
static void taskDeregister( char* msg);

// local (static) storage

static GATEWAYLIST* gatewayList = NULL;      // list of (tcp) gateways
static SPOOFER*   spoofer = NULL;            // spoofer
static TASKLIST*  taskQueueList = NULL;      // task PID-to-queue correspondence
static TASKLIST*  messageLookupTable[MAX_MESSAGES];   // entry points to start of message list

static char       qName[NAME_LENGTH];        // router queue name
static mqd_t      mq;                        // router input q
static pid_t      gatewayPID = QNX_ERROR;    // gateway PID
static mqd_t      gatewayQueue = QNX_ERROR;  // gateway Q
static volatile unsigned char taskRunning=1; // task running
static TASKKILL   taskKillTable[TASK_KILL_COUNT];


// SPECIFICATION:    signal handler, causes program to stop, called by QNX
//                   Parameter:
//                   signum - signal number being processed, see signal.h
//
// ERROR HANDLING:   none

void signalHandler( int signum)
{
   signum = signum;                          // remove compilier warning
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
   static char rev[] = "$ProjectRevision: 1.21 $";     // rev code
   
   _LOG_ERROR( __FILE__, line, TRACE_ROUTER, code, err);
   printf("\nBuild %s. \nAn internal software error has occured.\n\n", rev);
   printf("Wait 1 minute then turn off power.  Wait 5 seconds,\n"); 
   printf("and turn power back on. Follow the disconnect procedure.\n\n");
   printf("Call Cobe Service.  Thank you. \n ");
   shutdown();                              // shutdown router
}



// SPECIFICATION:    main entry point to start router
//                   argc and argv are used to access the command line
//                      parameter 1 = local queue name
//                                2 = remote node number
//                                3 = remote queue name
//                   arge is used to access the environment for spawning gateway
//
// ERROR HANDLING:   Calls fatalError().

void
main(int argc, char** argv, char** arge)
{
   pid_t          pid;                       // msg received from pid
   pid_t          qproxy;                    // message queue proxy
   char           msg[BSIZE];                // receive message buffer
   MSGHEADER*     msgHeader;                 // pointer to message header
   struct sigevent qnotify;                  // q notify structure
   struct sched_param param;                 // scheduler parameters
   int            k;                         // loop counter

   // set priority and scheduling method to round robin
   setprio( 0, 11);
   sched_getparam( 0, &param);
   sched_setscheduler( 0, SCHED_RR, &param);

   // check parameter list
   if(argc < 4)
   {
      fatalError(__LINE__, 0, "Not enough parameters");
   }

   // signal handlers
   signal( SIGHUP, signalHandler);
   signal( SIGINT, signalHandler);
   signal( SIGQUIT, signalHandler);          // used by procedure
   signal( SIGTERM, signalHandler);
   signal( SIGPWR, signalHandler);           // shutdown and power fail

   // clear the message lookup table
   for(k=0;k<MAX_MESSAGES;k++)
      messageLookupTable[k] = NULL;

   // setup the task kill table
   for(k=0;k<TASK_KILL_COUNT;k++)
   {
      taskKillTable[k].pid = QNX_ERROR;
      taskKillTable[k].mq = QNX_ERROR;
   }
   strcpy(taskKillTable[SAFEDRV].name,"safe_drv");
   taskKillTable[SAFEDRV].signal = SIGHUP;
   strcpy(taskKillTable[CTLDRV].name,"ctl_drv");
   taskKillTable[CTLDRV].signal = SIGHUP;
   strcpy(taskKillTable[PFSAVE].name,"pfsave");
   taskKillTable[PFSAVE].signal = SIGKILL;
   strcpy(taskKillTable[SAFEXEC].name,"saf_exec");
   taskKillTable[SAFEXEC].signal = SIGKILL;
   strcpy(taskKillTable[TRACELOG].name,"tracelogger");
   taskKillTable[TRACELOG].signal = SIGALRM;
   strcpy(taskKillTable[DATALOG].name,"datalogr");
   taskKillTable[DATALOG].signal = SIGHUP;
   strcpy(taskKillTable[METER].name,"meter");
   taskKillTable[METER].signal = SIGHUP;
   strcpy(taskKillTable[GUI].name,"gui");
   taskKillTable[GUI].signal = SIGHUP;
   strcpy(taskKillTable[PHOTON].name,"Photon");
   taskKillTable[PHOTON].signal = SIGHUP;
   strcpy(taskKillTable[EVERESTLOG].name,"everest_logger");
   taskKillTable[EVERESTLOG].signal = SIGHUP;

   // spawn gateway and open gateway queue
   if( getnid() != atoi( argv[2]))           // local node not remote node
   {
      char gatewayQueueName[NAME_LENGTH];    // message queue name

      // spawn gateway
      gatewayPID = qnx_spawn( 0,NULL,0,-1,-1,0,"gateway",argv,arge,NULL,-1);
      if(gatewayPID == QNX_ERROR)
      {
         fatalError( __LINE__, errno, "qnx_spawn()");
      }

      // build gateway q name and try to open it
      k = 0;
      strcpy( gatewayQueueName, argv[1]);
      strcat( gatewayQueueName, "Gateway");
      while( gatewayQueue == QNX_ERROR)      // try for ten seconds
      {
         sleep(1);
         gatewayQueue = mq_open( gatewayQueueName, O_WRONLY, 0, 0);
         if(k++ > 10)
         {
            fatalError( __LINE__, errno, "mq_open()");
         }
      }
   }
   else                                      // warn about test mode
   {
      _LOG_ERROR( __FILE__, __LINE__, TRACE_ROUTER, 0, "TEST MODE, no gateway started");
   }


   // open router input queue

   strncpy( qName, argv[1], NAME_LENGTH-1);
   qName[NAME_LENGTH-1] = 0;
   openRouterQueue( qName, &mq, &qproxy, &qnotify);


   // init system message processing

   sinVerInitialize();

   // loop processing messages

   while(taskRunning)                        // cleared by signals
   {
      // get a message
      pid = Receive( 0, &msg[0], sizeof( msg));
      if(pid == QNX_ERROR)                  // error occured
      {
         if(errno == EINTR)                 // signal interrupted
         {
            continue;
         }
         fatalError( __LINE__, errno, "Receive()");
      }


      // check for messages in queue

      if( pid != qproxy)                     // not from queue
      {
         // if not system message, then reply to attempt to unblock sender
         if(!sinVerMessage( pid, (SINVERMSG*) msg))
         {
            fatalError( __LINE__, pid, "unexpected msg");
         }
      }
      else
      {

         // reset queue notify for next time
         if(mq_notify( mq, &qnotify) == QNX_ERROR)
         {
            if(errno != EBUSY)              // EBUSY set if already enabled, ignore
            {
               fatalError( __LINE__, errno, "mq_notify()");
            }
         }

         // pull all data from queue
         while( (mq_receive( mq, msg, BSIZE, 0) != QNX_ERROR) &&
                (taskRunning))
         {

            // if CRC bad, FATAL_ERRO()
            if(!validFocusMsgCRC( msg))
            {
               char buffer[NAME_LENGTH];
               msgHeader = (MSGHEADER*) msg;
               sprintf( buffer, "-bad message CRC, NID=%d, PID=%d, MsgID=%d",
                        msgHeader->taskNID, msgHeader->taskPID, msgHeader->msgID);
               fatalError( __LINE__,0 , buffer);
            }
            else
            {
               msgHeader = (MSGHEADER*) msg;

               // CRC good, process this message

               switch( msgHeader->osCode)
               {
               case MSG_MULTICAST:           // distribute message
                  if( spoofMessage( (MSGHEADER*) msg) == 0)
                  {
                     distributeMessage( (MSGHEADER*) msg);
                  }
                  break;

               case TASK_REGISTER:           // register this task
                  taskRegister( msg);
                  break;
               case TASK_DEREGISTER:         // remove task from list(s)
                  taskDeregister( msg);
                  break;
               case MESSAGE_REGISTER:        // register message
                  messageRegister( msg, BOUNCE);
                  break;
               case MESSAGE_DEREGISTER:      // deregister message from an active task
                  messageDeregister( msg, ACTIVE);
                  break;
               case MESSAGE_TRACE:           // trace message
                  break;
               case GATEWAY_REGISTER:
                  taskDeregister( msg);
                  gatewayRegister( msg);
                  break;
               case SPOOFER_REGISTER:
                  taskDeregister( msg);
                  spooferRegister( msg);
                  break;
               case SPOOFED_MESSAGE:
                  distributeMessage( (MSGHEADER*) msg);
                  break;
               case MESSAGE_REGISTER_NO_BOUNCE: // register message
                  messageRegister( msg, NO_BOUNCE);
                  break;
               default:
                  fatalError( __LINE__, msgHeader->osCode, "bad message type");
               }
            }
         }
      }
   }

   // terminate program, close all open queues

   shutdown();                               // kill all tasks
   exit(1);
}


// SPECIFICATION:    multicast message
//                   Parameter:
//                   msg = pointer to message to distribute
//
// ERROR HANDLING:   Calls fatalError().

static
void
distributeMessage( MSGHEADER* msg)
{
   TASKLIST*   t;                         // task list
   short k;                               // counter
   unsigned    msgID = msg->msgID;        // message id
   pid_t       origPID = msg->taskPID;    // pid of originator

   if(msg == NULL)                       // internal error check
   {
      fatalError( __LINE__, 0, "-msg NULL");
   }

   // send to gateway
   // gateway will not be enabled in the test mode where the
   // second command line parameter (remote node) = running node number

   if((msg->taskNID == getnid()) &&       // not from other gateway
      (gatewayQueue != QNX_ERROR) &&      // and gateway active
      (msg->osCode != SPOOFED_MESSAGE))
   {
      k = 0;

      // loop on send to recover from errors
      mq_check(gatewayQueue);
      while((mq_send( gatewayQueue, msg, msg->length, 0) == QNX_ERROR) &&
            (taskRunning))
      {
         if((errno==EINTR) && ( k<3 ))   // signals
         {
            k++;
         }
         else
         {
            fatalError( __LINE__, errno, "mq_send() to gateway");
         }
      }
   }

   processGateways( msg);

   // loop thru tasks which are registered for this message
   t = messageLookupTable[msgID];

   while( (t) && (taskRunning))
   {
      if(!validFocusMsgCRC( t))          // internal error
      {
         fatalError( __LINE__, 0, "CRC error");
      }
      k=0;

      mq_check(t->mq);
      if ( (t->bounce == BOUNCE)
         ||( (t->bounce == NO_BOUNCE) && (t->h.taskPID != origPID) ) )
      {
         while( (mq_send( t->mq, msg, msg->length, 0) == QNX_ERROR) 
              &&(taskRunning) )
         {
            if((errno== EINTR) && (k<3))  // signal
            {
               k++;
            }
            else
               if(errno == EAGAIN)          // queue full
            {
               char buffer[ERROR_LENGTH]; // error message
               char qmsg[BSIZE];          // queue dump buffer
               struct _psinfo psdata;     // process data
   
               // get task name
   
               qnx_psinfo( PROC_PID, t->h.taskPID, &psdata, 0, 0);
               sprintf( buffer, __FILE__ "-PID=%d, %s, queue full", t->h.taskPID, psdata.un.proc.name);
               _LOG_ERROR( __FILE__, __LINE__, TRACE_ROUTER, t->h.taskPID, buffer);
   
               // dump queue to trace log
               while( mq_receive( t->mq, qmsg, BSIZE, 0) != QNX_ERROR)
               {
                  MSGHEADER* h = (MSGHEADER*) qmsg;
                  sprintf( buffer,"%02d %05d %03d %d.%d",
                           h->taskNID,                      // task NID
                           h->taskPID,                      // PID
                           h->msgID,                        // message ID
                           h->sendTime.tv_sec % 10,         // time, sec
                           h->sendTime.tv_nsec/1000000);    // time, ms
                  _LOG_ERROR( __FILE__, __LINE__, TRACE_ROUTER, t->h.taskPID, buffer);
               }
               fatalError( __LINE__, errno, "-mq_send()");
            }
            else                             // any other error is fatal
            {
               fatalError( __LINE__, errno, "-mq_send()");
            }
         }
      }                                   // end while
      // move to next PID in list
      if(t)
      {
         t = (TASKLIST*) t->next;
      }
   }  // end while linked list
}


// SPECIFICATION:    Send message to all gateways
//                   Parameters:
//                   *msg - message to check
//
// ERROR HANDLING:   Terminates program.

static
void
processGateways( MSGHEADER* msg)
{
   // send to TCP/IP gateways

   GATEWAYLIST* g = gatewayList;

   while( g)
   {
      int k = 0;

      // loop on send to recover from errors
      mq_check(g->mq);
      while((mq_send( g->mq, msg, msg->length, 0) == QNX_ERROR) &&
            (taskRunning))
      {
         if((errno==EINTR) && ( k<3 ))   // signals
         {
            k++;
         }
         else
         {
            _LOG_ERROR( __FILE__, __LINE__, TRACE_ROUTER, 0, "killed gateway");
            taskDeregister( (char*) &g->h);
            break;
         }
      }
      g = (GATEWAYLIST*) g->next;
   }

}


// SPECIFICATION:    Open router queue.
//                   Parameters:
//                   *qname - queuue name string
//                   *mqd - message queue descriptor
//                   *qproxy - proxy used for this queue
//                   *qnotify - message queue notify structure
//
// ERROR HANDLING:   Terminates program.

static void openRouterQueue(  char* qname,
                              mqd_t* mqd,
                              pid_t* qproxy,
                              struct sigevent* qnotify)
{
   struct mq_attr attr;             // message queue attributes

   // open queue
   attr.mq_maxmsg = MAX_Q_MSGS;     // set max number of messages
   attr.mq_msgsize = BSIZE;         // set message size
   attr.mq_flags = MQ_NONBLOCK;     // set non-block
   *mqd = mq_open( qname, O_RDWR | O_CREAT, 0666, &attr);
   if(*mqd == QNX_ERROR)
   {
      fatalError(__LINE__, errno, "mq_open()");
   }

   // create proxy for message queue
   *qproxy = qnx_proxy_attach( 0, 0, 0, -1);
   if(*qproxy == QNX_ERROR)
   {
      fatalError( __LINE__,errno , "qnx_proxy_attach()");
   }

   // setup queue notify
   qnotify->sigev_signo = -(*qproxy);
   if(mq_notify( *mqd, qnotify) == QNX_ERROR)
   {
      if(errno != EBUSY)
      {
         fatalError( __LINE__, errno, "mq_notify()");
      }
   }
   Trigger( *qproxy);                // avoid race condition by checking queue
}


// SPECIFICATION:    Shutdown router and all tasks connected to it.
//                   This will send SIGHUP to all tasks and close all
//                   open queues.  This routine is called when the router
//                   terminates.
//
// ERROR HANDLING:   None.

static void shutdown(void)
{
   int i;
   GATEWAYLIST* g = gatewayList;
   GATEWAYLIST* g1;

   taskRunning = 0;                 // clear task running flag

   // raise the priority of this task and the tracelogger
   qnx_scheduler(0,0,-1,26,0);
   if (taskKillTable[TRACELOG].pid != QNX_ERROR)
      qnx_scheduler(0,taskKillTable[TRACELOG].pid,-1,25,0);

   for(i=0;i<TASK_KILL_COUNT;i++)
   {
      if ( (taskKillTable[i].pid != QNX_ERROR)
         &&(taskKillTable[i].pid != 0) )
      {
         kill(taskKillTable[i].pid,taskKillTable[i].signal);
      }
   }

   // if gateway Q active, close it
   if(gatewayQueue != QNX_ERROR)
   {
      mq_close( gatewayQueue);      // close queue
   }
   // if gateway active, signal it
   if(gatewayPID != QNX_ERROR)
   {
      kill( gatewayPID, SIGHUP);    // terminate gateway
   }

   // if TCP/IP gateway Q active, close it

   while(g)
   {
      kill( g->h.taskPID, SIGHUP);  // signal task
      mq_close( g->mq);             // close queue
      g1 = g;                       // copy entry
      g = g->next;                  // move down chain
      free( g1);                    // free memory
   }
   gatewayList = 0;                    // clear tasklist
   

}



// SPECIFICATION:    Register the given message.
//                   Parameters:
//                   *msg - register message from task
//
// ERROR HANDLING:   Terminates program.

static void messageRegister( char* msg, bounce_t bounce)
{
   MSGHEADER* mhdr = (MSGHEADER*) msg;
   unsigned spoofMsg = 0;
   unsigned msgID = mhdr->msgID;    // get message ID

   // internal checks
   if(msgID >= MAX_MESSAGES)
   {
      fatalError( __LINE__, msgID, "msgID");
   }

   if(spoofer)
   {
      if((spoofer->h.taskPID == mhdr->taskPID) &&
         (spoofer->h.taskNID == mhdr->taskNID))
      {
         spoofer->msgs[msgID]++;
         spoofMsg = 1;
      }
   }

   if(spoofMsg == 0)
   {

      TASKLIST* t = taskQueueList;           // linked list pointer
      TASKLIST* ml = NULL;
      int found = 0;
      while(t)                               // scan list to find queue name
      {
         if(t->h.taskPID == mhdr->taskPID)   // matching task?
         {
            // has this pid already registered for this msgID?
            ml = messageLookupTable[msgID];
            found = 0;
            while (ml)
            {
               if (ml->h.taskPID == mhdr->taskPID)
               {
                  // its already registered so just bump the counter
                  ml->counter++;
                  found = 1;

                  // if any instance is set to bounce, all will bounce
                  if (ml->bounce == NO_BOUNCE)
                     ml->bounce = bounce;

                  // update the CRC for this entry
                  updateFocusMsgCRC( ml);
                  break;
               }
               ml = ml->next;
            }

            // if we didn't find it, we have to add it
            if (!found)
            {
               TASKLIST *newEntry = malloc( sizeof(TASKLIST));

               if(newEntry == NULL)
               {
                  fatalError( __LINE__, 0, "malloc()");
               }
               // clear entry
               memset( newEntry, 0, sizeof( TASKLIST));

               // copy the message header
               memcpy(&newEntry->h, mhdr, sizeof(MSGHEADER));

               // reset the length
               newEntry->h.length = sizeof(TASKLIST);

               // set the counter
               newEntry->counter = 1;

               // set the bounce instruction
               newEntry->bounce = bounce;

               // get the queue from the task queue list
               newEntry->mq = t->mq;

               // push it on to the look-up table
               newEntry->next = messageLookupTable[msgID];

               // update the CRC for this entry
               updateFocusMsgCRC( newEntry);

               // reset table head
               messageLookupTable[msgID] = newEntry;
            }
            break;                           // break while loop
         }
         t = t->next;                        // move down chain
      }
   }

}



// SPECIFICATION:    Deregister the given message.
//                   Parameters:
//                   *msg - register message from task
//
// ERROR HANDLING:   Terminates program.

static void messageDeregister( char* msg, TASKSTATUS status)
{
   unsigned spoofMsg = 0;
   MSGHEADER* mhdr = (MSGHEADER*) msg;
   unsigned msgID = mhdr->msgID;          // get message ID

   // internal checks
   if(msgID >= MAX_MESSAGES)
   {
      fatalError( __LINE__, msgID, "msgID");
   }


   if(spoofer)
   {
      if((spoofer->h.taskPID == mhdr->taskPID) &&
         (spoofer->h.taskNID == mhdr->taskNID))
      {
         spoofer->msgs[msgID]--;
         spoofMsg = 1;
      }
   }

   if(spoofMsg == 0)
   {
      TASKLIST* t = messageLookupTable[msgID];  // grab the start of the list
      TASKLIST* last = NULL;

      while(t)                               // scan list
      {
         if(t->h.taskPID == mhdr->taskPID)  // matching task?
         {
            // if this is an active task (i.e. we are not in the middle of
            // deregistering it) and the pid is registered for this message
            // more than once, just decrement it
            // but if the pid is only registered once or if we are deregistering
            // the task, we have to unlink it instead.
            if ( ( (--t->counter <= 0 ) && (status == ACTIVE) )
                 ||(status == INACTIVE) )
            {
               // remove this one from the list
               if (last != NULL)
               {
                  last->next = t->next;
                  free(t);

                  // recrc
                  updateFocusMsgCRC(last);
               }
               else
               {
                  messageLookupTable[msgID] = t->next;
                  free(t);
               }

            }
            else
               // recrc because we changed the counter
               updateFocusMsgCRC(t);

            break;
         }
         else
            last = t;                        // keep last entry
         t = t->next;                        // move down chain
      }
   }
}


// SPECIFICATION:    Check this message to see if it needs to be spoofed.
//                   If so, send it to the spoofer
//                   Parameters:
//                   *msg - pointer to the message
//
// ERROR HANDLING:   Terminates program.

static int spoofMessage( MSGHEADER* msg)
{
   int flag = 0;

   // check to see if spoofer active

   if(spoofer)
   {
      if(spoofer->msgs[msg->msgID] != 0)
      {
         int k = 0;

         // loop on send to recover from errors
         mq_check(spoofer->mq);
         while((mq_send( spoofer->mq, msg, msg->length, 0) == QNX_ERROR) &&
               (taskRunning))
         {
            if((errno==EINTR) && ( k<3 ))   // signals
            {
               k++;
            }
            else
            {
               _LOG_ERROR( __FILE__, __LINE__, TRACE_ROUTER, 0, "killed spoofer");
               kill(spoofer->h.taskPID, SIGHUP);
               free(spoofer);
               spoofer = NULL;
               break;
            }
         }
         flag = 1;
      }
   }
   return( flag);
}


// SPECIFICATION:    Register the given task.
//                   Parameters:
//                   *msg - register message from task
//
// ERROR HANDLING:   Terminates program.

static void gatewayRegister( char* msg)
{
   GATEWAYLIST *newEntry = malloc( sizeof(GATEWAYLIST));
   struct _psinfo    psdata;                 // process data
   pid_t pid;
	int i;

   if(newEntry == NULL)
   {
      fatalError( __LINE__, 0, "malloc()");
   }

   // clear entry
   memset( newEntry, 0, sizeof( GATEWAYLIST));

   // copy message, and verify
   memcpy( newEntry, msg, sizeof( MSGHEADER));
   if(memcmp(newEntry, msg, sizeof( MSGHEADER)) != 0)
   {
      fatalError( __LINE__, 0, "memcmp()");
   }

   // set up list entry
   newEntry->next = gatewayList;
   newEntry->h.length = sizeof( GATEWAYLIST);
   pid = ((GATEWAYLIST*) msg)->h.taskPID;

   // check to see task present
   if(qnx_psinfo( PROC_PID, pid, &psdata, 0, NULL) != pid)
   {
      fatalError( __LINE__, 0, "qnx_psinfo()");
   }

   // create queue name and open it
   sprintf( msg, "%s%d", basename(psdata.un.proc.name), pid);
   newEntry->mq = mq_open(msg, O_RDWR | O_NONBLOCK, 0, 0);
   if(newEntry->mq == QNX_ERROR)
   {
      char buffer[80];
      sprintf( buffer, "mq_open(%s)", msg);
      fatalError( __LINE__, 0, buffer);
   }

   // if this task is one of the essential shutdown tasks, we need
   // to capture the PID and mq
   for(i=0;i<TASK_KILL_COUNT;i++)
   {
      if (strcmp(taskKillTable[i].name,basename(psdata.un.proc.name)) == 0)
      {
         taskKillTable[i].pid = pid;
         taskKillTable[i].mq = newEntry->mq;
         break;
      }
   }

   // update CRC and add to list
   updateFocusMsgCRC( newEntry);
   gatewayList = newEntry;
}


// SPECIFICATION:    Register the given task.
//                   Parameters:
//                   *msg - register message from task
//
// ERROR HANDLING:   Terminates program.

static void spooferRegister( char* msg)
{
   struct _psinfo    psdata;                 // process data
   pid_t pid;
   pid_t proc_pid = PROC_PID;
   MSGHEADER* mhdr = (MSGHEADER*) msg;
   unsigned remote = (mhdr->taskNID != getnid());

   if(remote)
   {
      proc_pid = qnx_vc_attach( mhdr->taskNID, PROC_PID, 1000, 0);
   }

   if(spoofer)
   {
      mq_close( spoofer->mq);
      kill( spoofer->h.taskPID, SIGHUP);
      free( spoofer);
   }

   spoofer = malloc( sizeof(SPOOFER));

   if(spoofer == NULL)
   {
      fatalError( __LINE__, 0, "malloc()");
   }

   // clear entry
   memset( spoofer, 0, sizeof( SPOOFER));

   // copy message, and verify
   memcpy( spoofer, msg, sizeof( MSGHEADER));
   if(memcmp(spoofer, msg, sizeof( MSGHEADER)) != 0)
   {
      fatalError( __LINE__, 0, "memcmp()");
   }

   // set up list entry

   spoofer->h.length = sizeof( SPOOFER);
   pid = ((MSGHEADER*) msg)->taskPID;

   // check to see task present
   if(qnx_psinfo( proc_pid, pid, &psdata, 0, NULL) != pid)
   {
      fatalError( __LINE__, 0, "qnx_psinfo()");
   }

   // create queue name and open it

   sprintf( msg, "%d", mhdr->taskNID);
   if(setenv( "MQ_NODE", msg, 1) != 0)  // set remote node
   {
      fatalError( __LINE__, 0, "setenv()");
   }

   sprintf( msg, "%s%d", basename(psdata.un.proc.name), pid);
   spoofer->mq = mq_open(msg, O_RDWR | O_NONBLOCK, 0, 0);
   if(spoofer->mq == QNX_ERROR)
   {
      fatalError( __LINE__, errno, "mq_open()");
   }

   if(setenv( "MQ_NODE", NULL, 1) != 0)  // set remote node
   {
      fatalError( __LINE__, 0, "setenv()");
   }


   // update CRC and add to list
   updateFocusMsgCRC( spoofer);

   if(remote)
   {
      qnx_vc_detach( proc_pid);
   }
}


// SPECIFICATION:    Register the given task.
//                   Parameters:
//                   *msg - register message from task
//
// ERROR HANDLING:   Terminates program.

static void taskRegister( char* msg)
{
   TASKLIST *newEntry = malloc( sizeof(TASKLIST));
   struct _psinfo    psdata;                 // process data
   pid_t pid;
   int i;
   struct _trace_info info;
   pid_t id;

   if(newEntry == NULL)
   {
      fatalError( __LINE__, 0, "malloc()");
   }

   // clear entry
   memset( newEntry, 0, sizeof( TASKLIST));

   // copy message, and verify
   memcpy( newEntry, msg, sizeof( MSGHEADER));
   if(memcmp(newEntry, msg, sizeof( MSGHEADER)) != 0)
   {
      fatalError( __LINE__, 0, "memcmp()");
   }

   // set up list entry
   newEntry->next = taskQueueList;
   newEntry->h.length = sizeof( TASKLIST);
   pid = ((TASKLIST*) msg)->h.taskPID;

   // check to see task present
   if(qnx_psinfo( PROC_PID, pid, &psdata, 0, NULL) != pid)
   {
      fatalError( __LINE__, 0, "qnx_psinfo()");
   }

   // create queue name and open it
   sprintf( msg, "%s%d", basename(psdata.un.proc.name), pid);
   newEntry->mq = mq_open(msg, O_RDWR | O_NONBLOCK, 0, 0);
   if(newEntry->mq == QNX_ERROR)
   {
      fatalError( __LINE__, errno, "mq_open()");
   }

   // update CRC and add to list
   updateFocusMsgCRC( newEntry);
   taskQueueList = newEntry;

   // if this task is one of the essential shutdown tasks, we need
   // to capture the PID and mq
   for(i=0;i<TASK_KILL_COUNT;i++)
   {
      if (strcmp(taskKillTable[i].name,basename(psdata.un.proc.name)) == 0)
      {
         taskKillTable[i].pid = pid;
         taskKillTable[i].mq = newEntry->mq;

         // see if the tracelogger is up by now
         if (taskKillTable[TRACELOG].pid == QNX_ERROR)
         {
            if (qnx_trace_info(0, &info) == -1)
               _LOG_ERROR( __FILE__, __LINE__, TRACE_ROUTER, 0, "No trace info");
            else if (info.reader != 0)
            {
               taskKillTable[TRACELOG].pid = info.reader;
            }
         }

         break;
      }
   }
   // see if Photon is up
   if (taskKillTable[PHOTON].pid == QNX_ERROR)
   {
      id = 1;
      while ( (id = qnx_psinfo(0, id, &psdata, 0, 0)) != -1) 
      {
         if ( (stricmp(basename(psdata.un.proc.name),"Photon")) == 0 )
         {
            if (id != 0)
               taskKillTable[PHOTON].pid = id;
            break;
         }
         id++;
      }
   }
}


// SPECIFICATION:    Deregister the given task.
//                   Parameters:
//                   *msg - deregister message from task
//
// ERROR HANDLING:   Terminates program.

static void taskDeregister( char* msg)
{
   TASKLIST *newEntry = (TASKLIST*) msg;
   TASKLIST *p1;
   TASKLIST *p2;
   GATEWAYLIST *gnewEntry = (GATEWAYLIST*) msg;
   GATEWAYLIST *gp1;
   GATEWAYLIST *gp2;
   TASKLIST* t;
   TASKLIST *taskToGo = malloc( sizeof(TASKLIST));
   struct _psinfo    psdata;                 // process data
   int i;

   // we'll need the name below
   qnx_psinfo( PROC_PID, newEntry->h.taskPID, &psdata, 0, NULL);

   // internal check, deregister with empty list

   if(taskQueueList == NULL)
   {
   }
   else
      if(newEntry->h.taskPID == taskQueueList->h.taskPID) // front of chain
   {
      // clear entry
      memset( taskToGo, 0, sizeof( TASKLIST));

      // copy the header from the message
      memcpy(&taskToGo->h,msg,sizeof(MSGHEADER));

      // reset the length
      taskToGo->h.length = sizeof(TASKLIST);

      // spin the message lookup table and remove any references to this task
      // in the message linked lists
      for (i=0;i<MAX_MESSAGES;i++)
      {
         taskToGo->h.msgID = (unsigned short)i;
         t = messageLookupTable[i];
         if (t)
            messageDeregister((char *)taskToGo, INACTIVE);
      }

      p1 = taskQueueList->next;                    // move chain
      mq_close( taskQueueList->mq);                // close q
      free( taskQueueList);                        // free entry
      taskQueueList = p1;                          // fix head pointer
   }
   else                                            // scan chain
   {
      p1 = taskQueueList;                          // one-back
      p2 = p1->next;                               // next
      while( p2)                                   // loop thru chain
      {
         if(p2->h.taskPID == newEntry->h.taskPID) // match?
         {
            // clear entry
            memset( taskToGo, 0, sizeof( TASKLIST));

            // copy the header from the message
            memcpy(&taskToGo->h,msg,sizeof(MSGHEADER));

            // reset the length
            taskToGo->h.length = sizeof(TASKLIST);

            // spin the message lookup table and remove any references to this task
            for (i=0;i<MAX_MESSAGES;i++)
            {
               taskToGo->h.msgID = (unsigned short)i;
               t = messageLookupTable[i];
               if (t)
                  messageDeregister((char *)taskToGo, INACTIVE);
            }

            mq_close( p2->mq);                     // close q
            p1->next = p2->next;                   // fix chain
            updateFocusMsgCRC( p1);                // fix CRC
            free( p2);                             // free memory

            break;
         }
         p1 = p2;                                  // adjust pointers
         p2 = p2->next;
      }
   }
   free(taskToGo);

   // internal check, deregister with empty list

   if(gatewayList == NULL)
   {
   }
   else
      if(gnewEntry->h.taskPID == gatewayList->h.taskPID) // front of chain
   {
      gp1 = gatewayList->next;                         // move chain
      mq_close( gatewayList->mq);                     // close q
      free( gatewayList);                             // free entry
      gatewayList = gp1;                               // fix head pointer
   }
   else                                            // scan chain
   {
      gp1 = gatewayList;                               // one-back
      gp2 = gp1->next;                               // next
      while( gp2)                                   // loop thru chain
      {
         if(gp2->h.taskPID == gnewEntry->h.taskPID) // match?
         {
            mq_close( gp2->mq);                     // close q
            gp1->next = gp2->next;                   // fix chain
            updateFocusMsgCRC( gp1);                // fix CRC
            free( gp2);                             // free memory
            break;
         }
         gp1 = gp2;                                  // adjust pointers
         gp2 = gp2->next;
      }
   }

   // also check spoofer

   if((spoofer) &&
      (newEntry->h.taskPID == spoofer->h.taskPID) &&
      (newEntry->h.taskNID == spoofer->h.taskNID))
   {
      mq_close( spoofer->mq);
      kill( spoofer->h.taskPID, SIGHUP);
      free( spoofer);
      spoofer = NULL;
   }

   // if this task is one of the essential shutdown tasks, we need
   // to remove it from the shutdown list
   // make sure both the PID and the name match our shutdown list
   // before removing it
   for(i=0;i<TASK_KILL_COUNT;i++)
   {
      if ( (taskKillTable[i].pid == newEntry->h.taskPID)
         &&( strcmp(taskKillTable[i].name,basename(psdata.un.proc.name)) == 0) )
      {
         taskKillTable[i].pid = QNX_ERROR;
         taskKillTable[i].mq = QNX_ERROR;
         break;
      }
   }
}

