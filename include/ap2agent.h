/*
* Copyright (c) 1995, 1996 by Cobe BCT, Inc.  All rights reserved.
*
* AP2AGENT.h
*
* Abstract : 12/04/96
*          Interface file for AN2 msg definitions 
*/

#ifndef _AP2AGENT_H                 /* include guard */
#define _AP2AGENT_H

// NOTE THIS IS USED FOR TESTING...
// #define RUN_WINDOWS

// AGENT Interface return values
// This ENUM describes the possible return values that an AP2 Agent method
// may return.  They are used to help determine the reason for failure.
//  
enum AGENT_INTERFACE_RETURN
{
   AGENT_OK = 0,
   AGENT_FAIL = 1,
   AGENT_ALREADY_INITIALIZED,
   AGENT_FILE_DOES_NOT_EXIST,
   AGENT_SECTION_DOES_NOT_EXIST,
   AGENT_VARIABLE_DOES_NOT_EXIST,
   AGENT_VALUE_OUT_OF_RANGE_HIGH,
   AGENT_VALUE_OUT_OF_RANGE_LOW,
   AGENT_FILE_NOT_WRITEN,
   AGENT_FAILED_COMMIT,
   AGENT_FAILED_REJECT,
   AGENT_INVALID_CRC,
   AGENT_INVALID_LINE,
   AGENT_INCONSISTENT_DATA,
   AGENT_VALUE_INCORRECT_TYPE,
   AGENT_FILE_NOT_WRITEABLE,
};

// AGENT Connected state enumeration.
// This describes the possible return values for connected state as an enum.
//  
enum AGENT_CONNECTED_STATES
{
   AP2_CONNECTED = 0,
   AP2_NOT_CONNECTED,
   AP2_CONNECTION_UNKNOWN
};

static const int MAX_SECTION_BUFFER_LENGTH = 128;

// Message definition for the basis of AgentInterface.  This buffer will be sent
// by the agent when a section has been updated in the File store.  Contained in
// the message is the name of the section which was updated.
typedef struct 
{
   char section_name[MAX_SECTION_BUFFER_LENGTH+1];
} AP2_Section_Name_Struct;


#define MACHINE_DIRECTORY "/d/machine"
#define CONFIG_FILE_NAME "config.dat"
#define DEFAULT_AP2_STATUS_DAT "ap2_status.dat"
#define DEFAULT_AP2_DIRECTORY "/trima/tcpip"
#define DEFAULT_UPDATE_DIRECTORY "/d/update"
#define VALID_FILE_DIRECTORY "/d"
#define LOCAL_HOST_IP_ADDRESS "127.0.0.1"
#define AGENT_SECTIONS_UPDATE_COMPLETE "AGENT_SECTIONS_UPDATE_COMPLETE"

#ifdef AGENT_FILE_PATHS

#ifdef RUN_WINDOWS
#define PROPER_DIRECTORY_SLASH "\\"
#define AP2_AGENT_FILE_PATH_NAME "d:\\ram"
#else
#define PROPER_DIRECTORY_SLASH "/"
#define AP2_AGENT_FILE_PATH_NAME "/ram"
#endif

#endif

#endif                              /* include guard */  
