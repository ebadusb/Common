/*
 * $Header$
 *
 * Tools for reading in configuration files
 *
 * $Log: $
 *
 */

#include <vxWorks.h>
#include <dosFsLib.h>
#include <ioLib.h>
#include <usrLib.h>

#include "config_file_tools.h"
#include "crcgen.h"


bool ConfigFileTools::checkCRC(FILE * fp, const char * crcName)
{
	bool status = false;
	unsigned long crc;
	unsigned long compareCRC = INITCRC_DEFAULT;
	int result;

	FILE * crcFP = fopen(crcName, "r");
	if ( !crcFP )
	{
		if ( _errorLevel ) DataLog(*_errorLevel) << name() << " CRC file open failed for " << crcName << ": " << errnoMsg << endmsg;
		goto done;
	}

	result = fscanf(crcFP, "%lx", &crc);
	fclose(crcFP);

   if ( result != 1 )
	{
		if ( _errorLevel ) DataLog(*_errorLevel) << name() << " CRC file read failed for " << crcName << ": " << errnoMsg << endmsg;
		goto done;
	}

	enum { BufferSize = 256 };
	unsigned char buffer[BufferSize];

	int readCount;
	while ( (readCount = fread(buffer, 1, BufferSize, fp)) > 0 )
	{
		crcgen32(&compareCRC, buffer, readCount);
	}

	if ( crc != compareCRC )
	{
		if ( _errorLevel ) DataLog(*_errorLevel) << name() << " CRC check failed for " << crcName <<
			": expected=" << hex << crc << " actual=" << compareCRC << dec << endmsg;
		goto done;
	}

	status = true;
	fseek(fp, 0, SEEK_SET);

done:
	return status;
}

bool ConfigFileTools::backupDataFile(const char * fileName, const char * crcName, const char * backupFileName, const char * backupCRCName)
{
	struct stat fileStat;
	bool status = true;

	if ( stat((char *)fileName, &fileStat) == OK &&
		  stat((char *)crcName, &fileStat) == OK )
	{
		unprotectDataFile(backupFileName);
		unprotectDataFile(backupCRCName);
		status = ( copyDataFile(fileName, backupFileName) && copyDataFile(crcName, backupCRCName) );
		protectDataFile(backupFileName);
		protectDataFile(backupCRCName);
	}
	else
	{
		if ( _errorLevel ) DataLog(*_errorLevel) << name() << " primary file does not exist - backup skipped " << fileName << endmsg;
	}

	return status;
}

void ConfigFileTools::protectDataFile(const char * name)
{
	int fd = open(name, O_RDONLY, 0);
	if ( fd >= 0 )
	{
		struct stat fileStat;
		fstat(fd, &fileStat);
		fileStat.st_attrib |= DOS_ATTR_RDONLY;
		ioctl(fd, FIOATTRIBSET, fileStat.st_attrib);
		close(fd);
	}
}

void ConfigFileTools::unprotectDataFile(const char * name)
{
	int fd = open(name, O_RDONLY, 0);
	if ( fd >= 0 )
	{
		struct stat fileStat;
		fstat(fd, &fileStat);
		fileStat.st_attrib &= ~DOS_ATTR_RDONLY;
		ioctl(fd, FIOATTRIBSET, fileStat.st_attrib);
		close(fd);
	}
}

bool ConfigFileTools::copyDataFile(const char * source, const char * dest)
{
	FILE * fpSrc = fopen(source, "r");
	FILE * fpDest = fopen(dest, "w");
	bool	copyDone = false;

	if ( fpSrc && fpDest )
	{
		enum {BufferSize = 256};
		void * buffer = malloc(BufferSize);
		int readBytes;

		while ( (readBytes = fread(buffer, 1, BufferSize, fpSrc)) > 0 )
		{
			fwrite(buffer, 1, readBytes, fpDest);
		}

		copyDone = true;
	}

	if ( fpSrc ) fclose(fpSrc);
	if ( fpDest ) fclose(fpDest);
	return copyDone;
}

int ConfigFileTools::stripLine(char * lineData, int len)
{
	int resultLen = 0;
	int startIdx = 0;

	// Strip white space from beginning
	//
	while ( startIdx < len &&
           isspace(lineData[startIdx]) )
	{
		startIdx += 1;
	}

	// Strip white space from beginning
	//
	int endIdx = len-1;
	while ( endIdx >= startIdx &&
           isspace(lineData[endIdx]) )
	{
		endIdx -= 1;
	}

	// Check for comment on line and truncate at start of comment if found
	//
	int	commentStart = startIdx;
	enum { Normal, String } commentState = Normal;

	while ( commentStart <= endIdx )
	{
		switch ( commentState )
		{
		case Normal:
			if ( lineData[commentStart] == '#' ) endIdx = commentStart;		// found start of a comment, truncate line
		   if ( lineData[commentStart] == '"' ) commentState = String;		// found start of string, need to ignore '#' chars
		   commentStart += 1;
		   break;

		case String:
			if ( lineData[commentStart] == '"' ) commentState = Normal;		// found end of string
			if ( lineData[commentStart] == '\\' ) commentStart += 1;			// need to skip possible \" sequence
			commentStart += 1;
			break;
		}
	}

	if ( startIdx <= endIdx )
	{
		resultLen = endIdx-startIdx+1;
		memmove(lineData, &lineData[startIdx], resultLen*sizeof(char));
		lineData[resultLen] = '\0';
	}

	return resultLen;
}

bool ConfigFileTools::getParamLong(const char * data, long * ref)
{
	int len = strlen(data);
	char * endPtr = NULL;

	if ( data[0] == '0' &&
		  len > 2 )
	{
		if ( data[1] == 'b' || data[1] == 'B' )
		{
			*ref = strtol(&data[2], &endPtr, 2);
		}
		else if ( data[1] == 'x' || data[1] == 'X' )
		{
			*ref = strtol(&data[2], &endPtr, 16);
		}
	}
	else
	{
		*ref = strtol(data, &endPtr, 10);
	}

	return (endPtr && *endPtr == '\0');
}

bool ConfigFileTools::getParamULong(const char * data, unsigned long * ref)
{
	int len = strlen(data);
	char * endPtr = NULL;

	if ( data[0] == '0' &&
		  len > 2 )
	{
		if ( data[1] == 'b' || data[1] == 'B' )
		{
			*ref = strtoul(&data[2], &endPtr, 2);
		}
		else if ( data[1] == 'x' || data[1] == 'X' )
		{
			*ref = strtoul(&data[2], &endPtr, 16);
		}
	}
	else
	{
		*ref = strtoul(data, &endPtr, 10);
	}

	return (endPtr && *endPtr == '\0');
}

bool ConfigFileTools::getParamDouble(const char * data, double * ref)
{
	char * endPtr = NULL;
	*ref = strtod(data, &endPtr);
	return (endPtr && *endPtr == '\0');
}

bool ConfigFileTools::getParamBool(const char * data, bool * ref)
{
	bool result = true;
	if ( strIEqual(data, "true") )
	{
		*ref = true;
	}
	else if ( strIEqual(data, "false") )
	{
		*ref = false;
	}
	else
	{
		result = false;
	}

	return result;
}

bool ConfigFileTools::getParamString(const char * data, char *& ref)
{
	int len = strlen(data);
	bool result = false;

	if ( data[0] == '"' &&
		  data[len-1] == '"' )
	{
		// Allocate to maximum possible length (may be less due to \x sequences)
		//
		ref = new char[len-1];
		result = true;

		// Scan string, replacing \x sequences as necessary
		//
		int	readIdx = 1;
		int	writeIdx = 0;
		while ( readIdx < len-1 &&
				  result )
		{
			if ( data[readIdx] != '\\' )
			{
				ref[writeIdx++] = data[readIdx++];
			}
			else
			{
				if ( readIdx >= len-2 )
				{
					// Can't have \ as last character in string
					//
					result = false;
				} 
				else
				{
					switch ( data[readIdx+1] )
					{
					case 'b': ref[writeIdx++] = '\b'; break;
					case 'n': ref[writeIdx++] = '\n'; break;
					case 'r': ref[writeIdx++] = '\r'; break;
					case 't': ref[writeIdx++] = '\t'; break;
					case '"': ref[writeIdx++] = '"'; break;
					default: ref[writeIdx++] = data[readIdx+1];
					}

					readIdx += 2;
				}
			}
		}

		ref[writeIdx] = '\0';
	}

	return result;
}

bool ConfigFileTools::strIEqual(const char * s1, const char * s2)
{
	while ( *s1 && *s2 )
	{
		if ( toupper(*s1) != toupper(*s2) ) break;
		s1++;
		s2++;
	}

	return ( !*s1 && !*s2 );
}


