/*
 * copyright 1995, 1996 Cobe BCT Lakewood, Colorado
 *
 * $Header: //bctquad3/home/BCT_Development/vxWorks/Common/include/rcs/sinver.h 1.2 2001/05/24 23:09:49 jl11312 Exp ms10234 $
 * $Log: sinver.h $
 * Revision 1.1  1999/05/24 23:26:45  TD10216
 * Initial revision
 * Revision 1.2  1996/07/24 19:50:11  SS03309
 * fix MKS 
 * Revision 1.2  1996/07/22 14:43:51  SS03309
 * Revision 1.5  1996/07/19 15:21:35  SS03309
 * added void to sinVerInitialize for lint
 * Revision 1.4  1996/06/27 16:32:52  SS03309
 * Comments
 *
 * sinver.h
 *
 * Process "sin ver" system messages
 *
 * DOCUMENTS
 * Requirements:     I:\ieee1498\SSS2.DOC
 * Select OMT:       I:\router
 * Test:             I:\ieee1498\STP2.DOC
 *
 * EXAMPLE:
 */

#ifndef SINVER_H                    // prevent multiple includes
#define SINVER_H

// includes

#include <sys/sys_msg.h>

// defines, typedef, and structs

typedef struct  {
   struct _sysmsg_hdr      header;
   struct _sysmsg_version  version;
   } SINVERMSG;

// check and respond to sin ver messages

#ifdef __cplusplus
extern "C" {
#endif

// SPECIFICATION:    getAppRevisionData can be provided to override
//                   the common project version information.  The
//                   default version simply returns NULL, so that
//                   the common build revision data will be used.
//
// ERROR HANDLING:   none

const char * getAppRevisionData( );

// SPECIFICATION:    sinVerInitialize should be called once
//                   before receiving any messages.  If it is not
//                   called the sin program will not send messages
//                   to this program.
//
// ERROR HANDLING:   none

void sinVerInitialize( void);

// SPECIFICATION:    sinVerMessage returns true if message was from sin ver
//                   Parameters:
//                   pid - PID to reply to
//                   msg - Message sent
//
// ERROR HANDLING:   none

unsigned char sinVerMessage( pid_t pid, SINVERMSG* msg);

#ifdef __cplusplus
};
#endif

#endif
