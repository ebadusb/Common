/*
 * Copyright (C) 2003 Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: K:/BCT_Development/vxWorks/Common/os/rcs/load_module.c 1.2 2003/05/23 16:10:43Z jl11312 Exp jl11312 $
 * $Log: load_module.c $
 */

#include <a_out.h>
#include <loadLib.h>
#include "load_module.h"

static void * allocDataSegment(unsigned long size)
{
	void	* pSeg = (void *)LD_NO_ADDRESS;

	if ( size > 0 )
	{
		/*
		 *	Allocate an extra page after the data segment and mark it as not present
		 * to aid checking for data overwrites.
		 */
		unsigned long	paddedSize = (size + 0xfff) & 0xfffff000;
		pSeg = valloc(paddedSize+0x1000);

		if ( pSeg )
		{
			unsigned long	endPage = ((unsigned long)pSeg) + paddedSize;
			vmBaseStateSet(NULL, (void *)endPage, 0x1000, VM_STATE_MASK_VALID, VM_STATE_VALID_NOT);

			/*
			 *	Fill the segment with a known value.  To better detect memory overwrites,
			 * align the segment at the end of the page.
			 */
			memset(pSeg, 0xcc, paddedSize);
			pSeg = (void *)( ((unsigned long)pSeg) + paddedSize - size );
		}
	}

	return pSeg;
}

STATUS loadModuleFromFile(const char * fileName)
{
   int      loadFD = open(fileName, O_RDONLY, DEFAULT_FILE_PERM);
   STATUS   status = (loadFD >= 0) ? OK : ERROR;

	unsigned long	textSize = 0;
	unsigned long	dataSize = 0;
	unsigned long	bssSize = 0;
	void * pText = NULL;
	void * pData = NULL;
	void * pBSS = NULL;

	/*
	 *	Read file header and allocate space for module segments
	 */
	if ( status == OK )
	{
		struct exec aoutHeader;
		if ( read(loadFD, (char *)&aoutHeader, sizeof(aoutHeader)) != sizeof(aoutHeader) )
		{
			fprintf(stderr, "\"%s\": failed to read header\n", fileName);
			status = ERROR;
		}
		else
		{
			/*
			 *	Round text size up to an integer number of pages and allocate on a page boundary
			 * so that it can be protected against writes.
			 */
			textSize = (aoutHeader.a_text + 0xfff) & 0xfffff000;
			pText = valloc(textSize);

			/*
			 *	Data and BSS are also rounded up to an integer number of pages.
			 */
			dataSize = aoutHeader.a_data;
			pData = allocDataSegment(dataSize);

			bssSize = aoutHeader.a_bss;
			pBSS = allocDataSegment(bssSize);

			if ( !pText ||
				  (dataSize > 0 && !pData) ||
				  (bssSize > 0 && !pBSS) )
			{
				fprintf(stderr, "\"%s\": failed to allocate memory for module (text=%lu data=%lu bss=%lu)\n",
									fileName, aoutHeader.a_text, aoutHeader.a_data, aoutHeader.a_bss);
				status = ERROR;  
			}

		}
	}

	if ( status == OK )
	{
		lseek(loadFD, 0, SEEK_SET);
		if ( loadModuleAt(loadFD, LOAD_GLOBAL_SYMBOLS, (char **)&pText, (char **)&pData, (char **)&pBSS) == NULL )
		{
			fprintf(stderr, "\"%s\": failed to load module\n", fileName);
			status = ERROR;
		}
		else
		{
			/*
			 *	Protect text segment against writes
			 */
			fprintf(stderr, "\"%s\": loaded T=0x%08lx(0x%08lx) D=0x%08lx(0x%08lx) B=0x%0xlx(0x%08lx)\n",
						fileName, (unsigned long)pText, textSize, (unsigned long)pData, dataSize, (unsigned long)pBSS, bssSize);
			vmBaseStateSet(0, pText, textSize, VM_STATE_MASK_WRITABLE, VM_STATE_WRITABLE_NOT);
		}
	}

	if ( loadFD >= 0 ) close(loadFD);
	if ( status != OK )
	{
		if ( pText) free(pText);
		if ( pData && pData != LD_NO_ADDRESS ) free(pData);
		if ( pBSS && pBSS != LD_NO_ADDRESS ) free(pBSS);
	}

	return status;
}
