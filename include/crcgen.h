/*************************************************************************************
 * Copyright (c) 1997 Cobe BCT, Inc.
 *
 * FILENAME: crcgen.h
 * PURPOSE: function prototype for crcgen32()
 * CHANGELOG:
 *  $Header: //bctquad3/home/BCT_Development/vxWorks/Common/include/rcs/crcgen.h 1.4 2001/08/22 14:33:20 rm70006 Exp ms10234 $
 *  $Log: crcgen.h $
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
int crcgen32(unsigned long* pcrc, char* pdata, long length);


#ifdef __cplusplus
}
#endif

#endif
