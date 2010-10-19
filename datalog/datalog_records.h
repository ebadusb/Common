/*
 * Copyright (C) 2002 Gambro BCT, Inc.  All rights reserved.
 *
 * $Header$
 * $Log: datalog_records.h $
 * Revision 1.2  2004/06/07 16:55:54Z  jl11312
 * - correct compiler wanrings for simpc under VXWorks 5.5
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
Packing_Structure_Directive struct DataLog_HeaderRecord
{
	DataLog_UINT16	_recordType Packing_Element_Directive;
	DataLog_UINT16	_charSize Packing_Element_Directive;
	DataLog_UINT16	_intSize Packing_Element_Directive;
	DataLog_UINT16	_longSize Packing_Element_Directive;
	DataLog_UINT16	_floatSize Packing_Element_Directive;
	DataLog_UINT16	_doubleSize Packing_Element_Directive;
	DataLog_UINT16	_taskIDSize Packing_Element_Directive;
	DataLog_TaskID	_currentTaskID Packing_Element_Directive;
	DataLog_UINT16	_nodeIDSize Packing_Element_Directive;
	DataLog_UINT16	_version Packing_Element_Directive;
};

Packing_Structure_Directive struct DataLog_NetworkHeaderRecord
{
	DataLog_UINT16	_recordType Packing_Element_Directive;
	DataLog_NodeID _nodeID Packing_Element_Directive;
	DataLog_TimeStampStart _start;
	DataLog_UINT16 _nodeNameLen Packing_Element_Directive;
};

Packing_Structure_Directive struct DataLog_LogLevelRecord
{
	DataLog_UINT16	_recordType Packing_Element_Directive;
	DataLog_TimeStamp _timeStamp;
	DataLog_InternalID _levelID;
	
#ifdef DATALOG_NETWORK_SUPPORT
	DataLog_NodeID _nodeID Packing_Element_Directive;
#endif /* ifdef DATALOG_NETWORK_SUPPORT */

	DataLog_UINT16	_nameLen Packing_Element_Directive;
};

Packing_Structure_Directive struct DataLog_PrintOutputRecord
{
	DataLog_UINT16	_recordType Packing_Element_Directive;
	DataLog_TimeStamp _timeStamp;
	DataLog_InternalID _levelID;
	DataLog_TaskID	_taskID Packing_Element_Directive;
	
#ifdef DATALOG_NETWORK_SUPPORT
	DataLog_NodeID _nodeID Packing_Element_Directive;
#endif /* ifdef DATALOG_NETWORK_SUPPORT */

	DataLog_UINT16	_formatLen Packing_Element_Directive;
	DataLog_UINT16	_fileNameLen Packing_Element_Directive;
	DataLog_UINT16 _lineNum Packing_Element_Directive;
};

Packing_Structure_Directive struct DataLog_StreamOutputRecord
{
	DataLog_UINT16	_recordType Packing_Element_Directive;
	DataLog_TimeStamp _timeStamp;
	DataLog_InternalID _levelID;
	DataLog_TaskID	_taskID Packing_Element_Directive;
	
#ifdef DATALOG_NETWORK_SUPPORT
	DataLog_NodeID _nodeID Packing_Element_Directive;
#endif /* ifdef DATALOG_NETWORK_SUPPORT */

	DataLog_UINT16	_fileNameLen Packing_Element_Directive;
	DataLog_UINT16 _lineNum Packing_Element_Directive;
};

Packing_Structure_Directive struct DataLog_PeriodicOutputRecord
{
	DataLog_UINT16	_recordType Packing_Element_Directive;
	DataLog_TimeStamp _timeStamp;
	DataLog_InternalID _setID Packing_Element_Directive;
	
#ifdef DATALOG_NETWORK_SUPPORT
	DataLog_NodeID _nodeID Packing_Element_Directive;
#endif /* ifdef DATALOG_NETWORK_SUPPORT */

	DataLog_UINT16 _itemCount Packing_Element_Directive;
};

Packing_Structure_Directive struct DataLog_PeriodicSetRecord
{
	DataLog_UINT16	_recordType Packing_Element_Directive;
	DataLog_TimeStamp _timeStamp;
	DataLog_InternalID _setID Packing_Element_Directive;
	
#ifdef DATALOG_NETWORK_SUPPORT
	DataLog_NodeID _nodeID Packing_Element_Directive;
#endif /* ifdef DATALOG_NETWORK_SUPPORT */

	DataLog_UINT16	_nameLen Packing_Element_Directive;
};

Packing_Structure_Directive struct DataLog_PeriodicItemRecord
{
	DataLog_UINT16	_recordType Packing_Element_Directive;
	DataLog_TimeStamp _timeStamp;
	DataLog_InternalID _keyCode Packing_Element_Directive;
	
#ifdef DATALOG_NETWORK_SUPPORT
	DataLog_NodeID _nodeID Packing_Element_Directive;
#endif /* ifdef DATALOG_NETWORK_SUPPORT */

	DataLog_UINT16	_keyLen Packing_Element_Directive;
	DataLog_UINT16	_descLen Packing_Element_Directive;
	DataLog_UINT16	_formatLen Packing_Element_Directive;
};

Packing_Structure_Directive struct DataLog_TaskCreateRecord
{
	DataLog_UINT16	_recordType Packing_Element_Directive;
	DataLog_TimeStamp _timeStamp;
	DataLog_InternalID	_levelID;
	DataLog_TaskID	_taskID Packing_Element_Directive;

#ifdef DATALOG_NETWORK_SUPPORT
	DataLog_NodeID _nodeID Packing_Element_Directive;
#endif /* ifdef DATALOG_NETWORK_SUPPORT */

	DataLog_UINT16	_nameLen Packing_Element_Directive;
};

Packing_Structure_Directive struct DataLog_TaskDeleteRecord
{
	DataLog_UINT16	_recordType Packing_Element_Directive;
	DataLog_TimeStamp _timeStamp;
	DataLog_InternalID _levelID;
	DataLog_TaskID	_taskID Packing_Element_Directive;

#ifdef DATALOG_NETWORK_SUPPORT
	DataLog_NodeID _nodeID Packing_Element_Directive;
#endif /* ifdef DATALOG_NETWORK_SUPPORT */
};

Packing_Structure_Directive struct DataLog_FileCloseRecord
{
	DataLog_UINT16	_recordType Packing_Element_Directive;
	DataLog_TimeStamp _timeStamp;
};

Packing_Structure_Directive struct DataLog_WriteTimeRecord
{
	DataLog_UINT16	_recordType Packing_Element_Directive;
	DataLog_TimeStamp _timeStamp;
};

#endif /* ifndef _DATALOG_RECORDS_INCLUDE */

