/*
 * $Header: Z:/vxWorks/Common/include/rcs/datalog_reserved_stream.h 1.6 2006/11/30 17:42:24Z jmedusb Exp jmedusb $ 
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
 * Revision 1.3  2006/10/23 20:29:48Z  jmedusb
 * Added the Standard Header
 *
 */

#ifndef _DATALOG_RESERVED_
#define _DATALOG_RESERVED_

#include "datalog_levels.h"
#include "datalog.h"

//This defines the function pointer that takes only one argument, the stream reference
class DataLogRes;
typedef DataLogRes & (* DataLogReservedManip)(DataLogRes &);

#define DataLogReserved(name, level) DataLogRes(name, level, __FILE__, __LINE__)

class DataLogRes
{
	public:
		template < class T >
			friend DataLogRes & manipTaggedItem(DataLogRes & stream, const char * tagName, T tagValue);
		//Manipulator for settting precision
		friend DataLogRes & manipPrecision(DataLogRes & stream, int tagValue);
		friend DataLogRes & endmsg(DataLogRes & stream);
		friend DataLogRes & hex(DataLogRes & stream);
		friend DataLogRes & dec(DataLogRes & stream);

        virtual ~DataLogRes() {}
        //Keep track of the log level this way we can reference the parent stream
		//Passes the line and file info of the calling location
        DataLogRes(const char * messageName, DataLog_Level & level, const int file, const int line) : _logLevel(level)
        {
            level(file, line) << RESERVED_HEADER << (messageName == NULL ? DEFAULT_NAME : messageName);
		}

		//Constructor uses the line and file of this header file
		DataLogRes(const char * messageName, DataLog_Level & level) : _logLevel(level)
        {
            DataLogRes(messageName, level, __FILE__, __LINE__);
		}

        //This is so we can use endmsg from from DataLog_Stream
        DataLogRes & operator << (DataLogReservedManip func)
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
inline DataLogRes & manipTaggedItem(DataLogRes & stream, const char * tagName, T tagValue)
{
	DataLog(stream._logLevel) << tagName << tagValue;
	return stream;
}

//This is the manipulator function that does the work
inline DataLogRes & manipPrecision(DataLogRes & stream, int tagValue)
{
	DataLog(stream._logLevel).precision(tagValue);
	return stream;
}

inline DataLogRes & endmsg(DataLogRes & stream)
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
inline DataLogRes & dec(DataLogRes & stream)
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
	DataLogRes & (* _func)(DataLogRes & stream, const char * name, S value);

public:
	//Pointer to Stream Manipulator Function for TaggedItem
	DataLogReservedIManip(	
		DataLogRes & (* func)(DataLogRes & stream, const char * name, S value),
		const char * name,			//Tag Name
		S value)					//Tag Value
	  : _func(func), _value(value), _name(name) { }

    template < class T >
		friend DataLogRes & operator << (DataLogRes stream, const DataLogReservedIManip<T> & manip);
	template < class T >
		friend DataLogRes & manipTaggedItem(DataLogRes & stream, const char * tagName, T tagValue);
};

//This is the manipulator class for the Precision manipulator
class DataLogReservedIManipP
{
	int _value;
    //Precision form for manipulator takes an S precision value
	DataLogRes & (* _pFunc)(DataLogRes & stream, int value);

public:
	//Pointer to Stream Manipulator Function for Precision
	DataLogReservedIManipP(	
		DataLogRes & (* func)(DataLogRes & stream, int value),
        int value)					//Tag Value
	  : _pFunc(func), _value(value) { }

	friend DataLogRes & operator << (DataLogRes stream, const DataLogReservedIManipP & manip);
	friend DataLogRes & manipPrecision(DataLogRes & stream, int tagValue);
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
inline DataLogRes & operator << (DataLogRes stream, const DataLogReservedIManip<S> & manip)
{
	return (manip._func)(stream, manip._name, manip._value);
}

inline DataLogRes & operator << (DataLogRes stream, const DataLogReservedIManipP & manip)
{
	return (manip._pFunc)(stream, manip._value);
}

#endif _DATALOG_RESERVED_
