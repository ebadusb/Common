/*************************************************************************************
 * Copyright (c) 1997 Cobe BCT, Inc.
 *
 * FILENAME: crcgen.h
 * PURPOSE: function prototype for crcgen32()
 * CHANGELOG:
 *  $Header: K:/BCT_Development/vxWorks/Common/include/rcs/crcgen.h 1.7 2003/05/21 20:01:26Z jl11312 Exp jl11312 $
 *  $Log: crcgen.h $
 *  Revision 1.6  2002/05/09 19:44:55Z  ms10234
 *  changed data from const char to const unsigned char
 *  Revision 1.4  2001/08/22 14:33:20  rm70006
 *  IT 4766.
 *  Moved calculate crc function by file function here from GUI.
 *  Revision 1.3  2000/06/07 22:51:22  bd10648
 *  Changed seed to 0xFFFFFFFF so that a block of zeros
 *  gets a non-zero crc.
 *  Revision 1.2  2000/06/07 22:04:21  bd10648
 *  Moved CRC seed into this header file from softcrc.h
 *  Revision 1.1  1999/05/24 23:26:24  TD10216
 *  Initial revision
 *  Revision 1.3  1998/08/18 23:00:39  TD07711
 *    make header file c++ compatible
 *  8/18/98 - dyes - make header file c++ portable
 *  5/19/97 - dyes - added usage comments
 *  4/30/97 - dyes
 *************************************************************************************/

#ifndef _CRCGEN_HEADER
#define _CRCGEN_HEADER


#ifdef __cplusplus
extern "C" {
#endif

/*
// crcgen32() generates a 32bit CRC over the input data
// inputs:
//    pcrc - ptr to initial crc value, the crc value is modified as each byte of input data
//           is read.
//    pdata - ptr to data buffer over which crc is calculated
//    length - number of bytes in the data buffer over which crc is calculated
// outputs:
//    *pcrc is updated with new crc value
//    returns 0 if success
//    returns -1 if invalid argument (NULL ptr or length < 0)
*/

int crcgen32(unsigned long* pcrc, const unsigned char* pdata, long length);

/*
// file_crcgen32() generates a 32bit CRC over the contents of a file.
// inputs:
//    filename - filename (plus path) to be crc'ed.  file must obviously exist.
//    pcrc - ptr to initial crc value, the crc value is modified as each byte of input data
//           is read.
// outputs:
//    *pcrc is updated with new crc value
//    returns 0 if success
//    returns -1 if failed
*/
int file_crcgen32 (const char *filename, unsigned long *pcrc);

/*
 *	softcrc() provides a command line interface for performing CRC checks
 * over lists of files.  See softcrc.cpp for usage information.
 */
int softcrc(const char * cmdLine);

/*// common seed for all users to get common results.*/
#define INITCRC_DEFAULT (0xFFFFFFFFL)


#ifdef __cplusplus
}
#endif

#endif
