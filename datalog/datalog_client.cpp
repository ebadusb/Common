/*
 * Copyright (C) 2002 Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: I:/BCT_Development/vxWorks/Common/datalog/rcs/datalog_client.cpp 1.5 2003/12/09 14:14:16Z jl11312 Exp $
 * $Log: datalog_client.cpp $
 * Revision 1.5  2003/12/09 14:14:16Z  jl11312
 * - corrected time stamp problem (IT 6668)
 * - removed obsolete code/data types (IT 6664)
 * Revision 1.4  2003/03/27 16:26:57Z  jl11312
 * - added support for new datalog levels
 * Revision 1.3  2003/02/25 16:10:09Z  jl11312
 * - modified buffering scheme to help prevent buffer overruns
 * Revision 1.2  2003/01/31 19:52:50  jl11312
 * - new stream format for datalog
 * Revision 1.1  2002/08/22 20:19:03  jl11312
 * Initial revision
 *
 */

#include "datalog.h"

#include <fcntl.h>
#include "datalog_internal.h"

#ifdef DATALOG_LEVELS_INIT_SUPPORT
# include "datalog_levels.h"
#endif /* ifdef DATALOG_LEVELS_INIT_SUPPORT */

DataLog_NetworkClientTask::DataLog_NetworkClientTask(int clientSocket, struct sockaddr_in * clientAddr)
{
	_clientSocket = clientSocket;
	inet_ntoa_b(clientAddr->sin_addr, _asciiAddr);

	_tempBuffer = new DataLog_BufferData[DataLog_BufferSize];
	_isExiting = false;
	_criticalOutput = false;
}

void DataLog_NetworkClientTask::main(void)
{
	_state = WaitStart;

	while ( !_isExiting )
	{
		DataLog_NetworkPacket	packet;
		if ( readData((DataLog_BufferData * )&packet, sizeof(packet)) )
		{
			handlePacket(packet);
		}
	}
}

void DataLog_NetworkClientTask::handlePacket(const DataLog_NetworkPacket & packet)
{
	switch ( _state )
	{
	case WaitStart:
		switch ( packet._type )
		{
		case DataLog_StartTraceOutputRecord:
		case DataLog_StartCriticalOutputRecord:
			DataLog_BufferManager::createChain(_dataChain);
			_state = WaitEnd;
			_criticalOutput = ( packet._type == DataLog_StartCriticalOutputRecord);
			break;

		case DataLog_EndConnection:
			{
#ifdef DATALOG_LEVELS_INIT_SUPPORT
				DataLog(log_level_datalog_info)
#else /* ifdef DATALOG_LEVELS_INIT_SUPPORT */
				DataLog_Default
#endif /* ifdef DATALOG_LEVELS_INIT_SUPPORT */
					<< "Normal shutdown of client task for " << _asciiAddr << endmsg;

				_isExiting = true;
			}
			break;

		default:
			processInvalidPacket(packet);
			break;
		}
		break;

	case WaitEnd:
		switch ( packet._type )
		{
		case DataLog_OutputRecordData:
			if ( packet._length <= DataLog_BufferSize && readData(_tempBuffer, packet._length) )
			{
				DataLog_BufferManager::writeToChain(_dataChain, _tempBuffer, packet._length);
			}
			else
			{
				processInvalidPacket(packet);
			}
			break;

		case DataLog_EndOutputRecord:
			DataLog_BufferManager::addChainToList( (_criticalOutput) ? DataLog_BufferManager::CriticalList : DataLog_BufferManager::TraceList, _dataChain);
			_dataChain._head = DATALOG_NULL_SHARED_PTR;
			_state = WaitStart;
			break;

		default:
			processInvalidPacket(packet);
			break;
		}
		break;

	default:
		processInvalidPacket(packet);
		break;
	}
}

void DataLog_NetworkClientTask::processInvalidPacket(const DataLog_NetworkPacket & packet)
{
	DataLog_CommonData	common;

#ifdef DATALOG_LEVELS_INIT_SUPPORT
	DataLog(log_level_datalog_error)
#else /* ifdef DATALOG_LEVELS_INIT_SUPPORT */
	DataLog_Default
#endif /* ifdef DATALOG_LEVELS_INIT_SUPPORT */
		<< "Invalid packet from " << _asciiAddr <<
         " state=" << (int)_state <<
         " packet type=" << (int)packet._type <<
         " packet length=" << (int)packet._length << endmsg;

   common.setTaskError(DataLog_BadNetworkClientData, __FILE__, __LINE__);

	int	bytesLeft = packet._length;
	bool	readError = false;
	while ( bytesLeft > 0 && !readError )
	{
		int	bytesToRead = (bytesLeft > DataLog_BufferSize) ? DataLog_BufferSize : bytesLeft;
		readError = readData(_tempBuffer, bytesToRead);
		bytesLeft -= bytesToRead;
	}

	_state = WaitStart;
}

bool DataLog_NetworkClientTask::readData(DataLog_BufferData * buffer, size_t size)
{
	size_t	bytesRead = 0;
	bool	readError = false;

	while ( bytesRead < size && !readError )
	{
		int	readResult =  read(_clientSocket, (char *)&buffer[bytesRead], size-bytesRead);
		if ( readResult > 0 )
		{
			bytesRead += readResult;
		}
		else
		{
#ifdef DATALOG_LEVELS_INIT_SUPPORT
			DataLog(log_level_datalog_error)
#else /* ifdef DATALOG_LEVELS_INIT_SUPPORT */
			DataLog_Default
#endif /* ifdef DATALOG_LEVELS_INIT_SUPPORT */
				<< "Client task for " << _asciiAddr << " shutting down for read error: " <<
               readResult << " errno = " << errnoGet() << endmsg;

			DataLog_CommonData	common;
			common.setTaskError(DataLog_BadNetworkClientData, __FILE__, __LINE__);
			_isExiting = true;

			readError = true;
		}
	}

	return !readError;
}

