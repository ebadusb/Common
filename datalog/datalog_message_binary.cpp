/*
 * Copyright (C) 2002 Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: K:/BCT_Development/vxWorks/Common/datalog/rcs/datalog_message_binary.cpp 1.2 2003/02/25 16:10:16 jl11312 Exp jl11312 $
 * $Log: datalog_message_binary.cpp $
 * Revision 1.2  2003/02/25 16:10:16  jl11312
 * - modified buffering scheme to help prevent buffer overruns
 * Revision 1.1  2003/02/06 20:41:44  jl11312
 * Initial revision
 *
 */

#include "datalog.h"
#include "datalog_internal.h"
#include "datalog_records.h"

DataLog_Result datalog_WriteBinaryRecord(DataLog_Handle handle, DataLog_UINT16 type, DataLog_UINT16 subType, void * data, size_t size)
{
	DataLog_Result	result = DataLog_OK;
	DataLog_CommonData common;
	DataLog_BufferChain outputChain;
	unsigned long reserveBuffers = (handle->_type == DataLog_HandleInfo::CriticalHandle) ? 0 : common.criticalReserveBuffers();

	DataLog_UINT16	recordType = DataLog_BinaryRecordID;
	DataLog_UINT32	recordSize = size+4;

	DataLog_BufferManager::createChain(outputChain, reserveBuffers);
	DataLog_BufferManager::writeToChain(outputChain, (DataLog_BufferData *)&recordType, sizeof(recordType));
	DataLog_BufferManager::writeToChain(outputChain, (DataLog_BufferData *)&recordSize, sizeof(recordSize));
	DataLog_BufferManager::writeToChain(outputChain, (DataLog_BufferData *)&type, sizeof(type));
	DataLog_BufferManager::writeToChain(outputChain, (DataLog_BufferData *)&subType, sizeof(subType));
	DataLog_BufferManager::writeToChain(outputChain, (DataLog_BufferData *)data, size);
	DataLog_BufferManager::addChainToList( (handle->_type == DataLog_HandleInfo::CriticalHandle) ?
									DataLog_BufferManager::CriticalList : DataLog_BufferManager::TraceList, outputChain);

	return result;
}

