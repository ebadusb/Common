/*************************************************************************************
 * Copyright (c) 1997 Cobe BCT, Inc.
 *
 * FILENAME: checkself.c
 * PURPOSE: checkself() is used to verify crc of self
 * CHANGELOG:
 * $Header: //bctquad3/home/BCT_Development/vxWorks/Common/softcrc/rcs/checkself.cpp 1.2 2002/08/14 15:40:05Z pn02526 Exp ms10234 $
 * $Log: checkself.cpp $
 * Revision 1.1  1999/05/24 17:32:05  TD10216
 * Initial revision
 * Revision 1.1  1999/03/20 00:45:20  TD10216
 * Initial revision
 * Revision 1.1  1997/11/21 02:07:13  TD07711
 * Initial revision
 *  11/12/97 - dyes
 *************************************************************************************/

#include "crcgen.h"
#include "checkself.h"
#include "error.h"

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#ifdef VXWORKS
 // Temporarily disabled in vxWorks
 int checkself(int argc, char** argv, char* start, char* filename)
 {
     return 0;
 }
#else
 static char Last_init_data=1;  // must be assigned nonzero to be placed in initialized data region

 // SPECIFICATION:  checkself() - generates a CRC over text and a portion of initialized data, and
 // verifies against the CRC in specified file.
 // inputs:
 //    argc, argv - passed through to checkself.  If "-checkself <file>" is present then instead of doing
 //    the check and returning, checkself writes the calculated crc to <file> and exits(0).
 //    start - starting address of region to verify
 //    filename - file containing CRC to verify
 // outputs:
 //    returns 0 if CRC verifies, else -1
 // ERROR HANDLING:
 //    asserts that args are non null
 //    asserts that size of region to verify is > 0
 int checkself(int argc, char** argv, char* start, char* filename)
 {
    unsigned long calc_crc = 0;
    unsigned long exp_crc;
    FILE* infile;
    FILE* outfile=0;
    int i;
    long size = &Last_init_data - start;
    char buf[200];

    ASSERT(argc > 0);
    ASSERT(argv != 0);
    ASSERT(start != 0);
    ASSERT(filename != 0);

    //
    // calculate crc
    //
    ASSERT(size > 0);
    if (crcgen32(&calc_crc, start, size) == -1) {
       sprintf(buf, "checkself: crcgen32 failed, start=0x%08x size=%ld\n",
               start, size);
       _LOG_ERROR(__FILE__, __LINE__, TRACE_CODE, errno, buf);
       return -1;
    }

    //
    // look for "-checkself filename" on command line
    // if present, write crc and exit
    //
    for (i=0; i < (argc-1); i++) {
       if (strcmp(argv[i], "-checkself") == 0) {
          if ((outfile = fopen(argv[i+1], "w")) == 0) {
             fprintf(stderr, "checkself: fopen failed on %s, errno= %d %s\n",
                     argv[i+1], errno, strerror(errno));
             exit(1);
          }
          fprintf(outfile, "0x%08x\n", calc_crc);
          printf("checkself: start=0x%08x size=%ld calc_crc=0x%08x placed in %s\n",
                 start, size, calc_crc, argv[i+1]);
          exit(0);
       }
    }

    //
    // read expected crc from file specified
    //
    if ((infile = fopen(filename, "r")) == 0) {
       sprintf(buf, "checkself: fopen failed on %.100s, errno= %d %.40s\n",
               filename, errno, strerror(errno));
       _LOG_ERROR(__FILE__, __LINE__, TRACE_CODE, 0, buf);
       return -1;
    }
    if (fscanf(infile, "%x", &exp_crc) != 1) {
       sprintf(buf, "checkself: fscanf failed, errno= %d %.100s\n",
               errno, strerror(errno));
       _LOG_ERROR(__FILE__, __LINE__, TRACE_CODE, errno, buf);
       return -1;
    }

    //
    // verify calculated == expected
    //
    if (calc_crc != exp_crc) {
       sprintf(buf, "checkself: bad crc, exp=0x%08x calc=0x%08x file=%.100s start=0x%08x size=%ld\n",
               exp_crc, calc_crc, filename, start, size);
       _LOG_ERROR(__FILE__, __LINE__, TRACE_CODE, errno, buf);
       return -1;
    }

    //
    // log good crc and return
    //
    sprintf(buf, "checkself: good crc, exp=0x%08x calc=0x%08x file=%.100s start=0x%08x size=%ld\n",
            exp_crc, calc_crc, filename, start, size);
    _LOG_ERROR(__FILE__, __LINE__, TRACE_CODE, 0, buf);
    return 0;
 }
#endif
