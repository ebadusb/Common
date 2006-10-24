/*
 * $Header: Z:/vxWorks/Common/include/rcs/datalog_reserved_stream.h 1.3 2006/10/23 20:29:48Z jmedusb Exp jmedusb $ 
 *
 *  This defined the DataLogReserved class constructor and stream
 *  manipulator template functions, as well as the dec/hex and
 *  precision stream manipulators needed for Reserved Message logging.
 *
 * 	This class simple forwards the Tag Name and Value to the DataLog_Stream class
 *  using the format specified in the DLOG Reserved Message format
 * 
 *  USAGE:
 *  DataLog_Reserved("MESSAGE_NAME", logLevel)
 *  << taggedItem("X", Y) << dec
 *  << taggedItem("Y", X) << hex
 *  ...
 *  << taggedItem("TAG_N", value) << precision(int)
 *  << endmsg;
 *
 * $Log: datalog_reserved_stream.h $
 *
 */

#ifndef _DATALOG_RESERVED_
#define _DATALOG_RESERVED_

#include "datalog_levels.h"
#include "datalog.h"

//This defines the function pointer that takes only one argument, the stream reference
class DataLogReserved;
typedef DataLogReserved & (* DataLogReservedManip)(DataLogReserved &);


class DataLogReserved
{
	public:
		template < class T >
			friend DataLogReserved & manipTaggedItem(DataLogReserved & stream, const char * tagName, T tagValue);
		//Manipulator for settting precision
		friend DataLogReserved & manipPrecision(DataLogReserved & stream, int tagValue);
		friend DataLogReserved & endmsg(DataLogReserved & stream);
		friend DataLogReserved & hex(DataLogReserved & stream);
		friend DataLogReserved & dec(DataLogReserved & stream);

        virtual ~DataLogReserved() {}
        //Keep track of the log level this way we can reference the parent stream
        DataLogReserved(const char * messageName, DataLog_Level & level) : _logLevel(level)
        {
            DataLog(level) << RESERVED_HEADER << (messageName == NULL ? DEFAULT_NAME : messageName);
		}

        //This is so we can use endmsg from from DataLog_Stream
        DataLogReserved & operator << (DataLogReservedManip func)
		{ 
			return (*func)(*this);
		}

		// This is used to specify that this is a reserved message
		static const char * RESERVED_HEADER;
		static const char * DEFAULT_NAME;
	private:
		DataLog_Level & _logLevel;
};

//This is the manipulator function that does the work
template < class T >
inline DataLogReserved & manipTaggedItem(DataLogReserved & stream, const char * tagName, T tagValue)
{
	DataLog(stream._logLevel) << tagName << tagValue;
	return stream;
}

//This is the manipulator function that does the work
inline DataLogReserved & manipPrecision(DataLogReserved & stream, int tagValue)
{
	DataLog(stream._logLevel).precision(tagValue);
	return stream;
}

inline DataLogReserved & endmsg(DataLogReserved & stream)
{
	DataLog(stream._logLevel) << endmsg;
	return stream;
}

//Hex precision for integral values
inline DataLogReserved & hex(DataLogReserved & stream)
{
	DataLog(stream._logLevel) << hex;
	return stream;
}

//Decimal precision for integral values
inline DataLogReserved & dec(DataLogReserved & stream)
{
	DataLog(stream._logLevel) << dec;
	return stream;
}

//Here S is used so as not to clash with the References to T in DataLogReserved
template < class S >
class DataLogReservedIManip
{
	S _value;
	const char * _name;
	//TaggedItem form for manipulator take a tag name and S tagValue
	DataLogReserved & (* _func)(DataLogReserved & stream, const char * name, S value);

public:
	//Pointer to Stream Manipulator Function for TaggedItem
	DataLogReservedIManip(	
		DataLogReserved & (* func)(DataLogReserved & stream, const char * name, S value),
		const char * name,			//Tag Name
		S value)					//Tag Value
	  : _func(func), _value(value), _name(name) { }

    template < class T >
		friend DataLogReserved & operator << (DataLogReserved stream, const DataLogReservedIManip<T> & manip);
	template < class T >
		friend DataLogReserved & manipTaggedItem(DataLogReserved & stream, const char * tagName, T tagValue);
};

//This is the manipulator class for the Precision manipulator
class DataLogReservedIManipP
{
	int _value;
    //Precision form for manipulator takes an S precision value
	DataLogReserved & (* _pFunc)(DataLogReserved & stream, int value);

public:
	//Pointer to Stream Manipulator Function for Precision
	DataLogReservedIManipP(	
		DataLogReserved & (* func)(DataLogReserved & stream, int value),
        int value)					//Tag Value
	  : _pFunc(func), _value(value) { }

	friend DataLogReserved & operator << (DataLogReserved stream, const DataLogReservedIManipP & manip);
	friend DataLogReserved & manipPrecision(DataLogReserved & stream, int tagValue);
};

//This calls the taggedItem manipulator
template < class S >
inline DataLogReservedIManip<S> taggedItem(const char * tagName, S tagValue)
{
	return DataLogReservedIManip<S>(manipTaggedItem, tagName, tagValue);
}

//This calls the precision manipulator manipulator
inline DataLogReservedIManipP setPrecision(int tagValue)
{
	return DataLogReservedIManipP(manipPrecision, tagValue);
}

template < class S >
inline DataLogReserved & operator << (DataLogReserved stream, const DataLogReservedIManip<S> & manip)
{
	return (manip._func)(stream, manip._name, manip._value);
}

inline DataLogReserved & operator << (DataLogReserved stream, const DataLogReservedIManipP & manip)
{
	return (manip._pFunc)(stream, manip._value);
}

#endif _DATALOG_RESERVED_
