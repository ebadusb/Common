/*
 * Copyright (C) 2002 Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: //bctquad3/home/BCT_Development/vxWorks/Common/datalog/rcs/datalog_client.cpp 1.5 2003/12/09 14:14:16Z jl11312 Exp rm70006 $
 * $Log: datalog_client.cpp $
 *
 */

#include <vxWorks.h>
#include <errnoLib.h>
#include <fcntl.h>
#include <ioLib.h>

#include "datalog.h"
#include "datalog_internal.h"

DataLog_NetworkClientTask::DataLog_NetworkClientTask(int clientSocket, struct sockaddr_in * clientAddr)
{
	_clientSocket = clientSocket;
	inet_ntoa_b(clientAddr->sin_addr, _asciiAddr);

	_isExiting = false;
	_exitCode = 0;
}

void DataLog_NetworkClientTask::exit(int code)
{
	_exitCode = code;
	_isExiting = true; 
}

int DataLog_NetworkClientTask::main(void)
{
	DataLog_CommonData	common;
	_clientBuffer = new DataLog_ClientBuffer(common.getCurrentMaxBufferSize());
	_state = WaitStart;

	while ( !_isExiting )
	{
		DataLog_NetworkPacket	packet;
		if ( readData((DataLog_BufferData * )&packet, sizeof(packet)) )
		{
			handlePacket(packet);
		}
	}

	return _exitCode;
}

void DataLog_NetworkClientTask::handlePacket(const DataLog_NetworkPacket & packet)
{
	switch ( _state )
	{
	case WaitStart:
		switch ( packet._type )
		{
		case DataLog_NotifyBufferSize:
			processBufferSizeRecord(packet._length);
			break;

		case DataLog_StartOutputRecord:
			_dataStream = &(_clientBuffer->streamWriteStart());
			_state = WaitEnd;
			break;

		case DataLog_EndConnection:
			{
				DataLog_Critical	critical;
				DataLog(critical) << "Normal shutdown of client task for " << _asciiAddr << endmsg;
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
			if ( packet._length <= MaxDataSize && readData(_tempBuffer, packet._length) )
			{
				_dataStream->write(_tempBuffer, packet._length);
			}
			else
			{
				processInvalidPacket(packet);
			}
			break;

		case DataLog_EndOutputRecord:
			_clientBuffer->streamWriteComplete();
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

void DataLog_NetworkClientTask::processBufferSizeRecord(DataLog_UINT16 packetLength)
{
	size_t	newBufferSize;
	if ( readData((DataLog_BufferData *)&newBufferSize, sizeof(newBufferSize)) )
	{
		if ( newBufferSize > _clientBuffer->size() )
		{
			delete _clientBuffer;
			_clientBuffer = new DataLog_ClientBuffer(newBufferSize);
		}
	}
}

void DataLog_NetworkClientTask::processInvalidPacket(const DataLog_NetworkPacket & packet)
{
	DataLog_CommonData	common;
	DataLog_Critical		critical;

	DataLog(critical) << "Invalid packet from " << _asciiAddr <<
                     " state=" << (int)_state <<
                     " packet type=" << (int)packet._type <<
                     " packet length=" << (int)packet._length << endmsg;

   common.setTaskError(DataLog_BadNetworkClientData, __FILE__, __LINE__);

	int	bytesLeft = packet._length;
	bool	readError = false;
	while ( bytesLeft > 0 && !readError )
	{
		int	bytesToRead = (bytesLeft > MaxDataSize) ? MaxDataSize : bytesLeft;
		readError = readData(_tempBuffer, bytesToRead);
		bytesLeft -= bytesToRead;
	}

	_state = WaitStart;
}

bool DataLog_NetworkClientTask::readData(DataLog_BufferData * buffer, size_t size)
{
	int	bytesRead = 0;
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
			DataLog_Critical	critical;
			DataLog(critical) << "Client task for " << _asciiAddr << " shutting down for read error: " << readResult <<
										" errno = " << errnoGet() << endmsg;

			DataLog_CommonData	common;
			common.setTaskError(DataLog_BadNetworkClientData, __FILE__, __LINE__);
			_isExiting = true;

			readError = true;
		}
	}

	return !readError;
}

