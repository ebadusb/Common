/*
 * copyright 1995, 1996 Cobe BCT Lakewood, Colorado
 *
 * $Header: K:/BCT_Development/Common/router/rcs/sinver.c 1.2 1999/08/13 23:00:55 BS04481 Exp jl11312 $
 * $Log: sinver.c $
 * Revision 1.1  1999/05/24 23:29:54  TD10216
 * Initial revision
 * Revision 1.2  1996/07/24 19:50:10  SS03309
 * fix MKS 
 * Revision 1.2  1996/07/22 14:43:50  SS03309
 * Revision 1.9  1996/07/19 16:23:33  SS03309
 * lint changes
 *
 * Respond to sin ver messages
 *
 * DOCUMENTS
 * Requirements:     I:\ieee1498\SSS2.DOC
 * Select OMT:       I:\router
 * Test:             I:\ieee1498\STP2.DOC
 */

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/kernel.h>
#include <sys/psinfo.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "sinver.h"

// defines

#define VER_LEN 4       // QNX revision string length
#define MKS_MAJOR 18    // MKS rev number
#define MKS_MINOR1 20
#define MKS_MINOR2 21

#define QNX_MAJOR  0    // QNX rev number
#define QNX_MINOR1 1
#define QNX_MINOR2 2

typedef struct
{
   struct _sysmsg_hdr_reply      header;
   struct _sysmsg_version_reply  reply;
} VER_REPLY;

typedef struct
{
   struct _sysmsg_hdr_reply      header;
   struct _sysmsg_signal         signal;
} SIGNAL_MESSAGE;

// static data

static VER_REPLY reply;

// SPECIFICATION:    sinVerInitialize should be called once before
//                   receiving any messages.
//                   if system calls fail, this feature won't work
//                   correctly, but this feature is not part of the
//                   safety system, so errors are ignored.
//
// ERROR HANDLING:   none

void sinVerInitialize()
{
   struct _psinfo psdata;                    // process data
   struct stat   statBuffer;                 // file status
   struct tm tmbuf;                          // time info
   long bits;                               // process flags
static char rev[] = "$ProjectRevision: 5.20 $";     // rev code

   char vString[VER_LEN];                    // short version string

// fix 1.1 to be 1.01, leaving 1.10 = 1.10

   vString[VER_LEN-1] = 0;
   vString[QNX_MAJOR] = rev[MKS_MAJOR];
   if (rev[MKS_MINOR2] == ' ')
   {
      vString[QNX_MINOR1] = '0';
      vString[QNX_MINOR2] = rev[MKS_MINOR1];
   }
   else
   {
      vString[QNX_MINOR1] = rev[MKS_MINOR1];
      vString[QNX_MINOR2] = rev[MKS_MINOR2];
   }

// default struct

   reply.header.status = EOK;
   reply.header.zero = 0;
   strcpy( reply.reply.name, "");
   strcpy( reply.reply.date, "");


   reply.reply.version = atoi( vString);     // display project number
   reply.reply.letter = ' ';
   reply.reply.more = 0;

// get name

   if (qnx_psinfo( PROC_PID, getpid(), &psdata, 0, 0) != -1)
   {
      long k = sizeof( reply.reply.name) - 1;
      strncpy( reply.reply.name, basename( psdata.un.proc.name), k);
      reply.reply.name[k] = '\0';                           // force Null
      if (stat( psdata.un.proc.name, &statBuffer) != -1)    // get file status
      {
         _localtime(&statBuffer.st_mtime, &tmbuf);          // modify time
         sprintf( reply.reply.date, "%02d%02d%02d %02d%02d",
            tmbuf.tm_year % 100, tmbuf.tm_mon + 1, tmbuf.tm_mday,
            tmbuf.tm_hour, tmbuf.tm_min);
      }
   }
   bits = _PPF_SERVER;
   qnx_pflags( bits, bits, 0, 0);
}

// SPECIFICATION:    sinVerMessage returns true if message was from sin ver
//                   Parameters:
//                   pid - PID to reply to
//                   msg - Message sent
//
// ERROR HANDLING:   none

unsigned char sinVerMessage( pid_t pid, SINVERMSG* msg)
{
   unsigned char retVal = 0;

   if (msg->header.type == _SYSMSG)
   {
      if (msg->header.subtype == _SYSMSG_SUBTYPE_VERSION)
      {
         Reply( pid, &reply, sizeof( reply));
         retVal = 1;
      }
      else
      if (msg->header.subtype == _SYSMSG_SUBTYPE_SIGNAL)
      {
         reply.header.status = ENOSYS;
         Reply( pid, &reply.header, sizeof( reply.header));
         kill( getpid(), ((SIGNAL_MESSAGE*)msg)->signal.bits);
      }
      else if (msg->header.subtype != _SYSMSG_SUBTYPE_DEATH)
      {
         reply.header.status = ENOSYS;
         Reply( pid, &reply.header, sizeof( reply.header));
      }
   }
   return retVal;
}


