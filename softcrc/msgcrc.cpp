/*
 * copyright 1995, 1996 Cobe BCT Lakewood, Colorado
 *
 * $Header: //bctquad3/home/BCT_Development/vxWorks/Common/softcrc/rcs/msgcrc.cpp 1.1 2002/05/01 14:45:54 ms10234 Exp ms10234 $
 * $Log: msgcrc.cpp $
 * Revision 1.1  1999/05/24 23:30:00  TD10216
 * Initial revision
 * Revision 1.2  1996/07/24 19:49:20  SS03309
 * fix MKS 
 * Revision 1.2  1996/07/22 14:42:47  SS03309
 * Revision 1.4  1996/07/19 13:53:27  SS03309
 * lint
 *
 * Compute 32 bit CRCs on buffers and focus messages
 *
 * DOCUMENTS
 * Requirements:     I:\ieee1498\SSS2.DOC
 * Select OMT:       I:\router
 * Test:             I:\ieee1498\STP2.DOC
 */

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

unsigned long crc32( unsigned char* blk_adr, unsigned long blk_len)
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

unsigned long addToCrc32( unsigned char* blk_adr,
                          unsigned long blk_len,
                          unsigned long crc)
{
   crc = crc ^ INITCRC_DEFAULT;
   crcgen32( &crc, blk_adr, blk_len );
   return (crc ^ INITCRC_DEFAULT);
}

