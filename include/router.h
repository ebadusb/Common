/*
 * Copyright (c) 1996-1999 by Cobe BCT, Inc.  All rights reserved.
 *
 * $Header: //bctquad3/home/BCT_Development/vxWorks/Common/include/rcs/router.h 1.1 1999/08/31 17:58:58 BS04481 Exp ms10234 $
 * $Log: router.h $ 
 *
 *
 * TITLE:      router.h, Focussed System message routing.
 *
 * ABSTRACT:   Uses Mqueue to route messages in the Focussed Syste
 *
 * DOCUMENTS
 * Requirements:
 * Test:
 */

#ifndef ROUTER_H
#define ROUTER_H

#include <mqueue.h>
#include <process.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

void messageLoop(pid_t gatePID, char* ownPrefix);
void shutdown(void);

#ifdef __cplusplus
};
#endif

#endif  //ROUTER_H


