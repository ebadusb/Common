/*
 * $Header: $
 *
 * Tools for reading in configuration files
 *
 * $Log: $
 *
 */

#ifndef _CONFIG_FILE_TOOLS_INCLUDE
#define _CONFIG_FILE_TOOLS_INCLUDE

#include "datalog.h"

class ConfigFileTools
{
public:

	ConfigFileTools() : _logLevel(0), _errorLevel(0) {}
	ConfigFileTools(DataLog_Level *log, DataLog_Level *error) : _logLevel(log), _errorLevel(error) {}

   virtual const char * name(void) { return "ConfigFileTools"; }
	bool checkCRC(FILE * fp, const char * crcName);

	bool backupDataFile(const char * fileName, const char * crcName, const char * backupFileName, const char * backupCRCName);
	void protectDataFile(const char * name);
	void unprotectDataFile(const char * name);
	bool copyDataFile(const char * source, const char * dest);

	int stripLine(char * lineData, int len);
	bool getParamLong(const char * data, long * ref);
	bool getParamULong(const char * data, unsigned long * ref);
	bool getParamDouble(const char * data, double * ref);
	bool getParamBool(const char * data, bool * ref);
	bool getParamString(const char * data, char *& ref);

	bool strIEqual(const char * s1, const char * s2);

	void logLevel( DataLog_Level *l ) 	{ _logLevel = l; }
	void errorLevel( DataLog_Level *l ) { _errorLevel = l; }

protected:

	DataLog_Level * _logLevel;
	DataLog_Level * _errorLevel;

};


#endif /* ifndef _CONFIG_FILE_TOOLS_INCLUDE */

