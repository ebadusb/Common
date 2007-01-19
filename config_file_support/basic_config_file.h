/*
 * $Header: //bctquad3/home/BCT_Development/Atreus/control/sysinit/rcs/basic_config_file.h 1.1 2005/05/16 22:35:00Z ms10234 Exp $
 *
 * Utilities for reading in configuration files
 *
 * $Log: basic_config_file.h $
 * Revision 1.1  2005/05/16 22:35:00Z  ms10234
 * Initial revision
 *
 */

#ifndef _BASIC_CONFIG_FILE_INCLUDE
#define _BASIC_CONFIG_FILE_INCLUDE

#include "config_file_tools.h"


class BasicConfigFile : public ConfigFileTools
{
public:

	enum ReadStatus { ReadOK, ReadBackupOK, ReadDefaultOK, ReadFailed };
	enum WriteStatus { WriteOK, WriteNotAllowed, WriteBackupFailed, WriteFailed };
	virtual ReadStatus readFile(DataLog_Level * logLevel, DataLog_Level * errorLevel) = 0;
	virtual WriteStatus writeFile(DataLog_Level * logLevel, DataLog_Level * errorLevel) { return WriteNotAllowed; }

   virtual const char * fileName(void) = 0;
   virtual const char * crcFileName(void) = 0;
   virtual const char * backupFileName(void) = 0;
   virtual const char * backupCRCFileName(void) = 0;
   virtual const char * defaultFileName(void) = 0;

	const ReadStatus getStatusFromRead() const { return _statusFromRead; }

protected:

	ReadStatus _statusFromRead;

};

#endif /* ifndef _BASIC_CONFIG_FILE_INCLUDE */

