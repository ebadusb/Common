// $Header: K:/BCT_Development/vxWorks/Common/tools/config_parse/rcs/parameter.cpp 1.2 2005/05/11 15:15:09Z jl11312 Exp jl11312 $
//
// Configuration file parameter classes
//
// $Log: parameter.cpp $
// Revision 1.2  2005/05/11 15:15:09Z  jl11312
// - corrected archive types
// Revision 1.3  2005/01/20 15:13:34Z  jl11312
// - added suport for export file for shared enumeration types
// Revision 1.2  2005/01/18 15:43:38Z  jl11312
// - implemented array support
// - implemented {@@ const@@} option
// Revision 1.1  2004/07/16 13:56:55Z  jl11312
// Initial revision
//

#include "stdafx.h"

#pragma warning(disable:4786)
#include "parameter.h"

Parameter::Parameter(const string & section, const string & name, const string & value)
	: _section(section), _stringValue(value), _const(false)
{
	if ( name[name.size()-1] == ']' )
	{
		// Indexed parameter
		int startIndex = name.size()-1;
		while ( startIndex > 0 && name[startIndex] != '[' )
		{
			startIndex -= 1;
		}

		_parameterName = name.substr(0, startIndex);
		_index = atoi(name.substr(startIndex+1, name.size()-startIndex-2).c_str());

		char indexStr[20];  
		sprintf(indexStr, "[%d]", _index);
		_indexString = indexStr;
	}
	else
	{
		_indexString = "";
		_index = -1;
		_parameterName = name;
	}

	_variableName = _parameterName;
}

LongParameter::LongParameter(const string & section, const string & name, const string & stringValue, long value)
	: Parameter(section, name, stringValue), _value(value)
{
}

DoubleParameter::DoubleParameter(const string & section, const string & name, const string & stringValue, double value)
	: Parameter(section, name, stringValue), _value(value)
{
}

StringParameter::StringParameter(const string & section, const string & name, const string & value)
	: Parameter(section, name, value)
{
}

BoolParameter::BoolParameter(const string & section, const string & name, const string & stringValue, bool value)
	: Parameter(section, name, stringValue), _value(value)
{
}

EnumParameter::EnumParameter(const string & section, const string & name, const string & value)
	: Parameter(section, name, value)
{
}

string Parameter::getTypeName(void)
{
	string typeString = "bad-type";
	switch ( getType() )
	{
	case Parameter::TLong: typeString = "TLong"; break;
	case Parameter::TDouble: typeString = "TDouble"; break;
	case Parameter::TString: typeString = "TString"; break;
	case Parameter::TBool: typeString = "TBool"; break;
	case Parameter::TEnum: typeString = "TEnum"; break;
	}

	return typeString;
}

string Parameter::getDeclarationString(void)
{
	string typeString = "bad-type";
	switch ( getType() )
	{
		case Parameter::TLong: typeString = "long"; break;
		case Parameter::TDouble: typeString = "double"; break;
		case Parameter::TString: typeString = "const char *"; break;
		case Parameter::TBool: typeString = "bool"; break;
		case Parameter::TEnum: typeString = "T" + _parameterName; break;
	}

	return typeString;
}

string Parameter::getPtrDeclarationString(void)
{
	string typeString = "bad-type";
	switch ( getType() )
	{
		case Parameter::TLong: typeString = "long *"; break;
		case Parameter::TDouble: typeString = "double *"; break;
		case Parameter::TString: typeString = "const char **"; break;
		case Parameter::TBool: typeString = "bool *"; break;
		case Parameter::TEnum: typeString = "T" + _parameterName + " *"; break;
	}

	return typeString;
}

void Parameter::generateAccessClassHeader(FILE * fp, bool readWrite, int arraySize)
{
	string declarationString = getDeclarationString();

	fprintf(fp,
		"      class _C_%s\n"
		"      {\n"
		"      public:\n"
		"        const char * name(void) { return _name[_mapIdx]; }\n",
		_variableName.c_str());

	if ( !_validateFunc.empty() ||
		  getType() == TEnum )
	{
		fprintf(fp,
			"        static bool validate(void * valuePtr);\n");
	}

	if ( getType() == TLong ||
		  getType() == TDouble )
	{
		if ( !_rangeFunc.empty() )
		{
			fprintf(fp,
				"        static bool getRange(%s & min, %s & max);\n",
				declarationString.c_str(), declarationString.c_str());
		}
		else if ( !_min.empty() && !_max.empty() )
		{
			fprintf(fp,
				"        static bool getRange(%s & min, %s & max) { min=%s; max=%s; return true; }\n",
				declarationString.c_str(), declarationString.c_str(), _min.c_str(), _max.c_str());
	   }
		else
		{
			fprintf(fp,
				"        static bool getRange(%s & min, %s & max) { return false; }\n",
				declarationString.c_str(), declarationString.c_str());
		}
	}

	if ( readWrite )
	{
		fprintf(fp,
			"        void initialize(const DataMap * dataMap, int mapIdx) { _dataMap = dataMap; _mapIdx = mapIdx; }\n");

		if ( !_const )
		{
			fprintf(fp,
				"        bool set(%s value);\n",
				declarationString.c_str());
		}

		fprintf(fp,
		   "      private:\n"
		   "        const DataMap * _dataMap;\n"
		   "        int _mapIdx;\n");
	}
	else
	{
		fprintf(fp,
			"        void initialize(int mapIdx) { _mapIdx = mapIdx; }\n"
		   "      private:\n"
		   "        int _mapIdx;\n");
	}

	fprintf(fp,
		"      };\n\n");
}

void Parameter::generateValidateFunction(FILE * fp)
{
	switch ( getType() )
	{
	case TLong:
		fprintf(fp, "  long value = *(long *)valuePtr;\n");
		break;

	case TDouble:
		fprintf(fp, "  double value = *(double *)valuePtr;\n");
		break;

	case TEnum:
		fprintf(fp, "  T%s value = *(T%s *)valuePtr;\n", _parameterName.c_str(), _parameterName.c_str());
		break;

	case TBool:
		fprintf(fp, "  bool value = *(bool *)valuePtr;\n");
		break;

	case TString:
		fprintf(fp, "  const char * value = *(const char **)valuePtr;\n");
		break;
	}

	fprintf(fp, "%s\n", _validateFunc.c_str());
}

void Parameter::generateRangeFunction(FILE * fp)
{
	fprintf(fp, "%s\n", _rangeFunc.c_str());
}

void Parameter::generateSetFunction(FILE * fp)
{
	fprintf(fp, 
		"  bool setOK = true;\n");

	fprintf(fp,
		"  %s valuePtr = (%s)_dataMap[_mapIdx]._value;\n",
	   getPtrDeclarationString().c_str(), getPtrDeclarationString().c_str());

	if ( hasRange() )
	{
		fprintf(fp,
			"  if ( value < %s || value > %s ) setOK = false;\n\n",
			_min.c_str(), _max.c_str());
	}
	else if ( hasRangeFunc() )
	{
		fprintf(fp,
			"  %s min, max;\n"
			"  if ( getRange(min, max) &&\n"
			"       ( value < min || value > max )) setOK = false;\n\n",
			getDeclarationString().c_str());
	}

	if ( !_validateFunc.empty() || getType() == TEnum )
	{
		fprintf(fp,
			"  if ( !validate((void *)&value) ) setOK = false;\n\n");
	}

	if ( getType() == TString )
	{
		fprintf(fp,
			"  if ( setOK )\n"
			"  {\n"
			"    if ( *valuePtr ) delete[] *valuePtr;\n"
			"    *valuePtr = new char[strlen(value)+1];\n"
			"    strcpy(*(char **)valuePtr, value);\n"
			"  }\n");
	}
	else
	{
		fprintf(fp, 
			"  if ( setOK ) *valuePtr = value;\n");
   }

	fprintf(fp,
		"  return setOK;\n");
}

