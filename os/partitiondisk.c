/*
** This file modified from the Windriver source file usrFdiskPartLib.c
**
**  The changes allow a disk to be partitioned using specific sizes for
** each partition.  The usrFdiskPartCreate function creates partitions
** using percentages of the disk.  The function in this file, partitionDisk,
** takes parameters for the size of the first three partitions, in Mbs.  
** the fourth partition will take the remainder of the disk space.  This
** allows for the creation of disk partitions samller than 1% of the disk
** size.
*/

/* includes */

#include "vxWorks.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "dosFsLib.h"
#include "private/dosFsLibP.h"	/* for byte swapping macros */
#include "dpartCbio.h"

/* defines */

/* partition table structure offsets */
#define PART_SIG_ADRS           0x1fe   /* dos partition signature  */
#define PART_SIG_MSB            0x55    /* msb of the partition sig */
#define PART_SIG_LSB            0xaa    /* lsb of the partition sig */
#define PART_IS_BOOTABLE        0x80    /* a dos bootable partition */
#define PART_NOT_BOOTABLE       0x00    /* not a bootable partition */
#define PART_TYPE_DOS4          0x06    /* dos 16b FAT, 32b secnum  */
#define PART_TYPE_DOSEXT        0x05    /* msdos extended partition */
#define PART_TYPE_DOS3          0x04    /* dos 16b FAT, 16b secnum  */
#define PART_TYPE_DOS12         0x01    /* dos 12b FAT, 32b secnum  */
#define PART_TYPE_DOS32         0x0b    /* dos 32b FAT, 32b secnum  */
#define PART_TYPE_DOS32X        0x0c    /* dos 32b FAT, 32b secnum  */
#define PART_TYPE_WIN95_D4      0x0e    /* Win95 dosfs  16bf 32bs   */
#define PART_TYPE_WIN95_EXT     0x0f    /* Win95 extended partition */

#define BOOT_TYPE_OFFSET    0x0   /* boot type                      */
#define STARTSEC_HD_OFFSET  0x1   /* beginning sector head value    */
#define STARTSEC_SEC_OFFSET 0x2   /* beginning sector               */
#define STARTSEC_CYL_OFFSET 0x3   /* beginning cylinder             */
#define SYSTYPE_OFFSET      0x4   /* system indicator               */
#define ENDSEC_HD_OFFSET    0x5   /* ending sector head value       */
#define ENDSEC_SEC_OFFSET   0x6   /* ending sector                  */
#define ENDSEC_CYL_OFFSET   0x7   /* ending cylinder                */
#define NSECTORS_OFFSET     0x8   /* sector offset from reference   */
#define NSECTORS_TOTAL      0xc   /* number of sectors in part      */


STATUS partitionDisk
(
CBIO_DEV_ID dev, /* device representing the entire disk */
int     nPart,   /* how many partitions needed, default=1, max=4 */
int     size1,   /* space (in Mbs) for first partition */
int     size2,   /* space (in Mbs) for second partition */
int     size3    /* space (in Mbs) for third partition */
);


/*****************************************************************************
*
* partitionDisk - create an FDISK-like partition table on a disk
*
* This function may be used to create a basic PC partition table.
* Such partition table however is not intended to be compatible with
* other operating systems, it is intended for disks connected to a
* VxWorks target, but without the access to a PC which may be used to
* create the partition table.
*
* This function is capable of creating only one partition table - the
* MBR, and will not create any Bootable or Extended partitions.
* Therefore, 4 partitions are supported.
*
* <dev> is a CBIO device handle for an entire disk, e.g. a handle
* returned by dcacheDevCreate(), or if dpartCbio is used, it can be either
* the Master partition manager handle, or the one of the 0th partition
* if the disk does not contain a partition table at all.
*
* The <nPart> argument contains the number of partitions to create. If
* <nPart> is 0 or 1, then a single partition covering the entire disk is
* created.
* If <nPart> is between 2 and 4, then the arguments <size1>, <size2>
* and <size3> contain the size ( in Megabytes ) 
* of disk space to be assigned to the 1st, 2nd, and 3rd partitions
* respectively. The last partition (partition 3) will be assigned the
* remainder of space left (space hog).
*
* Partition sizes will be round down to be multiple of whole tracks
* so that partition Cylinder/Head/Track fields will be initialized 
* as well as the LBA fields. Although the CHS fields are written they
* are not used in VxWorks, and can not be guaranteed to work correctly
* on other systems.
*
* RETURNS: OK or ERROR writing a partition table to disk
*
*/
STATUS partitionDisk
(
CBIO_DEV_ID dev, /* device representing the entire disk */
int     nPart,   /* how many partitions needed, default=1, max=4 */
int     size1,   /* space (in Mbs) for first partition */
int     size2,   /* space (in Mbs) for second partition */
int     size3    /* space (in Mbs) for third partition */
)
{
   PART_TABLE_ENTRY partTbl[4];
   int totalSecs, trackSecs, cylSecs, totalTracks, i;
   int blockBytes;
   int size[ 3 ]; /* Only need to save the 3 given partition sizes.  the 4th is calculated to be the rest of the disk */
   int noTracks;
   caddr_t secBuf = NULL ;
   const char dummyString[] =
   "Wind River Systems Inc., DosFs 2.0 Partition Table";
   STATUS stat = OK;

   CBIO_PARAMS cbioParams;

   size[0] = size1;
   size[1] = size2;
   size[2] = size3;

   bzero((caddr_t) partTbl, sizeof(partTbl));

   /* Verify the device handle, possibly create wrapper */

   if ( cbioDevVerify(dev) == ERROR )
   {
      return(ERROR);
   }

   /* if first time usage, a RESET may be needed on the device */

   if ( cbioIoctl( dev, CBIO_RESET, 0) == ERROR )
   {
      printErr ("usrFdiskPartCreate error: device is not ready\n");
      return(ERROR);
   }

   /* Get CBIO device parameters */

   if ( ERROR == cbioParamsGet (dev, &cbioParams) )
   {
      printErr ("usrFdiskPartCreate error: cbioParamsGet returned error\n");
      return(ERROR);
   }

   totalSecs = cbioParams.nBlocks ;
   trackSecs = cbioParams.blocksPerTrack;
   blockBytes = cbioParams.bytesPerBlk;

   if ( trackSecs < 1 )
      trackSecs = 1;

   cylSecs = trackSecs * cbioParams.nHeads ;

   if ( totalSecs < trackSecs * 4 )
   {
      printErr ("usrFdiskPartCreate error: disk too small %d blocks\n",
                totalSecs);
      errno = EINVAL;
      return(ERROR);
   }

   /* also, part table CHS fields have certain limitations for CHS values */

   if ( trackSecs < 1 || trackSecs > 63 )
   {
      trackSecs = 64 ;
      cylSecs = trackSecs * cbioParams.nHeads ;
   }

   if ( cylSecs < 1 )
      cylSecs = trackSecs ;

   while ( (totalSecs/cylSecs) > 1023 )
      cylSecs = cylSecs << 1 ;

   /* rest of calculation made in tracks, round, less chance of overflowing */

   totalTracks = totalSecs / trackSecs ;

#ifdef	DEBUG
   printErr( "  totalTracks %d, trackSecs %d, cylSecs %d\n",
             totalTracks, trackSecs, cylSecs );
#endif

   /* reserve one track for MBR */
   totalTracks = totalTracks - 1 ;

   /* initialize the tracks to 0 */
   for ( i=0 ; i<4 ; i++ )
      partTbl[ i ].spare = 0;

   if ( nPart == 0 || nPart == 1 )
   {
      partTbl[ 0 ].spare = totalTracks;
      totalTracks = 0;
   }

   for ( i=0 ; ( i<3 && totalTracks>0 && i<nPart-1 ) ; i++ )
   {
      noTracks = ( ( size[ i ]*( 1024*1024 /* 1Mb */ ) / blockBytes ) / trackSecs );
      if ( size[ i ] == 0 || noTracks > totalTracks )
         noTracks = totalTracks;
      totalTracks -= noTracks;
      partTbl[ i ].spare = noTracks;
   }

   if ( nPart > 3 )
      partTbl[ 3 ].spare = totalTracks;


   /* normalize the entire partition table, calc offset etc.*/
   for ( i=0, totalTracks = 1; i<4; i++ )
   {
      if ( partTbl[i].spare == 0 )
         continue ;

      partTbl[i].offset = totalTracks * trackSecs ;
      partTbl[i].nBlocks = partTbl[i].spare * trackSecs ;
      totalTracks += partTbl[i].spare ;

      /*
       * If the partition is greater than or equal to 2GB,
       * use FAT32x partition type.  Else if the partition 
       * is greater than or equal to 65536, use BIGDOS FAT 
       * 16bit FAT, 32Bit sector number.  Else if the partition 
       * is and greater or equal to 32680 use SMALLDOS FAT, 
       * 16bit FAT, 16bit sector num.  Else use FAT12 for 
       * anything smaller than 32680.  Note: some systems 
       * may want to change this to use the Windows95 partiton 
       * types that support (LBA) INT 13 extensions, since the 
       * only thing VxWorks can truely ensure is the LBA fields, 
       * mostly since vxWorks does not use the BIOS (PC BIOS is 
       * NOT deterministic, bad for a RTOS, plus they tend not 
            * to be present on non-x86 targets.) and cannot ensure
       * the BIOS translation of CHS.  Of course, the 32bit VxWorks
       * RTOS would never need such a hack as CHS translation.  
       * The reason we don't use the LBA field now is that 
       * NT 3.51 and NT 4.0 will not recognize the new partition 
       * types (0xb-0xf).   That is one reason this is shipped 
       * in source.
       * 
       * TODO: Reconsider using partition types 0xb-0xf when 
       * MS gets their trip together.
       */

      if ( partTbl[i].nBlocks >= 0x400000 )
         partTbl[i].flags = PART_TYPE_DOS32X;
      else if ( partTbl[i].nBlocks >= 65536 )
         partTbl[i].flags = PART_TYPE_DOS4;
      else if ( partTbl[i].nBlocks >= 32680 )
         partTbl[i].flags = PART_TYPE_DOS3;
      else
         partTbl[i].flags = PART_TYPE_DOS12;
   }

   /* allocate a local secBuf for the read sectors MBR/Part data */

   if ( (secBuf = malloc (cbioParams.bytesPerBlk)) == NULL )
   {
      printErr ("usrFdiskPartCreate: Error allocating sector buffer.\n");
      return(ERROR);
   }


   /* start filling the MBR buffer */

   bzero( secBuf, cbioParams.bytesPerBlk) ;

   /* fill the top with a silly RET sequence, not JMP */

   secBuf[0] = 0x90 ; /* NOP */
   secBuf[1] = 0x90 ; /* NOP */
   secBuf[2] = 0xc3 ; /* RET */

   bcopy( dummyString, secBuf+3, sizeof(dummyString));

   /* write bottom 0x55aa signature */

   secBuf[ PART_SIG_ADRS ]     = PART_SIG_MSB; /* 0x55 */
   secBuf[ PART_SIG_ADRS + 1 ] = PART_SIG_LSB; /* 0xaa */

   /* 
    * When the sector size is larger than 512 bytesPerSector we write the 
    * signature to the end of the sector as well as at offset 510/511.
    */

   if ( 512 < cbioParams.bytesPerBlk )
   {
      secBuf[ cbioParams.bytesPerBlk - 2 ] = PART_SIG_MSB; /* 0x55 */
      secBuf[ cbioParams.bytesPerBlk - 1 ] = PART_SIG_LSB; /* 0xAA */
   }


   /* Now, fill the 4 partition entries, careful with byte ordering */

   for ( i = 0; i < 4; i ++ )
   {
      FAST cyl, head, tsec, s1 ;

      /* calculate offset of current partition table entry */

      FAST partOffset = DOS_BOOT_PART_TBL + i * 16 ;

      /* fill in fields */

      secBuf[ partOffset + BOOT_TYPE_OFFSET]  = 
      (i) ? PART_NOT_BOOTABLE : PART_IS_BOOTABLE;

      secBuf[ partOffset + SYSTYPE_OFFSET]  = partTbl[i].flags ;

      /* LBA number of sectors */

      VX_TO_DISK_32( partTbl [ i ].nBlocks,
                     &secBuf[ partOffset + NSECTORS_TOTAL]);
      /* LBA offset */

      VX_TO_DISK_32( partTbl [ i ].offset,
                     &secBuf[ partOffset + NSECTORS_OFFSET]);

      /* beginning of partition in CHS */

      if ( partTbl [ i ].nBlocks > 0 )
      {
         s1 = partTbl [ i ].offset ;
         cyl = s1 / cylSecs ;
         head = (s1 - (cyl * cylSecs)) / trackSecs ;
         tsec = 1 + s1 - (cyl * cylSecs) - (head*trackSecs);
      }
      else
      {
         cyl = head = tsec = 0 ;   /* unused table entry */
      }

#ifdef	DEBUG
      printErr("  start cyl %d hd %d s %d\n", cyl, head, tsec );
#endif
      secBuf[ partOffset + STARTSEC_CYL_OFFSET ] = cyl & 0xff ;
      secBuf[ partOffset + STARTSEC_SEC_OFFSET ] = ((cyl>>2) & 0xc0) | tsec ;
      secBuf[ partOffset + STARTSEC_HD_OFFSET ] = head ;

      /* end of partition in CHS */

      if ( partTbl [ i ].nBlocks > 0 )
      {
         s1 = partTbl [ i ].offset + partTbl [ i ].nBlocks - 1  ;
         cyl = s1 / cylSecs ;
         head = (s1 - (cyl * cylSecs)) / trackSecs ;
         tsec = 1 + s1 - (cyl * cylSecs) - (head*trackSecs);
      }
      else
      {
         cyl = head = tsec = 0 ;   /* unused table entry */
      }

#ifdef	DEBUG
      printErr("  end   cyl %d hd %d s %d\n", cyl, head, tsec );
#endif
      secBuf[ partOffset + ENDSEC_CYL_OFFSET ] = cyl & 0xff ;
      secBuf[ partOffset + ENDSEC_SEC_OFFSET ] = ((cyl>>2) & 0xc0) | tsec ;
      secBuf[ partOffset + ENDSEC_HD_OFFSET ] = head ;

   }

   (void)  cbioIoctl( dev, CBIO_DEVICE_LOCK, 0) ;

   /* Last but not least, write the MBR to disk */

   stat = cbioBlkRW( dev, 0 , 1, secBuf, CBIO_WRITE, NULL ) ;

   /* flush and invalidate cache immediately */

   stat |= cbioIoctl( dev, CBIO_CACHE_INVAL, 0) ;

   cbioRdyChgdSet (dev, TRUE) ; /* force re-mount */

   (void)  cbioIoctl( dev, CBIO_DEVICE_UNLOCK, 0) ;

   free(secBuf);

   return stat;
}

