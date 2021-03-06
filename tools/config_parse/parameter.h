// $Header$
//
// Configuration file parameter classes
//
// $Log: parameter.h $
// Revision 1.2  2005/05/11 15:15:08Z  jl11312
// - corrected archive types
// Revision 1.3  2005/01/20 15:13:15Z  jl11312
// - added suport for export file for shared enumeration types
// Revision 1.2  2005/01/18 15:12:36Z  jl11312
// - implemented array support
// - implemented {@@ const@@} option
// Revision 1.1  2004/07/09 18:55:26Z  jl11312
// Initial revision
//

#ifndef _PARAMETER_INCLUDE
#define _PARAMETER_INCLUDE

#include <string>
using namespace std;

class Parameter
{
public:
	Parameter(const string & section, const string & name, const string & value);

	enum Type { TLong, TDouble, TString, TBool, TEnum };
	virtual Type getType(void) = 0;

	virtual string getTypeName(void);
	virtual string getDeclarationString(void);
	virtual string getPtrDeclarationString(void);

   const string & sectionName(void) { return _section; }
	const string & variableName(void) { return _variableName; }
	const string & parameterName(void) { return _parameterName; }
	const string & value(void) { return _stringValue; }

	const string & indexString(void) { return _indexString; }
	int index(void) { return _index; }

	const string & validateFunction(void) { return _validateFunc; }
	void setValidateFunction(const string & func) { _validateFunc = func; }

	bool isConst(void) { return _const; }
   bool isOverridden(void) { return _overrideUserConfig; }
	void setConst(void) { _const = true; }
   void setOverridden(void) { _overrideUserConfig = true; }

	bool hasRange(void) { return ( !_min.empty() && !_max.empty() ); }
	bool hasRangeFunc(void) { return ( !_rangeFunc.empty() ); }

	void setRange(string min, string max) { _min=min; _max=max; }
	void setRangeFunc(const string & func) { _rangeFunc = func; }

	virtual void setVariableName(const string & name) { _variableName = name; }

	void generateAccessClassHeader(FILE * fp, bool readWrite, int arraySize);
	void generateValidateFunction(FILE * fp);
	void generateRangeFunction(FILE * fp);
	void generateSetFunction(FILE * fp);

	union Value
	{
		long _lValue;
		double _dValue;
		const char * _sValue;
		bool _bValue;
	};

protected:
	string _section;
	string _parameterName;	// name of parameter
	string _variableName;  	// variable name to be used (usually the same as _parameterName)  
	string _stringValue;

	string _indexString;
	int    _index;

 	string _min;
	string _max;

	bool   _const;

	string _rangeFunc;
	string _validateFunc;
   bool  _overrideUserConfig;
};

class LongParameter : public Parameter
{
public:
	LongParameter(const string & section, const string & name, const string & stringValue, long value);
	virtual Type getType(void) { return TLong; }
   void setValue(long value, const string& stringValue){ _value = value; _stringValue = stringValue; }
protected:
	long _value;
};

class DoubleParameter : public Parameter
{
public:
	DoubleParameter(const string & section, const string & name, const string & stringValue, double value);
	virtual Type getType(void) { return TDouble; }
   virtual void setValue(double value, const string& stringValue){ _value = value; _stringValue = stringValue; }
protected:
	double _value;
};

class StringParameter : public Parameter
{
public:
	StringParameter(const string & section, const string & name, const string & value);
	virtual Type getType(void) { return TString; }
   void setValue(string stringValue){ _stringValue = stringValue; }
};

class BoolParameter : public Parameter
{
public:
   BoolParameter(const string & section, const string & name, const string & stringValue, bool value);
   virtual Type getType(void) { return TBool; }
   void setValue(bool value, const string& stringValue){ _value = value; _stringValue = stringValue;}

protected:
	bool _value;
};

class EnumParameter : public Parameter
{
public:
	EnumParameter(const string & section, const string & name, const string & value);
	virtual Type getType(void) { return TEnum; }
   void setValue(const string& stringValue){ _stringValue = stringValue; }
};

#endif /* ifndef _PARAMETER_INCLUDE */

