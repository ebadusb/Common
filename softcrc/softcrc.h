/*************************************************************************************
 * Copyright (c) 1997 Cobe BCT, Inc.
 *
 * FILENAME: softcrc.h
 * PURPOSE: header file for crc utilities
 * CHANGELOG:
 *  4/30/97 - dyes
 *  5/1/97 - dyes - add defines for crc filenames
 *  5/5/97 - dyes - add filelistItem arg to doitem()
 *  5/16/97 - dyes - add ignore(), doignore(), and IGNORE_KEYWORD
 *  6/23/97 - dyes - moved CRCFILE locations
 *
 * $Header: I:/BCT_Development/vxWorks/Common/softcrc/rcs/softcrc.h 1.3 2000/12/08 23:22:55 td07711 Exp td07711 $
 * $Log: softcrc.h $
 * Revision 1.6  1998/11/03 02:54:30  TM02109
 * Added ability to do multipule filelist's and multipule chroot's
 * Revision 1.5  1998/08/26 00:06:53  TD10216
 * changing cfg_main.dat -> config.dat
 * Revision 1.4  1997/05/23 21:29:47  TD07711
 * moved CRC file paths - /d/machine/crc/xxx.crc
 *************************************************************************************/

 #ifndef _SOFTCRC_HEADER
 #define _SOFTCRC_HEADER

 // crc filenames for restricted use
 // fieldcrc can only update the FIELD files
 // machcrc can only update the MACH files
 // softcrc has no restrictions
 #define FIELD_CRCFILE    "/d/machine/crc/machine.crc"
 #define MACH_CRCFILE     "/d/machine/crc/config.crc"

 // cmd line arg defaults and limits
 #define INITCRC_DEFAULT 0
 #define VERBOSE_DEFAULT 1
 #define VERBOSE_MAX     3
 #define LIMIT_DEFAULT 10
 #define LIMIT_MAX     100
 #define READBUF_DEFAULT  (4096*4) // read buffer size
 #define READBUF_MAX      1000000
 #define FILELIST_MAX 10

 // buffer sizes
 #define BUF_SIZE       256     // error msg buffer

 // filelist keyword for ignoring specified pathname
 //  ex:  IGNORE /focus  ignores /focus directory
 #define IGNORE_KEYWORD  "IGNORE"

 // function protos
 void parseCmdline(int argc, char** argv);
 char* getnextitem(FILE* filelist);
 void doitem(unsigned long* pcrc, char* pathname, int filelistItem);
 void dofile(unsigned long* pcrc, char* pathname);
 void dodir(unsigned long* pcrc, char* pathname);
 void usage(char* msg);
 void logerror(char* msg);
 void logerrno(char* msg);
 void loginfo(char* msg);
 void logdebug(char* msg);
 void logwarning(char* msg);
 void doignore(char* pathname);
 int ignore(char* pathname);

 #endif
