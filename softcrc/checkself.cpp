/*************************************************************************************
 * Copyright (c) 1997 Cobe BCT, Inc.
 *
 * FILENAME: checkself.c
 * PURPOSE: checkself() is used to verify crc of self
 * CHANGELOG:
 * $Header: I:/BCT_Development/vxWorks/Common/softcrc/rcs/checkself.cpp 1.10 2003/06/17 18:57:13Z td07711 Exp td07711 $
 * $Log: checkself.cpp $
 * Revision 1.5  2003/02/26 23:03:53Z  td07711
 * modified for vxworks
 * Revision 1.4  2003/01/09 20:53:25Z  pn02526
 * Change #ifdef VXWORKS to ENABLE_CRC_CHECKING 
 * Revision 1.3  2002/12/20 14:30:39  ms10234
 * Changed function definition to take const char *'s
 * Revision 1.2  2002/08/14 15:40:05Z  pn02526
 * TEMPORARILY disabled for vxWorks.
 * Revision 1.1  1999/05/24 17:32:05  TD10216
 * Initial revision
 * Revision 1.1  1999/03/20 00:45:20  TD10216
 * Initial revision
 * Revision 1.1  1997/11/21 02:07:13  TD07711
 * Initial revision
 *  11/12/97 - dyes
 *************************************************************************************/

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <symLib.h>

#include "crcgen.h"
#include "checkself.h"
#include "error.h"
#include "datalog.h"


extern SYMTAB_ID sysSymTbl; // system sysbol table ID needed for symFindByName()


int checkself( char* startSymbol, char* endSymbol, const char* filename)
{
    // PURPOSE:  verifies program loaded in memory has not changed.
    // Verifies load integrity when run over text and initialized data segments following load.
    // Verifies no text segment overwrites when run periodically thereafter.
    // 
    // checkself calculates a CRC over the data from startSymbol to endSymbol-1
    //
    // If filename exists, it reads the contained value as a previously saved CRC and verifies
    // that it is the same as the one calculated.  
    // Returns -1 if not validated, 0 if validated
    //
    // If filename does not exist, then filename is created and the calculated CRC is saved
    // for validation on subsequent boots.
    // Returns -1 if file not written, else 0


    DataLog_Level log( "checkself" );

    //
    // determine region to CRC
    //
    char* startAddress;
    char* endAddress;
    SYM_TYPE symType; 
    FILE* logfile = stderr;
    if( symFindByName( sysSymTbl, startSymbol, &startAddress, &symType ) == ERROR )
    {
	log( __FILE__, __LINE__ ) << "ERROR failed to find" << startSymbol << endmsg;
	return -1;
    }
    if( symFindByName( sysSymTbl, endSymbol, &endAddress, &symType ) == ERROR )
    {
	log( __FILE__, __LINE__ ) << "ERROR failed to find" << endSymbol << endmsg;
	return -1;
    }
    long size = endAddress - startAddress;

    //
    // calculate crc
    //
    unsigned int calc_crc = 0;
    if( crcgen32( (unsigned long*)&calc_crc, (const unsigned char*)startAddress, size ) == -1 )
    {
       log( __FILE__, __LINE__ ) << "ERROR crcgen32 failed, start=" << 
	   startAddress << " size=" << size << endmsg;
       return -1;
    }


    //
    // read expected crc from file specified
    //
    unsigned int exp_crc;
    FILE* infile;
    FILE* outfile=0;
    if( (infile = fopen( filename, "r" )) == 0 ) 
    {
       //
       //  create new crc file if needed
       //
       if( (outfile = fopen( filename, "w" )) == 0 )
       {
	  log( __FILE__, __LINE__ ) << "ERROR fopen failed on " << filename << endmsg;
	  return -1;
       }
       fprintf( outfile, "0x%08x\n", calc_crc );
       fclose( outfile );
       log( __FILE__, __LINE__ ) << "start=" << startAddress << " size=" << size
	   << " calc_crc=" << calc_crc << " created crcfile=" << filename << endmsg;
       return 0;
    }

    if( fscanf( infile, "%x", &exp_crc) != 1 )
    {
       log( __FILE__, __LINE__ ) << "ERROR fscanf failed" << endmsg;
       fclose( infile );
       return -1;
    }

    //
    // verify calculated == expected
    //
    fclose( infile );
    if( calc_crc != exp_crc ) 
    {
       log( __FILE__, __LINE__ ) << "ERROR bad crc, exp=" << exp_crc 
	   << " calc=" << calc_crc << " file=" << filename << " start=" << startAddress
	   << " size=" << size << endmsg;
       return -1;
    }

    //
    // log good crc and return
    //
    log( __FILE__, __LINE__ ) << "good crc, exp=" << exp_crc 
	<< " calc=" << calc_crc << " file=" << filename << " start=" << startAddress
	<< " size=" << size << endmsg;
    return 0;
 }


