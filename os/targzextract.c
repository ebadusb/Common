//******************************************************************************
//
// This file was derived from the vxWorks tarLib.c.  The original tarLib.c
// file exctracted and archived file system directories.  This version only
// deals with the extraction of files and directories.  The format of the 
// archive is a 'zlib' compressed 'tar' archive of files and directories.
//
//******************************************************************************

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
/* tarLib.c - UNIX tar compatible library */

/* Copyright 1993-1999 Wind River Systems, Inc. */
/* Copyright (c) 1993, 1994 RST Software Industries Ltd. */

/*
modification history
--------------------
03g,11nov99,jkf  need to fill all eight bytes with 0x20 for the checksum 
                 calc correct for MKS toolkit 6.2 generated tar file.
03f,31jul99,jkf  T2 merge, tidiness & spelling.
03e,30jul98,lrn  partial doc cleanup
03b,24jun98,lrn  fixed bug causing 0-size files to be extracted as dirs,
       improved tarHelp to list parameters
03a,07jun98,lrn  derived from RST usrTapeLib.c, cleaned all tape related stuff
02f,15jan95,rst  adding TarToc functionality
02d,28mar94,rst  added Tar utilities.
*/

/*
DESCRIPTION

This library implements functions for archiving, extracting and listing
of UNIX-compatible "tar" file archives.
It can be used to archive and extract entire file hierarchies
to/from archive files on local or remote disks, or directly to/from
magnetic tapes.

SEE ALSO: dosFsLib

CURRENT LIMITATIONS
This Tar utility does not handle MS-DOS file attributes,
when used in conjunction with the MS-DOS file system.
The maximum subdirectory depth supported by this library is 16,
while the total maximum path name that can be handled by 
tar is limited at 100 characters.
*/

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


#include <vxWorks.h>
#include <taskLib.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>

#include <zlib.h>

/* data types */
/*
 * VxWorks Tar Utility, part of usrTapeLib (for now).
 * UNIX tar(1) tape format - header definition
 */

#define TBLOCK		512	/* TAR Tape block size, part of TAR format */
#define NAMSIZ		100	/* size of file name in TAR tape header */
#define	MAXLEVEL	16	/* max. # of subdirectory levels, arbitrary */
#define	NULLDEV		"/null"	/* data sink */

#ifdef __cplusplus
extern "C" { 
#endif

int tarExtract ( const char *file     /* archive file name */, 
                 const char *location /* location for extraction */ );

#ifdef __cplusplus
};
#endif

typedef union hblock
{
   char dummy[TBLOCK];
   struct header
   {
      char name[NAMSIZ];
      char mode[8];
      char uid[8];
      char gid[8];
      char size[12];
      char mtime[12];
      char chksum[8];
      char linkflag;
      char linkname[NAMSIZ];
   }
   dbuf;
} MT_HBLOCK ;

/* Control structure used internally for reentrancy etc. */
typedef struct
{
   gzFile  fd;        /* current file descriptor */
   int     bFactor;    /* current blocking factor */
   int     bValid;        /* number of valid blocks in buffer */
   int     recurseCnt;    /* recusrion counter */
   MT_HBLOCK *   pBuf;       /* working buffer */
   MT_HBLOCK *   pBnext;        /* ptr to next block to return */
} MT_TAR_SOFT ;


/* locals */
char  bZero[ TBLOCK ] ; /* zeroed block */

/*******************************************************************************
*
* mtChecksum - calculate checksums for tar header blocks
*
* RETURNS: the checksum value
*/

int mtChecksum ( void *  pBuf, unsigned   size )
{
   register int sum = 0 ;
   register unsigned char *p = (unsigned char*)pBuf ;

   while ( size > 0 )
   {
      sum += *p++;
      size -= sizeof(*p);
   }

   return(sum);
}

/*******************************************************************************
*
* tarRdBlks - read <nBlocks> blocks from tar file
*
* RETURNS: number of blocks actually got, or ERROR.
*/

int tarRdBlks ( MT_TAR_SOFT *pCtrl,    /* control structure */ 
                MT_HBLOCK **ppBlk,     /* where to return buffer address */ 
                unsigned int nBlocks,  /* how many blocks to get */ 
                const bool datalogRunning
              )
{
   register int rc ;

   if ( pCtrl->bValid <= 0 )
   {
      /* buffer empty, read more blocks from file */

      rc = gzread( pCtrl->fd, pCtrl->pBuf->dummy, pCtrl->bFactor * TBLOCK);

      if ( rc == ERROR )
         return ERROR ;
      else if ( rc == 0 )
         return OK ;
      else if ( (rc % TBLOCK) != 0 )
      {
         fprintf( stdout, "tarRdBlks: file block not multiple of %d\n", TBLOCK);
         if ( datalogRunning ) fprintf( stderr, "tarRdBlks: file block not multiple of %d\n", TBLOCK);
         return ERROR;
      }


      pCtrl->bValid = rc / TBLOCK ;
      pCtrl->pBnext = pCtrl->pBuf ;
   }

   rc = min((unsigned int)pCtrl->bValid, nBlocks) ;
   *ppBlk = pCtrl->pBnext ;
   pCtrl->bValid -= rc ;
   pCtrl->pBnext += rc ;

   return( rc ) ;
}

/*******************************************************************************
*
* mtAccess - check existence of path for a new file or directory <name>
*
* RETURNS: OK or ERROR
*/

int mtAccess ( const char *name, const bool datalogRunning  )
{
   char tmpName [ NAMSIZ ] ;
   struct stat st ;
   int i ;
   static char slash = '/' ;
   register char *pSlash ;

   strncpy( tmpName, name, NAMSIZ ) ;
   pSlash = tmpName ;

   for ( i=0; i<MAXLEVEL; i++ )
   {
      if ( (pSlash = strchr(pSlash, slash)) == NULL )
         return OK;

      *pSlash = '\0' ;

      if ( stat( tmpName, &st ) == 0 )
      {
         if ( S_ISDIR(st.st_mode ) == 0 )
         {
            fprintf( stdout, "Path %s is not a directory\n", tmpName);
            if ( datalogRunning ) fprintf( stderr, "Path %s is not a directory\n", tmpName);
            return ERROR;
         }
      }
      else
      {
         mkdir( tmpName );
      }

      *pSlash = slash ; /* restore slash position */
      pSlash++;
   }
   fprintf( stdout, "Path too long %s\n", name );
   if ( datalogRunning ) fprintf( stderr, "Path too long %s\n", name );
   return ERROR;
}

/*******************************************************************************
*
* tarExtractFile - extract one file or directory from tar file
*
* Called from tarExtract for every file/dir found in archive file.
*
* RETURNS: OK or ERROR
*/

int tarExtractFile ( MT_TAR_SOFT   *pCtrl,     /* control structure */ 
                     MT_HBLOCK     *pBlk,      /* header block */ 
                     const char    *location,
                     const bool    datalogRunning 
                   )
{
   register int   rc;
   FILE *  fd;
   int     sum = ERROR ;  /* checksum */
   int     size = 0 ;     /* file size in bytes */
   int     nblks = 0;     /* file size in TBLOCKs */
   int     mode ;
   char    fn[ NAMSIZ*2 ]; /* file/dir name */

   /* Put the destination location in front of our file name */
   memset( fn, 0, NAMSIZ*2 );
   if ( location )
   {
      strcpy( fn, location );
      strcat( fn, "/" );
   }


   /* Check the checksum of this header */

   rc = sscanf( pBlk->dbuf.chksum, "%o", &sum ) ; /* extract checksum */

   memset( pBlk->dbuf.chksum, 0x20, 8 );

   if ( mtChecksum( pBlk->dummy, TBLOCK ) != sum )
   {
      fprintf( stdout, "bad checksum %d != %d\n", mtChecksum( pBlk->dummy, TBLOCK), sum );
      if ( datalogRunning ) fprintf( stderr, "bad checksum %d != %d\n", mtChecksum( pBlk->dummy, TBLOCK), sum );
      return ERROR;
   }

   /* Parse all fields of header that we need, and store them safely */
   sscanf( pBlk->dbuf.mode, "%o", &mode );
   sscanf( pBlk->dbuf.size, "%12o", &size );
   strcat( fn, pBlk->dbuf.name );

   /* Handle directory */
   if ( (size == 0) && ( fn[ strlen(fn) - 1 ] == '/' ) )
   {
      if ( strcmp(fn, "./") == 0 )
         return OK;

      if ( fn[ strlen(fn) - 1 ] == '/' )  /* cut the slash */
         fn[ strlen(fn) - 1 ] = '\0' ;

      /* Must make sure that parent exists for this new directory */
      if ( mtAccess(fn, datalogRunning ) == ERROR )
      {
         return ERROR;
      }

      if ( mkdir( fn ) == ERROR )
      {
         fprintf( stdout, "failed to create directory %s, %s\n", fn, strerror(errno));
         if ( datalogRunning ) fprintf( stderr, "failed to create directory %s, %s\n", fn, strerror(errno));
         return ERROR;
      }

      fprintf( stdout, "created directory %s.\n", fn );
      return OK;
   }

   /* non-empty file has a trailing slash, we better treat it as file and */
   if ( fn[ strlen(fn) - 1 ] == '/' ) /* cut a trailing slash */
      fn[ strlen(fn) - 1 ] = '\0' ;

   /* Filter out links etc */

   if ( (pBlk->dbuf.linkflag != '\0') &&
        (pBlk->dbuf.linkflag != '0') &&
        (pBlk->dbuf.linkflag != ' ') )
   {
      fprintf( stdout, "we do not support links, %s\n", fn );
      if ( datalogRunning ) fprintf( stderr, "we do not support links, %s\n", fn );
      return ERROR;
   }

   /* Handle Regular File - calculate number of blocks */
   if ( size > 0 )
      nblks = ( size / TBLOCK ) +  ((size % TBLOCK)? 1 : 0 ) ;

   /* Must make sure that directory exists for this new file */
   if ( mtAccess(fn, datalogRunning ) == ERROR )
   {
      return ERROR;
   }

   /* Create file */
   fd = fopen( fn, "w+" ) ;
   if ( fd == NULL )
   {
      fprintf( stdout, "failed to create file %s, %s -- exiting!\n", fn, strerror(errno));
      if ( datalogRunning ) fprintf( stderr, "failed to create file %s, %s -- exiting!\n", fn, strerror(errno));
      return ERROR;
   }

   fprintf( stdout, "extracting file %s, size %d bytes, %d blocks\n", fn, size, nblks );
   if ( datalogRunning ) fprintf( stderr, "extracting file %s, size %d bytes, %d blocks\n", fn, size, nblks );

   /* Loop until entire file extracted */
   while ( size > 0 )
   {
      MT_HBLOCK *pBuf;
      register int wc ;

      rc = tarRdBlks( pCtrl, &pBuf, nblks, datalogRunning ) ;
      fprintf( stdout, "\ttarExtract: bytes remaining->%d\r", size);

      if ( rc < 0 )
      {
         fprintf( stdout, "error reading archive file\n");
         if ( datalogRunning ) fprintf( stderr, "error reading archive file\n");
         fclose(fd);
         return ERROR;
      }

      wc = fwrite( pBuf->dummy, sizeof(char), min( rc*TBLOCK, size ), fd ) ;

      if ( wc == ERROR )
      {
         fprintf( stdout, "error writing file\n");
         if ( datalogRunning ) fprintf( stderr, "error writing file\n");
         break;
      }

      size -= rc*TBLOCK ;
      nblks -= rc ;
   }


   /* Close the newly created file */
   return( fclose(fd) ) ;

}

/*******************************************************************************
*
* tarExtract - extract all files from a tar formatted file
*
* This is a UNIX-tar compatible utility that extracts entire
* file hierarchies from tar-formatted archive.
* The files are extracted with their original names and modes.
* In some cases a file cannot be created on disk, for example if
* the name is too long for regular DOS file name conventions,
* in such cases entire files are skipped, and this program will
* continue with the next file. Directories are created in order
* to be able to create all files in the archive file.
*
* The <file> argument may be any file name that contains a tar 
* formatted archive. 
*
* The <bfactor> dictates the blocking factor the file was written with.
*
* All informative and error message are printed to standard error.
*
* There is no way to selectively extract tar archives with this
* utility. It extracts entire archives.
*/

int tarExtract ( const char *file     /* archive file name */, 
                 const char *location /* location for extraction */ )
{
   register int   rc ;     /* return codes */
   MT_TAR_SOFT   ctrl ;      /* control structure */
   int  retval = 0;
   int bfactor = 20 ;

   bool datalogRunning = false;
   if ( taskNameToId( "dlog_out" ) != ERROR ) {  datalogRunning = true; }

   fprintf( stdout, "Extracting from file %s\n", file );
   if ( datalogRunning ) fprintf( stderr, "Extracting from file %s\n", file );

   memset( &ctrl, 0, sizeof(ctrl) );
   memset( bZero, 0, sizeof(bZero) );

   /* Open file and initialize control structure */
   ctrl.fd = gzopen( file, "r" );

   if ( ctrl.fd < 0 )
   {
      fprintf( stdout, "Failed to open file: %s\n", strerror(errno) );
      if ( datalogRunning ) fprintf( stderr, "Failed to open file: %s\n", strerror(errno) );
      return ERROR;
   }

   ctrl.bFactor = bfactor ;
   ctrl.pBuf = (MT_HBLOCK*)malloc( bfactor * TBLOCK ) ;
   if ( ctrl.pBuf == NULL )
   {
      fprintf( stdout, "Not enough memory, exiting.\n" );
      if ( datalogRunning ) fprintf( stderr, "Not enough memory, exiting.\n" );
      gzclose( ctrl.fd );
      return ERROR ;
   }
   memset( ctrl.pBuf, 0, bfactor*TBLOCK );

   /* all exits from now via goto in order to free the buffer */

   /* Read the first block and adjust blocking factor */

   rc = gzread( ctrl.fd, ctrl.pBuf->dummy, ctrl.bFactor*TBLOCK ) ;

   if ( rc == ERROR )
   {
      fprintf( stdout, "read error at the beginning of file, exiting.\n" );
      if ( datalogRunning ) fprintf( stderr, "read error at the beginning of file, exiting.\n" );
      retval = ERROR ;
      goto finish;
   }
   else if ( rc == 0 )
   {
      fprintf( stdout, "empty file, exiting.\n" );
      if ( datalogRunning ) fprintf( stderr, "empty file, exiting.\n" );
      goto finish;
   }
   else if ( (rc % TBLOCK) != 0 )
   {
      fprintf( stdout, "file block not multiple of %d, exiting.\n", TBLOCK);
      if ( datalogRunning ) fprintf( stderr, "file block not multiple of %d, exiting.\n", TBLOCK);
      retval = ERROR ;
      goto finish;
   }

   ctrl.bValid = rc / TBLOCK ;
   ctrl.pBnext = ctrl.pBuf ;
   if ( ctrl.bFactor != ctrl.bValid )
   {
      fprintf( stdout, "adjusting blocking factor to %d\n", ctrl.bValid );
      if ( datalogRunning ) fprintf( stderr, "adjusting blocking factor to %d\n", ctrl.bValid );
      ctrl.bFactor = ctrl.bValid ;
   }

   /* End of overture, start processing files until end of file */

   for (;;)
   {
      MT_HBLOCK * pBlk ;

      if ( tarRdBlks( &ctrl, &pBlk, 1, datalogRunning ) != 1 )
      {
         retval = ERROR ;
         goto finish;
      }

      if ( bcmp( pBlk->dummy, bZero, TBLOCK) == 0 )
      {
         fprintf( stdout, "end of file encountered, read until eof...\n");
         if ( datalogRunning ) fprintf( stderr, "end of file encountered, read until eof...\n");
         while ( tarRdBlks( &ctrl, &pBlk, 1, datalogRunning ) > 0 ) ;
         fprintf( stdout, "done.\n");
         if ( datalogRunning ) fprintf( stderr, "done.\n");
         retval = 0 ;
         goto finish;
      }

      if ( tarExtractFile( &ctrl, pBlk, location, datalogRunning ) == ERROR )
      {
         retval = ERROR;
         goto finish;
      }

   } /* end of FOREVER */

   finish:
   gzclose( ctrl.fd );
   free( ctrl.pBuf );
   return( retval );
}

