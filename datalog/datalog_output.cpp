/*
 * Copyright (C) 2002 Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: //bctquad3/home/BCT_Development/vxWorks/Common/datalog/rcs/datalog_output.cpp 1.4 2002/09/23 15:35:36Z jl11312 Exp rm70006 $
 * $Log: datalog_output.cpp $
 * Revision 1.3  2002/08/22 20:19:11  jl11312
 * - added network support
 * Revision 1.2  2002/08/15 20:53:57  jl11312
 * - added support for periodic logging
 * Revision 1.1  2002/07/18 21:20:58  jl11312
 * Initial revision
 *
 */

#include <vxWorks.h>
#include <fcntl.h>
#include <ioLib.h>

#include "datalog.h"
#include "datalog_internal.h"

DataLog_OutputTask::DataLog_OutputTask(void)
{
	_isRunning = true;
	_isExiting = false;
	_exitCode = 0;
}

void DataLog_OutputTask::exit(int code)
{
	_exitCode = code;
	_isRunning = false;
	_isExiting = true;
	datalog_SendSignal("DataLog_Output");
}

int DataLog_OutputTask::main(void)
{
	DataLog_CommonData 	common;
	DataLog_InputBuffer	inputBuffer;

	//
	// Setup buffer area for reading data to be output
	//
	size_t	maxBufferSize = common.getCurrentMaxBufferSize();
	DataLog_BufferData	* tempBuffer = new DataLog_BufferData[maxBufferSize];
	handleBufferSizeChange(maxBufferSize);

	//
	// Write record for system level output
	//
	writeSystemLevelRecord();

	//
	// Main processing loop
	//
	bool	timeStampWritten;
	while ( !_isExiting )
	{
		//
		// Write a time stamp record to specify when this batch of data was written
		// if we produce any output.
		//
		timeStampWritten = false;

		//
		// Wait for data available to be output
		//
		datalog_WaitSignal("DataLog_Output", -1);
		if ( datalog_WaitSignal("DataLog_DataLost", 0) )
		{
			writeTimeStampRecord();
			timeStampWritten = true;

			writeMissedLogDataRecord();
		}

		//
		// Clear critical output available indication if it is present, since
		// we are about to scan the critical output buffers anyway.
		//
		datalog_WaitSignal("DataLog_CriticalOutput", 0);

		bool	attachOK = inputBuffer.attachToFirstBuffer();
		while ( attachOK && _isRunning )
		{
			//
			// Check if the local buffer is large enough.  If not, increase the
			// buffer size and notify the network client (if any) of the new size.
			//
			if ( inputBuffer.size() > maxBufferSize )
			{
				delete[] tempBuffer;
				maxBufferSize = inputBuffer.size();
				tempBuffer = new DataLog_BufferData[maxBufferSize];
				handleBufferSizeChange(maxBufferSize);
			}

			//
			// Copy data from the output buffer
			//
			size_t bytesRead = inputBuffer.read(tempBuffer, maxBufferSize);
			if ( bytesRead > 0 )
			{
				if ( !timeStampWritten )
				{
					writeTimeStampRecord();
					timeStampWritten = true;
				}

				writeOutputRecord(tempBuffer, bytesRead);
			}

			//
			// If we have gotten a signal for critical output available, restart
			// scan at beginning of buffer list (critical buffers are at the
			// beginning of the list).  We also clear any pending output signal
			// since we will be rescanning the entire buffer list.
			//
			if ( datalog_WaitSignal("DataLog_CriticalOutput", 0) )
			{
				attachOK = inputBuffer.attachToFirstBuffer();
				datalog_WaitSignal("DataLog_Output", 0);
			}
			else
			{
				attachOK = inputBuffer.attachToNextBuffer();
			}
		}
	}

	delete[] tempBuffer;
	shutdown();

	return _exitCode;
}

void DataLog_OutputTask::writeMissedLogDataRecord(void)
{
	DataLog_UINT16	missedDataValue = 0x55ff;

	writeOutputRecord((DataLog_BufferData *)&missedDataValue, sizeof(missedDataValue));
}

void DataLog_OutputTask::writeTimeStampRecord(void)
{
	DataLog_WriteTimeRecord writeTimeRecord;

	writeTimeRecord._recordType = DataLog_WriteTimeRecordID;
	datalog_GetTimeStamp(&writeTimeRecord._timeStamp);
	writeOutputRecord((DataLog_BufferData *)&writeTimeRecord, sizeof(writeTimeRecord));
}

void DataLog_OutputTask::writeSystemLevelRecord(void)
{
	//
	// Output record for system log level
	//
	DataLog_LogLevelRecord	systemLevelRecord;

	systemLevelRecord._recordType = DataLog_LogLevelRecordID;
	datalog_GetTimeStamp(&systemLevelRecord._timeStamp);
	systemLevelRecord._levelID = DATALOG_SYSTEM_LEVEL_ID;

#ifndef DATALOG_NO_NETWORK_SUPPORT
	systemLevelRecord._nodeID = datalog_NodeID();
#endif /* ifndef DATALOG_NO_NETWORK_SUPPORT */

	systemLevelRecord._nameLen = strlen(DATALOG_SYSTEM_LEVEL_NAME);

	size_t  bufferSize = sizeof(systemLevelRecord)+ systemLevelRecord._nameLen * sizeof(char);
	DataLog_BufferData * buffer = new DataLog_BufferData[bufferSize];
	memcpy(buffer, &systemLevelRecord, sizeof(systemLevelRecord));
	memcpy(&buffer[sizeof(systemLevelRecord)], DATALOG_SYSTEM_LEVEL_NAME, systemLevelRecord._nameLen * sizeof(char));
	writeOutputRecord(buffer, bufferSize);
	delete[] buffer;
}

DataLog_LocalOutputTask::DataLog_LocalOutputTask(const char * platformName)
{
	DataLog_CommonData common;

	//
	// Output directly to disk file
	//
	_outputFD = open(common.connectName(), O_WRONLY | O_CREAT, 0666);
	if ( _outputFD < 0 )
	{
		perror(common.connectName());
		common.setTaskError(DataLog_OpenOutputFileFailed, __FILE__, __LINE__);
	}

	ftruncate(_outputFD, 0);
	writeLogFileHeader(platformName);
}

void DataLog_LocalOutputTask::handleBufferSizeChange(size_t size)
{
}

void DataLog_LocalOutputTask::writeOutputRecord(DataLog_BufferData * buffer, size_t size)
{
	write(_outputFD, (char *)buffer, size);
}

void DataLog_LocalOutputTask::shutdown(void)
{
	writeFileCloseRecord();
	close(_outputFD);
}

void DataLog_LocalOutputTask::writeLogFileHeader(const char * platformName)
{
	static const char * plainTextMsg = "CONFIDENTIAL: This file is intended only for the use of Gambro BCT and contains information that "
												  "is proprietary and confidential. You are hereby notified that any use, dissemination, distribution, "
												  "or copying of this file is strictly prohibited.";
	write(_outputFD, (char *)plainTextMsg, strlen(plainTextMsg));
	write(_outputFD, "\032\004\000", 3);

	DataLog_UINT32	byteOrderMark = 0x12345678;
	write(_outputFD, (char *)&byteOrderMark, sizeof(byteOrderMark));

	static DataLog_HeaderRecord fileHeader =
	{
		DataLog_HeaderRecordID,
		sizeof(char),
		sizeof(int),
		sizeof(long),
		sizeof(float),
		sizeof(double),
		sizeof(DataLog_TaskID),
		DATALOG_CURRENT_TASK,

#ifndef DATALOG_NO_NETWORK_SUPPORT
		sizeof(DataLog_NodeID),
#else /* ifndef DATALOG_NO_NETWORK_SUPPORT */
		0,
#endif /* ifndef DATALOG_NO_NETWORK_SUPPORT */

		(DATALOG_MAJOR_VERSION<<8) | DATALOG_MINOR_VERSION
	};

	write(_outputFD, (char *)&fileHeader, sizeof(fileHeader));

	DataLog_UINT16	platformNameLen = strlen(platformName);
	write(_outputFD, (char *)&platformNameLen, sizeof(DataLog_UINT16));
	write(_outputFD, (char *)platformName, platformNameLen);

	DataLog_TimeStampStart start;
	datalog_GetTimeStampStart(&start);
	write(_outputFD, (char *)&start, sizeof(start));
}

void DataLog_LocalOutputTask::writeFileCloseRecord(void)
{
	DataLog_FileCloseRecord fileCloseRecord;

	fileCloseRecord._recordType = DataLog_FileCloseRecordID;
	datalog_GetTimeStamp(&fileCloseRecord._timeStamp);
	writeOutputRecord((DataLog_BufferData *)&fileCloseRecord, sizeof(fileCloseRecord));	
}

DataLog_NetworkOutputTask::DataLog_NetworkOutputTask(long connectTimeout)
{
	DataLog_CommonData common;

	//
	// Output to network socket
	//
	_outputFD = socket(AF_INET, SOCK_STREAM, 0);
	sockaddr_in	addr;

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(common.connectName());
	addr.sin_port = htons(common.connectPort());

	struct timeval	timeout = { connectTimeout, 0 };
	if ( connectWithTimeout(_outputFD, (sockaddr *)&addr, sizeof(addr), &timeout) != OK )
	{
		perror(common.connectName());
		common.setTaskError(DataLog_NetworkConnectionFailed, __FILE__, __LINE__);
	}
}

void DataLog_NetworkOutputTask::handleBufferSizeChange(size_t size)
{
	DataLog_NetworkPacket	packet;

	packet._type = DataLog_NotifyBufferSize;
	packet._length = sizeof(size_t);
	write(_outputFD, (char *)&packet, sizeof(packet));
	write(_outputFD, (char *)&size, sizeof(size));
}

void DataLog_NetworkOutputTask::writeOutputRecord(DataLog_BufferData * buffer, size_t size)
{
	DataLog_NetworkPacket	packet;
	int	dataIndex = 0;

	//
	// Notify network client of start of output record data
	//
	packet._type = DataLog_StartOutputRecord;
	packet._length = 0;
	write(_outputFD, (char *)&packet, sizeof(packet));

	//
	// Send output record data
	//
	while ( dataIndex < size )
	{
		packet._type = DataLog_OutputRecordData;
		packet._length = ( size-dataIndex < DataLog_NetworkClientTask::MaxDataSize ) ? size-dataIndex : DataLog_NetworkClientTask::MaxDataSize;
		write(_outputFD, (char *)&packet, sizeof(packet));
		write(_outputFD, (char *)&buffer[dataIndex], packet._length);
		dataIndex += packet._length;
	}

	//
	// Notify network client of end of output record data
	//
	packet._type = DataLog_EndOutputRecord;
	packet._length = 0;
	write(_outputFD, (char *)&packet, sizeof(packet));
}

void DataLog_NetworkOutputTask::shutdown(void)
{
	writeConnectionEndRecord();
   ::shutdown(_outputFD, 2);
   close(_outputFD);
}

void DataLog_NetworkOutputTask::writeConnectionEndRecord(void)
{
	DataLog_NetworkPacket	packet;

	packet._type = DataLog_EndConnection;
	packet._length = 0;
	write(_outputFD, (char *)&packet, sizeof(packet));
}

