/*
 * Copyright 2002 GambroBCT Lakewood, Colorado
 *
 * $Header: //bctquad3/home/BCT_Development/vxWorks/Common/softcrc/rcs/msgcrc.cpp 1.2 2002/07/17 18:54:38 ms10234 Exp $
 * $Log: msgcrc.cpp $
 * Revision 1.2  2002/07/17 18:54:38  ms10234
 * Added msgcrc.cpp and made vxWorks.h the first include file
 * Revision 1.1  1999/05/24 23:30:00  TD10216
 * Initial revision
 *
 * Compute 32 bit CRCs on buffers and focus messages
 *
 */

#include <vxWorks.h>

#include "msgcrc.h"
#include "crcgen.h"

/*
// SPECIFICATION:    compute 32 bit CRC on block "blk_adr" of length "blk_len"
//                   Parameter:
//                   blk_adr - pointer to block
//                   blk_len - length of block
//
// ERROR HANDLING:   none
*/

unsigned long msgcrc32( unsigned char* blk_adr, unsigned long blk_len)
{
   unsigned long crc = INITCRC_DEFAULT;
   crcgen32( &crc, blk_adr, blk_len );
   return crc ^ INITCRC_DEFAULT;
}

/*
// SPECIFICATION:    add to existing 32 bit CRC
//                   Parameter:
//                   blk_adr - pointer to block
//                   blk_len - length of block
//                   crc - existing 32 bit CRC
//
// ERROR HANDLING:   none
*/

unsigned long addToMsgCrc32( unsigned char* blk_adr,
                          unsigned long blk_len,
                          unsigned long crc)
{
   crc = crc ^ INITCRC_DEFAULT;
   crcgen32( &crc, blk_adr, blk_len );
   return (crc ^ INITCRC_DEFAULT);
}

