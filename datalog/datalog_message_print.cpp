/*
 * Copyright (C) 2002 Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: //bctquad3/home/BCT_Development/vxWorks/Common/datalog/rcs/datalog_message_print.cpp 1.4 2002/10/08 12:09:50Z jl11312 Exp rm70006 $
 * $Log: datalog_message_print.cpp $
 *
 */

#include <vxWorks.h>
#include <stdarg.h>
#include "datalog.h"
#include "datalog_internal.h"

enum PrintArg_Type
{
	PrintArg_Char,
	PrintArg_Int,
	PrintArg_Long,
	PrintArg_Float,
	PrintArg_Double,
	PrintArg_String,
	PrintArg_Done,
	PrintArg_Error
};

static PrintArg_Type getNextFormatArg(const char * format, size_t& currentIndex, unsigned int& starModifierCount);
static DataLog_Result datalog_VPrint(DataLog_Handle handle, const char * file, int line, const char * format, va_list argList);

DataLog_Result datalog_Print(DataLog_Handle handle, const char * file, int line, const char * format, ...)
{
	va_list	argList;
	va_start(argList, format);
	return datalog_VPrint(handle, file, line, format, argList);
}

DataLog_Result datalog_PrintToDefault(const char * file, int line, const char * format, ...)
{
	va_list	argList;
	va_start(argList, format);

	DataLog_Handle handle;
	DataLog_Result	result = datalog_GetDefaultLevel(&handle);
	if ( result == DataLog_OK )
	{
		result = datalog_VPrint(handle, file, line, format, argList);
	}

	return result;
}

DataLog_Result datalog_VPrint(DataLog_Handle handle, const char * file, int line, const char * format, va_list argList)
{
	DataLog_Result	result = DataLog_OK;
	DataLog_CommonData * common = datalog_GetCommonDataPtr();
	DataLog_OutputBuffer * outputBuffer = NULL;
	DataLog_EnabledType logOutput = DataLog_LogEnabled;
	DataLog_ConsoleEnabledType consoleOutput = DataLog_ConsoleDisabled;

	va_list	savedArgList = argList;

	/*
	 * Build initial portion of output record.  The arguments portion of the record is appended
	 * later, since we don't know the size of that area before parsing.
	 */
	DataLog_PrintOutputRecord printOutputRecord;
	printOutputRecord._recordType = DataLog_PrintOutputRecordID;
	datalog_GetTimeStamp(&printOutputRecord._timeStamp);

	switch ( handle->_type )
	{
	case DataLog_HandleInfo::TraceHandle:
		{
			printOutputRecord._levelID = handle->_traceData._id;
			printOutputRecord._taskID = datalog_CurrentTask();

			logOutput = handle->_traceData._logOutput;
			consoleOutput = handle->_traceData._consoleOutput;

			DataLog_CommonData::TaskInfoPtr	taskInfo = common->findTask(DATALOG_CURRENT_TASK);
			if ( taskInfo != DATALOG_NULL_SHARED_PTR )
			{
				logOutput = ( logOutput == DataLog_LogEnabled ) ? DataLog_LogEnabled : taskInfo->_logOutput;
				consoleOutput = ( consoleOutput == DataLog_ConsoleEnabled ) ? DataLog_ConsoleEnabled : taskInfo->_consoleOutput;
		   }

			outputBuffer = (common->getTaskTraceBuffer(DATALOG_CURRENT_TASK))->_buffer;
		}
		break;

	case DataLog_HandleInfo::IntHandle:
		{
			printOutputRecord._levelID = handle->_intData._id;
			printOutputRecord._taskID = DATALOG_CURRENT_TASK;

			logOutput = handle->_intData._logOutput;
			outputBuffer = handle->_intData._buffer;
		}
		break;

	case DataLog_HandleInfo::CriticalHandle:
		{
			printOutputRecord._levelID = 0;
			printOutputRecord._taskID = handle->_criticalData._id;

			consoleOutput = DataLog_ConsoleEnabled;
			outputBuffer = handle->_criticalData._buffer;
	   }
		break;

	default:
		{
			/*
			 *	Invalid handle type for print.  Report the error and choose reasonable
			 * defaults if continuing.
			 */
			common->setTaskError(DataLog_InvalidHandle, __FILE__, __LINE__);
			result = DataLog_Error;

			printOutputRecord._levelID = 0;
			printOutputRecord._taskID = DATALOG_CURRENT_TASK;
			outputBuffer = (common->getTaskTraceBuffer(DATALOG_CURRENT_TASK))->_buffer;
		}
		break;
	}

#ifndef DATALOG_NO_NETWORK_SUPPORT
	printOutputRecord._nodeID = datalog_NodeID();
#endif /* ifndef DATALOG_NO_NETWORK_SUPPORT */

	printOutputRecord._formatLen = strlen(format);
	printOutputRecord._fileNameLen = strlen(file);
	printOutputRecord._lineNum = line;

	if ( logOutput == DataLog_LogEnabled )
	{
		outputBuffer->partialWriteStart();
		outputBuffer->partialWrite((DataLog_BufferData *)&printOutputRecord, sizeof(printOutputRecord));
		outputBuffer->partialWrite((DataLog_BufferData *)format, printOutputRecord._formatLen * sizeof(char));
		outputBuffer->partialWrite((DataLog_BufferData *)file, printOutputRecord._fileNameLen * sizeof(char));

		unsigned int	starModifierCount;
		size_t			currentIndex = 0;
		PrintArg_Type	argType;
		bool				argsDone = false;

		while ( !argsDone )
		{
			argType = getNextFormatArg(format, currentIndex, starModifierCount);
			if ( argType == PrintArg_Done )
			{
				argsDone = true;
		   }
		   else if ( argType == PrintArg_Error )
		   {
				/*
				 *	Send error notification, but otherwise ignore error
				 */
				common->setTaskError(DataLog_PrintFormatError, __FILE__, __LINE__);
				result = DataLog_Error;
		   }
		   else
		   {
				for ( unsigned int i=0; i<starModifierCount; i++ )
				{
					int	fieldLen = va_arg(argList, int);
					outputBuffer->partialWrite((DataLog_BufferData *)&fieldLen, sizeof(int));
				}

				switch ( argType )
				{
				case PrintArg_Char:
					{
						char	chArg = va_arg(argList, char);
						outputBuffer->partialWrite((DataLog_BufferData *)&chArg, sizeof(char));
					}
					break;

				case PrintArg_Int:
					{
						int	intArg = va_arg(argList, int);
						outputBuffer->partialWrite((DataLog_BufferData *)&intArg, sizeof(int));
					}
					break;

				case PrintArg_Long:
					{
						long	longArg = va_arg(argList, long);
						outputBuffer->partialWrite((DataLog_BufferData *)&longArg, sizeof(long));
					}
					break;

				case PrintArg_Float:
					{
						double	doubleArg = va_arg(argList, double);
						float		floatArg = (float)doubleArg;
						outputBuffer->partialWrite((DataLog_BufferData *)&floatArg, sizeof(float));
						printf("float = %g\n", floatArg);
					}
					break;

				case PrintArg_Double:
					{
						double	doubleArg = va_arg(argList, double);
						outputBuffer->partialWrite((DataLog_BufferData *)&doubleArg, sizeof(double));
						printf("double = %g\n", doubleArg);
					}
					break;

				case PrintArg_String:
					{
						const char * strArg = va_arg(argList, char *);
						DataLog_UINT16	strLen = strlen(strArg);
						outputBuffer->partialWrite((DataLog_BufferData *)&strLen, sizeof(DataLog_UINT16));
						outputBuffer->partialWrite((DataLog_BufferData *)strArg, strLen * sizeof(char));
					}
					break;

				default:
					break;
			   }
			}
		}

		outputBuffer->partialWriteComplete();
	}

	if ( consoleOutput == DataLog_ConsoleEnabled )
	{
		argList = savedArgList;

		printf("%s(%d): ", file, line);
		vprintf(format, argList);
		printf("\n");
	}

	return result;
}

static PrintArg_Type getNextFormatArg(const char * format, size_t& currentIndex, unsigned int& starModifierCount)
{
	enum ParseState
	{
		WaitPercent,
		WaitType
	};

	PrintArg_Type	result = 	PrintArg_Done;
	ParseState		parseState = WaitPercent;
	bool	longModifierPresent = false;
	bool	parseDone = false;
	char	type = '\0';

	starModifierCount = 0;
	while ( format &&
			  format[currentIndex] != '\0' &&
			  !parseDone )
	{
		if ( parseState == WaitPercent &&
		     format[currentIndex] == '%' )
		{
			parseState = WaitType;
		}
		else if ( parseState == WaitType )
		{
			switch ( format[currentIndex] )
			{
			case '%':
				if ( format[currentIndex-1] == '%' )
				{
					//
					// %% combination corresponds to single % on output
					//
					parseState = WaitPercent;
			   }
			   else
			   {
					result = PrintArg_Error;
					parseDone = true;
			   }

				break;
  
			case '*':
				starModifierCount += 1;
				break;

			case 'l':
				longModifierPresent = true;
				break;

			case 'c':
				result = PrintArg_Char;
				parseDone = true;
				break;

			case 'd':
			case 'u':
			case 'x':
				result = ( longModifierPresent ) ? PrintArg_Long : PrintArg_Int;
				parseDone = true;
				break;

			case 'f':
			case 'g':
				result = ( longModifierPresent ) ? PrintArg_Double : PrintArg_Float;
				parseDone = true;
				break;

			case 's':
				result = PrintArg_String;
				parseDone = true;
				break;
			}
		}

		currentIndex += 1;
	}

	if ( format[currentIndex] == '\0' &&
		  result == PrintArg_Done && 
		  parseState == WaitType )
	{
		//
		// Reached end of format string with incomplete format specifier
		//
		result = PrintArg_Error;
	}

	return result;
}
