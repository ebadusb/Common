/*
 * Copyright (C) 2002 Gambro BCT, Inc.  All rights reserved.
 *
 * $Header$
 * $Log: datalog_output.cpp $
 * Revision 1.15  2008/05/19 21:14:02Z  estausb
 * Modified the windows port of datalog.
 * Revision 1.13  2003/12/09 14:14:34Z  jl11312
 * - corrected time stamp problem (IT 6668)
 * - removed obsolete code/data types (IT 6664)
 * Revision 1.12  2003/11/10 17:46:20Z  jl11312
 * - corrections from data log unit tests (see IT 6598)
 * Revision 1.11  2003/10/03 12:35:06Z  jl11312
 * - improved DataLog_Handle lookup time
 * - modified datalog signal handling to eliminate requirement for a name lookup and the semaphore lock/unlock that went with it
 * Revision 1.10  2003/06/18 18:56:14Z  jl11312
 * - handle both SIGINT and SIGQUIT
 * Revision 1.9  2003/02/25 20:43:04Z  jl11312
 * - added support for logging platform specific information in log header
 * Revision 1.8  2003/02/25 16:10:23  jl11312
 * - modified buffering scheme to help prevent buffer overruns
 * Revision 1.6  2003/02/06 20:41:30  jl11312
 * - added support for binary record type
 * - added support for symbolic node names in networked configurations
 * - enabled compression/encryption of log files
 * Revision 1.5  2002/11/20 16:52:06  rm70006
 * Changed code to match changes made to new inet.h
 * Revision 1.4  2002/09/23 15:35:36Z  jl11312
 * - fixed port number setting
 * Revision 1.3  2002/08/22 20:19:11  jl11312
 * - added network support
 * Revision 1.2  2002/08/15 20:53:57  jl11312
 * - added support for periodic logging
 * Revision 1.1  2002/07/18 21:20:58  jl11312
 * Initial revision
 *
 */

#include "datalog.h"

#include <fcntl.h>

#include "datalog_internal.h"
#include "datalog_records.h"
#include "zlib.h"

DataLog_BufferData * datalog_DefaultEncryptFunc(DataLog_BufferData * input, size_t inputLength, size_t * outputLength);
static DataLog_EncryptFunc * encryptFunc = datalog_DefaultEncryptFunc;

DataLog_Result datalog_SetEncryptFunc(DataLog_EncryptFunc * func)
{
	encryptFunc = func;
	return DataLog_OK;
}

DataLog_BufferData * datalog_DefaultEncryptFunc(DataLog_BufferData * input, size_t inputLength, size_t * outputLength)
{
	for ( size_t i=0; i<inputLength; i++ )
	{
		input[i] = input[i] ^ 0xa5;
	}

	*outputLength = inputLength;
	return input;
}

DataLog_OutputTask::DataLog_OutputTask(void)
{
	_state = Run;
	_outputSignal = datalog_CreateSignal("DataLog_Output");
	_dataLostSignal = datalog_CreateSignal("DataLog_DataLost");
}

void DataLog_OutputTask::exit(void)
{
	_state = Exit;
	datalog_SendSignal(_outputSignal);
}

void DataLog_OutputTask::exitImmediately(void)
{
	_state = ExitImmediately;
	datalog_SendSignal(_outputSignal);
}

void DataLog_OutputTask::main(void)
{
	DataLog_CommonData 	common;

	//
	// Write record for system level output
	//
	writeSystemLevelRecord();

	//
	// Main processing loop
	//
	bool	timeStampWritten;
	while ( _state == Run )
	{
		//
		// Write a time stamp record to specify when this batch of data was written
		// if we produce any output.
		//
		timeStampWritten = false;

		//
		// If there is no pending data to be processed, perform a flush
		// operation to insure the output file is up to date.
		//
		if ( !datalog_WaitSignal(_outputSignal, 0) )
		{
			flushOutput();
			datalog_WaitSignal(_outputSignal, 10000);
		}

		//
		// Wait for data available to be output
		//
		if ( datalog_WaitSignal(_dataLostSignal, 0) )
		{
			writeTimeStampRecord();
			timeStampWritten = true;

			writeMissedLogDataRecord();
		}

		DataLog_BufferChain data;
		bool isCritical;
		while ( _state != ExitImmediately &&
				  DataLog_BufferManager::getNextChain(data, &isCritical) )
		{
			if ( !timeStampWritten )
			{
				writeTimeStampRecord();
				timeStampWritten = true;
			}

			startOutputRecord(isCritical);

			DataLog_BufferPtr ptr = data._head;
			size_t length = data._head->_length;

			while ( ptr &&
                 length > 0 )
			{
				size_t writeLength = (length > DataLog_BufferSize) ? DataLog_BufferSize : length;
				writeOutputRecord(ptr->_data, writeLength);
				length -= writeLength;
				ptr = ptr->_next;
			}

			endOutputRecord();
			DataLog_BufferManager::addChainToList(DataLog_BufferManager::FreeList, data);
		}
	}

	shutdown();
}

void DataLog_OutputTask::writeMissedLogDataRecord(void)
{
	DataLog_UINT16	missedDataValue = 0x55ff;

	startOutputRecord(true);
	writeOutputRecord((DataLog_BufferData *)&missedDataValue, sizeof(missedDataValue));
	endOutputRecord();
}

void DataLog_OutputTask::writeTimeStampRecord(void)
{
	DataLog_WriteTimeRecord writeTimeRecord;

	writeTimeRecord._recordType = DataLog_WriteTimeRecordID;
	datalog_GetTimeStamp(&writeTimeRecord._timeStamp);

	startOutputRecord(false);
	writeOutputRecord((DataLog_BufferData *)&writeTimeRecord, sizeof(writeTimeRecord));
	endOutputRecord();	
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

#ifdef DATALOG_NETWORK_SUPPORT
	systemLevelRecord._nodeID = datalog_NodeID();
#endif /* ifdef DATALOG_NETWORK_SUPPORT */

	systemLevelRecord._nameLen = strlen(DATALOG_SYSTEM_LEVEL_NAME);

	size_t  bufferSize = sizeof(systemLevelRecord)+ systemLevelRecord._nameLen * sizeof(char);
	DataLog_BufferData * buffer = new DataLog_BufferData[bufferSize];
	memcpy(buffer, &systemLevelRecord, sizeof(systemLevelRecord));
	memcpy(&buffer[sizeof(systemLevelRecord)], DATALOG_SYSTEM_LEVEL_NAME, systemLevelRecord._nameLen * sizeof(char));

	startOutputRecord(false);
	writeOutputRecord(buffer, bufferSize);
	endOutputRecord();

	delete[] buffer;
}


DataLog_LocalOutputTask::DataLog_LocalOutputTask(const char * platformName, const char * nodeName, const char * platformInfo)
{
	DataLog_CommonData common;

	//
	// Output directly to disk file
	//
	_compressedFile = NULL;
	_outputFD = open(common.connectName(), DATALOG_OUTPUT_FILE_OPT, 0666);

	if ( _outputFD < 0 )
	{
		perror(common.connectName());
		common.setTaskError(DataLog_OpenOutputFileFailed, __FILE__, __LINE__);
		_state = Exit;
	}
	else
	{
		writeLogFileHeader(platformName, nodeName, platformInfo);
	}
}

void DataLog_LocalOutputTask::startOutputRecord(bool isCritical)
{
}

void DataLog_LocalOutputTask::writeOutputRecord(DataLog_BufferData * buffer, size_t size)
{
	if ( !_compressedFile )
	{
		_compressedFile = gzdopen(_outputFD, "wb1");
		if ( !_compressedFile )
		{
			DataLog_CommonData	common;

			perror(common.connectName());
			common.setTaskError(DataLog_OpenOutputFileFailed, __FILE__, __LINE__);
		}
	}

	DataLog_BufferData * encryptedBuffer = buffer;
	size_t  encryptedSize = size; 

	if ( encryptFunc )
	{
		encryptedBuffer = (*encryptFunc)(buffer, size, &encryptedSize);
	}

	gzwrite(_compressedFile, encryptedBuffer, encryptedSize);
}

void DataLog_LocalOutputTask::endOutputRecord()
{
}

void DataLog_LocalOutputTask::shutdown(void)
{
	if ( _outputFD >= 0 )
	{
		writeFileCloseRecord();
		if ( _compressedFile )
		{
			gzclose(_compressedFile);
		}
		else
		{
			close(_outputFD);
		}
	}

	_outputFD = -1;
	_compressedFile = NULL;
}

void DataLog_LocalOutputTask::flushOutput(void)
{
	if ( _compressedFile )
	{
		gzflush(_compressedFile, Z_SYNC_FLUSH);
	}
}

void DataLog_LocalOutputTask::writeLogFileHeader(const char * platformName, const char * nodeName, const char * platformInfo)
{
	DataLog_CommonData common;
	static const char * plainTextMsg = "CONFIDENTIAL: This file is intended only for the use of Terumo BCT and contains information that "
												  "is proprietary and confidential. You are hereby notified that any use, dissemination, distribution, "
												  "or copying of this file is strictly prohibited."
												  "\nLog file: ";
	write(_outputFD, (char *)plainTextMsg, strlen(plainTextMsg));

	write(_outputFD, (char *)common.connectName(), strlen(common.connectName()));
	if ( platformInfo )
	{
		write(_outputFD, "\n", 1);
		write(_outputFD, (char *)platformInfo, strlen(platformInfo));
	}

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

#ifdef DATALOG_NETWORK_SUPPORT
		sizeof(DataLog_NodeID),
#else /* ifdef DATALOG_NETWORK_SUPPORT */
		0,
#endif /* ifdef DATALOG_NETWORK_SUPPORT */

		(DATALOG_MAJOR_VERSION<<8) | DATALOG_MINOR_VERSION
	};

	write(_outputFD, (char *)&fileHeader, sizeof(fileHeader));
  int x = sizeof(unsigned long);
	DataLog_UINT16	platformNameLen = strlen(platformName);
	write(_outputFD, (char *)&platformNameLen, sizeof(DataLog_UINT16));
	write(_outputFD, (char *)platformName, platformNameLen*sizeof(char));

#ifdef DATALOG_NETWORK_SUPPORT
	DataLog_NodeID nodeId = datalog_NodeID();
	write(_outputFD, (char *)&nodeId, sizeof(DataLog_NodeID));
#endif /* ifdef DATALOG_NETWORK_SUPPORT */

	DataLog_TimeStampStart start;
	datalog_GetTimeStampStart(&start);
	write(_outputFD, (char *)&start, sizeof(start));

#ifdef DATALOG_NETWORK_SUPPORT
	DataLog_UINT16	nodeNameLen = strlen(nodeName);
	write(_outputFD, (char *)&nodeNameLen, sizeof(DataLog_UINT16));
	write(_outputFD, (char *)nodeName, nodeNameLen*sizeof(char));
#endif /* ifdef DATALOG_NETWORK_SUPPORT */
}

void DataLog_LocalOutputTask::writeFileCloseRecord(void)
{
	DataLog_FileCloseRecord fileCloseRecord;

	fileCloseRecord._recordType = DataLog_FileCloseRecordID;
	datalog_GetTimeStamp(&fileCloseRecord._timeStamp);

	startOutputRecord(true);
	writeOutputRecord((DataLog_BufferData *)&fileCloseRecord, sizeof(fileCloseRecord));
	endOutputRecord();
}

DataLog_NetworkOutputTask::DataLog_NetworkOutputTask(long connectTimeout, const char * nodeName)
{
	DataLog_CommonData common;

	//
	// Output to network socket
	//
	_outputFD = socket(AF_INET, SOCK_STREAM, 0);
	sockaddr_in	addr;

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr((char *)common.connectName());
	addr.sin_port = htons(common.connectPort());

	struct timeval	timeout = { connectTimeout, 0 };
	if ( connectWithTimeout(_outputFD, (sockaddr *)&addr, sizeof(addr), &timeout) != OK )
	{
		perror(common.connectName());
		common.setTaskError(DataLog_NetworkConnectionFailed, __FILE__, __LINE__);
		_state = Exit;
	}
	else
	{
		writeLogFileNetworkHeader(nodeName);
	}
}

void DataLog_NetworkOutputTask::writeLogFileNetworkHeader(const char * nodeName)
{
	size_t	bufferSize = sizeof(DataLog_NetworkHeaderRecord)+strlen(nodeName)*sizeof(char);
	DataLog_BufferData * buffer = new DataLog_BufferData[bufferSize];
	DataLog_NetworkHeaderRecord * networkHeader;

	networkHeader = (DataLog_NetworkHeaderRecord *)buffer;
	networkHeader->_recordType = DataLog_NetworkHeaderRecordID;
	networkHeader->_nodeID = datalog_NodeID();
	datalog_GetTimeStampStart(&networkHeader->_start);
	networkHeader->_nodeNameLen = strlen(nodeName);
	memcpy(&buffer[sizeof(DataLog_NetworkHeaderRecord)], nodeName, sizeof(char)*networkHeader->_nodeNameLen);

	startOutputRecord(true);
	writeOutputRecord(buffer, bufferSize);
	endOutputRecord();
}

void DataLog_NetworkOutputTask::startOutputRecord(bool isCritical)
{
	DataLog_NetworkPacket	packet;
	int	dataIndex = 0;

	//
	// Notify network client of start of output record data
	//
	packet._type = (isCritical) ? DataLog_StartCriticalOutputRecord : DataLog_StartTraceOutputRecord;
	packet._length = 0;
	write(_outputFD, (char *)&packet, sizeof(packet));
}

void DataLog_NetworkOutputTask::writeOutputRecord(DataLog_BufferData * buffer, size_t size)
{
	DataLog_NetworkPacket	packet;
	size_t dataIndex = 0;

	//
	// Send output record data
	//
	while ( dataIndex < size )
	{
		packet._type = DataLog_OutputRecordData;
		packet._length = ( size-dataIndex < DataLog_BufferSize ) ? size-dataIndex : DataLog_BufferSize;
		write(_outputFD, (char *)&packet, sizeof(packet));
		write(_outputFD, (char *)&buffer[dataIndex], packet._length);
		dataIndex += packet._length;
	}
}

void DataLog_NetworkOutputTask::endOutputRecord(void)
{
	//
	// Notify network client of end of output record data
	//
	DataLog_NetworkPacket	packet;

	packet._type = DataLog_EndOutputRecord;
	packet._length = 0;
	write(_outputFD, (char *)&packet, sizeof(packet));
}

void DataLog_NetworkOutputTask::shutdown(void)
{
	if ( _outputFD >= 0 )
	{
		writeConnectionEndRecord();
		::shutdown(_outputFD, 2);
		close(_outputFD);
	}

	_outputFD = -1;
}

void DataLog_NetworkOutputTask::writeConnectionEndRecord(void)
{
	DataLog_NetworkPacket	packet;

	packet._type = DataLog_EndConnection;
	packet._length = 0;
	write(_outputFD, (char *)&packet, sizeof(packet));
}

void DataLog_NetworkOutputTask::writeTimeStampRecord(void)
{
	//
	// Log file write time stamps are not used from network connections.
	// The time stamp will be written by the node performing the physical
	// write to the disk file.
	//
}

