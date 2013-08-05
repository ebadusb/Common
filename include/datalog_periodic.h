/*
 * Copyright (C) 2002 Gambro BCT, Inc.  All rights reserved.
 *
 * $Header$
 * $Log: datalog_periodic.h $
 * Revision 1.2  2004/01/26 18:51:28Z  jl11312
 * - modifications for building under Tornado 2.2
 * Revision 1.1  2003/02/25 16:11:37Z  jl11312
 * Initial revision
 * Revision 1.3  2003/01/21 20:53:55  ms10234
 * Removed const version of periodic string member funcs because the compiler is too stupid to see const and non-const as different.
 * Revision 1.2  2002/11/14 15:53:13Z  jl11312
 * - added templates for periodic logging through member functions
 * Revision 1.1  2002/08/15 21:03:18  jl11312
 * Initial revision
 *
 */

#ifndef _DATALOG_PERIODIC_INCLUDE
#define _DATALOG_PERIODIC_INCLUDE

#include <stddef.h>
#include "datalog_port.h"

#ifdef __cplusplus

class DataLog_PeriodicItemBase
{
public:
   DataLog_PeriodicItemBase(DataLog_SetHandle set, size_t size, const char * key, const char * description, const char * format);

   virtual ~DataLog_PeriodicItemBase();
   virtual void updateItem(void) = 0;
   virtual bool itemChanged(void);

	size_t currentSize(void) { return _size; }
	size_t currentAllocSize(void) { return _allocSize; }
	size_t getData(void * buffer, size_t maxSize);

	DataLog_InternalID getKeyCode(void) { return _keyCode; }

protected:
	void setItemData(size_t size, const void * data);
	void setItemString(const char * str);

   enum { BUFFER_SIZE_INC = 32 };
   void * _data;         	// current value
   void * _oldData;      	// previous value

   size_t _size;         	// current size (in bytes)
   size_t _oldSize;      	// previous size (in bytes)

   size_t _allocSize;    	// allocated size of _data
   size_t _oldAllocSize; 	// allocated size of _oldData

private:
	void copyItemData(size_t size, const void * data);

   DataLog_InternalID   _keyCode;

   DataLog_SetHandle _set;
   DataLog_Lock		_lock;
};

//
// Periodic logging support for simple reference types
//
template<class Value> class DataLog_PeriodicItemRef : public DataLog_PeriodicItemBase
{
public:
	DataLog_PeriodicItemRef(DataLog_SetHandle set, const Value& ref, const char * key, const char * description, const char * format)
		: DataLog_PeriodicItemBase(set, sizeof(Value), key, description, format), _ref(ref) { }

	virtual void updateItem(void) { setItemData(sizeof(_ref), (const void *)&_ref); }

private: 
	const Value & _ref;
};

template<class Value> inline DataLog_Result datalog_AddRef(DataLog_SetHandle handle, const Value& ref, const char * key, const char * description, const char * format)
{
   DataLog_PeriodicItemRef<Value> * item = new DataLog_PeriodicItemRef<Value>(handle, ref, key, description, format);
   return DataLog_OK;
}

//
// Periodic logging support for simple function types
//
template<class Value> class DataLog_PeriodicItemFunc : public DataLog_PeriodicItemBase
{
public:
	DataLog_PeriodicItemFunc(DataLog_SetHandle set, Value (* func)(void), const char * key, const char * description, const char * format)
		: DataLog_PeriodicItemBase(set, sizeof(Value), key, description, format), _func(func) { }

	virtual void updateItem(void) { Value data = (*_func)(); setItemData(sizeof(data), (const void *)&data); }

private:
	Value (* _func)(void);
};

template<class Value> inline DataLog_Result datalog_AddFunc(DataLog_SetHandle handle, Value (* func)(void), const char * key, const char * description, const char * format)
{
   DataLog_PeriodicItemFunc<Value> * item = new DataLog_PeriodicItemFunc<Value>(handle, func, key, description, format);
   return DataLog_OK;
}

template<class Value, class Arg> class DataLog_PeriodicItemArgFunc : public DataLog_PeriodicItemBase
{
public:
	DataLog_PeriodicItemArgFunc(DataLog_SetHandle set, Value (* func)(const Arg&), const Arg& arg, const char * key, const char * description, const char * format)
		: DataLog_PeriodicItemBase(set, sizeof(Value), key, description, format), _func(func), _arg(arg) { }

	virtual void updateItem(void) { Value data = (*_func)(_arg); setItemData(sizeof(data), (const void *)&data); }

private:
	Value (* _func)(const Arg&);
	const Arg& _arg;
};

template<class Value, class Arg> inline DataLog_Result datalog_AddArgFunc(DataLog_SetHandle handle, Value (* func)(const Arg&), const Arg& arg, const char * key, const char * description, const char * format)
{
   DataLog_PeriodicItemArgFunc<Value, Arg> * item = new DataLog_PeriodicItemArgFunc<Value, Arg>(handle, func, arg, key, description, format);
   return DataLog_OK;
}

//
// Periodic logging support for member functions
//
template<class Value, class T> class DataLog_PeriodicItemMemberFunc : public DataLog_PeriodicItemBase
{
public:
	DataLog_PeriodicItemMemberFunc(DataLog_SetHandle set, const T * obj, Value (T::* func)(void), const char * key, const char * description, const char * format)
		: DataLog_PeriodicItemBase(set, sizeof(Value), key, description, format), _obj(obj), _func(func) { }

	DataLog_PeriodicItemMemberFunc(DataLog_SetHandle set, const T * obj, Value (T::* func)(void) const, const char * key, const char * description, const char * format)
		: DataLog_PeriodicItemBase(set, sizeof(Value), key, description, format), _obj(obj), _func(func) { }

	virtual void updateItem(void) { Value data = (_obj->*_func)(); setItemData(sizeof(data), (const void *)&data); }

private:
	const T * _obj;
	Value (T::* _func)(void) const;
};

template<class Value, class T> inline DataLog_Result datalog_AddMemberFunc(DataLog_SetHandle handle, const T * obj, Value (T::* func)(void), const char * key, const char * description, const char * format)
{
   DataLog_PeriodicItemMemberFunc<Value, T> * item = new DataLog_PeriodicItemMemberFunc<Value, T>(handle, obj, func, key, description, format);
   return DataLog_OK;
}

template<class Value, class T> inline DataLog_Result datalog_AddMemberFunc(DataLog_SetHandle handle, const T * obj, Value (T::* func)(void) const, const char * key, const char * description, const char * format)
{
   DataLog_PeriodicItemMemberFunc<Value, T> * item = new DataLog_PeriodicItemMemberFunc<Value, T>(handle, obj, func, key, description, format);
   return DataLog_OK;
}

//
// Periodic logging support for char pointer references
//
typedef char * DataLog_CharPtr;
typedef const char * DataLog_ConstCharPtr;

class DataLog_PeriodicItemStringRef : public DataLog_PeriodicItemBase
{
public:
	DataLog_PeriodicItemStringRef(DataLog_SetHandle set, const DataLog_CharPtr& ref, const char * key, const char * description, const char * format)
		: DataLog_PeriodicItemBase(set, DataLog_PeriodicItemBase::BUFFER_SIZE_INC, key, description, format), _ref(ref) { }

	DataLog_PeriodicItemStringRef(DataLog_SetHandle set, const DataLog_ConstCharPtr& ref, const char * key, const char * description, const char * format)
		: DataLog_PeriodicItemBase(set, DataLog_PeriodicItemBase::BUFFER_SIZE_INC, key, description, format), _ref(ref) { }

	virtual void updateItem(void)	{ setItemString(_ref); }

private:
	const DataLog_ConstCharPtr& _ref;
};

template<> inline DataLog_Result datalog_AddRef<DataLog_CharPtr>(DataLog_SetHandle handle, const DataLog_CharPtr& ref, const char * key, const char * description, const char * format)
{
   DataLog_PeriodicItemStringRef * item = new DataLog_PeriodicItemStringRef(handle, ref, key, description, format);
   return DataLog_OK;
}

template<> inline DataLog_Result datalog_AddRef<DataLog_ConstCharPtr>(DataLog_SetHandle handle, const DataLog_ConstCharPtr& ref, const char * key, const char * description, const char * format)
{
   DataLog_PeriodicItemStringRef * item = new DataLog_PeriodicItemStringRef(handle, ref, key, description, format);
   return DataLog_OK;
}

//
// Periodic logging support for char pointer functions
//
class DataLog_PeriodicItemStringFunc : public DataLog_PeriodicItemBase
{
public:
	DataLog_PeriodicItemStringFunc(DataLog_SetHandle set, DataLog_CharPtr (* func)(void), const char * key, const char * description, const char * format)
		: DataLog_PeriodicItemBase(set, DataLog_PeriodicItemBase::BUFFER_SIZE_INC, key, description, format), _func((DataLog_ConstCharPtr (*)(void))func) { }

	DataLog_PeriodicItemStringFunc(DataLog_SetHandle set, DataLog_ConstCharPtr (* func)(void), const char * key, const char * description, const char * format)
		: DataLog_PeriodicItemBase(set, DataLog_PeriodicItemBase::BUFFER_SIZE_INC, key, description, format), _func(func) { }

	virtual void updateItem(void) { DataLog_ConstCharPtr data = (*_func)(); setItemString(data); }

private:
	DataLog_ConstCharPtr (* _func)(void);
};

inline DataLog_Result datalog_AddFunc(DataLog_SetHandle handle, DataLog_CharPtr (* func)(void), const char * key, const char * description, const char * format)
{
   DataLog_PeriodicItemStringFunc * item = new DataLog_PeriodicItemStringFunc(handle, func, key, description, format);
   return DataLog_OK;
}

inline DataLog_Result datalog_AddFunc(DataLog_SetHandle handle, DataLog_ConstCharPtr (* func)(void), const char * key, const char * description, const char * format)
{
   DataLog_PeriodicItemStringFunc * item = new DataLog_PeriodicItemStringFunc(handle, func, key, description, format);
   return DataLog_OK;
}

template <class Arg> class DataLog_PeriodicItemStringArgFunc : public DataLog_PeriodicItemBase
{
public:
	DataLog_PeriodicItemStringArgFunc(DataLog_SetHandle set, DataLog_CharPtr (* func)(const Arg&), const Arg& arg, const char * key, const char * description, const char * format)
		: DataLog_PeriodicItemBase(set, DataLog_PeriodicItemBase::BUFFER_SIZE_INC, key, description, format), _func(func), _arg(arg) { }

	DataLog_PeriodicItemStringArgFunc(DataLog_SetHandle set, DataLog_ConstCharPtr (* func)(const Arg&), const Arg& arg, const char * key, const char * description, const char * format)
		: DataLog_PeriodicItemBase(set, DataLog_PeriodicItemBase::BUFFER_SIZE_INC, key, description, format), _func(func), _arg(arg) { }

	virtual void updateItem(void) { DataLog_ConstCharPtr data = (*_func)(_arg); setItemString(data); }

private:
	DataLog_ConstCharPtr (* _func)(const Arg&);
	const Arg& _arg;
};

template<class Arg> inline DataLog_Result datalog_AddArgFunc(DataLog_SetHandle handle, DataLog_CharPtr (* func)(const Arg&), const Arg& arg, const char * key, const char * description, const char * format)
{
   DataLog_PeriodicItemStringArgFunc<Arg> * item = new DataLog_PeriodicItemStringArgFunc<Arg>(handle, func, arg, key, description, format);
   return DataLog_OK;
}

template<class Arg> inline DataLog_Result datalog_AddArgFunc(DataLog_SetHandle handle, DataLog_ConstCharPtr (* func)(const Arg& arg), const Arg& arg, const char * key, const char * description, const char * format)
{
   DataLog_PeriodicItemStringArgFunc<Arg> * item = new DataLog_PeriodicItemStringArgFunc<Arg>(handle, func, arg, key, description, format);
   return DataLog_OK;
}

//
// Periodic logging support for char pointer member functions
//
template <class T> class DataLog_PeriodicItemStringMemberFunc : public DataLog_PeriodicItemBase
{
public:
	DataLog_PeriodicItemStringMemberFunc(DataLog_SetHandle set, const T * obj, DataLog_CharPtr (T::* func)(void) const, const char * key, const char * description, const char * format)
		: DataLog_PeriodicItemBase(set, DataLog_PeriodicItemBase::BUFFER_SIZE_INC, key, description, format), _obj(obj), _func(func) { }

	DataLog_PeriodicItemStringMemberFunc(DataLog_SetHandle set, const T * obj, DataLog_ConstCharPtr (T::* func)(void) const, const char * key, const char * description, const char * format)
		: DataLog_PeriodicItemBase(set, DataLog_PeriodicItemBase::BUFFER_SIZE_INC, key, description, format), _obj(obj), _func(func) { }

	virtual void updateItem(void) { DataLog_ConstCharPtr data = (_obj->*_func)(); setItemString(data); }

private:
	const T * _obj;
	DataLog_ConstCharPtr (T::* _func)(void) const;
};

template<class T> inline DataLog_Result datalog_AddMemberFunc(DataLog_SetHandle handle, const T * obj, DataLog_CharPtr (T::* func)(void), const char * key, const char * description, const char * format)
{
   DataLog_PeriodicItemStringMemberFunc<T> * item = new DataLog_PeriodicItemStringMemberFunc<T>(handle, obj, func, key, description, format);
   return DataLog_OK;
}

template<class T> inline DataLog_Result datalog_AddMemberFunc(DataLog_SetHandle handle, const T * obj, DataLog_ConstCharPtr (T::* func)(void), const char * key, const char * description, const char * format)
{
   DataLog_PeriodicItemStringMemberFunc<T> * item = new DataLog_PeriodicItemStringMemberFunc<T>(handle, obj, func, key, description, format);
   return DataLog_OK;
}

template<class T> inline DataLog_Result datalog_AddMemberFunc(DataLog_SetHandle handle, const T * obj, DataLog_CharPtr (T::* func)(void) const, const char * key, const char * description, const char * format)
{
   DataLog_PeriodicItemStringMemberFunc<T> * item = new DataLog_PeriodicItemStringMemberFunc<T>(handle, obj, func, key, description, format);
   return DataLog_OK;
}

template<class T> inline DataLog_Result datalog_AddMemberFunc(DataLog_SetHandle handle, const T * obj, DataLog_ConstCharPtr (T::* func)(void) const, const char * key, const char * description, const char * format)
{
   DataLog_PeriodicItemStringMemberFunc<T> * item = new DataLog_PeriodicItemStringMemberFunc<T>(handle, obj, func, key, description, format);
   return DataLog_OK;
}

#endif /* ifdef __cplusplus */

#endif /* ifndef _DATALOG_PERIODIC_INCLUDE */

