/*****************************************************************************\
**
** LogDecoderBase.h
**
** DLog decoder subtype.
**
\*****************************************************************************/

#ifndef LOGDECODERBASE_H
#define LOGDECODERBASE_H

/* Includes ******************************************************************/

#include <map>
#include <sstream>
#include <cstring>
#include <cstdio>

#include "LogRecord.h"
#include "LogFileUtil.h"
#include "zlib.h"

namespace DECODER {

/* Constants *****************************************************************/

/* Globals *******************************************************************/

/* Macros ********************************************************************/

/* Prototypes ****************************************************************/

/* Functions *****************************************************************/

/* Classes *******************************************************************/

class LogDecoderUncompressed {
public:
	LogDecoderUncompressed(void) {}
	virtual ~LogDecoderUncompressed(void) {}

	virtual void ProcessData(FILE *file) const = 0;
};

class LogDecoderCompressed {
public:
	LogDecoderCompressed(void) {}
	virtual ~LogDecoderCompressed(void) {}

	virtual void ProcessData(gzFile file) const = 0;
};

template <class _TYPE>
class LogRegistration {
public:
	typedef _TYPE Interface;
	typedef _TYPE Data;
	typedef unsigned int HandlerID;

	template <class _HANDLER>
	void RegisterHandler(_HANDLER &handler) 
	{
		HandlerID id = reinterpret_cast<HandlerID>(&handler);

		HandlerMap::const_iterator iter = mHandlerMap.find(id);
		HandlerMap::const_iterator end = mHandlerMap.end();

		if (iter == end) {
			mHandlerMap[id] = new HandlerBinder<_HANDLER>(handler);
		}
	}

	template <class _HANDLER>
	void UnregisterHandler(_HANDLER &handler) 
	{
		HandlerID id = reinterpret_cast<HandlerID>(&handler);

		HandlerMap::const_iterator iter = mHandlerMap.find(id);
		HandlerMap::const_iterator end = mHandlerMap.end();

		if (iter != end) {
			delete iter->second;
			mHandlerMap.erase(iter);
		}
	}

	void ProcessData(const Data &data) const 
	{
		HandlerMap::const_iterator iter = mHandlerMap.begin();
		HandlerMap::const_iterator end = mHandlerMap.end();

		for (; iter != end; ++iter) {
			(*iter->second)(data);
		}
	}

private:
	class HandlerBase {
	public:
		virtual void operator()(const Data &data) = 0;
	};

	template <class _HANDLER>
	class HandlerBinder : public HandlerBase {
	public:
		typedef _HANDLER Handler;

		HandlerBinder(Handler &handler) : mHandler(handler) {}
		virtual ~HandlerBinder(void) {}

		virtual void operator()(const Data &data) { mHandler(data); }

	private:
		Handler &mHandler;
	};

	typedef std::map<HandlerID, HandlerBase*> HandlerMap;

	HandlerMap mHandlerMap;
};

class DecoderLogInfo : public LogDecoderUncompressed, public LogRegistration<RecordLogInfo> {
public:
	virtual void ProcessData(FILE *file) const
	{
		RecordLogInfo log;
		std::ostringstream message;
		Int8 tempChar = 0;

		while ((tempChar = fgetc(file)) != EOF) {
			if (tempChar == '\0') {
				break;
			}
			message << tempChar;
		}

		log.mMessage = message.str();
		//Sample mMessage: 
		/*
		CONFIDENTIAL: This file is intended only for the use of Gambro BCT blah blah .... 
		Log file: /machine/log/1T00500_20080623_003_000034.dlog
		Trima Build  BUILD REVISION: 8.503  CRC: 0x36c5a5d9  BUILD DATE: Tue 06/17/2008  04:56 PM  BUILD DIR: D:\trima\5.r  BUILD COMPUTER:  usbll31e5y6.bct.gambro.net  BUILD USER:  ssunusb
		*/
		std::istringstream in(log.mMessage.substr(log.mMessage.find("Log file:")), std::istringstream::in);
		
			std::string temp;
			in >> temp; in >> temp;//"Log file:"
			in >> log.mLogFilename;
			if(log.mLogFilename.find("1T") != log.mLogFilename.npos)
				log.mDeviceType = DECODER::TRIMA;
			else if( ( log.mLogFilename.find("1S") != log.mLogFilename.npos) || ( log.mLogFilename.find("1P") != log.mLogFilename.npos) )
				log.mDeviceType = DECODER::OPTIA;
			else if( log.mLogFilename.find("1C") != log.mLogFilename.npos)
				log.mDeviceType = DECODER::CES;
			else if( log.mLogFilename.find("1A") != log.mLogFilename.npos)
				log.mDeviceType = DECODER::ATREUS;
			else if( log.mLogFilename.find("1W") != log.mLogFilename.npos)
				log.mDeviceType = DECODER::MASSIVE;

			
			if(log.mDeviceType == DECODER::TRIMA) {
				in >> temp; in >> temp; in >> temp; in >> temp; //\nTrima Build  BUILD REVISION:
				in >> log.mBuildRevision;
				in >> temp; //CRC:
				in >> log.mCrc;
				in >> temp; in >> temp;//BUILD DATE:
				in >> log.mBuildDate;/*day*/ in >> temp; log.mBuildDate.append(" " + temp); /*date*/
				in >> temp; log.mBuildDate.append(" " + temp);/*time*/ in >> temp; log.mBuildDate.append(" " + temp); /*AM or PM*/
				in >> temp; in >> temp;//BUILD DIR:
				in >> log.mBuildDir;
				in >> temp; in >> temp;//BUILD COMPUTER:
				in >> log.mBuildComputer;
				in >> temp; in >> temp;//BUILD USER:
				in >> log.mBuildUser;
			}
			else { //Hopefully Optia, CES and Atreus behave similarly
				//Read until '='. If you reach a space without and '=' at the end, discard it. 
				Int8 tmpChar = 0;
				std::string temp;
				while ((tmpChar = in.get()) != '='); //revision=
				in >> log.mBuildRevision;
				while ((tmpChar = in.get()) != '='); //date=
				in >> log.mBuildDate;/*day*/ in >> temp; log.mBuildDate.append(" " + temp); /*date*/
				while ((tmpChar = in.get()) != '='); //time=
				in >> temp; log.mBuildDate.append(" " + temp);/*time*/ in >> temp; log.mBuildDate.append(" " + temp); /*AM or PM*/
				while ((tmpChar = in.get()) != '='); //directory=
				in >> log.mBuildDir;
				while ((tmpChar = in.get()) != '='); //computer=
				in >> log.mBuildComputer;
				while ((tmpChar = in.get()) != '='); //id=
				in >> log.mBuildUser;
				if(log.mDeviceType == DECODER::OPTIA)
				{	
						while ((tmpChar = in.get()) != '='); //version=
						in >> log.mVersion;
				}
			}

		this->LogRegistration<RecordLogInfo>::ProcessData(log); 
	}
};

class DecoderLogHeader : public LogDecoderUncompressed, public LogRegistration<RecordLogHeader> {
public:
	virtual void ProcessData(FILE *file) const
	{
		RecordLogHeader log;
		Uint16 platformNameLength = 0;
		Uint16 nodeNameLength = 0;

		fread(&log.mSizeOfChar, sizeof(log.mSizeOfChar), 1, file);
		fread(&log.mSizeOfInt, sizeof(log.mSizeOfInt), 1, file);
		fread(&log.mSizeOfLong, sizeof(log.mSizeOfLong), 1, file);
		fread(&log.mSizeOfFloat, sizeof(log.mSizeOfFloat), 1, file);
		fread(&log.mSizeOfDouble, sizeof(log.mSizeOfDouble), 1, file);
		fread(&log.mSizeOfTaskID, sizeof(log.mSizeOfTaskID), 1, file);
		fread(&log.mLogTaskID, sizeof(log.mLogTaskID), 1, file);
		fread(&log.mSizeOfNodeID, sizeof(log.mSizeOfNodeID), 1, file);
		fread(&log.mLogVersion, sizeof(log.mLogVersion), 1, file);
 		fread(&platformNameLength, sizeof(platformNameLength), 1, file);
		fgetstr(file, log.mPlatformName, platformNameLength);
		fread(&log.mLogNodeID, sizeof(log.mLogNodeID), 1, file);
		log.mDay = fgetc(file);
		log.mMonth = fgetc(file);
		fread(&log.mYear, sizeof(log.mYear), 1, file);
		log.mHour = fgetc(file);
		log.mMin = fgetc(file);
		log.mSec = fgetc(file);
		fread(&nodeNameLength, sizeof(nodeNameLength), 1, file);
		fgetstr(file, log.mNodeName, nodeNameLength);

		this->LogRegistration<RecordLogHeader>::ProcessData(log); 
	}
};

class DecoderTaskCreated : public LogDecoderCompressed, public LogRegistration<RecordTaskCreated> {
public:
	virtual void ProcessData(gzFile file) const
	{
		RecordTaskCreated log;
		Uint16 length = 0;

		egzread(file, &log.mTimeSeconds, sizeof(log.mTimeSeconds));
		egzread(file, &log.mTimeNanosec, sizeof(log.mTimeNanosec)); 

		egzread(file, &log.mLevelID, sizeof(log.mLevelID)); 
		egzread(file, &log.mTaskID, sizeof(log.mTaskID));
		egzread(file, &log.mNodeID, sizeof(log.mNodeID));

		egzread(file, &length, sizeof(length));
		egzreadstr(file, log.mTaskName, length);

		this->LogRegistration<RecordTaskCreated>::ProcessData(log); 
	}
};

class DecoderTaskDeleted : public LogDecoderCompressed, public LogRegistration<RecordTaskDeleted> {
public:
	virtual void ProcessData(gzFile file) const
	{
		RecordTaskDeleted log;

		egzread(file, &log.mTimeSeconds, sizeof(log.mTimeSeconds));
		egzread(file, &log.mTimeNanosec, sizeof(log.mTimeNanosec)); 

		egzread(file, &log.mLevelID, sizeof(log.mLevelID)); 
		egzread(file, &log.mTaskID, sizeof(log.mTaskID));
		egzread(file, &log.mNodeID, sizeof(log.mNodeID));

		this->LogRegistration<RecordTaskDeleted>::ProcessData(log); 
	}
};

class DecoderTimeStamp : public LogDecoderCompressed, public LogRegistration<RecordTimeStamp> {
public:
	virtual void ProcessData(gzFile file) const
	{
		RecordTimeStamp log;

		egzread(file, &log.mTimeSeconds, sizeof(log.mTimeSeconds));
		egzread(file, &log.mTimeNanosec, sizeof(log.mTimeNanosec)); 

		this->LogRegistration<RecordTimeStamp>::ProcessData(log); 
	}
};

class DecoderEndOfFile : public LogDecoderCompressed, public LogRegistration<RecordEndOfFile> {
public:
	virtual void ProcessData(gzFile file) const
	{
		RecordEndOfFile log;

		egzread(file, &log.mTimeSeconds, sizeof(log.mTimeSeconds));
		egzread(file, &log.mTimeNanosec, sizeof(log.mTimeNanosec)); 

		this->LogRegistration<RecordEndOfFile>::ProcessData(log); 
	}
};

class DecoderPeriodicSet : public LogDecoderCompressed, public LogRegistration<RecordPeriodicSet> {
public:
	virtual void ProcessData(gzFile file) const
	{
		RecordPeriodicSet log;
		Uint16 length = 0;

		egzread(file, &log.mTimeSeconds, sizeof(log.mTimeSeconds));
		egzread(file, &log.mTimeNanosec, sizeof(log.mTimeNanosec)); 

		egzread(file, &log.mSetID, sizeof(log.mSetID)); 
		egzread(file, &log.mNodeID, sizeof(log.mNodeID)); 

		egzread(file, &length, sizeof(length));
		egzreadstr(file, log.mSetName, length);

		this->LogRegistration<RecordPeriodicSet>::ProcessData(log); 
	}
};

class DecoderPeriodicLogOutput : public LogDecoderCompressed, public LogRegistration<RecordPeriodicLogOutput> {
public:
	virtual void ProcessData(gzFile file) const
	{
		RecordPeriodicLogOutput log;
		Uint16 itemCount = 0;

		egzread(file, &log.mTimeSeconds, sizeof(log.mTimeSeconds));
		egzread(file, &log.mTimeNanosec, sizeof(log.mTimeNanosec)); 

		egzread(file, &log.mSetID, sizeof(log.mSetID)); 
		egzread(file, &log.mNodeID, sizeof(log.mNodeID)); 

		egzread(file, &itemCount, sizeof(itemCount));
		for (Int32 i = 0; i < itemCount; ++i) {
			RecordPeriodicLogOutput::Vector keyData;

			Uint16 itemSize = 0;
			Uint16 keyCode = 0;
			
			egzread(file, &itemSize, sizeof(itemSize));
			egzread(file, &keyCode, sizeof(keyCode));

			for (Int32 j = 0; j < itemSize && !gzeof(file); ++j) {
				keyData.push_back(egzgetc(file));
			}

			log.mData[keyCode] = keyData;
		}

		this->LogRegistration<RecordPeriodicLogOutput>::ProcessData(log); 
	}		
};

class DecoderBinary : public LogDecoderCompressed, public LogRegistration<RecordBinary> {
public:
	virtual void ProcessData(gzFile file) const
	{
		RecordBinary log;
		Uint32 size = 0;

		egzread(file, &size, sizeof(size));
		
		for (Uint32 i = 0; i < size && !gzeof(file); ++i) {
			log.mData.push_back(egzgetc(file));
		}

		this->LogRegistration<RecordBinary>::ProcessData(log); 
	}
};

class DecoderStartNetCon : public LogDecoderCompressed, public LogRegistration<RecordStartNetCon> {
public:
	virtual void ProcessData(gzFile file) const
	{
		RecordStartNetCon log;
		Uint16 length = 0;

		egzread(file, &log.mNodeID, sizeof(log.mNodeID));

		log.mDay = egzgetc(file);
		log.mMonth = egzgetc(file);

		egzread(file, &log.mYear, sizeof(log.mYear)); 

		log.mHour = egzgetc(file);
		log.mMin = egzgetc(file);
		log.mSec = egzgetc(file);

		egzread(file, &length, sizeof(length));
		egzreadstr(file, log.mName, length);

		this->LogRegistration<RecordStartNetCon>::ProcessData(log); 
	}
};

class DecoderPeriodicSetItem : public LogDecoderCompressed, public LogRegistration<RecordPeriodicSetItem> {
public:
	virtual void ProcessData(gzFile file) const
	{
		RecordPeriodicSetItem log;
		Uint16 lengthName = 0;
		Uint16 lengthDescription = 0;
		Uint16 lengthFormat = 0;

		egzread(file, &log.mTimeSeconds, sizeof(log.mTimeSeconds));
		egzread(file, &log.mTimeNanosec, sizeof(log.mTimeNanosec)); 

		egzread(file, &log.mKeyCode, sizeof(log.mKeyCode)); 
		egzread(file, &log.mNodeID, sizeof(log.mNodeID)); 

		egzread(file, &lengthName, sizeof(lengthName)); 
		egzread(file, &lengthDescription, sizeof(lengthDescription)); 
		egzread(file, &lengthFormat, sizeof(lengthFormat)); 

		egzreadstr(file, log.mName, lengthName);
		egzreadstr(file, log.mDescription, lengthDescription);
		egzreadstr(file, log.mFormat, lengthFormat);

		this->LogRegistration<RecordPeriodicSetItem>::ProcessData(log); 
	}
};

class DecoderLogLevel : public LogDecoderCompressed, public LogRegistration<RecordLogLevel> {
public:
	virtual void ProcessData(gzFile file) const
	{
		RecordLogLevel log;
		Uint16 length = 0;

		egzread(file, &log.mTimeSeconds, sizeof(log.mTimeSeconds));
		egzread(file, &log.mTimeNanosec, sizeof(log.mTimeNanosec)); 

		egzread(file, &log.mLevelID, sizeof(log.mLevelID)); 
		egzread(file, &log.mNodeID, sizeof(log.mNodeID));

		egzread(file, &length, sizeof(length));
		egzreadstr(file, log.mLevelName, length);

		this->LogRegistration<RecordLogLevel>::ProcessData(log); 
	}
};

class DecoderDataMissed : public LogDecoderCompressed, public LogRegistration<RecordDataMissed> {
public:
	virtual void ProcessData(gzFile file) const
	{
		RecordDataMissed log;

		this->LogRegistration<RecordDataMissed>::ProcessData(log); 
	}
};

class DecoderPrintOutput : public LogDecoderCompressed, public LogRegistration<RecordPrintOutput> {
public:
	virtual void ProcessData(gzFile file) const
	{
		RecordPrintOutput log;
		Uint16 formatLen = 0;
		Uint16 fileLen = 0;
		Char *format = NULL;
		String data;

		egzread(file, &log.mTimeSeconds, sizeof(log.mTimeSeconds));
		egzread(file, &log.mTimeNanosec, sizeof(log.mTimeNanosec)); 

		egzread(file, &log.mLevelID, sizeof(log.mLevelID)); 
		egzread(file, &log.mTaskID, sizeof(log.mTaskID)); 
		egzread(file, &log.mNodeID, sizeof(log.mNodeID)); 

		egzread(file, &formatLen, sizeof(formatLen));
		egzread(file, &fileLen, sizeof(fileLen));
		egzread(file, &log.mLine, sizeof(log.mLine));

		format = new Char[formatLen + 1];
		egzread(file, format, formatLen);
		format[formatLen] = '\0';

		egzgetstr(file, log.mFile, fileLen);

		const Char *index = format;

		while (index != NULL) {
			index = egzgetformatstr(file, index, data);
			log.mMessage += data;
		}

		delete format;

		this->LogRegistration<RecordPrintOutput>::ProcessData(log); 
	}
};

class DecoderStreamOutput : public LogDecoderCompressed, public LogRegistration<RecordStreamOutput> {
public:
	virtual void ProcessData(gzFile file) const
	{
		RecordStreamOutput log;
		Uint16 fileLen = 0;
		Uint16 dataLen = 0;
		Uint32 readLen  = 0;
		Uint16 format = 0;
		Uint16 precision = 0;
		std::ostringstream stream;

		Uint32 tempInt = 0;
		Uint16 tempShort = 0;
		Float tempFloat = 0;
		Double tempDouble = 0;

		egzread(file, &log.mTimeSeconds, sizeof(log.mTimeSeconds));
		egzread(file, &log.mTimeNanosec, sizeof(log.mTimeNanosec)); 

		egzread(file, &log.mLevelID, sizeof(log.mLevelID)); 
		egzread(file, &log.mTaskID, sizeof(log.mTaskID)); 
		egzread(file, &log.mNodeID, sizeof(log.mNodeID)); 

		egzread(file, &fileLen, sizeof(fileLen));
		egzread(file, &log.mLine, sizeof(log.mLine));
		egzgetstr(file, log.mFile, fileLen);

		egzread(file, &format, sizeof(format));
		format = ConvertToSTL(format);
		stream.setf(format);

		precision = egzgetc(file);
		stream.precision(precision);

		egzread(file, &dataLen, sizeof(dataLen));

		while (readLen < dataLen) {
			Int32 type = egzgetc(file);

			readLen += 1;

			switch (type) {
			case StreamArg_SignedChar:
				stream << (Char)egzgetc(file);
				readLen += 1;
				break;
			case StreamArg_UnsignedChar:
				stream << (Uchar)egzgetc(file);
				readLen += 1;
				break;
			case StreamArg_SignedInt:
			case StreamArg_SignedLong:
				egzread(file, &tempInt, sizeof(tempInt));
				stream << (Int32)tempInt;
				readLen += 4;
				break;
			case StreamArg_UnsignedInt:
			case StreamArg_UnsignedLong:
				egzread(file, &tempInt, sizeof(tempInt));
				stream << (Uint32)tempInt;
				readLen += 4;
				break;
			case StreamArg_String:
				egzread(file, &tempShort, sizeof(tempShort));
				for (Int32 i = 0; i < tempShort; ++i) {
					stream << (Char)egzgetc(file);
				}
				readLen += 2 + tempShort;
			   break;
			case StreamArg_Float:
				egzread(file, &tempFloat, sizeof(tempFloat));
				stream << tempFloat;
				readLen += 4;
				break;
			case StreamArg_Double:
				egzread(file, &tempDouble, sizeof(tempDouble));
				stream << tempDouble;
				readLen += 8;
				break;
			case StreamArg_Bool:
				stream << (egzgetc(file) != 0);
				readLen += 1;
				break;
			case StreamArg_Flag:
				egzread(file, &format, sizeof(format));
				format = ConvertToSTL(format);
				stream.flags(format);
				readLen += 2;
				break;
			case StreamArg_Precision:
				precision = egzgetc(file);
				stream.precision(precision);
				readLen += 1;
				break;
			}
		}

		log.mMessage = stream.str();

		this->LogRegistration<RecordStreamOutput>::ProcessData(log); 
	}
};

} // namespace DECODER

#endif // LOGDECODERBASE_H

