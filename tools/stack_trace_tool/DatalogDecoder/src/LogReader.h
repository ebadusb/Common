/*****************************************************************************\
**
** LogReader.h
**
** DLog reader implementation.
**
\*****************************************************************************/

#ifndef LOGREADER_H
#define LOGREADER_H

/* Includes ******************************************************************/

#include <map>
#include <stdlib.h>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <iostream>

#include "LogRecord.h"
#include "LogDecoder.h"
#include "LogReaderUtil.h"
#include "LogFileUtil.h"

namespace DECODER {

/* Constants *****************************************************************/

const Uint16 kLogVersion0301 = 0x0301;

const Uint16 kIDRecordHeader = 0x5500;
const Uint16 kIDRecordLogLevel = 0x5501;
const Uint16 kIDRecordPrintOutput = 0x5502;
const Uint16 kIDRecordStreamOutput = 0x5503;
const Uint16 kIDRecordPeriodicLogOutput = 0x5504;
const Uint16 kIDRecordPeriodicSet = 0x5505;
const Uint16 kIDRecordPeriodicSetItem = 0x5506;
const Uint16 kIDRecordTaskCreated = 0x5507;
const Uint16 kIDRecordTaskDeleted = 0x5508;
const Uint16 kIDRecordStartNetCon = 0x5509;
const Uint16 kIDRecordBinary = 0x55f0;
const Uint16 kIDRecordEndOfFile = 0x55fd;
const Uint16 kIDRecordTimeStamp = 0x55fe;
const Uint16 kIDRecordDataMissed = 0x55ff;

const Uint8 kLogSignature1 = 0x78;
const Uint8 kLogSignature2 = 0x56;
const Uint8 kLogSignature3 = 0x34;
const Uint8 kLogSignature4 = 0x12;

/* Classes *******************************************************************/

class LogReader {
public:
	enum {
		kResultNone = 0,
		kResultFileAlreadyOpen,
		kResultFileNotOpen,
		kResultFileOpenFailed,
		kResultInvalidHeaderType,
		kResultInvalidVersion,
		kResultUnexpectedEOF,
		kResultUnknownRecordType,
		kResultLast
	};

	/**
	 *
	 * LogReader
	 *
	 * Costructor. Adds the decoders to the decoder maps. Each data log version 
	 * has its own decoder map.
	 *
	 */
	LogReader(void) : mFileInfoP(NULL)
	{
		DecoderMap *decoderMap0301 = new DecoderMap;

		decoderMap0301->insert(std::make_pair(kIDRecordLogLevel, &mDecoderLogLevel));
		decoderMap0301->insert(std::make_pair(kIDRecordPrintOutput, &mDecoderPrintOutput));
		decoderMap0301->insert(std::make_pair(kIDRecordStreamOutput, &mDecoderStreamOutput));
		decoderMap0301->insert(std::make_pair(kIDRecordPeriodicLogOutput, &mDecoderPeriodicLogOutput));
		decoderMap0301->insert(std::make_pair(kIDRecordPeriodicSet, &mDecoderPeriodicSet));
		decoderMap0301->insert(std::make_pair(kIDRecordPeriodicSetItem, &mDecoderPeriodicSetItem));
		decoderMap0301->insert(std::make_pair(kIDRecordTaskCreated, &mDecoderTaskCreated));
		decoderMap0301->insert(std::make_pair(kIDRecordTaskDeleted, &mDecoderTaskDeleted));
		decoderMap0301->insert(std::make_pair(kIDRecordStartNetCon, &mDecoderStartNetCon));
		decoderMap0301->insert(std::make_pair(kIDRecordBinary, &mDecoderBinary));
		decoderMap0301->insert(std::make_pair(kIDRecordEndOfFile, &mDecoderEndOfFile));
		decoderMap0301->insert(std::make_pair(kIDRecordTimeStamp, &mDecoderTimeStamp));
		decoderMap0301->insert(std::make_pair(kIDRecordDataMissed, &mDecoderDataMissed));

		mVersionDecoderMap[kLogVersion0301] = decoderMap0301;
	}

	/**
	 *
	 * ~LogReader
	 *
	 * Destructor
	 *
	 */
	~LogReader(void) 
	{
		VersionDecoderMap::iterator iter = mVersionDecoderMap.begin();
		VersionDecoderMap::iterator end = mVersionDecoderMap.end();

		for (; iter != end; ++iter) {
			delete iter->second;
		}
	}

	/**
	 *
	 * Open
	 *
	 * Opens the data log with the given file name.
	 *
	 * @param filename The name of the file to open.
	 *
	 * @return The results of the opperation.
	 *
	 */
	Result Open(const char * filename)
	{
		Result result = kResultNone;
		FileInfo *fileInfoP = new FileInfo;
		FILE *fp = NULL;
		fpos_t fpPos = 0;

		fileInfoP->filename = filename;
		fileInfoP->file = 0;
		fileInfoP->pointer = NULL;
		fileInfoP->isEndOfFile = false;
		this->RegisterLogHeader(*fileInfoP);
		this->RegisterEndOfFile(*fileInfoP);

		if (mFileInfoP != NULL) {
			// File already open.
			result = kResultFileAlreadyOpen;
		}

		if (result == kResultNone) { 
			fp = fopen(fileInfoP->filename.c_str(), "rb");

			// Check if file pointer is valid.
			if (fp == NULL) {
				result = kResultFileOpenFailed;
			}
		}

		if (result == kResultNone) { 
			// Read log proprietary message.
			mDecoderLogInfo.ProcessData(fp);

			// Verify log proprietary message.
			if (feof(fp)) {
				result = kResultUnexpectedEOF;
			}
		}

		if (result == kResultNone) { 
			// Check file signature. 
			if (!((fgetc(fp) == kLogSignature1)
					&& (fgetc(fp) == kLogSignature2)
					&& (fgetc(fp) == kLogSignature3)
					&& (fgetc(fp) == kLogSignature4))) {
				result = kResultInvalidVersion;
			}
		}

		if (result == kResultNone) { 
			Uint16 recordType = 0;

			// Read record type		
			fread(&recordType, sizeof(recordType), 1, fp);

			if (recordType != kIDRecordHeader) {
				result = kResultInvalidHeaderType;
			}
		}

		if (result == kResultNone) { 
			// Read the log header
			mDecoderLogHeader.ProcessData(fp);

			VersionDecoderMap::const_iterator iter = mVersionDecoderMap.find(fileInfoP->header.mLogVersion);
			VersionDecoderMap::const_iterator end = mVersionDecoderMap.end();

			// Check the file verion.
			if (iter != end) {
				fileInfoP->decoders = *iter->second;
			}
			else {
			  result = kResultInvalidVersion;
			}
		}

		if (fp != NULL) {
			// Close the file
			fgetpos(fp, &fpPos);
			fclose(fp);
			fp = NULL;
		}

		if (result == kResultNone) { 
			// Open the file with ZLib
			fileInfoP->file = _open(fileInfoP->filename.c_str(), O_RDONLY | O_BINARY);
			_lseek(fileInfoP->file, (long)fpPos, SEEK_SET);
			fileInfoP->pointer = gzdopen(fileInfoP->file, "rb");
		}

		if (result == kResultNone) {
			mFileInfoP = fileInfoP;
		}
		else {
			this->UnregisterLogHeader(*fileInfoP);
			this->UnregisterEndOfFile(*fileInfoP);
			delete fileInfoP;
		}

		return result;
	}

	/**
	 *
	 * Close
	 *
	 * Closes the currently opened data log.
	 *
	 * @return The results of the opperation.
	 *
	 */
	Result Close(void)
	{
		Result result = kResultNone;

		if (mFileInfoP != NULL) { 
			gzclose(mFileInfoP->pointer);
			this->UnregisterLogHeader(*mFileInfoP);
			this->UnregisterEndOfFile(*mFileInfoP);
			delete mFileInfoP;
			mFileInfoP = NULL;
		}
		else {
			result = kResultFileNotOpen;
		}

		return result;
	}

	/**
	 *
	 * Read
	 *
	 * Reads the given number of records from the datalog file.
	 *
	 * @param numRecords The number of records to read.
	 * @param numRecordsRead The number of records that were read.
	 *
	 * @return The results of the opperation.
	 *
	 */
	Result Read(Int32 numRecords, Int32 *numRecordsRead = NULL)
	{
		Result result = kResultNone;
		Uint16 recordType = 0;
		Int32 count = 0;

		if (mFileInfoP == NULL) {
			result = kResultFileNotOpen;
		}

		// Read the records from the file.
 		for (; (result == kResultNone) && (count < numRecords); ++count) {
			if (egzread(mFileInfoP->pointer, &recordType, sizeof(recordType)) <= 0 ) {
				mFileInfoP->isEndOfFile = true;
				result = kResultUnexpectedEOF;
				break;
			}

			DecoderMap::const_iterator iter = mFileInfoP->decoders.find(recordType);
			DecoderMap::const_iterator end = mFileInfoP->decoders.end();

			if (iter != end) {
				iter->second->ProcessData(mFileInfoP->pointer);
			}
			else {
				result = kResultUnknownRecordType;
			}
		}

		if (numRecordsRead != NULL) {
			*numRecordsRead = count;	
		}

		return result;
	}

	/**
	 *
	 * IsEndOfFile
	 *
	 * Checks if the reader is at the end of the file.
	 *
	 * @return TRUE if at the end if the data log file.
	 *
	 */
	Bool IsEndOfFile(void)
	{
		return (mFileInfoP != NULL) ? mFileInfoP->isEndOfFile : true;
	}

	template <class _HANDLER>
	void RegisterLogInfo(_HANDLER &handler) { mDecoderLogInfo.RegisterHandler(handler); }

	template <class _HANDLER>
	void RegisterLogHeader(_HANDLER &handler) { mDecoderLogHeader.RegisterHandler(handler); }

	template <class _HANDLER>
	void RegisterTaskCreated(_HANDLER &handler) { mDecoderTaskCreated.RegisterHandler(handler); }

	template <class _HANDLER>
	void RegisterTaskDeleted(_HANDLER &handler) { mDecoderTaskDeleted.RegisterHandler(handler); }

	template <class _HANDLER>
	void RegisterTimeStamp(_HANDLER &handler) { mDecoderTimeStamp.RegisterHandler(handler); }

	template <class _HANDLER>
	void RegisterEndOfFile(_HANDLER &handler) { mDecoderEndOfFile.RegisterHandler(handler); }

	template <class _HANDLER>
	void RegisterPeriodicSet(_HANDLER &handler) { mDecoderPeriodicSet.RegisterHandler(handler); }

	template <class _HANDLER>
	void RegisterPeriodicLogOutput(_HANDLER &handler) { mDecoderPeriodicLogOutput.RegisterHandler(handler); }

	template <class _HANDLER>
	void RegisterBinary(_HANDLER &handler) { mDecoderBinary.RegisterHandler(handler); }

	template <class _HANDLER>
	void RegisterStartNetCon(_HANDLER &handler) { mDecoderStartNetCon.RegisterHandler(handler); }

	template <class _HANDLER>
	void RegisterPeriodicSetItem(_HANDLER &handler) { mDecoderPeriodicSetItem.RegisterHandler(handler); }

	template <class _HANDLER>
	void RegisterLogLevel(_HANDLER &handler) { mDecoderLogLevel.RegisterHandler(handler); }

	template <class _HANDLER>
	void RegisterDataMissed(_HANDLER &handler) { mDecoderDataMissed.RegisterHandler(handler); }

	template <class _HANDLER>
	void RegisterPrintOutput(_HANDLER &handler) { mDecoderPrintOutput.RegisterHandler(handler); }

	template <class _HANDLER>
	void RegisterStreamOutput(_HANDLER &handler) { mDecoderStreamOutput.RegisterHandler(handler); }

	template <class _HANDLER>
	void UnregisterLogInfo(_HANDLER &handler) { mDecoderLogInfo.UnregisterHandler(handler); }

	template <class _HANDLER>
	void UnregisterLogHeader(_HANDLER &handler) { mDecoderLogHeader.UnregisterHandler(handler); }

	template <class _HANDLER>
	void UnregisterTaskCreated(_HANDLER &handler) { mDecoderTaskCreated.UnregisterHandler(handler); }

	template <class _HANDLER>
	void UnregisterTaskDeleted(_HANDLER &handler) { mDecoderTaskDeleted.UnregisterHandler(handler); }

	template <class _HANDLER>
	void UnregisterTimeStamp(_HANDLER &handler) { mDecoderTimeStamp.UnregisterHandler(handler); }

	template <class _HANDLER>
	void UnregisterEndOfFile(_HANDLER &handler) { mDecoderEndOfFile.UnregisterHandler(handler); }

	template <class _HANDLER>
	void UnregisterPeriodicSet(_HANDLER &handler) { mDecoderPeriodicSet.UnregisterHandler(handler); }

	template <class _HANDLER>
	void UnregisterPeriodicLogOutput(_HANDLER &handler) { mDecoderPeriodicLogOutput.UnregisterHandler(handler); }

	template <class _HANDLER>
	void UnregisterBinary(_HANDLER &handler) { mDecoderBinary.UnregisterHandler(handler); }

	template <class _HANDLER>
	void UnregisterStartNetCon(_HANDLER &handler) { mDecoderStartNetCon.UnregisterHandler(handler); }

	template <class _HANDLER>
	void UnregisterPeriodicSetItem(_HANDLER &handler) { mDecoderPeriodicSetItem.UnregisterHandler(handler); }

	template <class _HANDLER>
	void UnregisterLogLevel(_HANDLER &handler) { mDecoderLogLevel.UnregisterHandler(handler); }

	template <class _HANDLER>
	void UnregisterDataMissed(_HANDLER &handler) { mDecoderDataMissed.UnregisterHandler(handler); }

	template <class _HANDLER>
	void UnregisterPrintOutput(_HANDLER &handler) { mDecoderPrintOutput.UnregisterHandler(handler); }

	template <class _HANDLER>
	void UnregisterStreamOutput(_HANDLER &handler) { mDecoderStreamOutput.UnregisterHandler(handler); }

public:
	typedef std::map<Uint16, LogDecoderCompressed*> DecoderMap;
	typedef std::map<Uint16, DecoderMap*> VersionDecoderMap;

	struct FileInfo {
		String filename;
		DecoderMap decoders;
		Int32 file;
		gzFile pointer;
		Bool isEndOfFile;

		RecordLogHeader header;
		RecordEndOfFile endOfFile;

		void operator()(const RecordLogHeader &data) { header = data; }
		void operator()(const RecordEndOfFile &data) { endOfFile = data; isEndOfFile = true; }
	};

	DecoderLogInfo mDecoderLogInfo;
	DecoderLogHeader mDecoderLogHeader;
	DecoderTaskCreated mDecoderTaskCreated;
	DecoderTaskDeleted mDecoderTaskDeleted;
	DecoderTimeStamp mDecoderTimeStamp;
	DecoderEndOfFile mDecoderEndOfFile;
	DecoderPeriodicSet mDecoderPeriodicSet;
	DecoderPeriodicLogOutput mDecoderPeriodicLogOutput;
	DecoderBinary mDecoderBinary;
	DecoderStartNetCon mDecoderStartNetCon;
	DecoderPeriodicSetItem mDecoderPeriodicSetItem;
	DecoderLogLevel mDecoderLogLevel;
	DecoderDataMissed mDecoderDataMissed;
	DecoderPrintOutput mDecoderPrintOutput;
	DecoderStreamOutput mDecoderStreamOutput;

	VersionDecoderMap mVersionDecoderMap;

	FileInfo *mFileInfoP;
};

} // namespace DECODER

#endif // LOGDECODERBASE_H

