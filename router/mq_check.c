/* Copyright (c) 1998 Cobe BCT Inc.
 *
 * FILENAME: mq_check.c
 * PURPOSE: implements mq_check() which is used to expose nearing msg queue overflow
 * CHANGELOG:
 *   $Header: Q:/home1/COMMON_PROJECT/Source/ROUTER/rcs/MQ_CHECK.C 1.3 1999/08/06 14:33:50 BS04481 Exp BS04481 $
 *   $Log: MQ_CHECK.C $
 *   Revision 1.2  1999/05/31 20:35:05  BS04481
 *   Remove unused MSGHEADER structure from messages. 
 *   Decrease maximum message size.  Add new version of 
 *   focusBufferMsg and focusInt32Msg that do not bounce the message
 *   back to the originator.  All changes to increase free memory.
 *   Revision 1.1  1999/05/24 23:29:43  TD10216
 *   Initial revision
 *   Revision 1.1  1998/08/03 20:29:45  TD07711
 *   Initial revision
 *   07/24/98 - dyes - initial version
 */

#include <errno.h>
#include <mqueue.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include "error.h"
#include "mq_check.h"

const int Absolute_limit = 2; // log warning if queue has space for 2 or fewer messages
const float Percent_limit = 0.10; // log warning if queue space remaining <= 10%
static int High_water_mark = 0;  // greatest number of messages every in queue

// FUNCTION: mq_check
// PURPOSE: logs warning to the tracelog if specified msg queue is nearing overflow.
//   This tracelog I/O may have side effect of slowing down the msg queueing and so
//   may also help avoid a queue full.
//   If an invalid mqdes is passed, mq_check will log an error.
// ARGUMENTS: mqd_t mqdes - msg queue descriptor to check
// RETURNS: void
void mq_check(mqd_t mqdes)
{
    struct mq_attr mqstat;
    long margin;
    long percent_limit;
    long limit;
    char buf[100];

    //
    // get mq attributes
    //
    if (mq_getattr(mqdes, &mqstat) == -1) {
        sprintf(buf, "ERROR mq_check: mq_getattr failed errno=%d pid=%d qid=%d",
                errno, getpid(), mqdes);
        _LOG_ERROR(__FILE__, __LINE__, TRACE_DISPATCHER, errno, buf);
        return;
    }

    //
    // determine queue space remaining and which limit to check against
    //
    margin = mqstat.mq_maxmsg - mqstat.mq_curmsgs;
    percent_limit = (float)mqstat.mq_maxmsg * Percent_limit;
    limit = percent_limit;
    if (limit < Absolute_limit) {
        limit = Absolute_limit;
    }

    if ( (mqstat.mq_curmsgs+1) > High_water_mark)
       High_water_mark = mqstat.mq_curmsgs +1;

    //
    // check queue space remaining and log warning if check fails
    //
    if (margin <= limit) {
        sprintf(buf, "Warning mq_check: queue almost full pid=%d qid=%d size=%d used=%d",
                getpid(), mqdes, mqstat.mq_maxmsg, mqstat.mq_curmsgs);
        _LOG_ERROR(__FILE__, __LINE__, TRACE_DISPATCHER, getpid(), buf);
    }
}

void mq_highWater()
{
   char eString[256];
   sprintf(eString,"High water mark for PID %d: %d messages"
           , getpid()
           , High_water_mark);
   _LOG_ERROR_NO_DISPLAY(__FILE__, __LINE__, TRACE_DISPATCHER, 0, eString);
}

