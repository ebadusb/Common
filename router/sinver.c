/*
 * copyright 1995, 1996 Cobe BCT Lakewood, Colorado
 *
 * $Header: K:/BCT_Development/Common/router/rcs/sinver.c 1.2 1999/08/13 23:00:55 BS04481 Exp jl11312 $
 * $Log: sinver.c $
 * Revision 1.2  1999/08/13 23:00:55  BS04481
 * In the sin ver reponse, do not reply to a sysmsg if the subtype 
 * is death.
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

// default version data

#define  MAX_REV_DATA_SIZE 64
static char revData[MAX_REV_DATA_SIZE] = "$ProjectRevision: 5.20 $";     // common project rev code

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
   long bits;                                // process flags
   char * decimalPosition = NULL;

   const char * appRevData = getAppRevisionData();
   if (appRevData)
   {
      strncpy(revData, appRevData, MAX_REV_DATA_SIZE);
      revData[MAX_REV_DATA_SIZE-1] = '\0';
   }

// default struct

   reply.header.status = EOK;
   reply.header.zero = 0;
   strcpy( reply.reply.name, "");
   strcpy( reply.reply.date, "");


   reply.reply.more = 0;

// fill in version information

   decimalPosition = strchr(revData, '.');
   if (!decimalPosition)
   {
      reply.reply.version = 0;
      reply.reply.letter = ' ';
   }
   else
   {
      int   major = 0;
      int   majorMult = 100;
      int   majorPos = -1;
      int   minorPos = 1;
      int   minor = 0;

      while ( majorMult <= 1000 &&
              decimalPosition[majorPos] >= '0' &&
              decimalPosition[majorPos] <= '9' )
      {
         major += majorMult * (decimalPosition[majorPos]-'0');
         majorPos -= 1;
         majorMult *= 10;
      }

      while ( minorPos <= 3 &&
              decimalPosition[minorPos] >= '0' &&
              decimalPosition[minorPos] <= '9' )
      {
         minor = (minor*10) + decimalPosition[minorPos]-'0';
         minorPos += 1;
      }

      reply.reply.version = major + minor/10;
      reply.reply.letter = minor%10 + '0';
   }

// fill in process name and date

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

// set bit to notify QNX that we will respond to a version message

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


