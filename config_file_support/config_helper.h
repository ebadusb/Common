/*
 * $Header: Q:/BCT_Development/vxWorks/Common/config_file_support/rcs/config_helper.h 1.2 2007/05/02 14:08:22Z jl11312 Exp jd11007 $
 *
 * Utilities for managing configuration files
 *
 * $Log: config_helper.h $
 * Revision 1.1  2007/01/18 21:44:37Z  MS10234
 * Initial revision
 * Revision 1.1  2005/05/16 22:31:38Z  ms10234
 * Initial revision
 *
 */

#ifndef _CONFIG_HELPER_INCLUDE
#define _CONFIG_HELPER_INCLUDE

#include "callback.h"
#include "datalog_levels.h"
#include "basic_config_file.h"

class ConfigFile;
class ConfigData
{
public:
	enum BackupPermissionsType { NoBackup, AllowBackup };
	enum ParamType { TLong, TDouble, TString, TBool, TEnum };
	typedef bool RangeFunc(void);
	typedef bool LongRangeFunc(long & min, long & max);
	typedef bool DoubleRangeFunc(double & min, double & max);
	typedef bool ValidateFunc(void * value);

	typedef ConfigFile * GetConfigFileObj(int index);

	struct ParamValue
	{
		union
		{
			long		_lValue;
			double	_dValue;
			bool		_bValue;
			const char * _sValue;
		} _value;

		ParamType _type;
   };

	struct EnumMap
	{
		const char * _name;
		long _value;
	};

	struct DataMap
	{
		const char * _sectionName;
		const char * _paramName;
		ParamType _type;
		void * _value;
		const EnumMap * _enumMap;
		RangeFunc * _rangeFunc;
		ValidateFunc * _validateFunc;

		void setMap(const char * sectionName, const char * paramName, ParamType type, void * value, const EnumMap * enumMap, RangeFunc * rangeFunc, ValidateFunc * validateFunc)
		  { _sectionName=sectionName; _paramName=paramName; _type=type; _value=value; _enumMap=enumMap; _rangeFunc=rangeFunc; _validateFunc=validateFunc; }
   };

	static void readConfigData(GetConfigFileObj * getConfigFileObj, 
									   DataLog_Level * logLevel, DataLog_Level * errorLevel, 
										BackupPermissionsType backupPermission=NoBackup );
	static void checkConfigDataErrors(GetConfigFileObj * getConfigFileObj,
												 CallbackBase configBackupReadCb, 
												 CallbackBase configDefaultReadCb, 
												 CallbackBase configReadFailedCb);
};

class ConfigFile : public BasicConfigFile
{
public:

   virtual const char * formatVersion(void) = 0;

	void backupPermissions(ConfigData::BackupPermissionsType permissions) { _backupPermissions = permissions; }

protected:   
	ReadStatus readData(const ConfigData::DataMap * const dataMap, int dataMapSize, const char * fileName, const char * crcName, const char * backupFileName, const char * backupCRCName, const char * defaultFileName=0 );
	WriteStatus writeData(const ConfigData::DataMap * const dataMap, int dataMapSize, const char * fileName, const char * crcName, const char * backupFileName, const char * backupCRCName);

	static int getParamByName(const ConfigData::DataMap * const dataMap, int dataMapSize, const char * sectionName, const char * paramName);
	static bool getParamValueByName(const ConfigData::DataMap * const dataMap, int dataMapSize, const char * sectionName, const char * paramName, ConfigData::ParamValue & value);

	static const char * getParamNameByRef(const ConfigData::DataMap * const dataMap, int dataMapSize, void * param);
	static const char * getSectionNameByRef(const ConfigData::DataMap * const dataMap, int dataMapSize, void * param);

	bool getParamEnum(const char * data, const ConfigData::EnumMap * enumMap, long * ref);
	bool writeEnum(FILE * fp, const ConfigData::DataMap & dataMap, long value);
	bool writeString(FILE * fp, const ConfigData::DataMap & dataMap, const char * str);

	void logData(DataLog_Level * level, const ConfigData::DataMap * const dataMap, int dataMapSize, ConfigFile::ReadStatus readStatus);
	void logEnum(DataLog_Level * level, const ConfigData::DataMap & dataMap, long value);
	void logString(DataLog_Level * level, const ConfigData::DataMap & dataMap, const char * str);

private:
	bool readDataFromFile(const ConfigData::DataMap * const dataMap, int dataMapSize, const char * fileName, const char * crcName);
	bool checkRange(const ConfigData::DataMap * const dataMap, const char * fileName, int line, int idx);

protected:

	ConfigData::BackupPermissionsType _backupPermissions;
};

#endif /* ifndef _CONFIG_HELPER_INCLUDE */

