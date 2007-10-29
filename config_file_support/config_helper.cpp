/*
 * $Header: Q:/BCT_Development/vxWorks/Common/config_file_support/rcs/config_helper.cpp 1.4 2007/10/29 17:22:49Z jl11312 Exp jd11007 $
 *
 * Utilities for managing configuration files
 *
 * $Log: config_helper.cpp $
 * Revision 1.3  2007/07/03 15:00:06Z  MS10234
 *  - check for NULL string before attempting to log (AtreusSW IT 774)
 * Revision 1.2  2007/05/02 14:08:22Z  jl11312
 * - changed logging to correctly log string and enum values (Taos IT 3234)
 * Revision 1.1  2007/01/18 21:44:36Z  MS10234
 * Initial revision
 * Revision 1.1  2005/05/16 22:31:37Z  ms10234
 * Initial revision
 *
 */

#include <vxWorks.h>
#include <dosFsLib.h>
#include <ioLib.h>
#include <usrLib.h>

#include "config_helper.h"
#include "crcgen.h"

void ConfigData::readConfigData(GetConfigFileObj * getConfigFileObj, 
										  DataLog_Level * logLevel, DataLog_Level * errorLevel, 
										  ConfigData::BackupPermissionsType backupPermissions)
{
	int	index = 0;
	ConfigFile * file;

	while ( (file=(*getConfigFileObj)(index)) != NULL )
	{
		file->backupPermissions(backupPermissions);
		ConfigFile::ReadStatus status = file->readFile(logLevel, errorLevel);

		// Take appropriate action for errors ...
		//
		if ( status != ConfigFile::ReadOK )
		{
			if ( status == ConfigFile::ReadBackupOK )
			{
				if ( errorLevel ) DataLog(*errorLevel) << "main file read failure for file " << file->fileName() 
															<< " format " << file->formatVersion()
															<< ", used backup file " << file->backupFileName() 
															<< endmsg;
			}
			else if ( status == ConfigFile::ReadDefaultOK )
			{
				if ( errorLevel ) DataLog(*errorLevel) << "main and backup file read failure for file " << file->fileName() 
															<< " format " << file->formatVersion()
															<< ", used default file " << file->defaultFileName()
															<< endmsg;
			}
			else
			{
				if ( errorLevel ) DataLog(*errorLevel) << "file read failure for file " << file->fileName()
															<< " format " << file->formatVersion()
															<< endmsg;
			}
		}
		
		index += 1;
	}
}

void ConfigData::checkConfigDataErrors(GetConfigFileObj * getConfigFileObj,
													CallbackBase configBackupReadCb, 
													CallbackBase configDefaultReadCb, 
													CallbackBase configReadFailedCb)
{
	int	index = 0;
	ConfigFile * file;

	while ( (file=(*getConfigFileObj)(index)) != NULL )
	{
		ConfigFile::ReadStatus status = file->getStatusFromRead();

		// Take appropriate action for errors ...
		//
		if ( status != ConfigFile::ReadOK )
		{
			if ( status == ConfigFile::ReadBackupOK )
			{
				configBackupReadCb();
			}
			else if ( status == ConfigFile::ReadDefaultOK )
			{
				configDefaultReadCb();
			}
			else
			{
				configReadFailedCb();
			}
		}
		
		index += 1;
	}
}

int ConfigFile::getParamByName(const ConfigData::DataMap * const dataMap, int dataMapSize, const char * sectionName, const char * paramName)
{
	int retVal = -1;

	for ( int i=0; i<dataMapSize; i++ )
	{
		if ( strcmp(dataMap[i]._sectionName, sectionName) == 0 &&
			  strcmp(dataMap[i]._paramName, paramName) == 0 )
		{
			retVal = i;
			break;
		}
	}

	return retVal;
}


ConfigFile::ReadStatus ConfigFile::readData(const ConfigData::DataMap * const dataMap, int dataMapSize, const char * fileName, const char * crcName, const char * backupFileName, const char * backupCRCName, const char * defaultFileName )
{
	ReadStatus	retVal = ReadFailed;

	if ( readDataFromFile(dataMap, dataMapSize, fileName, crcName) )
	{
		if ( _logLevel ) DataLog(*_logLevel) << name() << " read from primary file " << fileName << " complete" << endmsg;
		retVal = ReadOK;
		if ( _backupPermissions == ConfigData::AllowBackup &&
		     backupFileName &&
	        !backupDataFile(fileName, crcName, backupFileName, backupCRCName) )
		{
			if ( _logLevel ) 
				DataLog(*_logLevel) << name() << " backup primary file " << fileName << " failed" << endmsg;
		}
	}
	else if ( backupFileName &&
             readDataFromFile(dataMap, dataMapSize, backupFileName, backupCRCName) )
	{
		if ( _logLevel ) DataLog(*_logLevel) << name() << " read from backup file " << fileName << " complete" << endmsg;
		retVal = ReadBackupOK;
		if ( _backupPermissions == ConfigData::AllowBackup &&
	        !backupDataFile(backupFileName, backupCRCName, fileName, crcName) )
		{
			if ( _logLevel ) 
				DataLog(*_logLevel) << name() << " fix primary file from backup file " << backupFileName << " failed" << endmsg;
		}
	}
	else if ( defaultFileName &&
             readDataFromFile(dataMap, dataMapSize, defaultFileName, NULL) )
	{
		if ( _logLevel ) DataLog(*_logLevel) << name() << " read from default file " << fileName << " complete" << endmsg;
		retVal = ReadDefaultOK;
	}
	else
  	{
		DataLog(*_errorLevel) << name() << " read failed" << endmsg;
	}

	return (_statusFromRead = retVal);
}

ConfigFile::WriteStatus ConfigFile::writeData(const ConfigData::DataMap * const dataMap, int dataMapSize, const char * fileName, const char * crcName, const char * backupFileName, const char * backupCRCName)
{
	WriteStatus	retVal = WriteOK;
	FILE * dataFile = NULL;
	FILE * crcFile = NULL;

	if ( !fileName || !crcName || !backupFileName || !backupCRCName )
	{
		DataLog(*_errorLevel) << name() << " missing file name specification" << endmsg;
		retVal = WriteNotAllowed;
	}
	else if ( !backupDataFile(fileName, crcName, backupFileName, backupCRCName) )
	{
		retVal = WriteBackupFailed;
	}
	else
	{
		unprotectDataFile(fileName);
		unprotectDataFile(crcName);

		dataFile = fopen(fileName, "w+");
		crcFile = fopen(crcName, "w");
	}

	if ( !dataFile || !crcFile )
	{
		DataLog(*_errorLevel) << name() << " error opening data files: " << errnoMsg << endmsg;
		retVal = WriteFailed;
	}
	else
	{  
		const char * sectionName = NULL;
		for ( int idx=0; idx<dataMapSize; idx+=1 )
		{
			if ( !sectionName || strcmp(sectionName, dataMap[idx]._sectionName) != 0 )
			{
				if ( sectionName ) fprintf(dataFile, "\n");
				sectionName = dataMap[idx]._sectionName;
				fprintf(dataFile, "[%s]\n", sectionName);
			}

			fprintf(dataFile, "%s=", dataMap[idx]._paramName);
			switch ( dataMap[idx]._type )
			{
			case ConfigData::TLong:
				fprintf(dataFile, "%ld\n", *(long *)dataMap[idx]._value);
				break;

			case ConfigData::TDouble:
				fprintf(dataFile, "%.5lg\n", *(double *)dataMap[idx]._value);
				break;

			case ConfigData::TString:
				if ( !writeString(dataFile, dataMap[idx], *(const char **)dataMap[idx]._value) )
				{
					retVal = WriteFailed;
				}

				break;

			case ConfigData::TBool:
				fprintf(dataFile, "%s\n", ( *(bool *)dataMap[idx]._value) ? "true" : "false");
				break;

			case ConfigData::TEnum:
				if ( !writeEnum(dataFile, dataMap[idx], *(long *)dataMap[idx]._value) )
				{
					retVal = WriteFailed;
				}

				break;

			default:
				fprintf(dataFile, "unknown-type\n");  
			}
		}

		fflush(dataFile);
		fseek(dataFile, 0, SEEK_SET);
		enum { BufferSize = 256 };
		unsigned char buffer[BufferSize];
		unsigned long crc = INITCRC_DEFAULT;

		int readCount;
		while ( (readCount = fread(buffer, 1, BufferSize, dataFile)) > 0 )
		{
			crcgen32(&crc, buffer, readCount);
	   }

		fprintf(crcFile, "%lx\n", crc);
		if ( retVal == WriteOK )
		{
			if ( _logLevel ) DataLog(*_logLevel) << name() << " write to " << fileName << " complete" << endmsg;
		}
	}

	if ( dataFile) fclose(dataFile);
	if ( crcFile ) fclose(crcFile);

	protectDataFile(fileName);
	protectDataFile(crcName);

	return retVal;
}

void ConfigFile::logData(DataLog_Level * level, const ConfigData::DataMap * const dataMap, int dataMapSize, ConfigFile::ReadStatus readStatus)
{
	bool	logDataOK = true;

	if (readStatus == ConfigFile::ReadOK )
		DataLog(*level) << "config read OK from <" << fileName() << ">" << endmsg;
	else if (readStatus == ConfigFile::ReadBackupOK)
		DataLog(*level) << "config backup read OK from <" << backupFileName() << ">" << endmsg;
	else if (readStatus == ConfigFile::ReadDefaultOK)
		DataLog(*level) << "config default read OK from <" << defaultFileName() << ">" << endmsg;
	else
	{
		DataLog(*level) << "config default read failed from <" << defaultFileName() << ">" << endmsg;
		logDataOK = false;
	}

	if ( logDataOK )
	{
		DataLog(*level) << "ConfigFileData <" << fileName() << ">:";
	
		const char * sectionName = NULL;
		for ( int idx=0; idx<dataMapSize; idx+=1 )
		{
			if ( !sectionName || strcmp(sectionName, dataMap[idx]._sectionName) != 0 )
			{
				sectionName = dataMap[idx]._sectionName;
				DataLog(*level) << " [" << sectionName << "]";
			}
	
			DataLog(*level) << " " << dataMap[idx]._paramName << "=";
			switch ( dataMap[idx]._type )
			{
			case ConfigData::TLong:
				DataLog(*level) << *(long *)dataMap[idx]._value;
				break;
	
			case ConfigData::TDouble:
				DataLog(*level) << precision(5) << *(double *)dataMap[idx]._value;
				break;
	
			case ConfigData::TString:
				logString(level, dataMap[idx], *(const char **)dataMap[idx]._value);
				break;
	
			case ConfigData::TBool:
				DataLog(*level) << *(bool *)dataMap[idx]._value;
				break;
	
			case ConfigData::TEnum:
				logEnum(level, dataMap[idx], *(long *)dataMap[idx]._value);
				break;
	
			default:
				DataLog(*level) << "unknown-type";
				break;
			}
		}
	
		DataLog(*level) << endmsg;
	}
}

bool ConfigFile::getParamValueByName(const ConfigData::DataMap * const dataMap, int dataMapSize, const char * sectionName, const char * paramName, ConfigData::ParamValue & value)
{
	int idx = getParamByName(dataMap, dataMapSize, sectionName, paramName);
	if ( idx >= 0 )
	{
		value._type = dataMap[idx]._type;

		switch ( dataMap[idx]._type )
		{
		case ConfigData::TLong:
		case ConfigData::TEnum: value._value._lValue = *(long *)dataMap[idx]._value; break;
		case ConfigData::TDouble: value._value._dValue = *(double *)dataMap[idx]._value; break;
		case ConfigData::TString: value._value._sValue = (const char *)dataMap[idx]._value; break;
		case ConfigData::TBool: value._value._bValue = *(bool *)dataMap[idx]._value; break;
		}
	}

	return ( idx >= 0 );
}

const char * ConfigFile::getParamNameByRef(const ConfigData::DataMap * const dataMap, int dataMapSize, void * param)
{
	const char * retVal = NULL;

	for ( int i=0; i<dataMapSize; i++ )
	{
		if ( dataMap[i]._value == param )
		{
			retVal = dataMap[i]._paramName;
			break;
		}
	}

	return retVal;
}

const char * ConfigFile::getSectionNameByRef(const ConfigData::DataMap * const dataMap, int dataMapSize, void * param)
{
	const char * retVal = NULL;

	for ( int i=0; i<dataMapSize; i++ )
	{
		if ( dataMap[i]._value == param )
		{
			retVal = dataMap[i]._sectionName;
			break;
		}
	}

	return retVal;
}

bool ConfigFile::getParamEnum(const char * data, const ConfigData::EnumMap * enumMap, long * ref)
{
	int enumIdx = 0;
	bool found = false;

	while ( enumMap[enumIdx]._name != NULL &&
			  !found )
	{
		if ( strcmp(data, enumMap[enumIdx]._name) == 0 )
		{
			*ref = enumMap[enumIdx]._value;
			found = true;
		}
		else
		{
			enumIdx += 1;
		}
	}

	return found;
}

bool ConfigFile::writeString(FILE * fp, const ConfigData::DataMap & dataMap, const char * str)
{
	const char * currCh = str;

   putc('"', fp);
   while ( *currCh )
	{
		switch ( *currCh )
		{
		case '\b': fprintf(fp, "\\b"); break;
		case '\n': fprintf(fp, "\\n"); break;
		case '\r': fprintf(fp, "\\r"); break;
		case '\t': fprintf(fp, "\\t"); break;
		case '"': fprintf(fp, "\\\""); break;
		default: putc(*currCh, fp); break;
		}

		currCh++;
	}

	fprintf(fp, "\"\n");
	return true;
}

bool ConfigFile::writeEnum(FILE * fp, const ConfigData::DataMap & dataMap, long value)
{
	int	idx = 0;
	while ( dataMap._enumMap[idx]._name )
	{
		if ( dataMap._enumMap[idx]._value == value )
		{
			break;
		}
		else
		{
			idx += 1;
		}
	}

	bool retVal;
	if ( dataMap._enumMap[idx]._name )
	{
		fprintf(fp, "%s\n", dataMap._enumMap[idx]._name);
		retVal = true;
	}
	else
	{
		DataLog(*_errorLevel) << dataMap._paramName << ": can't find enum for value=" << value << endmsg;
		fprintf(fp, "0x%x\n", (unsigned long)value);
		retVal = false;
	}

	return retVal;
}

void ConfigFile::logString(DataLog_Level * level, const ConfigData::DataMap & dataMap, const char * str)
{
	if ( str )
	{
		const char * currCh = str;

		DataLog(*level) << '"';
		while ( *currCh )
		{
			switch ( *currCh )
			{
			case '\b': DataLog(*level) << "\\b"; 	break;
			case '\n': DataLog(*level) << "\\n"; 	break;
			case '\r': DataLog(*level) << "\\r";	break;
			case '\t': DataLog(*level) << "\\t"; 	break;
			case '"':  DataLog(*level) << "\\\""; 	break;
			default:   DataLog(*level) << *currCh; break;
			}
	
			currCh++;
		}

		DataLog(*level) << '"';
	}
}

void ConfigFile::logEnum(DataLog_Level * level, const ConfigData::DataMap & dataMap, long value)
{
	int	idx = 0;
	while ( dataMap._enumMap[idx]._name )
	{
		if ( dataMap._enumMap[idx]._value == value )
		{
			break;
		}
		else
		{
			idx += 1;
		}
	}

	bool retVal;
	if ( dataMap._enumMap[idx]._name )
	{
		DataLog(*level) << dataMap._enumMap[idx]._name;
	}
	else
	{
		DataLog(*level) << value;
	}
}

bool ConfigFile::checkRange(const ConfigData::DataMap * const dataMap, const char * fileName, int line, int idx)
{
	bool result = true;

	if ( dataMap[idx]._type == ConfigData::TLong )
	{
		long min, max;
		if ( (*(ConfigData::LongRangeFunc *)dataMap[idx]._rangeFunc)(min, max) &&
			  ( *(long *)dataMap[idx]._value < min || *(long *)dataMap[idx]._value > max ))
		{
			DataLog(*_errorLevel) << name() << " parameter value out of range [" <<
				min << ":" << max << "] " << fileName << ":" << line << endmsg;
			result = false;
		}
	}
	else if ( dataMap[idx]._type == ConfigData::TDouble )
	{
		double min, max;
		if ( (*(ConfigData::DoubleRangeFunc *)dataMap[idx]._rangeFunc)(min, max) &&
			  ( *(double *)dataMap[idx]._value < min || *(double *)dataMap[idx]._value > max ))
		{
			DataLog(*_errorLevel) << name() << " parameter value out of range [" <<
				min << ":" << max << "] " << fileName << ":" << line << endmsg;
			result = false;
		} 
	}
	else
	{
		DataLog(*_errorLevel) << name() << " internal error - invalid range function " << fileName << ":" << line << endmsg;
		result = false;
	}

	return result;
}

bool ConfigFile::readDataFromFile(const ConfigData::DataMap * const dataMap, int dataMapSize, const char * fileName, const char * crcName)
{
	enum { MaxLineSize = 256 };
	typedef void * ParamRef;

	bool status = false;
	char * lineData = new char[MaxLineSize];
	ParamRef * paramRef = new ParamRef[dataMapSize];

	char * currentSection = NULL;
	int line = 0;
	int paramCount = 0;

	FILE * fp = fopen(fileName, "r");
	if ( !fp )
	{
		DataLog(*_errorLevel) << name() << " data file open failed for " << fileName << ": " << errnoMsg << endmsg;
		goto done;
	}
 
	if ( crcName && !checkCRC(fp, crcName) )
	{
		goto done;
	}

	memset(paramRef, 0, dataMapSize*sizeof(ParamRef));
	lineData[MaxLineSize-1] = '\0';
	while ( fgets(lineData, MaxLineSize-1, fp) != NULL )
	{
		int	len = strlen(lineData);
		line += 1;

		if ( lineData[len-1] != '\n' )
		{
			DataLog(*_errorLevel) << name() << " data file line too long " << fileName << ":" << line << endmsg;
			goto done;
		}

		if ( (len = stripLine(lineData, len)) != 0 )
		{
			if ( lineData[0] == '[' &&
				  lineData[len-1] == ']' )
			{
				if ( currentSection ) delete currentSection;
				currentSection = new char[len-1];
				strncpy(currentSection, &lineData[1], len-2);
				currentSection[len-2] = '\0';
			}
			else if ( isalpha(lineData[0]) )
			{
				char * paramValue = strchr(lineData, '=');
				if ( !paramValue )
				{
					DataLog(*_errorLevel) << name() << " missing '=' " << fileName << ":" << line << endmsg;
					goto done;
				}

				*paramValue++ = '\0';

				int idx = getParamByName(dataMap, dataMapSize, currentSection, lineData);
				if ( idx < 0 )
				{
					DataLog(*_errorLevel) << name() << " no such parameter '" << lineData << "' " << fileName << ":" << line << endmsg;
					goto done;
				}

				for ( int p=0; p<paramCount; p++ )
				{
					if ( dataMap[idx]._value == paramRef[p] )
					{
						DataLog(*_errorLevel) << name() << " duplicate setting for parameter '" << lineData << "' " << fileName << ":" << line << endmsg;
						goto done;
					}
				}

				paramRef[paramCount] = dataMap[idx]._value;
				paramCount += 1;

				bool paramOK = false;
				switch ( dataMap[idx]._type )
				{
				case ConfigData::TLong:
					paramOK = getParamLong(paramValue, (long *)dataMap[idx]._value);
					break;

				case ConfigData::TDouble:
					paramOK = getParamDouble(paramValue, (double *)dataMap[idx]._value);
					break;

				case ConfigData::TBool:
					paramOK = getParamBool(paramValue, (bool *)dataMap[idx]._value);
					break;

				case ConfigData::TString:
					paramOK = getParamString(paramValue, *(char **)dataMap[idx]._value);
					break;

				case ConfigData::TEnum:
					paramOK = getParamEnum(paramValue, dataMap[idx]._enumMap, (long *)dataMap[idx]._value);
					break;
				}

				if ( !paramOK )
				{
					DataLog(*_errorLevel) << name() << " bad parameter value format " << fileName << ":" << line << endmsg;
					goto done;
				}

				if ( strcmp(dataMap[idx]._sectionName, "Version") == 0 &&
					  strcmp(dataMap[idx]._paramName, "FormatVersion") == 0 )
				{
					if ( dataMap[idx]._type != ConfigData::TString ||
						  strcmp(*(char **)dataMap[idx]._value, formatVersion()) != 0 )
					{
						DataLog(*_errorLevel) << name() << " wrong format version - expected \"" << formatVersion() << "\" " << fileName << ":" << line << endmsg;
						goto done;						
					}
				}

				if ( dataMap[idx]._rangeFunc && !checkRange(dataMap, fileName, line, idx) ) goto done;
				if ( dataMap[idx]._validateFunc &&
				     !(*dataMap[idx]._validateFunc)(dataMap[idx]._value) )
				{
					DataLog(*_errorLevel) << name() << " invalid parameter value " << fileName << ":" << line << endmsg;
					goto done;
				}
			}
		}
	}

	if ( paramCount != dataMapSize )
	{
		DataLog(*_errorLevel) << name() << " not all parameters assigned in file " << fileName << endmsg;
		goto done;
	}

	status = true;

done:
	if ( fp ) fclose(fp);
	if ( currentSection ) delete currentSection;
	delete paramRef;
	delete lineData;

	return status;
}

