/*
 * copyright 1995, 1996 Cobe BCT Lakewood, Colorado
 *
 * $Header: //bctquad3/home/BCT_Development/vxWorks/Common/softcrc/rcs/msgcrc.cpp 1.2 2002/07/17 18:54:38 ms10234 Exp ms10234 $
 * $Log: msgcrc.cpp $
 * Revision 1.3  2002/07/17 18:43:55  ms10234
 * Added vxWorks.h as the first include file
 * Revision 1.2  2002/07/03 20:10:36  ms10234
 * Completed some code cleanup and minor fixes.
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

