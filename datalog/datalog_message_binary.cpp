/*
 * Copyright (C) 2002 Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: K:/BCT_Development/vxWorks/Common/datalog/rcs/datalog_message_binary.cpp 1.2 2003/02/25 16:10:16Z jl11312 Exp jl11312 $
 * $Log: datalog_message_binary.cpp $
 *
 */

#include <vxWorks.h>
#include "datalog.h"
#include "datalog_internal.h"

DataLog_Result datalog_WriteBinaryRecord(DataLog_Handle handle, void * data, size_t size)
{
	DataLog_Result	result = DataLog_OK;
	DataLog_CommonData common;
	DataLog_OutputBuffer * outputBuffer = NULL;

	switch ( handle->_type )
	{
	case DataLog_HandleInfo::TraceHandle:
		outputBuffer = common.getTaskTraceBuffer(DATALOG_CURRENT_TASK);
		break;

	case DataLog_HandleInfo::IntHandle:
		outputBuffer = handle->_intData._buffer;
		break;

	case DataLog_HandleInfo::CriticalHandle:
		outputBuffer = common.getTaskCriticalBuffer(DATALOG_CURRENT_TASK);
		break;

	default:
		/*
		 *	Invalid handle type for print.  Report the error and choose reasonable
		 * defaults if continuing.
		 */
		common.setTaskError(DataLog_InvalidHandle, __FILE__, __LINE__);
		result = DataLog_Error;

		outputBuffer = common.getTaskTraceBuffer(DATALOG_CURRENT_TASK);
		break;
	}

	DataLog_UINT16	recordType = DataLog_BinaryRecordID;
	DataLog_UINT32	recordSize = size;

	outputBuffer->partialWrite((DataLog_BufferData *)&recordType, sizeof(recordType));
	outputBuffer->partialWrite((DataLog_BufferData *)&recordSize, sizeof(recordSize));
	outputBuffer->partialWrite((DataLog_BufferData *)data, size);
   outputBuffer->partialWriteComplete();
	return result;
}

