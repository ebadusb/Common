/*************************************************************************************
 * Copyright (c) 1997 Cobe BCT, Inc.
 *
 * FILENAME: checkself.h
 * PURPOSE: public header for checkself(), used to verify crc of self
 * CHANGELOG:
 * $Header: I:/BCT_Development/vxWorks/Common/include/rcs/checkself.h 1.3 2003/02/26 23:03:05Z td07711 Exp td07711 $
 * $Log: checkself.h $
 * Revision 1.2  2002/12/20 21:30:06Z  ms10234
 * Changed function declaration to take const char *'s
 * Revision 1.1  1999/05/28 01:40:22Z  TD10216
 * Initial revision
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
extern "C" 
#endif
int checkself( char* startSymbol, char* endSymbol, const char* filename );
    // PURPOSE:  verifies program loaded in memory has not changed.
    // Verifies load integrity when run over text and initialized data segments following load.
    // Verifies no text segment overwrites when run periodically thereafter.
    // 
    // checkself calculates a CRC over the data from startSymbol to endSymbol-1
    //
    // If filename exists, it reads the contained value as a previously saved CRC and verifies
    // that it is the same as the one calculated.  
    // Returns -1 if not validated, 0 if validated
    //
    // If filename does not exist, then filename is created and the calculated CRC is saved
    // for validation on subsequent boots.
    // Returns -1 if file not written, else 0


#endif // _CHECKSELF_HEADER

