/*
 * Copyright (C) 2003 Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: //bctquad3/home/BCT_Development/vxWorks/Common/os/rcs/readdir_stat.c 1.3 2003/11/05 19:28:02Z jl11312 Exp rm70006 $
 * $Log: readdir_stat.c $
 * Revision 1.3  2003/11/05 19:28:02Z  jl11312
 * - corrected handling of end of directory condition
 * Revision 1.2  2003/09/16 22:17:50Z  jl11312
 * - corrected time stamp (IT 6422)
 * Revision 1.1  2003/09/05 21:37:52Z  jl11312
 * Initial revision
 *
 */

#include <vxworks.h>
#include <private/dosFsLibP.h>
#include <private/iosLibP.h>
#include <string.h>

#include "readdir_stat.h"

struct dirent * readdir_stat(DIR * pDir, struct stat * pStat)
{
	DOS_FILE_DESC_ID pFd = (DOS_FILE_DESC_ID)fdTable[pDir->dd_fd-3].value;
	DOS_VOLUME_DESC_ID pVolDesc = pFd->pVolDesc;
   DOS_DIR_DESC_ID pDirDesc = pVolDesc->pDirDesc;

	DOS_FILE_DESC	resFd;
	DOS_FILE_HDL	resHdl;

	STATUS status;
	resFd.pFileHdl = &resHdl;
	resFd.pVolDesc = pFd->pVolDesc;
	status = (*pDirDesc->readDir)(pFd, pDir, &resFd);

   bzero ((char *) pStat, sizeof (struct stat));
	if ( status == OK )
	{
		/* Fill in modified date and time */
		pVolDesc->pDirDesc->dateGet( &resFd, pStat );
	
		pStat->st_dev = (u_long) pVolDesc; /* device ID = DEV_HDR addr */
		pStat->st_nlink = 1;		/* always only one link */
		pStat->st_size = resFd.pFileHdl->size;	/* file size, in bytes */
		pStat->st_blksize = pVolDesc->secPerClust << pVolDesc->secSizeShift; /* block = cluster */
		pStat->st_blocks = ( pStat->st_size + pStat->st_blksize ) / pStat->st_blksize;	/* no. of blocks */
		pStat->st_attrib = resFd.pFileHdl->attrib;	/* file attribute byte */
		 
		/* Set file type in mode field */
		pStat->st_mode = S_IRWXU | S_IRWXG | S_IRWXO;
		if( ( resFd.pFileHdl->attrib & DOS_ATTR_RDONLY ) != 0 )
			pStat->st_mode &= ~( S_IWUSR | S_IWGRP | S_IWOTH );
		 
		if ( resFd.pFileHdl->attrib & DOS_ATTR_DIRECTORY )
		{
			pStat->st_mode |= S_IFDIR;	/* set bits in mode field */
			pStat->st_size = pStat->st_blksize; /* make it look like dir uses one block */
		}
		else  /* if not a volume label */
		{
			pStat->st_mode |= S_IFREG;	/*  it is a regular file */
		}
	}
    
   return (status == OK) ? &pDir->dd_dirent : NULL;
}

