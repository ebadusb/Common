/*************************************************************************************
 * Copyright (c) 1997 Cobe BCT, Inc.
 *
 * FILENAME: checkself.h
 * PURPOSE: public header for checkself(), used to verify crc of self
 * CHANGELOG:
 * $Header: //bctquad3/home/BCT_Development/vxWorks/Common/include/rcs/checkself.h 1.1 1999/05/28 01:40:22Z TD10216 Exp ms10234 $
 * $Log: checkself.h $
 * Revision 1.2  1998/08/27 22:04:52  TD07711
 *   use #ifdef _cplusplus extern "C" to make usable by both .cpp and
 *   .c programs.
 * Revision 1.1  1997/11/21 00:50:42  TD07711
 * Initial revision
 *  11/12/97 - dyes
 *************************************************************************************/

#ifndef _CHECKSELF_HEADER
#define _CHECKSELF_HEADER


#ifdef __cplusplus
extern "C" {
#endif


// SPECIFICATION:  checkself() - generates a CRC over text and a portion of initialized data, and
// verifies against the CRC in specified file.
// inputs:
//    argc, argv - passed through to checkself.  If "-checkself filename" is present then instead of doing
//    the check and returning, checkself writes the calculated crc to /tmp/filename and exits(0).
//    start - starting address of region to verify
//    filename - file containing CRC to verify
// outputs:
//    returns 0 if CRC verifies, else -1
// ERROR HANDLING:
//    asserts that args are non null
//    asserts that size of region to verify is > 0
// USAGE:
//    1. The position of checkself.o in the executable's link order determines what initialized data is
//    covered by the check.  checkself.o must follow any other object/library files whose initialized
//    data you want to cover.
//    2. checkself() must be called before the executable has a chance to modify the initialized data
//    covered.
int checkself(int argc, char** argv, char* start_addr, char* filename);

#ifdef __cplusplus
}
#endif

#endif

