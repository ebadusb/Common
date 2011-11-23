// $Header$
//
// Configuration file class
//
// $Log: config_file.h $
// Revision 1.2  2005/05/11 15:53:17Z  jl11312
// - corrected archive types
// Revision 1.3  2005/01/20 17:25:22Z  jl11312
// - added suport for export file for shared enumeration types
// Revision 1.2  2005/01/13 21:33:18Z  jl11312
// - implemented array support
// - implemented {@@ const@@} option
// Revision 1.1  2004/07/14 16:11:15Z  jl11312
// Initial revision
//

#ifndef _CONFIG_FILE_INCLUDE
#define _CONFIG_FILE_INCLUDE

#include <string>
#include <vector>
#include <map>
#include "parameter.h"
#include "section.h"

using namespace std;

class ConfigFile
{
public:
	ConfigFile(const char * fileName);

	// Main parse function
	//
	bool parseFile(FILE * fp);

	// Project name related functions
	//
	void setProjectName(const char * name);

	// File attribute related functions
	//
	bool setFileName(const char * name);
	const char * fileName(void) { return _fileName.c_str(); }
	const char * formatVersion(void) { return _formatVersion.c_str(); }
	const char * dataVersion(void) { return _dataVersion.c_str(); }

	// Parameter/section related functions
	//
	void addParameter(Parameter * parameter);
	Parameter * getParameter(int param) { return _parameter[param]; }
	int parameterCount(void) { return _parameter.size(); }
	int sectionCount(void) { return _section.size(); }
   // This operation will find any existing parameter with a matching
   // parameter name and section name. Returns true if a duplicate is found and
   // assigns the paramter pointer to the duplicate vector element or returns
   // false if no duplicate is found.
   vector<Parameter *>::iterator ConfigFile::findDuplicate(Parameter*& paramPtr);
	// Class name related functions
	//
	const char * className(void) { return _className.c_str(); }

	// File attribute related functions
	//
	bool readWrite(void) { return _readWrite; }
 	bool hasBackup(void) { return _hasBackup; }
 	bool noOutputFile(void) { return _noOutputFile; }

	// Error related functions
	// 
	void printError(bool fatal, const char * format, ...);

	// Code generation related functions
	//
	void generateIncludeFileList(FILE * fp);
	void generateHeaderDataStructure(FILE * fp);
	void generateHeaderAccessClass(FILE * fp);
	void generateHeaderAccessObj(FILE * fp);
	void generateNameList(FILE * fp, const char * outputName);
	void generateSectionNameList(FILE * fp, const char * outputName);
	void generateConstructors(FILE * fp, const char * outputName);
	void generateLogDataFunction(FILE * fp, const char * outputName);
	void generateParameterValidateFunctions(FILE * fp, const char * outputName);
	void generateParameterRangeFunctions(FILE * fp, const char * outputName);
	void generateParameterSetFunctions(FILE * fp, const char * outputName);

	// Data file generation related functions
	//
	void generateDataFile(const char * dataFileDir);

private:
	int getChar(void);
	bool getNextToken(void);
	void getOption(void);
   void processParameter(void);
	void processFileOptions(void);
	void processParameterOptions(Parameter * parameter);
	bool convertNumericValue(Parameter::Value & value, Parameter::Type & type);
	string formatFileName(const string & base);
	void createDirectoryChain(const string & fileName);
	void validateSections(void);

private:
	struct EnumElement
	{
		string	_name;
		unsigned long _value;
	};

	struct EnumType
	{
		string _name;
		vector<EnumElement *> _id;
		bool _exportType;
	};

	string _projectName;

	string _fileName;		// name for .cfg file(s) being parsed
	int _lineNumber;		// current line number while reading .cfg file

	FILE * _fp;					      // FILE pointer for .cfg file
	bool _parseError;			      // true if an error was detected while parsing the .cfg file
	string _parseToken;		      // current token being parsed
	string _sectionName;          // current section name
	int _sectionArraySize;	      // array size for current section (0 if not an array)
	string _parameterName;	      // current parameter name
   string _variableName;         // current variable name
   string _stringValue;          // current string value

	vector<Parameter *> _parameter;
	vector<Section *>	  _section;
	vector<string>		  _include; 

	string _className;
	string _dataFileName;
	string _defaultFileName;
	string _backupFileName;
	string _crcName;
	string _backupCRCName;
	string _formatVersion;
	string _dataVersion;
   // Name of an optional included cfg file
   string _configFileName;

	bool _readWrite;
	bool _hasBackup;
	bool _noOutputFile;

public:
	enum EnumSelect { Local, Exported };
	static int enumCount(void);
	static void generateEnumList(FILE * fp, EnumSelect enumSelect);
	static void generateEnumMap(FILE * fp);

private:
	static vector<EnumType *>  _enumType;
};

#endif /* ifndef _CONFIG_FILE_INCLUDE */

