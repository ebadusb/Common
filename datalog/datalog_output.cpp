/*
 * Copyright (C) 2002 Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: //bctquad3/home/BCT_Development/vxWorks/Common/datalog/rcs/datalog_output.cpp 1.4 2002/09/23 15:35:36Z jl11312 Exp rm70006 $
 * $Log: datalog_output.cpp $
 * Revision 1.1  2002/07/18 21:20:58  jl11312
 * Initial revision
 *
 */

#include "datalog.h"
#include "datalog_internal.h"

#include <fcntl.h>
#include <ioLib.h>

DataLog_OutputTask::DataLog_OutputTask(const char * platformName)
{
	DataLog_CommonData common;

	_isRunning = true;
	_isExiting = false;
	_exitCode = 0;

	if ( common.connectType() == DataLog_CommonData::LogToFile )
	{
		_outputFile = open(common.connectName(), O_WRONLY | O_CREAT, 0666);
		if ( _outputFile < 0 )
		{
			common.setTaskError(DataLog_OpenOutputFileFailed, __FILE__, __LINE__);
		}

		ftruncate(_outputFile, 0);
		writeLogFileHeader(platformName);
	}
	else
	{
		assert(0);
	}

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

	size_t	maxBufferSize = common.getCurrentMaxBufferSize();
	DataLog_BufferData	* tempBuffer = NULL;

	tempBuffer = new DataLog_BufferData[maxBufferSize];
	if ( common.connectType() == DataLog_CommonData::LogToNetwork )
	{
		notifyNetworkBufferSize(maxBufferSize);
	}

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

				if ( common.connectType() == DataLog_CommonData::LogToNetwork )
			   {
					notifyNetworkBufferSize(maxBufferSize);
			   }
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

void DataLog_OutputTask::notifyNetworkBufferSize(size_t size)
{
	assert(0);
}

void DataLog_OutputTask::shutdown(void)
{
	DataLog_CommonData	common;

	if ( common.connectType() == DataLog_CommonData::LogToFile )
	{
		writeFileCloseRecord();
		close(_outputFile);
	}
	else
	{
		assert(0);
	}
}

void DataLog_OutputTask::writeLogFileHeader(const char * platformName)
{
	static const char * plainTextMsg = "CONFIDENTIAL: This file is intended only for the use of Gambro BCT and contains information that "
												  "is proprietary and confidential. You are hereby notified that any use, dissemination, distribution, "
												  "or copying of this file is strictly prohibited.";
	write(_outputFile, (char *)plainTextMsg, strlen(plainTextMsg));
	write(_outputFile, "\032\004\000", 3);

	DataLog_UINT32	byteOrderMark = 0x12345678;
	write(_outputFile, (char *)&byteOrderMark, sizeof(byteOrderMark));

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

	write(_outputFile, (char *)&fileHeader, sizeof(fileHeader));

	DataLog_UINT16	platformNameLen = strlen(platformName);
	write(_outputFile, (char *)&platformNameLen, sizeof(DataLog_UINT16));
	write(_outputFile, (char *)platformName, platformNameLen);

	DataLog_TimeStampStart start;
	datalog_GetTimeStampStart(&start);
	write(_outputFile, (char *)&start, sizeof(start));
}

void DataLog_OutputTask::writeMissedLogDataRecord(void)
{
	DataLog_UINT16	missedDataValue = 0x55ff;

	writeOutputRecord((DataLog_BufferData *)&missedDataValue, sizeof(missedDataValue));
}

void DataLog_OutputTask::writeOutputRecord(DataLog_BufferData * buffer, size_t size)
{
	DataLog_CommonData	common;

	if ( common.connectType() == DataLog_CommonData::LogToFile )
	{
		write(_outputFile, (char *)buffer, size);
	}
	else
	{
		assert(0);
	}
}

void DataLog_OutputTask::writeFileCloseRecord(void)
{
	DataLog_FileCloseRecord fileCloseRecord;

	fileCloseRecord._recordType = DataLog_FileCloseRecordID;
	datalog_GetTimeStamp(&fileCloseRecord._timeStamp);
	writeOutputRecord((DataLog_BufferData *)&fileCloseRecord, sizeof(fileCloseRecord));	
}

void DataLog_OutputTask::writeTimeStampRecord(void)
{
	DataLog_WriteTimeRecord writeTimeRecord;

	writeTimeRecord._recordType = DataLog_WriteTimeRecordID;
	datalog_GetTimeStamp(&writeTimeRecord._timeStamp);
	writeOutputRecord((DataLog_BufferData *)&writeTimeRecord, sizeof(writeTimeRecord));
}

