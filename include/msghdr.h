/*
 * Copyright (c) 1995, 1996 by Cobe BCT, Inc.  All rights reserved.
 *
 * $Header: Q:/home1/COMMON_PROJECT/Source/INCLUDE/rcs/MSGHDR.H 1.2 1999/05/31 20:35:00 BS04481 Exp TD10216 $
 * $Log: MSGHDR.H $
 * Revision 1.1  1999/05/24 23:26:41  TD10216
 * Initial revision
 * Revision 1.12  1999/03/29 17:10:12  TD10216
 * Revision 1.11  1999/03/15 18:10:05  MS10234
 * Change max message size to allow for bigger messages in the
 * future.  The size was changed from 256 to 512.
 * Revision 1.10  1998/11/06 22:36:01  TM02109
 * Added WIN32 ifdefs..
 * Revision 1.9  1998/10/23 19:38:46  TM02109
 * Modified to allow for inclusion of the buffmsgs.h and intmsgs.h files
 * without getting the entire array of class definitions.
 * Revision 1.8  1998/10/22 22:40:08  TM02109
 * Windows redefines MSG.
 * Revision 1.7  1998/10/22 22:28:22  TM02109
 * Modifications to allow for inclusion of just the messages and 
 * header information for objects that do not need the entire
 * messaging system.
 * Revision 1.6  1996/12/04 15:15:50  SS03309
 * Added new headers for spoofer and tcp_gate
 * Revision 1.5  1996/07/24 19:49:51  SS03309
 * fix MKS 
 * Revision 1.4  1996/06/27 16:30:17  SS03309
 * Comments
 *
 * TITLE:      msghdr.h, Focussed System message header.
 *
 * ABSTRACT:   This file defines the message header for the
 * Focus System dispatcher, router, and gateway.
 *
 * DOCUMENTS
 * Requirements:     I:\ieee1498\SSS2.DOC
 * Test:             I:\ieee1498\STD2.DOC
 *
 * PUBLIC FUNCTIONS:
 * none
 */

#ifndef MSGHDR_HPP                     // prevent multiple includes
#define MSGHDR_HPP

// Check to see if this is for the windows version or QNX version.
#ifndef WIN32
// QNX
#include <sys/types.h>
#include <time.h>

#else
// Windows

struct timespec 
{
    long    tv_sec;
    long    tv_nsec;    
};

typedef struct
{
   unsigned char buffer[1];   // length allocated in constructor
} MSGDATA;

typedef  int         pid_t;            /* Used for process IDs & group IDs */
typedef long         nid_t;            /* Used for network IDs         */

#endif

#define MAX_MESSAGES   1024            // max number of messages
#define BSIZE          304             // max message size

enum BOUNCE_FLAG
{
   BOUNCE=1,                                 // bounce message back to originator
   NO_BOUNCE=2                               // originator does not want to see message
};
typedef enum BOUNCE_FLAG bounce_t;


typedef struct
{
   unsigned short osCode;              // os message code
   unsigned short length;              // total message length, bytes
   unsigned long  CRC;                 // 32 bit CRC
   unsigned short msgID;               // enum message id
   nid_t          taskNID;             // task network node id
   pid_t          taskPID;             // task PID number
   struct timespec sendTime;           // time message sent, QNX time struct

} MSGHEADER;

enum OSCODE
{
   MSG_MULTICAST=0x5000,               // must be greater than 0x0fff
   TASK_REGISTER,                      // register task with router
   TASK_DEREGISTER,                    // deregister task with router
   MESSAGE_REGISTER,                   // register message with router
   MESSAGE_DEREGISTER,                 // deregister message with router
   MESSAGE_TRACE,                      // trace message events
   SPOOFED_MESSAGE,                    // spoofer has modified message
   SPOOFER_REGISTER,                   // register the spoofer task
   GATEWAY_REGISTER,                   // register a gateway task
   MESSAGE_REGISTER_NO_BOUNCE          // register message with instructions not
                                       // to bounce it back to the originator
};

#ifdef WIN32
// NOTE: Windows must not use the standard Trima struct name MSG.
//       MSG is already a defined type in windows.
typedef struct 
{
   MSGHEADER   header;
   MSGDATA     data;
} TRIMA_MSG;
#endif

#endif
