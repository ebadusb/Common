/*
 * Copyright (c) 1995, 1996 by Cobe BCT, Inc.  All rights reserved.
 *
 * $Header: K:/BCT_Development/Common/router/rcs/phdspat.cpp 1.2 2000/06/06 19:09:45 ms10234 Exp $
 * $Log: phdspat.cpp $
 * Revision 1.2  2000/06/06 19:09:45  ms10234
 * Removed message enumerations from the common project
 * Revision 1.1  1999/05/24 23:29:50  TD10216
 * Initial revision
 * Revision 1.12  1996/07/24 19:49:57  SS03309
 * fix MKS 
 * Revision 1.11  1996/06/28 17:24:45  SS03309
 * 10 - Add comment to see Photon Manuals, this
 *          may change if photon changes
 *          44 - move stucture to dispatch.hpp
 *          87 - magic number
 *          99 - change to exit
 *
 * TITLE:      phdspat.cpp, Focussed System message router - photon message
 *             dispatcher.
 *
 * ABSTRACT:   These classes support message routing in the Focussed System
 *             using the Photon graphics system.
 *
 * DOCUMENTS
 * Requirements:     I:\ieee1498\SSS2.DOC
 * Test:             I:\ieee1498\STD2.DOC
 *
 * See also photon rev 1.0 manuals for PtAppAddInput() and PtMainLoop().
 * Note: This module may need to be changed if photon interface changes.
 */

// #includes

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/irqinfo.h>
#include <sys/kernel.h>
#include <sys/name.h>
#include <sys/proxy.h>
#include <sys/types.h>
#include <unistd.h>

// messages
#include "crc.h"
#include "dispatch.hpp"
#include "phdspat.hpp"
#include "sinver.h"

// photon
#include <Pt.h>
#include <Ph.h>
#include <Ap.h>

// public data

extern dispatcher* dispatch ;       // dispatcher

// SPECIFICATION:    photon calls this function to process our messages
//                   Parameters:
//                   only the second and third are used.
//                   pid - pid sending this message
//                   message - pointer to message
//
// ERROR HANDLING:   none.

static int input_funct( void*, pid_t pid, void* message, size_t)
{
   // check for system messages
   if (!sinVerMessage( pid, (SINVERMSG*) message))
   {
      // if no signals, process our message
      if (dispatch->taskRunning)
      {
         dispatch->processMessage( pid);
      }
      else

// we're not running, check if clean up needed

      if (dispatch->mq != MQ_ERROR)
      {

// deregister this task with local router
         dispatcher::TASKLIST tl;

         tl.h.osCode = TASK_DEREGISTER;            // os message code
         tl.h.length = sizeof( dispatcher::TASKLIST); // message length, bytes
         tl.h.taskPID = getpid();                  // task PID number
         tl.mq = dispatch->mq;                     // messsage queue
         tl.next = NULL;                           // next in chain
         updateFocusMsgCRC( &tl);                  // update CRC
         mq_send( dispatch->routerQueue, &tl, tl.h.length, 0);

// close queue
         mq_close( dispatch->mq);
         mq_unlink( dispatch->queueName);
         dispatch->mq = MQ_ERROR;
         exit(1);                                  // tell photon to stop
      }
   }
   else
   {
      //
      // Prevent photon from notifying us of the same version message more
      // than once.
      //
      struct _sysmsg_hdr   * hdr = (struct _sysmsg_hdr *)message;
      hdr->type = 0xffff;
   }

   return Pt_CONTINUE;                             // tell photon to keep sending msgs
}


// SPECIFICATION:    dispatcher is used to distribute messages within a task
//                   this is the constructor
//                   argv[0] = program name
//                      [1] = local queue name
//                      [2] = remote router node (not used)
//                      [3] = remote queue name (not used)
//                      [4] = L to log internal events
//                   maxMessages is used to set Q length
//
// ERROR HANDLING:   none.

photonDispatcher::photonDispatcher(int argc, char** argv, int maxMessages) :
   dispatcher(argc, argv, maxMessages)
{
};

// SPECIFICATION:    destructor
//
// ERROR HANDLING:   none.

photonDispatcher::~photonDispatcher()
{
   PtAppRemoveInput(NULL, id);               // remove input loop
};

// SPECIFICATION:    message processing loop, doesn't return.
//
// ERROR HANDLING:   terminates program.

void
photonDispatcher::dispatchLoop()
{

   // set up sin ver processing
   sinVerInitialize();

   // hook function to photon for processing messages
   id = PtAppAddInput( NULL,  0, input_funct, NULL);

   // loop processing messages
   PtMainLoop();
};


