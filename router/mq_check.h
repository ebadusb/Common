/* Copyright (c) 1998 Cobe BCT Inc.
 *
 * FILENAME: mq_check.h
 * PURPOSE: function prototype for mq_check()
 * CHANGELOG:
 *   $Header: Q:/home1/COMMON_PROJECT/Source/ROUTER/rcs/MQ_CHECK.H 1.1 1999/05/24 23:29:44 TD10216 Exp BS04481 $
 *   $Log: MQ_CHECK.H $
 *   Revision 1.1  1998/08/03 20:29:47  TD07711
 *   Initial revision
 *   07/24/98 - dyes - initial version
 */

#ifndef MQ_CHECK_HPP
#define MQ_CHECK_HPP

#ifdef __cplusplus
extern "C" {
#endif

#include <mqueue.h>

// FUNCTION: mq_check
// PURPOSE: logs warning to the tracelog if specified msg queue is nearing overflow.
//   This tracelog I/O may have side effect of slowing down the msg queueing and so
//   may also help avoid a queue full.
//   If an invalid mqdes is passed, mq_check will log an error.
// ARGUMENTS: mqd_t mqdes - msg queue descriptor to check
// RETURNS: void
void mq_check(mqd_t mqdes);

#ifdef __cplusplus
};
#endif

#endif
