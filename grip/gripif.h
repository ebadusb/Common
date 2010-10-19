/*
 * Copyright (c) 2006 by Gambro BCT, Inc.  All rights reserved.
 *
 *  NOTE: MODIFICATIONS TO THIS FILE MAY NEED TO BE REFLECTED IN THE INTERFACE CONTROL DOCUMENT
 *        SERVICE TOOLS SUITE, MAINTAINED BY THE START GROUP, AND IN THE STS SOFTWARE.
 *
 * $Header$
 *
 * Derived from Taos thedif.h Revision 1.20  2005/06/01 09:23:01  jl11312
 *  and the STS ICD revision 1.4.
 * $Log: gripif.h $
 * Revision 1.5  2006/12/05 17:28:10Z  wtucusb
 * Added new enums for GRIP common ids
 * Revision 1.3  2006/11/29 22:18:02Z  wtucusb
 * Added Service Tools Suite, Design Input, Interface Control Document  Revision 1.6  November 26, 2006 Changes
 *  
 * Revision 1.2  2006/10/26 18:16:04Z  pn02526
 * Add heading comment about STS & its ICD.
 * Revision 1.1  2006/10/18 16:25:22  pn02526
 * Initial revision
 * Revision 1.1  2006/10/18 13:43:26  pn02526
 * Initial revision
 */
 
 
#ifndef __GRIPIF__
#define __GRIPIF__

#include <time.h>

#include "datalog.h"
#include "crcgen.h"

typedef long GRIP_MessageId;


enum GRIP_COMMON_MessageId 
{
   GRIP_FIRST_MESSAGE              = 11000, //Used for debug
   GRIP_CONNECT_REQUEST            = 11001, //Request a connection
   GRIP_HW_PERIODIC_STATUS_REQUEST = 11002, //Get periodic status
   GRIP_END_SERVICE_MODE           = 11005, //End service mode
   GRIP_MOVE_CURSOR_CMD            = 11006, //Used for moving cursor on screen calibration
   GRIP_SET_TIME                   = 11007, //Set the system time
   GRIP_END_CONNECTION             = 11008, //Terminate the connection
   GRIP_REPLY                      = 11011, //Acknowledge - for all messages
   GRIP_REQUEST_CURSOR_POSITION    = 11012, //Allows the caller to get the current position
   GRIP_CURRENT_CURSOR_POSITION    = 11013, //Gets the cursor location on the device
   GRIP_BROADCAST_REQUEST          = 11014, //Allows STS to get the broadcast message
   GRIP_BROADCAST_REPLY            = 11015, //Sends broadcast message from device
   GRIP_MACHINE_DATA_REQUEST       = 11018, //Allows caller to request machine summary data from the device
   GRIP_MACHINE_DATA_REPLY         = 11019, //Has machine summary data in reply
   GRIP_SCREEN_CALIBRATION_REQUEST = 11020,  // Allows STS to initiate a screen calibration request.
   GRIP_DISCOVER_REQUEST           = 11021, //Used for acquiring user name, password, serial number and device type from device OS
   GRIP_DISCOVER_REPLY             = 11022, //See above
};

enum GRIP_Status
{
   GRIP_CONNECTION_ALLOWED_OPERATIONAL      = 0,
   GRIP_CONNECTION_ALLOWED_SERVICE          = 1,
   GRIP_CONNECTION_ALLOWED_SINGLE_STEP      = 2,
   GRIP_CONNECTION_DENIED                   = 3,
   GRIP_REQUEST_ALLOWED                     = 4,
   GRIP_REQUEST_ALLOWED_OVERRIDE            = 5,
   GRIP_REQUEST_DENIED_NOT_IN_SERVICE_MODE  = 6,
   GRIP_REQUEST_DENIED_NOT_CLIENT           = 7,
   GRIP_BAD_MESSAGE_RECEIVED                = 8,
   GRIP_OPERATION_FAILED                    = 9,
};


//
// GRIP Message Header
//
struct GRIP_Header
{
   unsigned long som __attribute__ ((packed));        // start of message sentinel; always set to 0xF007BA11
   long msgId __attribute__ ((packed));      // message id; a value from one of the MessageId enums.
   long length __attribute__ ((packed));     // total message length in bytes
   time_t        time __attribute__ ((packed));       // time message was sent 
   unsigned long bodyCRC __attribute__ ((packed));    // CRC value of the body of the message
   unsigned long headerCRC __attribute__ ((packed));  // CRC value of the header of the message

   inline GRIP_Header() : som(0), msgId(0), length(0), time(0), bodyCRC(0), headerCRC(0) {}
};

enum {GRIP_HEADER_SIZE = sizeof(GRIP_Header)};


//
// GRIP Reply Message
//
struct GRIP_ReplyMsg
{
   GRIP_Header header;
   GRIP_Status status;
};

const unsigned int GRIP_ReplyMsgSize = sizeof(GRIP_ReplyMsg);


//
// GRIP Connect Request Message
//
struct GRIP_ConnectRequestMsg
{
  GRIP_Header header;
};

const unsigned int GRIP_ConnectRequestMsgSize = sizeof(GRIP_ConnectRequestMsg);



//
// GRIP HW Periodic Status Request Message
//
enum GRIP_BoardType 
{
	GRIP_CONTROL = 0,
    GRIP_SAFETY  = 1,
	GRIP_AIM     = 2
};

struct GRIP_HwPeriodicStatusRequestMsg
{
   GRIP_Header    header;
   GRIP_BoardType board __attribute__ ((packed));          // Board from which status being requested.
   unsigned int  period __attribute__ ((packed));         // Period of requested status in ms.
   unsigned int  port __attribute__ ((packed));           // Port to which the Response message goes.
};

const unsigned int GRIP_HwPeriodicStatusRequestMsgSize = sizeof(GRIP_HwPeriodicStatusRequestMsg);



//
// End Service Mode Message
//
struct GRIP_EndServiceModeMsg
{
   GRIP_Header header;
};

const unsigned int GRIP_EndServiceModeMsgSize = sizeof(GRIP_EndServiceModeMsg);



//
// GRIP Move Cursor Command
//
enum GRIP_CursorCommand
{
   GRIP_CURSOR_LEFT  = 1,
   GRIP_CURSOR_RIGHT = 2,
   GRIP_CURSOR_UP    = 3,
   GRIP_CURSOR_DOWN  = 4
};


struct GRIP_MoveCursorMsg
{
   GRIP_Header        header;
   GRIP_CursorCommand order;
};

const unsigned int GRIP_MoveCursorMsgSize = sizeof(GRIP_MoveCursorMsg);


//
// GRIP Set Time Message
//
struct GRIP_SetTimeMsg
{
   GRIP_Header header;
   time_t      time __attribute__ ((packed));        // Time in UTC
};

const unsigned int GRIP_SetTimeMsgSize = sizeof(GRIP_SetTimeMsg);



//
// End GRIP Connection Message
//
struct GRIP_EndConnectionMsg
{
   GRIP_Header header;
};

const unsigned int GRIP_EndConnectionMsgSize = sizeof(GRIP_EndConnectionMsg);



//
// GRIP Request Cursor Position Message
//
struct GRIP_RequestCursorPositionMsg
{
   GRIP_Header header;
};

const unsigned int GRIP_RequestCursorPositionMsgSize = sizeof(GRIP_RequestCursorPositionMsg);



//
// GRIP Current Cursor Position
//
struct GRIP_CurrentCursorPositionMsg
{
   GRIP_Header header;
   int       x __attribute__ ((packed));
   int       y __attribute__ ((packed));
};

const unsigned int GRIP_CurrentCursorPositionMsgSize = sizeof(GRIP_CurrentCursorPositionMsg);



//
// GRIP Screen_Calibration_Request
//
struct GRIP_ScreenCalibrationRequestMsg
{
   GRIP_Header header;
};

const unsigned int GRIP_ScreenCalibrationRequestMsgSize = sizeof(GRIP_ScreenCalibrationRequestMsg);



//
// Request GRIP Broadcast Message
//
struct GRIP_RequestBroadcastMsg
{
   GRIP_Header header;
};

const unsigned int GRIP_RequestBroadcastMsgSize = sizeof(GRIP_RequestBroadcastMsg);



//
// GRIP Broadcast Message
//
const int GRIP_HW_REV_SIZE           = 64;
const int GRIP_IP_ADDRESS_SIZE       = 32;
const int GRIP_LOG_FILENAME_SIZE     = 32;
const int GRIP_SERIAL_NUMBER_SIZE    = 16;
const int GRIP_CRC_SIZE              = 16;
const int GRIP_RELEASE_SIZE          = 16;
const int GRIP_BUILD_SIZE            = 16;
const int GRIP_PORT_NUMBER_SIZE      =  8;
const int GRIP_SEQUENCE_NUMBER_SIZE  =  7;
const int GRIP_VISTA_IF_VERSION_SIZE = 32;

struct GRIP_BroadcastMsg
{
   GRIP_Header   header;
   int          BroadcastCount __attribute__ ((packed));         // count of packets broadcasted
   char         HwRev[GRIP_HW_REV_SIZE];                         // Control & Safety Hdw Revisions
   char         IpAddress[GRIP_IP_ADDRESS_SIZE];                 // 172.80.90.34
   char         LogFile[GRIP_LOG_FILENAME_SIZE];                 // T00015_0504_0033
   char         SerialNumber[GRIP_SERIAL_NUMBER_SIZE];           // T00015
   char         CRC[GRIP_CRC_SIZE];                              // 0xadcef73
   char         Release[GRIP_RELEASE_SIZE];                      // Optia 3.3a
   char         Build[GRIP_BUILD_SIZE];                          // 1.191
   char         Port[GRIP_PORT_NUMBER_SIZE];                     // 37000
   char         sequenceNumber[GRIP_SEQUENCE_NUMBER_SIZE];       // 000000
   char         vipifVersionNumber[GRIP_VISTA_IF_VERSION_SIZE];  //
   unsigned int biometricList __attribute__ ((packed));          // Trima only
};

const unsigned int GRIP_BroadcastMsgSize = sizeof(GRIP_BroadcastMsg);



//
// Request GRIP Device Discover Message
//
struct GRIP_RequestDeviceDiscoverMsg
{
   GRIP_Header header;
};

const unsigned int GRIP_RequestDeviceDiscoverMsgSize = sizeof(GRIP_RequestDeviceDiscoverMsg);



//
// GRIP Discover Device Reply Message
//
const int GRIP_USERNAME_SIZE        = 32;
const int GRIP_PASSWORD_SIZE        = 32;
// see Broadcast Message   const int GRIP_SERIAL_NUMBER_SIZE   = 16;
const int GRIP_DEVICE_TYPE_SIZE     = 32;
const int GRIP_VERSION_SIZE         = 16;
const int GRIP_INTERFACE_REVISION_SIZE = 16;

struct GRIP_DiscoverDeviceReplyMsg
{
   GRIP_Header   header;
   char          Username[GRIP_USERNAME_SIZE]; //Character array for user name for FTP server
   char          Password[GRIP_PASSWORD_SIZE]; //Character array for password for FTP server
   char          SerialNumber[GRIP_SERIAL_NUMBER_SIZE]; //Character array for serial number - used for discovery of devices
   char          Type[GRIP_DEVICE_TYPE_SIZE]; //Character array for device type - "Atreus" "Optia" "Trima"
   char          Version[GRIP_VERSION_SIZE]; //Character array for version of device
   char          interfaceRevision[GRIP_INTERFACE_REVISION_SIZE]; //Character array for format revision of interface between STS and device

	GRIP_DiscoverDeviceReplyMsg() { memset(interfaceRevision,0,GRIP_INTERFACE_REVISION_SIZE); };
};

const unsigned int GRIP_DiscoverDeviceReplyMsgSize = sizeof(GRIP_DiscoverDeviceReplyMsg);



//
// Request GRIP Device Summary Data Message
//
struct GRIP_RequestDeviceSummaryDataMsg
{
   GRIP_Header header;
};

const unsigned int GRIP_RequestDeviceSummaryDataMsgSize = sizeof(GRIP_RequestDeviceSummaryDataMsg);



//
// GRIP Device Summary Data Reply Message
//
struct GRIP_DeviceSummaryDataReplyMsg
{
   GRIP_Header   header;
   float         TotalRunHours     __attribute__ ((packed)); //Float for number of hours
   int           NumProcedures     __attribute__ ((packed)); //Integer for total number of procedures
   int           numCompProcedures __attribute__ ((packed)); //Integer for total number of completed procedures
   int           NumCentHours      __attribute__ ((packed)); //Integer for number of centrifuge hours
};

const unsigned int GRIP_DeviceSummaryDataReplyMsgSize = sizeof(GRIP_DeviceSummaryDataReplyMsg);



enum GRIP_BufferStatus
{
   GRIP_BUFFER_UNDERRUN,
   GRIP_BUFFER_OVERRUN,
   GRIP_HEADER_CRC_INVALID,
   GRIP_BODY_CRC_INVALID,
   GRIP_BAD_SOM,
   GRIP_MESSAGE_ID_INVALID,
   GRIP_OK
};



//
// GRIP Interface
//
class GRIPIf
{
private:

   char _errorString[128];

protected:
   virtual bool validMsgId(const GRIP_Header *hdr);

public:

   enum {GRIP_SOM            = 0x000DECAF};
   enum {GRIP_BROADCAST_PORT = 37002};
   enum {GRIP_FTP_INFO_PORT  = 33000};
   enum {GRIP_COMMAND_PORT   = 37000};

   GRIPIf();

   virtual ~GRIPIf();

    // Return broadcast port number
   void BroadcastPort(string &s);
   unsigned short BroadcastPort();

    // Return command port number
   void CommandPort(string &s);
   unsigned short CommandPort();

   // This routine checks to see that the message is valid.
   GRIP_BufferStatus validHeader(const void *msg, unsigned long length);
   
   // findSOM
   int findSOM(const void *msg, unsigned long length);

   // Prepares a message for sending by setting up the message header.
   void prepareMsg (void *msg, GRIP_MessageId messageId, unsigned long sizeInBytes = GRIP_HEADER_SIZE);

   const char * errorString() const { return _errorString; };
};

// Extensions to the Data Log streams for GRIP
//
// Print the buffer status to the stream
//
DataLog_Stream & operator << (DataLog_Stream &os, const GRIP_BufferStatus &status);

//
// Print the status type to the stream
//
DataLog_Stream & operator << (DataLog_Stream &os, const GRIP_Status &status);

//
// Print the board type to the stream
//
DataLog_Stream & operator << (DataLog_Stream &os, const GRIP_BoardType &type);

//
// Prints the header to the stream.
//
DataLog_Stream & operator << (DataLog_Stream &os, const GRIP_Header *hdr);
#endif
