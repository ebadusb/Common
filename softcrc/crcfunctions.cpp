//## begin module%35EC152600BC.cm preserve=no
//	  %X% %Q% %Z% %W%
//## end module%35EC152600BC.cm

//## begin module%35EC152600BC.cp preserve=no
//## end module%35EC152600BC.cp

//## Module: CRCFunctions%35EC152600BC; Pseudo Package body
//## Subsystem: <Top Level>
//## Source file: D:\Program Files\Rational\Rational Rose 98 Enterprise Edition\c++\source\CRCFunctions.cpp

//## begin module%35EC152600BC.additionalIncludes preserve=no
#include <iostream.h>
#include <strstrea.h>
#include <fstream.h>
#include <stdio.h>
#include "crcgen.h"
#include "crc.h"
//## end module%35EC152600BC.additionalIncludes

//## begin module%35EC152600BC.includes preserve=yes
//## end module%35EC152600BC.includes

// CRCFunctions
#include "crcfunctions.h"

//## begin module%35EC152600BC.additionalDeclarations preserve=yes
static const int MAXBUFFERLENGTH = 256; // by TRIMA definition
//## end module%35EC152600BC.additionalDeclarations


// Class CRCFunctions 

CRCFunctions::CRCFunctions()
  //## begin CRCFunctions::CRCFunctions%.hasinit preserve=no
  //## end CRCFunctions::CRCFunctions%.hasinit
  //## begin CRCFunctions::CRCFunctions%.initialization preserve=yes
  //## end CRCFunctions::CRCFunctions%.initialization
{
  //## begin CRCFunctions::CRCFunctions%.body preserve=yes
  //## end CRCFunctions::CRCFunctions%.body
}


CRCFunctions::~CRCFunctions()
{
  //## begin CRCFunctions::~CRCFunctions%.body preserve=yes
  //## end CRCFunctions::~CRCFunctions%.body
}

//## Other Operations (implementation)
int CRCFunctions::generateCRC (char* crc, char *data_buffer, long buffer_length)
{
  //## begin CRCFunctions::generateCRC%904748907.body preserve=yes
   int return_value = -1;
   // Convert the character based crc to a long and then call the crc routine.
   unsigned long ul_crc;
   strstream tempStream;
   tempStream << "0x" << crc;
   tempStream >> ul_crc;
   // This function returns zero on success and -1 on failure.
   return_value = crcgen32(&ul_crc, data_buffer, buffer_length);
   if ( return_value == 0 )
   {
      sprintf( crc, "%08X", ul_crc );
   }
   return return_value;
  //## end CRCFunctions::generateCRC%904748907.body
}

int CRCFunctions::generateFileCRC (char* crc, const char* filename)
{
  //## begin CRCFunctions::generateFileCRC%904748908.body preserve=yes
   // Setup the return failure value.
   int return_value = -1;
   // Ensure that a non-null pointer was passed.
   if ( filename )
   {
      // Open the file for reading in the file for CRCing.
      fstream datfile(filename,ios::in+ios::nocreate);
      if (!datfile.fail()) 
      {
         return_value = generateCRC( crc, &datfile );
      }
      // Close the opened stream.
      datfile.close();
   }
   return return_value;
  //## end CRCFunctions::generateFileCRC%904748908.body
}

int CRCFunctions::generateCRC (char* crc, iostream* the_stream)
{
  //## begin CRCFunctions::generateCRC%904748909.body preserve=yes
   int return_value = 0;
   int read_count = 0;
   char buffer[MAXBUFFERLENGTH]; 
   // Convert the character based crc to a long and then call the crc routine.
   unsigned long ul_crc;
   strstream tempStream;
   tempStream << "0x" << crc;
   tempStream >> ul_crc;
   // Return to the beginning of the stream.
   the_stream->seekg(0);
   // Read in the stream on line at a time.
   do
   {
      the_stream->read(buffer,MAXBUFFERLENGTH); // istream call
      read_count = the_stream->gcount();
      if ( ( the_stream->fail() ) && ( read_count == MAXBUFFERLENGTH ) )
      {
         return_value = -1;
         break;
      }
      // Call the crcgen32 function instead of the generateCRC method so that
      // you do not convert the crc back and forth from char to unsigned long.
      return_value = crcgen32( &ul_crc, buffer, read_count );
   } while ( ( read_count == MAXBUFFERLENGTH ) && ( return_value == 0 ) ); 
   // If the return value is still zero the proper CRC was generated.  
   if ( return_value == 0 )
   {
      sprintf( crc, "%08X", ul_crc );
   }
   return return_value;
  //## end CRCFunctions::generateCRC%904748909.body
}

int CRCFunctions::validateFileCRC (const char *crc, const char* filename)
{
  //## begin CRCFunctions::validateFileCRC%904748910.body preserve=yes
   int return_value = 0;
   // Init the fileCRC to 0 to start the crc calculations.
   char fileCRC[9];
   sprintf( fileCRC, "%08X", 0);
   // Generate the crc for this file to check it against the crc which was passed in.
   return_value = generateFileCRC( fileCRC, filename );
   // Since the return value was 0 we will have a vaild CRC in fileCRC.
   if ( return_value == 0 )
   {
      // Compare the two CRC's without reguard to case.
      return_value = stricmp(fileCRC, crc);
   }
   return return_value;
  //## end CRCFunctions::validateFileCRC%904748910.body
}

int CRCFunctions::validateCRC (const char* crc, iostream* the_stream)
{
  //## begin CRCFunctions::validateCRC%904748911.body preserve=yes
   int return_value = 0;
   // Init the fileCRC to 0 to start the crc calculations.
   char fileCRC[9];
   sprintf( fileCRC, "%08X", 0);
   // Generate the crc for this file to check it against the crc which was passed in.
   return_value = generateCRC( fileCRC, the_stream );
   // Since the return value was 0 we will have a vaild CRC in fileCRC.
   if ( return_value == 0 )
   {
      // Compare the two CRC's without reguard to case.
      return_value = stricmp(fileCRC, crc);
   }
   return return_value;
  //## end CRCFunctions::validateCRC%904748911.body
}

#include "error.h"

int CRCFunctions::moveValidFile (const char* crc, const char* source, const char* dest)
{
  //## begin CRCFunctions::moveValidFile%904748912.body preserve=yes
   int return_value = -1;
   // Ensure that a non-null pointer was passed.
   if ( source && dest )
   {
      // Open the file for reading in the file for CRCing.
      fstream readfile(source,ios::in+ios::nocreate+ios::binary);
      if (!readfile.fail()) 
      {
         // Validate the CRC for the file given.
         return_value = validateCRC( crc, &readfile );
         if ( return_value == 0 )
         {
            // Seek to the beginning of the readfile.
            readfile.clear();
            readfile.seekg(0);
            if ( readfile.fail() )
            {
               return_value = -1;
            }
            else
            {
               // Open the output stream/file.
               fstream writefile(dest,ios::out+ios::binary);
               if (!writefile.fail())
               {
                  char buffer[MAXBUFFERLENGTH]; 
                  // Copy the readfile to the writefile stream.
                  int write_count = 0;
                  do 
                  {
                     // Read from the readfile.
                     readfile.read(buffer,MAXBUFFERLENGTH); // istream call
                     // Write to the writefile
                     write_count = readfile.gcount();
                     if ( ( readfile.fail() ) && ( write_count == MAXBUFFERLENGTH )  )
                     {
                        return_value = -1;
                        break;
                     }
                     writefile.write(buffer, write_count);
                     // Ensure that both the read and write did not fail.
                     if ( writefile.fail() )
                     {
                        return_value = -1;
                        break;
                     }
                  } while ( ( write_count == MAXBUFFERLENGTH ) &&  ( return_value == 0 ) );
               }
               // Close the writefile stream.
               writefile.close();
            }
         }
      }
      // Close the opened read stream.
      readfile.close();
   }
   return return_value;
   //## end CRCFunctions::moveValidFile%904748912.body
}

// Additional Declarations
  //## begin CRCFunctions%35EC152600BC.declarations preserve=yes
  //## end CRCFunctions%35EC152600BC.declarations

//## begin module%35EC152600BC.epilog preserve=yes
//## end module%35EC152600BC.epilog




static int CRCFunctions::getFileCrc(const char *filename, unsigned long *finalCrc)
{
   char buffer[MAXBUFFERLENGTH]; 
   int count;
   int status;
   int totalBytesRead = 0;
   unsigned long runningCrc = 0; 

   ifstream inStream (filename, ios::in + ios::binary + ios::nocreate);

   if (inStream.fail())
   {
      PROC_SLOGERROR ("Opening file failed.");
      inStream.close();
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
         sprintf (buffer, "Stream read failed (%d, %d) (%x).", count, totalBytesRead, inStream.rdstate());
         PROC_SLOGERROR (buffer);
         inStream.close();
         return -1;
      }
      
      //status = crcgen32 (&runningCrc, buffer, count);
      runningCrc = addToCrc32 ((unsigned char *)buffer, count, runningCrc);
   
   //} while ( ( count == MAXBUFFERLENGTH ) && ( status == 0 ) ); 
   } while ( count == MAXBUFFERLENGTH );

   *finalCrc = runningCrc;
   return 0;
}
