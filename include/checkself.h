/*************************************************************************************
 * Copyright (c) 1997 Cobe BCT, Inc.
 *
 * FILENAME: checkself.h
 * PURPOSE: public header for checkself(), used to verify crc of self
 * CHANGELOG:
 * $Header: I:/BCT_Development/vxWorks/Common/include/rcs/checkself.h 1.3 2003/02/26 23:03:05Z td07711 Exp td07711 $
 * $Log: checkself.h $
 * Revision 1.3  2003/02/26 23:03:05Z  td07711
 * mods for vxworks
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

int checkself( char* textSymbol, char* dataSymbol, const char* outfile, const char* crcfile );

    // PURPOSE:  verifies program loads into memory same way each time, i.e. it is bitwise the same
    // as was validated.  Relies upon being loaded at same address each time and with all external
    // references resolving identically as well, i.e. files defining the externs must also load
    // at same address each time.
    //
    // textSymbol - is the symbol defining the start of the program's text segment - if a NULL ptr, then
    // no checking of text segment is done.
    // dataSymbol - is the symbol defining the start of the program's data segment - if a NULL ptr, then
    // no checking of data segment is done.
    // 
    // outfile - is the full path name to the program's .out file.  Used to get size of text and data segments
    // crcfile - is the full path name to an ASCII file containing the expected CRC to check
    // 
    // if crcfile does not exist, then checkself creates it with the calculated CRC
    // returns -1 if error, 0 if file created OK.
    //
    // if crcfile does exist, then checkself reads the expected CRC and compares it to the calculated one.
    // Returns -1 if expected CRC does not match, 0 if it does match

#endif // _CHECKSELF_HEADER

