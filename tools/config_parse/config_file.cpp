// $Header: Q:/BCT_Development/vxWorks/Common/tools/config_parse/rcs/config_file.cpp 1.7 2007/10/29 17:18:35Z jl11312 Exp $
//
// Configuration file class
//
// $Log: config_file.cpp $
// Revision 1.7  2007/10/29 17:18:35Z  jl11312
// - added support for alternate variable names for enum parameters
// Revision 1.6  2007/05/02 13:59:51Z  jl11312
// - changed logging to correctly log string and enum values
// Revision 1.5  2007/01/04 16:42:56Z  MS10234
// 57 - add in file information fields that describe read-only and location attributes of the file
// Revision 1.4  2006/11/29 17:47:58Z  MS10234
// force the high bit on every enumerated element instead of just the first element
// Revision 1.3  2006/10/19 20:01:01Z  MS10234
// 63 - Add logging of the config files
// Revision 1.2  2005/05/11 16:36:02Z  jl11312
// - corrected archive types
// Revision 1.5  2005/02/09 21:14:16Z  jl11312
// - added default file name to list of parameters for config file read function
// Revision 1.4  2005/01/24 16:56:26Z  jl11312
// - corrected parsing of enum array parameters
// Revision 1.3  2005/01/20 17:23:39Z  jl11312
// - added suport for export file for shared enumeration types
// Revision 1.2  2005/01/18 15:19:47Z  jl11312
// - implemented array support
// - implemented @{ const @} option
// Revision 1.1  2004/07/16 20:33:26Z  jl11312
// Initial revision
//

#include "stdafx.h"

#pragma warning(disable:4786)
#include <ctype.h>
#include <stdarg.h>

#include "config_file.h"
#include "crc.h"

vector<ConfigFile::EnumType *>  ConfigFile::_enumType;

ConfigFile::ConfigFile(const char * fileName)
	: _fileName(fileName), _readWrite(false), _lineNumber(1), _parseError(false), _noOutputFile(false), _sectionArraySize(0)
{
}

int ConfigFile::enumCount(void)
{
	int count = 0;
	for ( unsigned int i=0; i<_enumType.size(); i++ )
	{
		count += _enumType[i]->_id.size();
	}

	return count;
}

bool ConfigFile::parseFile(FILE * fp)
{
	bool newSection = false;

	_fp = fp;

	// Add empty version section for file version data
	//
	Section * versionSection = new Section("Version", 0);
	_section.push_back(versionSection);

	// Add empty file info section
	//
	Section * fileInfoSection = new Section("FileInfo", 0);
	_section.push_back(fileInfoSection);

	processFileOptions();

	while ( !_parseError &&
		     !_parseToken.empty() )
	{
		if ( _parseToken[0] == '[' )
		{
			if ( isalpha(_parseToken[1]) )
			{
				if ( newSection ) printError("empty section \"%s\"", _sectionName.c_str());

				_sectionName = _parseToken.substr(1, _parseToken.size()-1);
				_sectionArraySize = 0;

				getNextToken();
				newSection = true;

				if ( _parseToken == "@{" )
				{
					if ( !getNextToken() )
					{
						printError("missing option name");
					}
					else if ( _parseToken == "array" )
					{
						Parameter::Value arraySize;
						Parameter::Type  arraySizeType;

						bool convertOK = getNextToken() &&
											   convertNumericValue(arraySize, arraySizeType) &&
												arraySizeType == Parameter::TLong  &&
												arraySize._lValue >= 1;

						if ( !convertOK )
						{
							printError("bad array size \"%s\"", _parseToken.c_str());
						}
						else
						{
							_sectionArraySize = arraySize._lValue;
							if ( !getNextToken() ||
								  _parseToken != "@}" )
							{
								printError("option missing \"@}\""); 
							}
							else
							{
								getNextToken();
							}
						}
					}
					else
					{
						printError("unrecognized section option");
					}
				}
			}
			else
			{
				printError("illegal section name");
			}
		}

		else if ( isalpha(_parseToken[0]) )
		{
			newSection = false;
			_parameterName = _parseToken;

			if ( _sectionName.empty() )
			{
				printError("parameter starts before first section");
			}
			else if ( !getNextToken() ||
				  _parseToken != "=" )
			{
				printError("missing '=' for parameter");
			}
			else if ( !getNextToken() )
			{
				printError("missing value for parameter");
			}
			else
			{
				processParameter();
			}
		}

		else
		{
			printError("expected section name or parameter");
		}
	}

	if ( newSection )
	{
		fprintf(stderr, "Empty section \"%s\" in file \"%s\"\n", _sectionName.c_str(), _fileName.c_str());
		_parseError = true;
	}

	if ( !_parseError )
	{
		validateSections();
	}

	return _parseError;
}

void ConfigFile::setProjectName(const char * name)
{
	_projectName = name;
}

bool ConfigFile::setFileName(const char * name)
{
	_dataFileName = name;

	if ( _dataFileName.find_first_of('\\') != string::npos )
	{
		printError("file name can not use \"\\\" style separators");
		return false;
	}

	string rootName = "/";
	rootName += _projectName;
	rootName += "/";

	if ( _dataFileName.find(rootName) == 0 )
	{
		// File is on /<project> partition, so there is no backup
		_hasBackup = false;
	}
	else if ( _dataFileName.find("/config/") == 0 )
	{
		// File is on /config partition, so we need backup and CRC protection
		_hasBackup = true;

		int extensionIdx = _dataFileName.find_last_of('.');
		int pathIdx = _dataFileName.find_last_of('/');

		if ( extensionIdx == string::npos )
		{
			printError("bad extension for file name");
			return false;
		}

		// Primary CRC file name is generated by replacing the extension with ".crc"
		// and adding the "crc" subdirectory to the path
		//
		_crcName = _dataFileName;
		_crcName.resize(extensionIdx);
		_crcName.insert(extensionIdx, ".crc");
		_crcName.insert(pathIdx, "/crc");

		// Backup file name is generated by adding the "backup" subdirectory to the path
		//
		_backupFileName = _dataFileName;
		_backupFileName.insert(pathIdx, "/backup");
		

		// Backup CRC file name is generated by replacing the extension with ".crc"
		// and adding the "backup/crc" subdirectory to the path
		//
		_backupCRCName = _dataFileName;
		_backupCRCName.resize(extensionIdx);
		_backupCRCName.insert(extensionIdx, ".crc");
		_backupCRCName.insert(pathIdx, "/backup/crc");
   }
	else
	{
		printError("file name must be in /<project> or /config partition");
		return false;
	}

	return true;
}

void ConfigFile::addParameter(Parameter * parameter)
{
	// Add parameter
	_parameter.push_back(parameter);

	// Add section name if necessary
	unsigned int s;
	for ( s=0; s<_section.size(); s++ )
	{
		if ( _section[s]->name() == parameter->sectionName() ) break;
	}

	if ( s >= _section.size() )
	{
		Section * section = new Section(parameter->sectionName(), _sectionArraySize);
		_section.push_back(section);
	}
}

void ConfigFile::printError(const char * format, ...)
{
	fprintf(stderr, "%s:%d ", _fileName.c_str(), _lineNumber);

	va_list marker;
	va_start(marker, format);
	vfprintf(stderr, format, marker);
	va_end(marker);

	fprintf(stderr, "\n");

	fprintf(stderr, "Current token = %s\n", _parseToken.c_str());
	_parseError = true;
}

int ConfigFile::getChar(void)
{
	int ch = fgetc(_fp);
	if ( ch == '\n' )
	{
		_lineNumber += 1;
	}

	return ch;
}

bool ConfigFile::getNextToken(void)
{
	if ( _parseError ) return false;

	bool firstCharFound = false;
	bool skippingComment = false;
	int ch;
	while ( !firstCharFound )
	{
		ch = getChar();
		if ( ch == EOF )
		{
			firstCharFound = true;
		}
		else if ( skippingComment )
		{
			if ( ch == '\n' ) skippingComment = false;
		}
		else if ( ch == '#' )
		{
			skippingComment = true;
		}
		else if ( !isspace(ch) )
		{
			firstCharFound = true;
		}
   }

	_parseToken = ch;
	if ( ch == EOF )
	{
		_parseToken.erase();
		return false;
	}

	if ( _parseToken[0] == '=' )
	{
		return true;
	}

	if ( _parseToken[0] == '\"' )
	{
		while ( (ch = getChar()) != EOF &&
				  ch != '\"' &&
				  ch != '\n' )
		{
			if ( ch == '\\' )
			{
				ch = getChar();
				if ( ch == '"' )
				{
					_parseToken.insert(_parseToken.end(), ch);
				}
				else
				{
					_parseToken.insert(_parseToken.end(), '\\');
					if ( ch != EOF ) ungetc(ch, _fp);
				}
			}
			else
			{
				_parseToken.insert(_parseToken.end(), ch);
			}
		}

		if ( ch == '\"' )
		{
			return true;
		}
		else
		{
			printError("string token missing closing \"");
			return false;
		}
	}

	if ( _parseToken[0] == '[' )
	{
		while ( (ch = getChar()) != EOF &&
				  ch != ']' &&
				  ch != '\n' )
		{
			_parseToken.insert(_parseToken.end(), ch);
		}

		if ( ch == ']' )
		{
			return true;
		}
		else
		{
			printError("section name missing closing ]");
			return false;
		}
	}

	if ( _parseToken[0] == '@' )
	{
		ch = getChar();
		if ( ch == '{' ||
			  ch == '}' )
		{
			_parseToken.insert(_parseToken.end(), ch);
			return true;
		}
		else
		{
			ungetc(ch, _fp);
		}
	}

	while ( (ch = getChar()) != EOF &&
			  !isspace(ch) &&
			  ch != '@' &&
			  ch != '#' &&
			  ch != '=' )
	{
		_parseToken.insert(_parseToken.end(), ch);
	}

	if ( ch != EOF ) ungetc(ch, _fp);
	return true;
}

void ConfigFile::getOption(void)
{
	int ch;
	bool done = false;
	bool started = false;

	_parseToken.erase();
	while ( (ch = getChar()) != EOF &&
			  !done )
	{
		if ( ch == '@' )
		{
			started = true;
			ch = getChar();
			if ( ch == '}' )
			{
				done = true;
			}
			else if ( ch != EOF )
			{
				ungetc(ch, _fp);
				_parseToken.insert(_parseToken.end(), '@');
			}
		}
		else if ( started ||
					 !isspace(ch) )
		{
			started = true;
			_parseToken.insert(_parseToken.end(), ch);
		}
	}

	if ( !done )
	{
		printError("option missing \"@}\"");
	}
}

void ConfigFile::processParameter(void)
{
	Parameter * parameterObj = NULL;

	if ( _parseToken[0] == '\"' )
	{
		parameterObj = new StringParameter(_sectionName, _parameterName, _parseToken.substr(1, _parseToken.size()-1));
	}
	else if ( isdigit(_parseToken[0]) ||
				 _parseToken[0] == '-' ||
				 _parseToken[0] == '.' )
	{
		Parameter::Value value;
		Parameter::Type type;

		if ( convertNumericValue(value, type) )
		{
			switch ( type )
			{
			case Parameter::TLong:
				parameterObj = new LongParameter(_sectionName, _parameterName, _parseToken, value._lValue);
				break;

			case Parameter::TDouble:
				parameterObj = new DoubleParameter(_sectionName, _parameterName, _parseToken, value._dValue);
				break;

			default:
				printError("internal error unhandled number type\n");
				_parseError = true;
			}
		}
	}
	else if ( _parseToken == "true" )
	{
		parameterObj = new BoolParameter(_sectionName, _parameterName, _parseToken, true);
	}
	else if ( _parseToken == "false" )
	{
		parameterObj = new BoolParameter(_sectionName, _parameterName, _parseToken, false);
	}
	else
	{
		for ( unsigned int enumIdx=0; enumIdx<_enumType.size() && !parameterObj; enumIdx++ )
		{
			if ( _parameterName.find(_enumType[enumIdx]->_name) == 0 )
			{
				for ( unsigned int id=0; id<_enumType[enumIdx]->_id.size() && !parameterObj; id++ )
				{
					if ( _enumType[enumIdx]->_id[id]->_name == _parseToken )
					{
						parameterObj = new EnumParameter(_sectionName, _parameterName, _parseToken);
					}
				}
			}
		}
	}

	if ( !parameterObj )
	{
		printError("invalid parameter value");
	}
	else
	{
		processParameterOptions(parameterObj);
		addParameter(parameterObj);
	}
}

void ConfigFile::processFileOptions(void)
{
	while ( getNextToken() &&
			  _parseToken == "@{" )
	{
		if ( !getNextToken() )
		{
			printError("missing option name");
		}
		else if ( _parseToken == "class-name" )
		{
			if ( getNextToken() &&
				  isalpha(_parseToken[0]) )
			{
				_className = _parseToken;
			}
			else
			{
				printError("bad class name");
			}
		}
		else if ( _parseToken == "no-output-file")
		{
			_noOutputFile = true;
		}
		else if ( _parseToken == "file-name" )
		{
			if ( getNextToken() &&
				  _parseToken[0] == '"' )
			{
				string dataFileName = _parseToken.substr(1, _parseToken.size()-1);
				setFileName(dataFileName.c_str());
			}
			else
			{
				printError("bad file name");
			}
		}
		else if ( _parseToken == "default-file-name" )
		{
			if ( getNextToken() &&
				  _parseToken[0] == '"' )
			{
				_defaultFileName = _parseToken.substr(1, _parseToken.size()-1);
			}
			else
			{
				printError("bad file name");
			}
		}
		else if ( _parseToken == "read-write" )
		{
			_readWrite = true;
		}
		else if ( _parseToken == "include" )
		{
			getOption();
			_include.push_back(_parseToken);
			_parseToken = "@}";
		}
		else if ( _parseToken == "enum" ||
					 _parseToken == "enum-export" )
		{
			EnumType * eType = new EnumType;
			eType->_exportType = ( _parseToken == "enum-export" );

			if ( !getNextToken() ||
				  !isalpha(_parseToken[0]) )
			{
				printError("expected enum name");
			}
			else
			{
				int	elementCount = 0;

				eType->_name = _parseToken;
				while ( getNextToken() &&
						  _parseToken != "@}" )
				{
					EnumElement * element = new EnumElement;
					element->_name = _parseToken;

					element->_value = 0xffffffff;
					crcgen32(&element->_value, (const unsigned char *)element->_name.c_str(), element->_name.size());

					// Force high bit on the crc value to ensure that sizeof(enum type) == sizeof(long)
					//
					element->_value |= 0x80000000;
					elementCount += 1;

					for ( unsigned int i=0; i<eType->_id.size(); i++ )
					{
						if ( eType->_id[i]->_value == element->_value )
						{
							printError("non-unique CRC values for enumerations \"%s\" and \"%s\"",
								eType->_id[i]->_name.c_str(), element->_name.c_str());
						}
					}

					eType->_id.push_back(element);
				}

				if ( elementCount <= 0 )
				{
					printError("enum type must have at least one enumeration specified");
				}
				else
				{
					_enumType.push_back(eType);
				}
			}
		}
		else if ( _parseToken == "format-version" )
		{
			if ( getNextToken() &&
				  _parseToken[0] == '"' )
			{
				_formatVersion = _parseToken.substr(1, _parseToken.size()-1);

				Parameter * param = new StringParameter("Version", "FormatVersion", _formatVersion);
				addParameter(param);
			}
			else
			{
				printError("bad format version");
			}
		}
		else if ( _parseToken == "data-version" )
		{
			if ( getNextToken() &&
				  _parseToken[0] == '"' )
			{
				_dataVersion = _parseToken.substr(1, _parseToken.size()-1);

				Parameter * param = new StringParameter("Version", "DataVersion", _dataVersion);
				addParameter(param);
			}
			else
			{
				printError("bad data version");
			}
		}
		else
		{
			printError("bad option name");
		}

		if ( !_parseError )
		{
			if ( _parseToken != "@}" &&
				  ( !getNextToken() || _parseToken != "@}" ))
			{
				printError("missing \"}@\"");
			}
	   }
	}

	Parameter * readOnlyParam;
	if ( _readWrite )
	{
		readOnlyParam = new BoolParameter("FileInfo", "ReadOnly", "false", false);
	}
	else
	{
		readOnlyParam = new BoolParameter("FileInfo", "ReadOnly", "true", true);
	}
	addParameter(readOnlyParam);
	Parameter * filenameParam = new StringParameter("FileInfo","FileName",_dataFileName.c_str());
	addParameter(filenameParam);


	if ( _noOutputFile &&
		  ( !_dataFileName.empty() ||
		    !_className.empty() ||
		    !_formatVersion.empty() ||
		    !_dataVersion.empty() ))
	{
		printError("file-name, class-name, format-version, and data-version can not be specified with \"no-output-file\" option");
	}
	else if ( !_noOutputFile &&
				 ( _dataFileName.empty() ||
				   _className.empty() ||
				   _formatVersion.empty() ||
				   _dataVersion.empty() ))
	{
		printError("file-name, class-name, format-version, and data-version must be specified");
	}
}

void ConfigFile::processParameterOptions(Parameter * parameter)
{
	bool rangeSet = false;
	bool rangeFuncSet = false;

	while ( getNextToken() &&
			  _parseToken == "@{" )
	{
		if ( parameter->index() > 0 )
		{
			printError("options for array parameters valid only for array element 0");
		}
		else if ( !getNextToken() )
		{
			printError("missing option name");
		}
		else if ( _parseToken == "variable-name" )
		{
			if ( parameter->getType() != Parameter::TEnum )
			{
				printError("variable-name option only supported for enum parameters");
			}

			getOption();
			parameter->setVariableName(_parseToken);
			_parseToken = "@}";
		}
		else if ( _parseToken == "const" )
		{
			parameter->setConst();
		}
		else if ( _parseToken == "validate" )
		{
			getOption();
			parameter->setValidateFunction(_parseToken);
			_parseToken = "@}";
		}
		else if ( _parseToken == "range-func" )
		{
			if ( rangeSet ) printError("parameter can not specify both range and range-func options");
			if ( parameter->getType() != Parameter::TLong &&
				  parameter->getType() != Parameter::TDouble )
			{
				printError("range-func option not supported for parameter type");
			}

			getOption();
			parameter->setRangeFunc(_parseToken);
			_parseToken = "@}";
			rangeFuncSet = true;
		}
		else if ( _parseToken == "range" )
		{
			if ( rangeFuncSet ) printError("parameter can not specify both range and range-func options");

			Parameter::Value min;
			Parameter::Type minType;
			string minString;

			Parameter::Value max;
			Parameter::Type maxType;
			string maxString;

			bool convertOK = getNextToken() && convertNumericValue(min, minType);
			if ( convertOK )
			{
				minString = _parseToken;
				convertOK = getNextToken() && convertNumericValue(max, maxType);
				maxString = _parseToken;
			}

			if ( parameter->getType() != Parameter::TLong &&
				  parameter->getType() != Parameter::TDouble )
			{
				printError("range option not supported for parameter type");
			}
			else if ( !convertOK )
			{
				printError("bad range specification");
			}
			else
			{
				if ( parameter->getType() == Parameter::TLong &&
					  minType == Parameter::TLong &&
					  maxType == Parameter::TLong )
				{
					// If range uses "0b" format, we need to convert to hex so that the generated code
					// can be compiled properly.
					//
					enum { TempSize = 40 };
					char temp[TempSize];
					if ( minString.find("0b") == 0 )
					{
						sprintf(temp, "0x%lx", (unsigned long)min._lValue);
						minString = temp;
					}

					if ( maxString.find("0b") == 0 )
					{
						sprintf(temp, "0x%lx", (unsigned long)max._lValue);
						maxString = temp;
					}

					((LongParameter *)parameter)->setRange(minString, maxString);
				}
				else if ( parameter->getType() == Parameter::TDouble )
				{
					((DoubleParameter *)parameter)->setRange(minString, maxString);
				}
				else
				{
					printError("incompatible range values for parameter type");
				}
			}

			rangeSet = true;
		}
		else
		{
			printError("bad option name");
		}

		if ( !_parseError )
		{
			if ( _parseToken != "@}" &&
				  ( !getNextToken() || _parseToken != "@}" ))
			{
				printError("missing \"}@\"");
			}
	   }
	}
}

bool ConfigFile::convertNumericValue(Parameter::Value & value, Parameter::Type & type)
{
	bool result = false;
	const char * token = _parseToken.c_str();

	if ( _parseToken.find("0x") == 0 )
	{
		char * endPtr;
		value._lValue = strtol(&token[2], &endPtr, 16);
		if ( *endPtr != '\0' )
		{
			printError("bad hex value");
		}
		else
		{
			type = Parameter::TLong;
			result = true;
		}
	}
	else if ( _parseToken.find("0b") == 0 )
	{
		char * endPtr;
		value._lValue = strtol(&token[2], &endPtr, 2);
		if ( *endPtr != '\0' )
		{
			printError("bad binary value");
		}
		else
		{
			type = Parameter::TLong;
			result = true;
		}
	}
	else if ( strchr(token, '.') ||
			    strchr(token, 'E') ||
				 strchr(token, 'e') ) 
	{
		char * endPtr;
		value._dValue = strtod(token, &endPtr);
		if ( *endPtr != '\0' )
		{
			printError("bad floating point value");
		}
		else
		{
			type = Parameter::TDouble;
			result = true;
		}
	}
	else
	{
		char * endPtr;
		value._lValue = strtol(token, &endPtr, 10);
		if ( *endPtr != '\0' )
		{
			printError("bad decimal value");
		}
		else
		{
			type = Parameter::TLong;
			result = true;
		}
	}

	return result;
}

string ConfigFile::formatFileName(const string & base)
{
	string result;
	string projectRoot = "/" + _projectName;

	if ( base.find(projectRoot+"/") == 0 )
	{
		int dirLen = projectRoot.size();

		string	upCaseRoot = _projectName;
		for ( unsigned int i=0; i<upCaseRoot.size(); i++ )	upCaseRoot[i] = toupper(upCaseRoot[i]);

		result = upCaseRoot;
		result += "_PATH \"";
		result += base.substr(dirLen, base.size()-dirLen);
		result += "\"";
	}
	else if ( base.find("/config/") == 0 )
	{
		int dirLen = strlen("/config");

		result = "CONFIG_PATH \"";
		result += base.substr(dirLen, base.size()-dirLen);
		result += "\"";
	}
	else
	{
		result = "\"";
		result += base;
		result = "\"";
	}

	return result;
}

void ConfigFile::createDirectoryChain(const string & fileName)
{
	string	pathName;
	int		pathEndIdx = fileName.find_first_of('/');

	while ( pathEndIdx != string::npos )
	{
		pathName = fileName.substr(0, pathEndIdx);
		CreateDirectory(pathName.c_str(), NULL);

		pathEndIdx = fileName.find_first_of('/', pathEndIdx+1);
	}
}

void ConfigFile::validateSections(void)
{
	for ( unsigned int sectIdx=0; sectIdx<_section.size(); sectIdx+=1 )
	{
		if ( _section[sectIdx]->arraySize() == 0 )
		{
			for ( unsigned int paramIdx=0; paramIdx<_parameter.size(); paramIdx+=1 )
			{
				if ( _parameter[paramIdx]->sectionName() == _section[sectIdx]->name() &&
					  _parameter[paramIdx]->index() != -1 )
				{
					printError("illegal specification of index for non-array section - section=%s parameter=%s%s",
									_section[sectIdx]->name().c_str(), _parameter[paramIdx]->variableName().c_str(), _parameter[paramIdx]->indexString().c_str());
				}
			}
		}
		else
		{
			for ( unsigned int paramIdx=0; paramIdx<_parameter.size(); paramIdx+=1 )
			{
				if ( _parameter[paramIdx]->sectionName() == _section[sectIdx]->name() )
				{
					if ( _parameter[paramIdx]->index() < 0 || 
						  _parameter[paramIdx]->index() >= _section[sectIdx]->arraySize() )
					{
						printError("parameter index out of range - section=%s parameter=%s%s",
									_section[sectIdx]->name().c_str(), _parameter[paramIdx]->variableName().c_str(), _parameter[paramIdx]->indexString().c_str());
					}
					else if ( _parameter[paramIdx]->index() == 0 )
					{
						// Make sure parameters exist for all array indices
						for ( int idx=1; idx<_section[sectIdx]->arraySize(); idx++ )
						{
							bool found = false;
							for ( unsigned int searchIdx=0; searchIdx<_parameter.size(); searchIdx++ )
							{
								if ( _parameter[searchIdx]->variableName() == _parameter[paramIdx]->variableName() &&
									  _parameter[searchIdx]->sectionName() == _section[sectIdx]->name() &&
									  _parameter[searchIdx]->index() == idx )
								{
									found = true;
									break; 
								}

							}

							if ( !found )
							{
								printError("missing parameter setting for %s[%d] in section %s",
										_parameter[paramIdx]->variableName().c_str(), idx, _section[sectIdx]->name().c_str());
							}
						}
					}
					else
					{
						// Make sure parameter with index==0 exists
						bool found = false;
						for ( unsigned int searchIdx=0; searchIdx<_parameter.size(); searchIdx++ )
						{
							if ( _parameter[searchIdx]->variableName() == _parameter[paramIdx]->variableName() &&
								  _parameter[searchIdx]->sectionName() == _section[sectIdx]->name() &&
								  _parameter[searchIdx]->index() == 0 )
							{
								found = true;
								break; 
							}
						}

						if ( !found )
						{
							printError("missing parameter setting for %s[0] in section %s",
									_parameter[paramIdx]->variableName().c_str(), _section[sectIdx]->name().c_str());
						}
					}
				}
			}
		}
	}
}

void ConfigFile::generateIncludeFileList(FILE * fp)
{
	for ( unsigned int i=0; i<_include.size(); i++ )
	{
		fprintf(fp, "#include %s\n", _include[i].c_str());
	}
}

void ConfigFile::generateEnumList(FILE * fp, EnumSelect enumSelect)
{
	for ( unsigned int i=0; i<_enumType.size(); i++ )
	{
		if ( (enumSelect == Local && !_enumType[i]->_exportType) ||
			  (enumSelect == Exported && _enumType[i]->_exportType) )
		{
			fprintf(fp, "  enum T%s {", _enumType[i]->_name.c_str());
			for ( unsigned int t=0; t<_enumType[i]->_id.size(); t++ )
			{
				fprintf(fp,
					"%s%s = 0x%lx",
					(t==0) ? " " : ", ",
					_enumType[i]->_id[t]->_name.c_str(), _enumType[i]->_id[t]->_value);
			}
	
			fprintf(fp,
				" };\n");
		}

		if ( enumSelect != Exported )
		{
			fprintf(fp,
				"  static bool validate%s(void *value)\n"
				"  {\n"
				"    return (",
				_enumType[i]->_name.c_str());

			for ( unsigned int t=0; t<_enumType[i]->_id.size(); t++ )
			{
				fprintf(fp,
					"%s*(unsigned long *)value == 0x%lx",
					(t==0) ? " " : " ||\n              ", _enumType[i]->_id[t]->_value);
			}
	
			fprintf(fp,
				" );\n"
				"  }\n\n");
		}
	}
}

void ConfigFile::generateEnumMap(FILE * fp)
{
	for ( unsigned int i=0; i<_enumType.size(); i++ )
	{
		for ( unsigned int t=0; t<_enumType[i]->_id.size(); t++ )
		{
			fprintf(fp,
				"  { \"%s\", 0x%lx },\n",
				_enumType[i]->_id[t]->_name.c_str(), _enumType[i]->_id[t]->_value);
		}
	}
}

void ConfigFile::generateHeaderDataStructure(FILE * fp)
{
	fprintf(fp,
		"public:\n"
		"  struct %s_Data\n"
		"  {\n",
		_className.c_str());

	unsigned int sectIdx;
	unsigned int paramIdx;

	for ( sectIdx=0; sectIdx<_section.size(); sectIdx++ )
	{
		fprintf(fp,
			"    struct\n"
			"    {\n");

		for ( paramIdx=0; paramIdx<_parameter.size(); paramIdx++ )
		{
			// entries are only generated for non-array sections (index==-1) or for parameters with index==0
			//
			if ( _parameter[paramIdx]->index() > 0 ) continue;

			if ( _section[sectIdx]->name() == _parameter[paramIdx]->sectionName() )
			{
				switch ( _parameter[paramIdx]->getType() )
				{
				case Parameter::TLong:
					fprintf(fp, "      long");
					break;

				case Parameter::TDouble:
					fprintf(fp, "      double");
					break;

				case Parameter::TString:
					fprintf(fp, "      const char *");
					break;

				case Parameter::TBool:
					fprintf(fp, "      bool");
					break;

				case Parameter::TEnum:
					fprintf(fp, "      T%s", _parameter[paramIdx]->parameterName().c_str());
					break;

				default:
					fprintf(fp, "      bad-type");
					break;
			   }

			
				if ( _section[sectIdx]->arraySize() > 0 )
				{
					fprintf(fp,
						" %s[%d];\n",
						_parameter[paramIdx]->variableName().c_str(), _section[sectIdx]->arraySize());
			   }
			   else
			   {
					fprintf(fp,
						" %s;\n",
						_parameter[paramIdx]->variableName().c_str());
			   }
         }
		}

		fprintf(fp,
			"    } %s;\n",
			_section[sectIdx]->name().c_str());
	}

	fprintf(fp,
		"  %s_Data(void) { memset(this, 0, sizeof(*this)); }\n",
		_className.c_str());

	fprintf(fp,
	   "  };\n\n"
		"  static const %s_Data & %s(void) { return _%s_Data; }\n\n"
		"private:\n"
		"  static %s_Data _%s_Data;\n\n",
		_className.c_str(), _className.c_str(), _className.c_str(), _className.c_str(), _className.c_str());
}

void ConfigFile::generateHeaderAccessClass(FILE * fp)
{
	fprintf(fp,
		"private:\n"
		"  class _C_%s : public ConfigFile\n"
		"  {\n"
		"    static const char * _sectionName[%d];\n"
		"    static const char * _name[%d];\n"
		"    DataMap _dataMap[%d];\n\n",
		_className.c_str(), _section.size(), _parameter.size(), _parameter.size());

	unsigned int sectIdx;
	unsigned int paramIdx;
	for ( sectIdx=0; sectIdx<_section.size(); sectIdx++ )
	{
		fprintf(fp,
			"    class _C_%s\n"
			"    {\n",
			_section[sectIdx]->name().c_str());

		for ( paramIdx=0; paramIdx<_parameter.size(); paramIdx++ )
		{
			if ( _parameter[paramIdx]->index() > 0 ) continue;
			if ( _section[sectIdx]->name() == _parameter[paramIdx]->sectionName() )
			{
				_parameter[paramIdx]->generateAccessClassHeader(fp, _readWrite, _section[sectIdx]->arraySize());
		   }
		}

		// End of declaration for current section
		//
		fprintf(fp,
			"    public:\n");

		if ( _readWrite )
		{
			fprintf(fp,
				"      _C_%s(const DataMap * dataMap);\n",
				_section[sectIdx]->name().c_str());
		}
		else
		{
			fprintf(fp,
				"      _C_%s(void);\n",
				_section[sectIdx]->name().c_str());
		}

		fprintf(fp,

			"      static const char * name(void) { return _sectionName[%d]; }\n\n",
			sectIdx);
	
		for ( paramIdx=0; paramIdx<_parameter.size(); paramIdx++ )
		{
			if ( _section[sectIdx]->name() == _parameter[paramIdx]->sectionName() &&
				  _parameter[paramIdx]->index() <= 0 )
			{
				if ( _section[sectIdx]->arraySize() > 0 )
				{
					fprintf(fp,
						"      _C_%s %s[%d];\n",
						_parameter[paramIdx]->variableName().c_str(), _parameter[paramIdx]->variableName().c_str(),
						_section[sectIdx]->arraySize());
				}
				else
				{
					fprintf(fp,
						"      _C_%s %s;\n",
						_parameter[paramIdx]->variableName().c_str(), _parameter[paramIdx]->variableName().c_str());
				}
         }
		}
	
		fprintf(fp, 
			"    };\n\n");
   }


	fprintf(fp,
		"		void logData( DataLog_Level * level, ConfigFile::ReadStatus s );\n"
		"		void logData( DataLog_Level * level, ConfigFile::WriteStatus s);\n\n");

	fprintf(fp,
	   "  public:\n"
		"    _C_%s(%s_Data & data);\n"
		"    const char * name(void) { return \"%s\"; }\n"
		"    const char * formatVersion(void) { return \"%s\"; }\n"
		"    const char * fileName(void) { return %s; }\n",
		_className.c_str(), _className.c_str(), _className.c_str(),
		_formatVersion.c_str(), formatFileName(_dataFileName).c_str());

	if ( _hasBackup )
	{
		fprintf(fp,
			"    const char * crcFileName(void) { return %s; }\n"
			"    const char * backupFileName(void) { return %s; }\n"
			"    const char * backupCRCFileName(void) { return %s; }\n\n",
			formatFileName(_crcName).c_str(), formatFileName(_backupFileName).c_str(),
			formatFileName(_backupCRCName).c_str());
	}
	else
	{
		fprintf(fp,
			"    const char * crcFileName(void) { return NULL; }\n"
			"    const char * backupFileName(void) { return NULL; }\n"
			"    const char * backupCRCFileName(void) { return NULL; }\n\n");
	}

	if ( !_defaultFileName.empty() )
	{
		fprintf(fp,
			"    const char * defaultFileName(void) { return %s; }\n\n",
			formatFileName(_defaultFileName).c_str());
	}
	else
	{
		fprintf(fp,
			"    const char * defaultFileName(void) { return NULL; }\n\n");
	}

	fprintf(fp,
		"    inline bool getParamValueByName(const char * sectionName, const char * paramName, ParamValue & value)\n"
		"    {\n"
		"      return ConfigFile::getParamValueByName(_dataMap, %d, sectionName, paramName, value);\n"
		"    }\n\n",
		_parameter.size());

	fprintf(fp,
		"    inline const char * getParamNameByRef(void * value)\n"
		"    {\n"
		"      return ConfigFile::getParamNameByRef(_dataMap, %d, value);\n"
		"    }\n\n"
		"    inline const char * getSectionNameByRef(void * value)\n"
		"    {\n"
		"      return ConfigFile::getSectionNameByRef(_dataMap, %d, value);\n"
		"    }\n\n"
		"	ReadStatus readFile(DataLog_Level * logLevel, DataLog_Level * errorLevel)\n"
		"	{\n"
		"		ConfigFile::ReadStatus status;\n"
		"		_logLevel = logLevel;\n"
		"		_errorLevel = errorLevel;\n"
		"		status = ConfigFile::readData(_dataMap, %d, fileName(), crcFileName(), backupFileName(), backupCRCFileName(), defaultFileName());\n"
		"		logData(&log_level_config_data_info, status);\n"
		"		return status;\n"
		"	}\n\n",
		_parameter.size(), _parameter.size(), _parameter.size());

	if ( _hasBackup )
	{
		fprintf(fp,
			"	WriteStatus writeFile(DataLog_Level * logLevel, DataLog_Level * errorLevel)\n"
			"	{\n"
			"		ConfigFile::WriteStatus status;\n"
			"		_logLevel = logLevel;\n"
			"		_errorLevel = errorLevel;\n"
			"		status = ConfigFile::writeData(_dataMap, %d, fileName(), crcFileName(), backupFileName(), backupCRCFileName());\n" 
			"		logData(&log_level_config_data_info, status);\n"
			"		return status;\n"
			"	}\n\n",
			_parameter.size());
	}

	for ( sectIdx=0; sectIdx<_section.size(); sectIdx++ )
	{
		fprintf(fp,
			"    _C_%s %s;\n",
			_section[sectIdx]->name().c_str(), _section[sectIdx]->name().c_str());
	} 

	fprintf(fp,
	   "  };\n\n");
}

void ConfigFile::generateHeaderAccessObj(FILE * fp)
{
	fprintf(fp,
		"  static _C_%s %s_Access;\n",
		_className.c_str(), _className.c_str());
}

void ConfigFile::generateSectionNameList(FILE * fp, const char * outputName)
{
	fprintf(fp, 
		"const char * %s::_C_%s::_sectionName[%d] =\n"
		"{\n",
		outputName, _className.c_str(), _section.size());

	for ( unsigned int sectIdx=0; sectIdx<_section.size(); sectIdx++ )
	{
		fprintf(fp,
			"%s\"%s\"",
			(sectIdx == 0) ? "  " : ",\n  ", _section[sectIdx]->name().c_str());
	} 

	fprintf(fp,
		"\n"
	   "};\n\n");
}

void ConfigFile::generateNameList(FILE * fp, const char * outputName)
{
	fprintf(fp, 
		"const char * %s::_C_%s::_name[%d] =\n"
		"{\n",
		outputName, _className.c_str(), _parameter.size());

	for ( unsigned int paramIdx=0; paramIdx<_parameter.size(); paramIdx++ )
	{
		fprintf(fp,
			"%s\"%s%s\"",
			(paramIdx == 0) ? "  " : ",\n  ", _parameter[paramIdx]->variableName().c_str(),
			_parameter[paramIdx]->indexString().c_str());
	} 

	fprintf(fp,
		"\n"
	   "};\n\n");
}

void ConfigFile::generateConstructors(FILE * fp, const char * outputName)
{
	fprintf(fp, 
		"%s::_C_%s::_C_%s(%s_Data & data)\n",
		outputName, _className.c_str(), _className.c_str(), _className.c_str());

	if ( _readWrite )
	{
		fprintf(fp,
			"  :");
		for ( unsigned int sectIdx = 0; sectIdx < _section.size(); sectIdx+=1 )
		{
			fprintf(fp, "%s%s(_dataMap)", (sectIdx==0) ? " " : ", ", _section[sectIdx]->name().c_str());
		}

		fprintf(fp, "\n");
	}

	fprintf(fp,
		"{\n");

	for ( unsigned int paramIdx=0; paramIdx<_parameter.size(); paramIdx++ )
	{
		unsigned int sectIdx;
		for ( sectIdx=0; sectIdx<_section.size(); sectIdx++ )
		{
			if ( _section[sectIdx]->name() == _parameter[paramIdx]->sectionName() ) break;
		}

		fprintf(fp,
			"  _dataMap[%d].setMap(_sectionName[%d], _name[%d], %s, (void *)&data.%s.%s%s,",
			paramIdx, sectIdx, paramIdx, _parameter[paramIdx]->getTypeName().c_str(),
			_section[sectIdx]->name().c_str(), _parameter[paramIdx]->variableName().c_str(),
			_parameter[paramIdx]->indexString().c_str());

		fprintf(fp,
			" %s,",
			(_parameter[paramIdx]->getType() == Parameter::TEnum) ? "_enumMap" : "NULL");

		if ( _parameter[paramIdx]->hasRange() || _parameter[paramIdx]->hasRangeFunc() )
		{
			fprintf(fp,
			   " (RangeFunc *)&%s::_C_%s::_C_%s::_C_%s::getRange,",
			   outputName, _className.c_str(), _parameter[paramIdx]->sectionName().c_str(), _parameter[paramIdx]->variableName().c_str());
		}
		else
		{
			fprintf(fp,
				" NULL,");
		}

		if ( !_parameter[paramIdx]->validateFunction().empty() ||
    		  _parameter[paramIdx]->getType() == Parameter::TEnum )
		{
			fprintf(fp, 
				" &%s::_C_%s::_C_%s::_C_%s::validate",
				outputName, _className.c_str(), _parameter[paramIdx]->sectionName().c_str(), _parameter[paramIdx]->variableName().c_str());
		}
		else
		{
			fprintf(fp, 
				" NULL");
		}

		fprintf(fp,
			");\n");
	}

	fprintf(fp, 
		"}\n\n");

	if ( _readWrite )
	{
		for ( unsigned int sectIdx = 0; sectIdx < _section.size(); sectIdx+=1 )
		{
		   fprintf(fp, 
		      "%s::_C_%s::_C_%s::_C_%s(const DataMap * dataMap)\n",
			  outputName, _className.c_str(), _section[sectIdx]->name().c_str(), _section[sectIdx]->name().c_str());

		   fprintf(fp,
			  "{\n");

		   for ( unsigned int paramIdx = 0; paramIdx < _parameter.size(); paramIdx+=1 )
		   {
				if ( _parameter[paramIdx]->sectionName() == _section[sectIdx]->name() )
				{
					fprintf(fp,
						"  %s%s.initialize(dataMap, %d);\n",
						_parameter[paramIdx]->variableName().c_str(), _parameter[paramIdx]->indexString().c_str(), paramIdx);
				}
		   }

		   fprintf(fp,
			  "}\n\n");
		}
	}
	else
	{
		for ( unsigned int sectIdx = 0; sectIdx < _section.size(); sectIdx+=1 )
		{
		   fprintf(fp, 
		      "%s::_C_%s::_C_%s::_C_%s(void)\n",
			  outputName, _className.c_str(), _section[sectIdx]->name().c_str(), _section[sectIdx]->name().c_str());

		   fprintf(fp,
			  "{\n");

		   for ( unsigned int paramIdx = 0; paramIdx < _parameter.size(); paramIdx+=1 )
		   {
				if ( _parameter[paramIdx]->sectionName() == _section[sectIdx]->name() )
				{
					fprintf(fp,
						"  %s%s.initialize(%d);\n",
						_parameter[paramIdx]->variableName().c_str(), _parameter[paramIdx]->indexString().c_str(), paramIdx);
				}
		   }

		   fprintf(fp,
			  "}\n\n");
		}

	}
}

void ConfigFile::generateLogDataFunction(FILE * fp, const char * outputName)
{
	// ReadStatus version...
	fprintf(fp, 
		"void %s::_C_%s::logData( DataLog_Level * level, ConfigFile::ReadStatus readStatus )\n",
		outputName, _className.c_str(), _className.c_str(), _className.c_str());

	fprintf(fp,
		"{\n");

	fprintf(fp,
		"   ConfigFile::logData(level, _dataMap, %d, readStatus);\n",
		_parameter.size());

	fprintf(fp, 
		"}\n\n");

	// WriteStatus version...
	fprintf(fp, 
		"void %s::_C_%s::logData( DataLog_Level * level, ConfigFile::WriteStatus writeStatus )\n",
		outputName, _className.c_str(), _className.c_str(), _className.c_str());

	fprintf(fp,
		"{\n");

	fprintf(fp,
		"   ConfigFile::logData(level, _dataMap, %d, writeStatus);\n",
		_parameter.size());

	fprintf(fp, 
		"}\n\n");
}


void ConfigFile::generateParameterValidateFunctions(FILE * fp, const char * outputName)
{
	for ( unsigned int paramIdx=0; paramIdx<_parameter.size(); paramIdx++ )
	{
		if ( _parameter[paramIdx]->index() > 0 ) continue;

		const string func = _parameter[paramIdx]->validateFunction();
		if ( !func.empty() ||
			  _parameter[paramIdx]->getType() == Parameter::TEnum )
		{
			fprintf(fp, 
				"bool %s::_C_%s::_C_%s::_C_%s::validate(void * valuePtr)\n"
				"{\n",
				outputName, _className.c_str(), _parameter[paramIdx]->sectionName().c_str(), _parameter[paramIdx]->variableName().c_str());

			if ( _parameter[paramIdx]->getType() == Parameter::TEnum )
			{
				fprintf(fp,
					"  if ( !%s::validate%s(valuePtr) ) return false;\n",
					outputName, _parameter[paramIdx]->parameterName().c_str());
			}

			if ( !func.empty() )
			{
				_parameter[paramIdx]->generateValidateFunction(fp);
			}
			else
			{
				fprintf(fp,
					"  return true;\n");
			}

			fprintf(fp,
			   "\n"
			   "}\n\n");
		}
	}
}

void ConfigFile::generateParameterRangeFunctions(FILE * fp, const char * outputName)
{
	for ( unsigned int paramIdx=0; paramIdx<_parameter.size(); paramIdx++ )
	{
		if ( _parameter[paramIdx]->index() > 0 ) continue;

		if ( _parameter[paramIdx]->hasRangeFunc() )
		{
			fprintf(fp, 
				"bool %s::_C_%s::_C_%s::_C_%s::getRange(%s & min, %s & max)\n"
				"{\n",
				outputName, _className.c_str(), _parameter[paramIdx]->sectionName().c_str(), _parameter[paramIdx]->variableName().c_str(),
				_parameter[paramIdx]->getDeclarationString().c_str(), _parameter[paramIdx]->getDeclarationString().c_str());

			_parameter[paramIdx]->generateRangeFunction(fp);

			fprintf(fp,
			   "\n"
			   "}\n\n");
		}
	}
}

void ConfigFile::generateParameterSetFunctions(FILE * fp, const char * outputName)
{
	if ( !_readWrite ) return;

	for ( unsigned int paramIdx=0; paramIdx<_parameter.size(); paramIdx++ )
	{
		if ( _parameter[paramIdx]->index() > 0 ) continue;
		if ( _parameter[paramIdx]->isConst() ) continue;

		fprintf(fp, 
			"bool %s::_C_%s::_C_%s::_C_%s::set(%s value)\n"
			"{\n",
			outputName, _className.c_str(), _parameter[paramIdx]->sectionName().c_str(),
			_parameter[paramIdx]->variableName().c_str(), _parameter[paramIdx]->getDeclarationString().c_str());

		_parameter[paramIdx]->generateSetFunction(fp);
		fprintf(fp,
		   "\n"
		   "}\n\n");
	}
}

void ConfigFile::generateDataFile(const char * dataFileDir)
{
	string fileName;

	if ( _hasBackup )
	{
		if ( _defaultFileName.empty() )
		{
			return;
		}

		fileName = dataFileDir + _defaultFileName;
   }
	else
	{
		fileName = dataFileDir + _dataFileName;
	}

	createDirectoryChain(fileName);

	FILE * fp = fopen(fileName.c_str(), "w");
	if ( !fp )
	{
		perror(fileName.c_str());
		exit(1);
	}

	fprintf(fp,
		"# Data file generated from \"%s\"\n"
		"#\n",
		_fileName.c_str());

	unsigned int sectIdx;
	unsigned int paramIdx;
	for ( sectIdx=0; sectIdx<_section.size(); sectIdx++ )
	{
		fprintf(fp,
			"\n"
			"[%s]\n",
			_section[sectIdx]->name().c_str());

		for ( paramIdx=0; paramIdx<_parameter.size(); paramIdx++ )
		{
			if ( _section[sectIdx]->name() == _parameter[paramIdx]->sectionName() )
			{
				fprintf(fp,
					"%s%s=",
					_parameter[paramIdx]->variableName().c_str(), _parameter[paramIdx]->indexString().c_str());

				if ( _parameter[paramIdx]->getType() == Parameter::TString )
				{
					string value = _parameter[paramIdx]->value();

					fputc('"', fp);
					for ( unsigned int i=0; i<value.size(); i++ )
					{
						if ( value[i] == '"' ) fputc('\\', fp);
						fputc(value[i], fp);
					}

					fprintf(fp, "\"\n");
				}
				else
				{
					fprintf(fp,
						"%s\n",
						_parameter[paramIdx]->value().c_str());
				}
		   }
		}
   }
}
