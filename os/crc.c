/*
 * Copyright (C) 2003 Gambro BCT, Inc.  All rights reserved.
 *
 * $Header$
 * $Log$
 *
 */

#include <vxworks.h>
#include "crcgen.h"
#include "crc_table.h"

// SPECIFICATION: crcgen32()
//   compute 32 bit CRC over a data buffer.
//   overwrites initial crc with the new crc.
// ERROR HANDLING:  returns -1 if arg error, else returns 0
//
int crcgen32(unsigned long * pcrc, const unsigned char * pdata, long length)
{
	int	result = -1;
	__asm__ volatile
	(
		"movl %0, %%ebx\n"			// ebx = pcrc
		"orl %%ebx, %%ebx\n"
		"jz crcgen32_done\n"
		"movl (%%ebx), %%ebx\n"		// ebx = *pcrc

		"movl %1, %%esi\n"			// esi = pdata
		"orl %%esi, %%esi\n"
		"jz crcgen32_done\n"

		"movl $0, %4\n"				// result = 0
		"movl %2, %%ecx\n"			// ecx = length
		"movl $%3, %%edi\n"			// edi = crctable

	"crcgen32_loop:\n"
		"decl %%ecx\n"					// while (length--)
		"jl crcgen32_done\n"

		"xorl %%eax, %%eax\n"		// eax = *pdata++
		"movb (%%esi), %%al\n"
		"incl %%esi\n"

		"movl %%ebx, %%edx\n" 		// eax = crctable[(*pcrc ^ *pdata) & 0xff]
		"xorb %%dl, %%al\n"
		"movl (%%edi,%%eax,4), %%eax\n"

		"shrl $8, %%edx\n"			// edx = *pcrc >> 8

		"xorl %%eax, %%edx\n"		// ebx = crctable[(*pcrc ^ *pdata) & 0xff] ^ (*pcrc >> 8)
		"movl %%edx, %%ebx\n"
		"jmp crcgen32_loop\n"

	"crcgen32_done:\n"
		"movl %0, %%eax\n"			
		"movl %%ebx, (%%eax)"		// *pcrc = ebx
		
			: "=m" (pcrc), "=m" (pdata), "=m" (length), "=m" (crctable)
			: "m" (result)
			: "eax", "ebx", "ecx", "edx", "edi", "esi"
   );

   return result;
}

