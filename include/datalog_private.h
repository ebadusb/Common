/*
 * Copyright (C) 2002 Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: //bctquad3/home/BCT_Development/vxWorks/Common/include/rcs/datalog_private.h 1.2 2002/11/14 15:53:13Z jl11312 Exp ms10234 $
 * $Log: datalog_private.h $
 *
 */

#ifndef _DATALOG_PRIVATE_INCLUDE
#define _DATALOG_PRIVATE_INCLUDE

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
template<class Value, class Arg> class DataLog_PeriodicItemFunc : public DataLog_PeriodicItemBase
{
public:
	DataLog_PeriodicItemFunc(DataLog_SetHandle set, Value (* func)(const Arg&), const Arg& arg, const char * key, const char * description, const char * format)
		: DataLog_PeriodicItemBase(set, sizeof(Value), key, description, format), _func(func), _arg(arg) { }

	virtual void updateItem(void) { Value data = (*_func)(_arg); setItemData(sizeof(data), (const void *)&data); }

private:
	Value (* _func)(const Arg&);
	const Arg& _arg;
};

template<class Value, class Arg> inline DataLog_Result datalog_AddFunc(DataLog_SetHandle handle, Value (* func)(const Arg&), const Arg& arg, const char * key, const char * description, const char * format)
{
   DataLog_PeriodicItemFunc<Value, Arg> * item = new DataLog_PeriodicItemFunc<Value, Arg>(handle, func, arg, key, description, format);
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
template <class Arg> class DataLog_PeriodicItemStringFunc : public DataLog_PeriodicItemBase
{
public:
	DataLog_PeriodicItemStringFunc(DataLog_SetHandle set, DataLog_CharPtr (* func)(const Arg&), const Arg& arg, const char * key, const char * description, const char * format)
		: DataLog_PeriodicItemBase(set, DataLog_PeriodicItemBase::BUFFER_SIZE_INC, key, description, format), _func(func), _arg(arg) { }

	DataLog_PeriodicItemStringFunc(DataLog_SetHandle set, DataLog_ConstCharPtr (* func)(const Arg&), const Arg& arg, const char * key, const char * description, const char * format)
		: DataLog_PeriodicItemBase(set, DataLog_PeriodicItemBase::BUFFER_SIZE_INC, key, description, format), _func(func), _arg(arg) { }

	virtual void updateItem(void) { DataLog_ConstCharPtr data = (*_func)(_arg); setItemString(data); }

private:
	DataLog_ConstCharPtr (* _func)(const Arg&);
	const Arg& _arg;
};

template<class Arg> inline DataLog_Result datalog_AddFunc(DataLog_SetHandle handle, DataLog_CharPtr (* func)(const Arg&), const Arg& arg, const char * key, const char * description, const char * format)
{
   DataLog_PeriodicItemStringFunc<Arg> * item = new DataLog_PeriodicItemStringFunc<Arg>(handle, func, arg, key, description, format);
   return DataLog_OK;
}

template<class Arg> inline DataLog_Result datalog_AddFunc(DataLog_SetHandle handle, DataLog_ConstCharPtr (* func)(const Arg& arg), const Arg& arg, const char * key, const char * description, const char * format)
{
   DataLog_PeriodicItemStringFunc<Arg> * item = new DataLog_PeriodicItemStringFunc<Arg>(handle, func, arg, key, description, format);
   return DataLog_OK;
}

#endif /* ifdef __cplusplus */

#endif /* ifndef _DATALOG_PRIVATE_INCLUDE */

