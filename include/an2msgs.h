/*
* Copyright (c) 1995, 1996 by Cobe BCT, Inc.	All rights reserved.
*
* $Header: Q:/home1/COMMON_PROJECT/Source/INCLUDE/rcs/AN2MSGS.H 1.3 1999/07/13 18:16:06 TD10216 Exp TD10216 $
* $Log: AN2MSGS.H $
* Revision 1.1  1999/05/24 23:26:05  TD10216
* Initial revision
* Revision 1.26  1999/05/06 19:41:50  TD10216
* I/T 3841
* Revision 1.25	1999/04/03 14:53:32	TD10216
* Revision 1.24	1999/03/29 17:10:37	TD10216
* Revision 1.23	1998/11/06 22:36:02	TM02109
* Added WIN32 ifdefs..
* Revision 1.22	1998/10/22 21:24:14	TD10216
* add invalid message request id to an2 message request types - used for 
* invalid requests
* Revision 1.21	1998/10/20 21:49:53	TM02109
* Added an2_run_update for running a script from the update 
* directory during service mode.
* Revision 1.20	1998/09/24 22:00:36	TM02109
* Mods for adding a reply port which is different then the default.
* Also added dataChangedComplete.
* Revision 1.19	1998/09/18 20:36:09	TM02109
* Updates to return values.
* Revision 1.18	1998/09/17 18:36:24	TM02109
* Updates for the Agent, an2_load_agent, an2_update_config...
* Revision 1.17	1998/09/03 19:57:32	TM02109
* an2_load_agent does not need the type of load anymore.
* Revision 1.16	1998/08/28 00:01:24	TM02109
* Added the CRC in the agent interface requests.	This will give you
* consistent data or warn you if you read from a file that is not what
* you would expect with AGENT_INCONSISTENT_DATA return.
* Revision 1.15	1998/08/20 23:46:48	TM02109
* Modified to allow for the proper working function of checking for 
* conditions which do not allow the service to run and reporting
* status in both the send_reply but also in LOG_ERROR
* Revision 1.14	1998/08/19 21:12:11	TM02109
* Updated for Agent file reading and first level AP2 function.
* Revision 1.13	1998/08/12 20:53:58	TM02109
* New an2inetd updates
* Revision 1.12	1998/08/11 00:16:41	TM02109
* Modified for AP2 broadcast startup and load_agent startup.
* Revision 1.11	1998/08/07 23:54:57	TM02109
* Changes to add Reject code and the code necessary for first level
* modifications to an2port and an2inetd.
* Revision 1.10	1997/05/29 18:04:02	SM02805
* Revision 1.9	1997/01/29 00:02:02	TM02109
* Added the machine name to the status message.
* Revision 1.8	1997/01/28 18:06:35	TM02109
* Modified the status message to send data necessary for setting
* up a procedure display.
* Revision 1.7	1997/01/17 04:03:03	TM02109
* Modified status message to include fs date, time and file index.
* Revision 1.6	1996/12/26 23:37:48	TM02109
* Implemented services for masking and unmasking QNX servers. IF 
* the servers are masked, external qnx commands submitted via qnx
* IPC are ignored.
* Revision 1.5	1996/12/17 17:23:43	TM02109
* Added the request for an2_comd.
* Revision 1.4	1996/12/13 20:23:28	TM02109
* Modified to add a password to the requests.
* Revision 1.3	1996/12/13 16:46:38	TM02109
* Added service support for disk crc update, status ping reply.
* Revision 1.2	1996/12/10 17:56:05	TM02109
* Modified with new services.
* Revision 1.1	1996/12/10 12:35:55	TM02109
* Initial revision
* Revision 1.2	1996/12/09 22:26:54	TM02109
* Use a char array for the port number.
* Revision 1.1	1996/12/09 19:06:42	TM02109
* Initial revision
*
* an2msgs.h
*
* Abstract : 12/04/96
*			Interface file for AN2 msg definitions 
*/
#ifndef _AN2MSGS_H						/* include guard */
#define _AN2MSGS_H

// AN2 Service Request Port Managed by the FS Host 
#define FS_AN2_SERVICE_PORT			7000	
#define FS_AN2_REPLY_PORT_CHAR		"7001"
#define FS_AN2_REPLY_PORT			7001
#define FS_AN2_BROADCAST_PORT_CHAR	"7002"
#define FS_AN2_BROADCAST_PORT		7002

#ifndef WIN32
// structures defined in buffmsg.hpp 
// buffmsg class must be explicitly called here for support
// in the process_fs_msg routine 
//
struct MSGDATA
{
	unsigned char buffer[1];	// length allocated in constructor
};

struct MSG
{
	MSGHEADER	header;
	MSGDATA		data;
};
#endif 

// AN2 Services
//
// NOTE: DO NOT REMOVE ANY OF THE OLD ENUMS UNLESS YOU WISH TO BREAK BACKWARD COMPAT.
//
// A remote AN2 Host may send service request messages to the AN2 Service Port 
// on a FS ( port number FS_AN2_SERVICE_PORT ) requesting services enumerated
// 
// NOTE: Please update AN2_SERVICES_STRINGS at the same time as AN2_SERVICES
// s/t the strings can be accessed by the enum index
//	
enum AN2_SERVICES
{
	NO_SERVICE = 0,
	START_AN2_PORT,
	START_FTP,					//	This service is no longer supported. 
	START_TCPIP_GATEWAY,
	START_DISKCRC_UPDATE,		//	This service is no longer supported. 
	START_COM_DATA,
	GET_STATUS,					//	Not used after Release 3.2
	STOP_AN2_PORT, 
	STOP_FTP,					//	This service is no longer supported. 
	STOP_TCPIP_GATEWAY, 
	STOP_COM_DATA,
	MASK_QNX_SERVERS,			//	This service is no longer supported. 
	UNMASK_QNX_SERVERS,			//	This service is no longer supported. 
	LOAD_AGENT_FILE,
	UPDATE_CONFIG_FILE,
	CONNECT_RESPONSE,
	NOT_CONNECTED_RESPONSE,
	RUN_UPDATE_SCRIPT,
	START_STATUS,				// Continuous an2_statusd: 3.3a & above
	STOP_STATUS,
	REGISTER_TCP_MSGS			// register for tcp_gate messages
};

#ifdef AN2INETD
char *AN2_SERVICES_STRINGS[] =
{
	"NO_SERVICE",
	"START_AN2_PORT",
	"START_FTP",				//	This service is no longer supported. 
	"START_TCPIP_GATEWAY",
	"START_DISKCRC_UPDATE",		//	This service is no longer supported. 
	"START_COM_DATA",
	"GET_STATUS",					//	Not used after Release 3.2
	"STOP_AN2_PORT", 
	"STOP_FTP",					//	This service is no longer supported. 
	"STOP_TCPIP_GATEWAY", 
	"STOP_COM_DATA",
	"MASK_QNX_SERVERS",			//	This service is no longer supported. 
	"UNMASK_QNX_SERVERS",		//	This service is no longer supported. 
	"LOAD_AGENT_FILE",
	"UPDATE_CONFIG_FILE",
	"CONNECT_RESPONSE",
	"NOT_CONNECTED_RESPONSE",
	"RUN_UPDATE_SCRIPT",
	"START_STATUS",				// Continuous an2_statusd: 3.3a & above
	"STOP_STATUS",
	"REGISTER_TCP_MSGS"			// register for tcp_gate messages
};
#else
extern char *AN2_SERVICES_STRINGS[];
#endif

// AN2 Current running services struct
typedef struct
{
	int		iSeqNo;					// packet count since service started
	int		iSystemState;			// Current system state of the focus system.
	int		iSubState;				// Current sub-state of the focus system.
	int		iServices;				// Bit field of START... AN2_SERVICES.
	int		iFileIndex;				// file index for run data file
	int		iCurrentAlarm;			// Current alarm name
	float	fProcedureTargetTime;	// procedure target time
	float	fCurrentProcedureTime;	// current procedure time
	float	fYieldTarget;			// Current projected yield target
	float	fCurrentYield;			// Current yield 
	float	fPlasmaTarget;			// Current projected plasma target
	float	fCurrentPlasma;			// Current plasma volume
	float	fRBCTarget;				// Current projected RBC target
	float	fCurrentRBC;			// Current RBC volume
	char	szMachineName[32];		// Name of machine status came from
	char	szMMDDYY[9];			// mm/dd/yy FS date
	char	szHHMMSS[9];			// hh:mm:ss FS time
	char	cInAlarmMode;			// Bool set when in an alarm
}SFSAN2Status;

// AN2 Service Request Responses
// The FS either allow a service request and if required starts
// the server required on the FS for the service OR the FS denies the service 
// request 
enum AN2_SERVICE_REQUEST_REPSONSE
{
	SERVICE_ALLOWED,								
	SERVICE_DENIED_DONOR_CONNECTED,	// ftp operations are not permitted during donor connected states
	SERVICE_DENIED,					// non-differentaited service denial...specific conditions will be defined as known 
	SERVICE_NOT_AVAILABLE,
	SERVICE_CONNECTED_ALREADY,
	SERVICE_COMD_NOT_AVAILABLE,
	SERVICE_DENIED_REQUESTOR_NOT_CURRENT_SERVER,
	SERVICE_DENIED_AP2_DISALLOWED,
	SERVICE_DENIED_NOT_IN_SERVICE_MODE,
	SERVICE_DENIED_INVALID_FILE,
	SERVICE_DENIED_INVALID_FILE_CRC,
	SERVICE_DENIED_INVALID_FILENAME,
	SERVICE_DENIED_INVALID_LOAD_REQUEST,
	SERVICE_DENIED_LOAD_AGENT_NOT_ENABLED,
	SERVICE_DENIED_CANNOT_UPDATE_AGENT,
	SERVICE_DENIED_INVALID_REQUEST_ID,
};

typedef struct 
{
	int	iAN2ServicePassword;	// AN2 Service password.
	int	iAN2ServiceFrequency;	// AN2 Frequency of requested service.
	char	szAN2HostIP[16];	// AN2 Host IP format ip addreess : xxx.xxx.xxx
	char	szAN2HostPort[8];	// AN2 Host Port
}HOST_IP_PORT_STRUCT;

typedef struct
{
	char	szFileName[32];
	char	szFileCRC[9];
}LOAD_AGENT_FILE_STRUCT;

// SPECIFICATION:	structure associated with
//						AN2 : AN2ServiceRequestMsg msg 
//
// ERROR HANDLING:	none.
//
// NOTES :
// To request an AN2 service, the AN2 Host submits a service request to the
// FS Host of interest on the FS_AN2_SERVICE_PORT using the FS msg 
// AN2ServiceRequestMsg and this structure.	The requestor supplies the
// requested service ID and the IP address and port ( UDP socket ) the requestor 
// is prepared to manage for erquested service communications. 
// 

typedef struct
{
	int	iAN2ServiceRequest;	// AN2 Service being requested from AN2 host
	union
	{
	HOST_IP_PORT_STRUCT host_struct;
	LOAD_AGENT_FILE_STRUCT load_struct;
	};
}SAN2ServiceRequest;

// SPECIFICATION: structure associated with
//			AN2 : AN2ServiceRequestReplyMsg msg 
//
// ERROR HANDLING:	none.
//
// NOTES :
// To reply to an AN2 service request, the FS Host replys to the requestor ( AN2 Host )
// using the AN2ServiceRequestReplyMsg msg and this structure. The FS supplies the
// requested service ID and the IP address and port ( UDP socket ) the FS 
// is prepared to manage for requested service communications.	The FS supplies to the 
// AN2 Host its state and the result of the service request : allowed or denied. 
// 
typedef struct
{
	int		iAN2ServiceRequested;		// AN2 Service being requested from AN2 host
	int		iFSHostPort;				// FS Host Port
	int		iFSState;					// FS State at time of service request
	int		iAN2ServiceRequestStatus;	// result of service request Service Started || Service Denied
	char	szFSHostIP[16];				// FS Host IP
}SAN2ServiceRequestReply;

// Specification
// Structure sent by an2_broadcast at Trima boot time to indicate its
// readiness to get donor and configuration info from an external host
// as well as to communicate some basic information about itself.
typedef struct
{
	int	iSeqNo;						// count of packets broadcasted
	char szTrimaHWRev[64];			// Control & Safety Hdw Revisions
	char szTrimaIP[32];				// 172.80.90.34
	char szTrimaLogFile[32];		// T00015_0504_0033
	char szTrimaSerialNumber[16];	// T00015
	char szTrimaCRC[16];			// 0xadcef73
	char szTrimaRelease[16];		// Trima 3.3a
	char szTrimaBuild[16];			// 1.191
	char szTrimaPort[8];            // 7001
}SAN2Broadcast;

// Specification
// Structure to be used for tcp_gate message registration
// The tcp_gate registration info is 'tacked on' to the normal
// SAN2ServiceRequest structure and ignored by an2inetd but 
// picked up by tcp_gate.
// Since we use fixed sized messages, specify registration through
// a structure which indicates a count of messages to register for
// and a fixed sized array where count MsgIds of interest are placed.
// USAGE: 
// A negative MsgId value in MsgIds indicates that the client
// wishes to de-register for that particular message
// To turn off all messages, use StopService or a count of 0
// To get all messages, send a count of -1.
#define MAXREG 64	// maximum registration # per call
typedef struct
{
	int count;			// # of messages being registered
	short MsgIds[MAXREG];		// array of MsgIds where count are valid
} TCPGate_Reg_Struct;

// what an2inetd sends to tcp_gate
typedef struct
{
	int iAN2ServiceRequest;
	HOST_IP_PORT_STRUCT host_struct; // Request header
	TCPGate_Reg_Struct Reg_Struct;	// Request info
} TCPGate_Reg_ReqStruct;

// what an2inetd & tcp_gate get
typedef struct
{
	MSGHEADER hdr;			// internal Msg Header
	TCPGate_Reg_ReqStruct Msg_Struct;	// Request info
} TCPGate_Reg_ReqMsg;

#endif										/* include guard */	
