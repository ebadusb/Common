/*
 *
 * Copyright (c) 1996 by Cobe BCT, Inc. All rights reserved.
 *
 * TITLE:           $Header: Q:/home1/COMMON_PROJECT/Source/INCLUDE/rcs/COMMONDEFS.H 1.3 1999/08/10 00:09:00 TD10216 Exp TD10216 $:
 *
 * AUTHOR:      I D Shukov
 *
 * ABSTRACT:   COBE BCT common shared definitions 
 *
 * $Log: COMMONDEFS.H $:
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
extern void BroadcastMain(void);
extern void GetHWVersionMessage(int argc, char **argv);
extern char *GetHwString(void);
extern char *GetReleaseID(void);
extern char *GetBuildID(void);
extern char *GetMachineCRC(void);
extern char *GetLogName(void);
#endif
