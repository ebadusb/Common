/*************************************************************************************
 * Copyright (c) 1997 Cobe BCT, Inc.
 *
 * FILENAME: checkself.c
 * PURPOSE: checkself() is used to verify crc of self
 * CHANGELOG:
 * $Header: I:/BCT_Development/vxWorks/Common/softcrc/rcs/checkself.cpp 1.10 2003/06/17 18:57:13Z td07711 Exp td07711 $
 * $Log: checkself.cpp $
 * Revision 1.10  2003/06/17 18:57:13Z  td07711
 * accomodate noncontiguous text/data segments.
 * Revision 1.9  2003/04/11 23:00:36Z  td07711
 * use checkself log levels
 * Revision 1.8  2003/03/12 01:59:42Z  td07711
 * add errno for fopen failure
 * Revision 1.7  2003/03/06 01:41:00Z  td07711
 * changed into sent to datalog
 * Revision 1.6  2003/03/03 19:19:36Z  td07711
 * safety stderr getting lost, use datalog directly instead
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
#include <a_out.h>
#include <ioLib.h>

#include "crcgen.h"
#include "checkself.h"
#include "error.h"
#include "datalog_levels.h"


extern SYMTAB_ID sysSymTbl; // system sysbol table ID needed for symFindByName()


int checkself( char* textSymbol, char* dataSymbol, const char* outfile, const char* crcfile )
{
    //
    // determine starting addresses of text and data segments
    //
    char* textAddress;
    char* dataAddress;
    SYM_TYPE symType; 
    FILE* logfile = stderr;
    if( textSymbol && symFindByName( sysSymTbl, textSymbol, &textAddress, &symType ) == ERROR )
    {
	DataLog( log_level_checkself_error ) << "failed to find textSymbol=" << textSymbol << endmsg;
	return -1;
    }
    if( dataSymbol && symFindByName( sysSymTbl, dataSymbol, &dataAddress, &symType ) == ERROR )
    {
	DataLog( log_level_checkself_error ) << "failed to find dataSymbol=" << dataSymbol << endmsg;
	return -1;
    }

    //
    // read .out file header for segment sizes
    //
    int outFD = open( outfile, O_RDONLY, 0644 );
    if( outFD == ERROR )
    {
	DataLog( log_level_checkself_error ) << "failed to open outfile=" << outfile 
	    << ", errno=0x" << hex << errno << endmsg;
	return -1;
    }
    struct exec aoutHeader;
    int numbytes = read( outFD, (char*)&aoutHeader, sizeof( aoutHeader ) );
    if( numbytes != sizeof( aoutHeader ) )
    {
	DataLog( log_level_checkself_error ) << "failed to read aoutHeader for " << outfile
	    << ", numbytes=" << numbytes << "errno=0x" << hex << errno << endmsg;
	close( outFD );
	return -1;
    }
    close( outFD );

    //
    // calculate crc
    //
    unsigned long calc_crc = 0;

    if( textSymbol )
    {
	DataLog( log_level_checkself_info ) << hex << textSymbol << " address=0x" << (unsigned long)textAddress
	    << " size=0x" << aoutHeader.a_text << endmsg;

	if( crcgen32( &calc_crc, (const unsigned char*)textAddress, aoutHeader.a_text ) == -1 )
	{
	    DataLog( log_level_checkself_error ) << "crcgen32 failed " << textSymbol << endmsg;
	    return -1;
	}
    }

    if( dataSymbol )
    {
	DataLog( log_level_checkself_info ) << hex << dataSymbol << " address=0x" << dataAddress
	    << " size=0x" << aoutHeader.a_data << endmsg;

	if( crcgen32( &calc_crc, (const unsigned char*)dataAddress, aoutHeader.a_data ) == -1 )
	{
	    DataLog( log_level_checkself_error ) << "crcgen32 failed " << dataSymbol << endmsg;
	    return -1;
	}
    }


    //
    // read expected crc from crcfile or create the crcfile as needed
    //
    unsigned long exp_crc;
    FILE* crcFILE = fopen( crcfile, "r" );

    if( crcFILE == 0 ) 
    {
       //
       //  create new crc file
       //
       crcFILE = fopen( crcfile, "w" );
       if( crcFILE == 0 )
       {
	   DataLog( log_level_checkself_error ) << "fopen failed to create " << crcfile 
	      << ", errno=0x" << hex << errno << endmsg;
	   return -1;
       }

       fprintf( crcFILE, "0x%08lx\n", calc_crc );
       fclose( crcFILE );
       
       DataLog( log_level_checkself_info ) << "created file=" << crcfile
	   << hex << " calc_crc=0x" << calc_crc << endmsg;
       return 0;
    }

    if( fscanf( crcFILE, "%lx", &exp_crc) != 1 )
    {
       DataLog( log_level_checkself_error ) << "fscanf failed on " << crcfile << endmsg;
       fclose( crcFILE );
       return -1;
    }
    fclose( crcFILE );

    //
    // verify calculated == expected
    //
    if( calc_crc != exp_crc ) 
    {
       DataLog( log_level_checkself_error ) << "bad crc, file=" << outfile
	   << hex << " exp=0x" << exp_crc << " calc=0x" << calc_crc << endmsg;
       return -1;
    }

    //
    // log good crc and return
    //
    DataLog( log_level_checkself_info ) << "good crc, file=" << outfile
	<< hex << " exp=0x" << exp_crc << " calc=0x" << calc_crc << endmsg;
    return 0;
 }


