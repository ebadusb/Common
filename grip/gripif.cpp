/*
* Copyright Gambro BCT 2006
*
* $Header: //bctquad3/HOME/BCT_Development/vxWorks/Common/grip/rcs/gripif.cpp 1.3 2006/10/26 18:15:01Z pn02526 Exp wtucusb $
*
* Derived from Taos thedif.cpp Revision 1.5  2004/08/09 11:36:52  ms10234
* $Log: gripif.cpp $
*/

#include <string>
#include <sstream>

#include "gripif.h"

#ifndef INITCRC_DEFAULT
#define INITCRC_DEFAULT 0xFFFFFFFF
#endif

//
// Default Constructor
//
GRIPIf::GRIPIf()
{
   memset (_errorString, 0, sizeof(_errorString));
}
   
//
// Default Destructor
//
GRIPIf::~GRIPIf()
{
}


//
// BroadcastPort as string.
//
void GRIPIf::BroadcastPort(string &s)
{
    ostringstream os;
    os << GRIP_BROADCAST_PORT; 
    s = os.str();
}

//
// BroadcastPort as unsigned short.
//
unsigned short GRIPIf::BroadcastPort()
{
   return (unsigned short)GRIP_BROADCAST_PORT;
}

//
// CommandPort as string.
//
void GRIPIf::CommandPort(string &s)
{
    ostringstream os;
    os << GRIP_COMMAND_PORT; 
    s = os.str();
}

//
// CommandPort as unsigned short.
//
unsigned short GRIPIf::CommandPort()
{
   return (unsigned short)GRIP_COMMAND_PORT;
}


//
// PrepareMsg does the following to the message header
// (1) Sets the som field
// (2) Calculates the length field
// (3) Sets the timestamp field
// (4) Sets the headerCRC field
// (5) Sets the bodyCrc field
//
void GRIPIf::prepareMsg (void *msg, GRIP_MessageId messageId, unsigned long sizeInBytes)
{
   GRIP_Header *hdr = (GRIP_Header *)msg;

   hdr->som            = GRIP_SOM;
   hdr->msgId          = messageId;
   hdr->length         = sizeInBytes - GRIP_HEADER_SIZE;
   hdr->time           = time (NULL);
   hdr->headerCRC      = 0;   // Initialize field before doing CRC.
   hdr->bodyCRC        = 0;   // Initialize field before doing CRC.

   // Compute the CRC for the message body if there is one. 
   if (sizeInBytes > GRIP_HEADER_SIZE)
   {
      unsigned char *msgptr = (unsigned char *)hdr + GRIP_HEADER_SIZE;
      const int MsgLength = sizeInBytes - GRIP_HEADER_SIZE;

      hdr->bodyCRC = INITCRC_DEFAULT;
      crcgen32 (&hdr->bodyCRC, msgptr, MsgLength);
      hdr->bodyCRC = hdr->bodyCRC ^ INITCRC_DEFAULT;
   }

   hdr->headerCRC = INITCRC_DEFAULT;
   crcgen32 (&hdr->headerCRC, (unsigned char *)hdr, GRIP_HEADER_SIZE - sizeof(unsigned long));
   hdr->headerCRC = hdr->headerCRC ^ INITCRC_DEFAULT;
}

// This routine only checks for common message Ids.
// Supply one via class derivation to check for device-specific Ids.
bool GRIPIf::validMsgId(const GRIP_Header *hdr)
{
    return(  hdr->msgId >= GRIP_FIRST_COMMON_MESSAGE && hdr->msgId <= GRIP_LAST_COMMON_MESSAGE );
}

//
// This routine checks to see that the message is valid.  The following
// Checks are made:
// 1.  Check that the message header CRC is correct
// 2.  Check that the SOM is correct
// 3.  Check that the message ID is a valid range
// 4.  Check that the message length is valid
// 5.  Check that the message body checksum is correct
//
// Usage:
//    (1) pass in pointer to message buffer and the size received.
//    (2) routine will return 0 if message passed validation.  Otherwise
//        routine will pass back the appropriate response status code.  Also,
//        routine will set a descriptive text string that can be gotten by calling
//        errorString;
//
GRIP_BufferStatus GRIPIf::validHeader(const void *msg, unsigned long length)
{
   unsigned long crcValue = INITCRC_DEFAULT;
   const GRIP_Header *hdr = (GRIP_Header *)msg;
   GRIP_BufferStatus retval = GRIP_OK;

   // Validate the Header.

   // If the size in bytes is less than the header size, then return BUFFER_UNDERFLOW
   if (length < GRIP_HEADER_SIZE)
   {
      ostringstream os;
      os << "Stream length less than size of header, value(" << length << "), expected(" << GRIP_HEADER_SIZE << ")"; 
      strcpy(_errorString, os.str().c_str() );
      return GRIP_BUFFER_UNDERRUN;
   }
   
   // 1.  Check that the SOM is correct
   if (hdr->som != GRIP_SOM)
   {
      // Log event for later debugging.
      ostringstream os;
      os << "Invalid SOM in header, value(" << hex << hdr->som << "), expected(" << GRIP_SOM << dec << ")"; 
      strcpy(_errorString, os.str().c_str() );
      return GRIP_BAD_SOM;
   }
   
   // 2.  Check that the message header CRC is correct
   crcgen32 (&crcValue, (unsigned char *)msg, GRIP_HEADER_SIZE - sizeof (unsigned long));
   crcValue = crcValue ^ INITCRC_DEFAULT;

   if (crcValue != hdr->headerCRC)
   {
      // Log event for later debugging.
      ostringstream os;
      os << "Invalid header CRC, value(" << hex << hdr->headerCRC << "), expected(" << crcValue << dec << ")"; 
      strcpy(_errorString, os.str().c_str() );
      return GRIP_HEADER_CRC_INVALID;
   }

   // 3.  Check that the message ID is a valid range (Uses a derived class's function for this.)
   if ( !validMsgId(hdr) )
   {
      // Log event for later debugging.
      ostringstream os;
      os << "Invalid Message ID in header, value(" << hex << hdr->msgId << "), expected(>=" << GRIP_FIRST_COMMON_MESSAGE << dec << ")"; 
      strcpy(_errorString, os.str().c_str() );
      return GRIP_MESSAGE_ID_INVALID;
   }
   
   // 4.  Check that the message length is valid
   if ( (length == GRIP_HEADER_SIZE) && (hdr->length != 0) )
   {
      ostringstream os;
      os << "Header length less than rx stream length, value(" <<  length << "), expected(" << GRIP_HEADER_SIZE + hdr->length << ")"; 
      strcpy(_errorString, os.str().c_str() );
      return GRIP_BUFFER_UNDERRUN;
   }
   else if ( (length != GRIP_HEADER_SIZE) && (hdr->length == 0) )
   {
      ostringstream os;
      os << "Stream length greater than size of header, value(" <<  length << "), expected(" << GRIP_HEADER_SIZE << ")"; 
      strcpy(_errorString, os.str().c_str() );
      return GRIP_BUFFER_OVERRUN;
   }
   else if ( (hdr->length + GRIP_HEADER_SIZE) < length)
   {
      ostringstream os;
      os << "Stream Length greater than message length, value=" <<  length << ", expected" << hdr->length + GRIP_HEADER_SIZE << ")"; 
      strcpy(_errorString, os.str().c_str() );
      return GRIP_BUFFER_OVERRUN;
   }
   else if ( (hdr->length + GRIP_HEADER_SIZE) > length)
   {
      ostringstream os;
      os << "Stream Length less than message length, value=" <<  length << ", expected" << hdr->length + GRIP_HEADER_SIZE << ")"; 
      strcpy(_errorString, os.str().c_str() );
      return GRIP_BUFFER_UNDERRUN;
   }
   
   // 5.  Check that the message body checksum is correct
   if (hdr->length > 0)
   {
      unsigned char *msgptr = (unsigned char *)msg + GRIP_HEADER_SIZE;

      unsigned long msgCRC = INITCRC_DEFAULT;
      crcgen32 (&msgCRC, msgptr, hdr->length);
      msgCRC = msgCRC ^ INITCRC_DEFAULT;

      if (msgCRC != hdr->bodyCRC)
      {
         // Log event for later debugging.
         ostringstream os;
         os << "Invalid Msg Body CRC, value(" << hex << hdr->bodyCRC << ", expected" << msgCRC << dec << ")"; 
         strcpy(_errorString, os.str().c_str() );
         return GRIP_BODY_CRC_INVALID;
      }
   }

   // Message Passed all validation.
   // Clear the error string
   _errorString[0] = 0;
   
   // Return Success
   return GRIP_OK;
}


//
// findSOM
//
int GRIPIf::findSOM(const void *msg, unsigned long length)
{
   const int *buffer = (int *)msg;
   const int bufferLength = (sizeof(int) - 1) / sizeof(int);

   if (length < sizeof(int))
   {
      return 0;
   }

   for (int i = 0; i < bufferLength; i++)
   {
      if (buffer[i] == GRIP_SOM)
      {
         return i;
      }
   }

   // we got nothing!
   return 0;
}


//
// Print the buffer status to the stream
//
DataLog_Stream & operator << (DataLog_Stream &os, const GRIP_BufferStatus &status)
{
   switch (status)
   {
   case GRIP_BUFFER_UNDERRUN:
      os << "BUFFER UNDERRUN";
      break;

   case GRIP_BUFFER_OVERRUN:
      os << "BUFFER OVERRUN";
      break;

   case GRIP_HEADER_CRC_INVALID:
      os << "HEADER CRC INVALID";
      break;

   case GRIP_BODY_CRC_INVALID:
      os << "BODY CRC INVALID";
      break;

   case GRIP_BAD_SOM:
      os << "BAD SOM";
      break;

   case GRIP_MESSAGE_ID_INVALID:
      os << "MESSAGE ID INVALID";
      break;

   case GRIP_OK:
      os << "OK";
      break;
   }

   os << "(" << (int)status << ")";

   return os;
}



//
// Print the status type to the stream
//
DataLog_Stream & operator << (DataLog_Stream &os, const GRIP_Status &status)
{
   switch (status)
   {
   case GRIP_CONNECTION_ALLOWED_OPERATIONAL:
      os << "GRIP CONNECTION ALLOWED OPERATIONAL";
      break;

   case GRIP_CONNECTION_ALLOWED_SERVICE:
      os << "GRIP CONNECTION ALLOWED SERVICE";
      break;

   case GRIP_CONNECTION_ALLOWED_SINGLE_STEP:
      os << "GRIP CONNECTION ALLOWED SINGLE STEP";
      break;

   case GRIP_CONNECTION_DENIED:
      os << "GRIP CONNECTION DENIED";
      break;

   case GRIP_REQUEST_ALLOWED:
      os << "GRIP REQUEST ALLOWED";
      break;

   case GRIP_REQUEST_ALLOWED_OVERRIDE:
      os << "GRIP REQUEST ALLOWED OVERRIDE";
      break;

   case GRIP_REQUEST_DENIED_NOT_IN_SERVICE_MODE:
      os << "GRIP REQUEST DENIED NOT IN SERVICE MODE";
      break;

   case GRIP_REQUEST_DENIED_NOT_CLIENT:
      os << "GRIP REQUEST DENIED NOT CLIENT";
      break;

   case GRIP_BAD_MESSAGE_RECEIVED:
      os << "GRIP BAD MESSAGE RECEIVED";
      break;

   case GRIP_OPERATION_FAILED:
      os << "GRIP OPERATION FAILED";
      break;
   }

   os << "(" << (int)status << ")";

   return os;
}



//
// Print the board type to the stream
//
DataLog_Stream & operator << (DataLog_Stream &os, const GRIP_BoardType &type)
{
   switch (type)
   {
   case GRIP_CONTROL:
      os << "CONTROL";
      break;

   case GRIP_SAFETY:
      os << "SAFETY";
      break;

   case GRIP_AIM:
	   os << "AIM";
	   break;
   }

   os << "(" << (int)type << ")";

   return os;
}



//
// Prints the header to the stream.
//
DataLog_Stream & operator << (DataLog_Stream &os, const GRIP_Header *hdr)
{

   os << "SOM("   << hex << hdr->som << dec 
      << ") ID("  << hdr->msgId
      << ") L("   << hdr->length
      << ") T("   << hex << hdr->time
      << ") BC("  << hdr->bodyCRC
      << ") HC("  << hdr->headerCRC << dec
      << ")";

   return os;
}
