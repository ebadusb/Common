/*
 * Copyright (C) 2002 Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: //bctquad3/home/BCT_Development/vxWorks/Common/datalog/rcs/datalog_records.h 1.2 2004/06/07 16:55:54Z jl11312 Exp rm70006 $
 * $Log: datalog_records.h $
 * Revision 1.1  2003/02/26 16:11:18Z  jl11312
 * Initial revision
 *
 */

#ifndef _DATALOG_RECORDS_INCLUDE
#define _DATALOG_RECORDS_INCLUDE

#include "datalog.h"

//
// This file defines record information for the data log file.  This information
// must agree with that used by the data log file decoder software.
//

enum
{
	DataLog_HeaderRecordID = 0x5500,
	DataLog_LogLevelRecordID = 0x5501,
	DataLog_PrintOutputRecordID = 0x5502,
	DataLog_StreamOutputRecordID = 0x5503,
	DataLog_PeriodicOutputRecordID = 0x5504,
	DataLog_PeriodicSetRecordID = 0x5505,
	DataLog_PeriodicItemRecordID = 0x5506,
	DataLog_TaskCreateRecordID = 0x5507,
	DataLog_TaskDeleteRecordID = 0x5508,
	DataLog_NetworkHeaderRecordID = 0x5509,

	DataLog_BinaryRecordID = 0x55f0,
	DataLog_EndOfNetworkOutputRecordID = 0x55fc,
	DataLog_FileCloseRecordID = 0x55fd,
	DataLog_WriteTimeRecordID = 0x55fe,
	DataLog_MissedDataRecordID = 0x55ff
};

/*
 *	All structures for log output must be packed to insure correct format
 */
struct DataLog_HeaderRecord
{
	DataLog_UINT16	_recordType __attribute__ ((packed));
	DataLog_UINT16	_charSize __attribute__ ((packed));
	DataLog_UINT16	_intSize __attribute__ ((packed));
	DataLog_UINT16	_longSize __attribute__ ((packed));
	DataLog_UINT16	_floatSize __attribute__ ((packed));
	DataLog_UINT16	_doubleSize __attribute__ ((packed));
	DataLog_UINT16	_taskIDSize __attribute__ ((packed));
	DataLog_TaskID	_currentTaskID __attribute__ ((packed));
	DataLog_UINT16	_nodeIDSize __attribute__ ((packed));
	DataLog_UINT16	_version __attribute__ ((packed));
};

struct DataLog_NetworkHeaderRecord
{
	DataLog_UINT16	_recordType __attribute__ ((packed));
	DataLog_NodeID _nodeID __attribute__ ((packed));
	DataLog_TimeStampStart _start;
	DataLog_UINT16 _nodeNameLen __attribute__ ((packed));
};

struct DataLog_LogLevelRecord
{
	DataLog_UINT16	_recordType __attribute__ ((packed));
	DataLog_TimeStamp _timeStamp;
	DataLog_InternalID _levelID;
	
#ifdef DATALOG_NETWORK_SUPPORT
	DataLog_NodeID _nodeID __attribute__ ((packed));
#endif /* ifdef DATALOG_NETWORK_SUPPORT */

	DataLog_UINT16	_nameLen __attribute__ ((packed));
};

struct DataLog_PrintOutputRecord
{
	DataLog_UINT16	_recordType __attribute__ ((packed));
	DataLog_TimeStamp _timeStamp;
	DataLog_InternalID _levelID;
	DataLog_TaskID	_taskID __attribute__ ((packed));
	
#ifdef DATALOG_NETWORK_SUPPORT
	DataLog_NodeID _nodeID __attribute__ ((packed));
#endif /* ifdef DATALOG_NETWORK_SUPPORT */

	DataLog_UINT16	_formatLen __attribute__ ((packed));
	DataLog_UINT16	_fileNameLen __attribute__ ((packed));
	DataLog_UINT16 _lineNum __attribute__ ((packed));
};

struct DataLog_StreamOutputRecord
{
	DataLog_UINT16	_recordType __attribute__ ((packed));
	DataLog_TimeStamp _timeStamp;
	DataLog_InternalID _levelID;
	DataLog_TaskID	_taskID __attribute__ ((packed));
	
#ifdef DATALOG_NETWORK_SUPPORT
	DataLog_NodeID _nodeID __attribute__ ((packed));
#endif /* ifdef DATALOG_NETWORK_SUPPORT */

	DataLog_UINT16	_fileNameLen __attribute__ ((packed));
	DataLog_UINT16 _lineNum __attribute__ ((packed));
};

struct DataLog_PeriodicOutputRecord
{
	DataLog_UINT16	_recordType __attribute__ ((packed));
	DataLog_TimeStamp _timeStamp;
	DataLog_InternalID _setID __attribute__ ((packed));
	
#ifdef DATALOG_NETWORK_SUPPORT
	DataLog_NodeID _nodeID __attribute__ ((packed));
#endif /* ifdef DATALOG_NETWORK_SUPPORT */

	DataLog_UINT16 _itemCount __attribute__ ((packed));
};

struct DataLog_PeriodicSetRecord
{
	DataLog_UINT16	_recordType __attribute__ ((packed));
	DataLog_TimeStamp _timeStamp;
	DataLog_InternalID _setID __attribute__ ((packed));
	
#ifdef DATALOG_NETWORK_SUPPORT
	DataLog_NodeID _nodeID __attribute__ ((packed));
#endif /* ifdef DATALOG_NETWORK_SUPPORT */

	DataLog_UINT16	_nameLen __attribute__ ((packed));
};

struct DataLog_PeriodicItemRecord
{
	DataLog_UINT16	_recordType __attribute__ ((packed));
	DataLog_TimeStamp _timeStamp;
	DataLog_InternalID _keyCode __attribute__ ((packed));
	
#ifdef DATALOG_NETWORK_SUPPORT
	DataLog_NodeID _nodeID __attribute__ ((packed));
#endif /* ifdef DATALOG_NETWORK_SUPPORT */

	DataLog_UINT16	_keyLen __attribute__ ((packed));
	DataLog_UINT16	_descLen __attribute__ ((packed));
	DataLog_UINT16	_formatLen __attribute__ ((packed));
};

struct DataLog_TaskCreateRecord
{
	DataLog_UINT16	_recordType __attribute__ ((packed));
	DataLog_TimeStamp _timeStamp;
	DataLog_InternalID	_levelID;
	DataLog_TaskID	_taskID __attribute__ ((packed));

#ifdef DATALOG_NETWORK_SUPPORT
	DataLog_NodeID _nodeID __attribute__ ((packed));
#endif /* ifdef DATALOG_NETWORK_SUPPORT */

	DataLog_UINT16	_nameLen __attribute__ ((packed));
};

struct DataLog_TaskDeleteRecord
{
	DataLog_UINT16	_recordType __attribute__ ((packed));
	DataLog_TimeStamp _timeStamp;
	DataLog_InternalID _levelID;
	DataLog_TaskID	_taskID __attribute__ ((packed));

#ifdef DATALOG_NETWORK_SUPPORT
	DataLog_NodeID _nodeID __attribute__ ((packed));
#endif /* ifdef DATALOG_NETWORK_SUPPORT */
};

struct DataLog_FileCloseRecord
{
	DataLog_UINT16	_recordType __attribute__ ((packed));
	DataLog_TimeStamp _timeStamp;
};

struct DataLog_WriteTimeRecord
{
	DataLog_UINT16	_recordType __attribute__ ((packed));
	DataLog_TimeStamp _timeStamp;
};

#endif /* ifndef _DATALOG_RECORDS_INCLUDE */

