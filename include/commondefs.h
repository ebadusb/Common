/*
 *
 * Copyright (c) 1996 by Cobe BCT, Inc. All rights reserved.
 *
 * TITLE:           $Header: Q:/home1/COMMON_PROJECT/Source/INCLUDE/rcs/COMMONDEFS.H 1.5 1999/08/14 00:10:33 TD10216 Exp TD10216 $:
 *
 * AUTHOR:      I D Shukov
 *
 * ABSTRACT:   COBE BCT common shared definitions 
 *
 * $Log: COMMONDEFS.H $:
 * Revision 1.4  1999/08/13 00:35:42  TD10216:
 * IT4219:
 * Revision 1.2  1999/07/08 23:46:04  TD10216:
 * IT4115:
 * Revision 1.1  1999/05/24 23:26:19  TD10216:
 * Initial revision:
 *
 */

#ifndef COMMONDEFS_H
#define COMMONDEFS_H

// for a better, safer world (formerly in procdefs.h)
typedef int iBool;

// for an2_update_config (formerly in guiproc.h)
enum CONFIG_UPDATE_RESPONSES
{
   RUN_INIT_UPDATE_COMPLETE,
   CONFIG_FILE_UPDATE_COMPLETE,
   RUN_INIT_UPDATE_FAILED,
};

// for an2_broadcast - All of these except BroadcastMain must be
// provided by a device. The GetHWVersionMessage calls back to the
// BroadcastMain routine and then exits.
extern void BroadcastMain(int, char **);
extern void GetHWVersionMessage(int argc, char **argv);
extern char *GetHwString(void);
extern char *GetReleaseID(void);
extern char *GetBuildID(void);
extern int 	GetMachineCRC(unsigned long *iCRC, char *cCRC);
extern char *GetLogName(void);

// for an2_connected
// This routine must be provided by a device
// where return of SERVICE_ALLOWED (an2msgs.h) means continue
extern int AN2_ConnectionOK(void);
// provided by the library
extern void an2_connected(int argc, char **argv);

// for an2_load_agent
// This routine must be provided by a device
// where non-zero return value means its 'OK'
extern int AN2_load_agentOK(void);
// provided by the library
extern void an2_load_agent(int argc, char** argv);

#endif
