/*
 * Copyright(C) 2003 Gambro BCT
 *
 * $Header$
 * $Log$
 *
 */

#include <vxworks.h>
#include <fstream.h>
#include <stdio.h>

#include "crcgen.h"
#include "datalog.h"

// file_crcgen32() generates a 32bit CRC over the contents of a file.
// inputs:
//    filename - filename (plus path) to be crc'ed.  file must obviously exist.
//    pcrc - ptr to initial crc value, the crc value is modified as each byte of input data
//           is read.
// outputs:
//    *pcrc is updated with new crc value
//    returns 0 if success
//    returns -1 if failed
int file_crcgen32 (const char *filename, unsigned long *pcrc)
{
	enum { MAXBUFFERLENGTH = 256 };
   unsigned char buffer[MAXBUFFERLENGTH];
   int count;
   int totalBytesRead = 0;

   ifstream inStream (filename, ios::in + ios::binary + ios::nocreate);

   if (inStream.fail())
   {
      inStream.close();
      DataLog_Critical	critical;
      DataLog(critical) << "Couldn't open file " << filename << " (" << errnoMsg <<")" << endmsg;
      return -1;
   }
   
   // Start at the beginning of the stream.
   inStream.seekg(0);
   
   // Read in the stream on block at a time.
   do
   {
      inStream.read (buffer, MAXBUFFERLENGTH);
      count = inStream.gcount();
      totalBytesRead += count;

      // There is something goofy with this stream class.  Fail bit is triggered
      // on the reading of the end of a file.  Don't know what causes it, but
      // ignore fail status.
      if (inStream.bad())
      {
			DataLog_Critical	critical;
			DataLog(critical) << "Stream read failed (" << count << ", " << totalBytesRead <<") (0x" << hex << inStream.rdstate() << dec << ")" << endmsg;
         inStream.close();
         return -1;
      }
      
      crcgen32 (pcrc, buffer, count);
   
   } while ( count == MAXBUFFERLENGTH );

   return 0;
}

