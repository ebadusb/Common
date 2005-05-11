// $Header: K:/BCT_Development/Taos/tools/config_parse/rcs/crc.h 1.1 2005/01/19 17:56:24Z jl11312 Exp jl11312 $
//
// CRC generation function
//
// $Log: crc.h $
// Revision 1.1  2005/01/19 17:56:24Z  jl11312
// Initial revision
//

#ifndef _CRC_INCLUDE
#define _CRC_INCLUDE

void crcgen32(unsigned long * pcrc, const unsigned char * pdata, long length);

#endif /* ifndef _CRC_INCLUDE */

