/*******************************************************************************
 *
 * Copyright (c) 2002 by Gambro BCT, Inc.  All rights reserved.
 *
 * TITLE:      datastore_private.h
 *             Common Data Store Interface
 *
 * AUTHOR:     Regis McGarry
 *
 * ABSTRACT:   This file defines the private template functions needed for 
 *             compilation.  This file should not be included on it's own, but
 *             only by datastore.h
 *
 * HISTORY:    $Log: datastore_private.h $
 * HISTORY:    Revision 1.24  2003/05/19 16:16:13Z  ms10234
 * HISTORY:    5816 - fixing misstyped name for Access function
 * HISTORY:    Revision 1.23  2003/04/11 22:15:48Z  td07711
 * HISTORY:    log SetSpoof and ClearSpoof
 * HISTORY:    Revision 1.22  2003/04/07 14:47:03Z  rm70006
 * HISTORY:    IT 5818.
 * HISTORY:    
 * HISTORY:    Fix log levels.
 * HISTORY:    Revision 1.21  2003/03/28 19:55:49Z  ms10234
 * HISTORY:    5948 - Changed code to make message synchronization between nodes more robust
 * HISTORY:    Revision 1.20  2002/11/18 18:29:28Z  jl11312
 * HISTORY:    - modifications to improve compile speed, reduce inline function sizes
 * HISTORY:    Revision 1.19  2002/11/07 00:11:53Z  td07711
 * HISTORY:    modified spoofer caching
 * HISTORY:    Revision 1.18  2002/11/06 15:42:48  rm70006
 * HISTORY:    Removed unnecessary new's.
 * HISTORY:    Removed some inline functions to relieve compiler problems.
 * HISTORY:    fixed bug in register call.
 * HISTORY:    Revision 1.16  2002/10/31 19:26:48Z  rm70006
 * HISTORY:    Changed internal stucture to use less symbols which improved datastore creation speed.
 * HISTORY:    Revision 1.15  2002/10/25 20:45:08Z  td07711
 * HISTORY:    support spoofer caching mechanism
 * HISTORY:    Revision 1.14  2002/10/18 23:15:19  td07711
 * HISTORY:    use CallbackBase class for spoofing callback
 * HISTORY:    Revision 1.13  2002/10/18 20:00:50  rm70006
 * HISTORY:    Add new cds type for proc.  this version allows dynamic roles.
 * HISTORY:    Revision 1.12  2002/09/25 16:04:32Z  rm70006
 * HISTORY:    Fixed bugs with fatal error check logging.
 * HISTORY:    Revision 1.11  2002/09/24 16:47:18Z  rm70006
 * HISTORY:    Add extra debugging ability.
 * HISTORY:    Revision 1.10  2002/09/19 16:05:18Z  rm70006
 * HISTORY:    Added fast get for large datastore items.
 * HISTORY:    Revision 1.9  2002/09/18 22:13:24Z  rm70006
 * HISTORY:    Change Get and Set to have built in locking.
 * HISTORY:    Revision 1.8  2002/09/13 20:09:05Z  rm70006
 * HISTORY:    Fix bug with lock/unlock.
 * HISTORY:    Revision 1.7  2002/08/30 15:26:30Z  rm70006
 * HISTORY:    Don't use set in register init routine.  
 * HISTORY:    Revision 1.6  2002/08/30 13:54:06Z  rm70006
 * HISTORY:    Fixed problem with datastore initialization.
 * HISTORY:    Revision 1.5  2002/08/29 18:08:23Z  rm70006
 * HISTORY:    fixed bug in RangeElement::Set routine.
 * HISTORY:    Revision 1.4  2002/08/23 14:53:51Z  rm70006
 * HISTORY:    changed binditem to work with 486 compiler bug
 * HISTORY:    Revision 1.3  2002/07/16 21:05:05Z  rm70006
 * HISTORY:    Fix bug in check for multiple writers.
 * HISTORY:    Revision 1.2  2002/06/24 20:37:26Z  rm70006
 * HISTORY:    Make max number of symbols 512 from 128.
 * HISTORY:    Revision 1.1  2002/06/24 19:30:42Z  rm70006
 * HISTORY:    Initial revision
 * HISTORY:    Revision 1.4  2002/06/17 18:40:44Z  rm70006
 * HISTORY:    Change design to make data store instance based instead of static.  Changed from taskVars to using the symbol table library.
 * HISTORY:    Revision 1.3  2002/06/04 18:52:54Z  rm70006
 * HISTORY:    Remove unneeded version of get
 * HISTORY:    Revision 1.2  2002/06/04 17:18:24Z  rm70006
 * HISTORY:    Added logging and new const ref get function.
 * HISTORY:    Revision 1.1  2002/05/28 19:00:25Z  rm70006
 * HISTORY:    Initial revision
*******************************************************************************/

#ifndef _DATASTORE_PRIVATE_INCLUDE
#define _DATASTORE_PRIVATE_INCLUDE

#include <a_out.h>
#include "error.h"

#include "datalog_levels.h"

////////////////////////////////////////////////////////////////
// BindItem
////////////////////////////////////////////////////////////////

template <class T> void BindItem(DataStore *ds, T **dataPtr, BIND_ITEM_TYPE item, bool &created)
{
	enum { SYM_NAME_SIZE = 200 };
   string nameKey(SYM_NAME_SIZE, ' ');

   SYM_TYPE symbolType;
   char *valPtr;

   ds->GetSymbolName (nameKey, item);

   // Search for Symbol.
   STATUS status = symFindByName(DataStore::getTable(), (char *)nameKey.c_str(), &valPtr, &symbolType);

   // If symbol exists
   if (status == OK)
   {
      // return a pointer to the value
      *dataPtr = (T *)valPtr;
      created = false;

      DataLog(log_level_cds_debug) << "Attaching item " << nameKey.c_str() << ", address " << dataPtr << ", " << *dataPtr << endmsg;
   }
   else
   {
      // create a new data element
      *dataPtr = new T; 

      // add to symbol table
      status = symAdd(DataStore::getTable(), (char *)nameKey.c_str(), (char *)*dataPtr, N_DATA, item);

      DataLog(log_level_cds_debug) << "Creating item " << nameKey.c_str() << ", address " << dataPtr << ", " << *dataPtr << endmsg;

      if (status == ERROR)
      {
         // Log Fatal Error
         DataLog_Critical _fatal;
         DataLog(_fatal) << "BindItem: symAdd failed in CDS " << ds->Name() << "." << endmsg;
         _FATAL_ERROR(__FILE__, __LINE__, "BindItem failed");
      }

      created = true;
   }
}



////////////////////////////////////////////////////////////////
// BaseElement
////////////////////////////////////////////////////////////////

template <class dataType> inline void BaseElement<dataType>::Access(AccessOp op) const
{
	if ( op == LockAccess )
	{
		_ds->Lock();
	}
	else
	{
		_ds->Unlock();
	}
}

template<> inline void BaseElement<bool>::Access(AccessOp op) const { }
template<> inline void BaseElement<char>::Access(AccessOp op) const { }
template<> inline void BaseElement<unsigned char>::Access(AccessOp op) const { }
template<> inline void BaseElement<short>::Access(AccessOp op) const { }
template<> inline void BaseElement<unsigned short>::Access(AccessOp op) const { }
template<> inline void BaseElement<int>::Access(AccessOp op) const { }
template<> inline void BaseElement<unsigned int>::Access(AccessOp op) const { }
template<> inline void BaseElement<long>::Access(AccessOp op) const { }
template<> inline void BaseElement<unsigned long>::Access(AccessOp op) const { }

//
// Default Constructor (no default value)
//
template <class dataType> BaseElement<dataType>::BaseElement() :
   ElementType(),
   _handle(0)
{
}



//
// Base Destructor
//
template <class dataType> BaseElement<dataType>::~BaseElement()
{
}



//
// Register method
//
template <class dataType> void BaseElement<dataType>::Register (DataStore *ds, PfrType pfr)
{
   bool created;

   ElementType::Register(ds, pfr);

   BindItem(ds, &_handle, ITEM_BASE_ELEMENT_SYMBOL_CONTAINER, created);

   if (created)
   {
      // Create the items in the symbol table entry
      CreateSymbolTableEntry();

      // Only add the element if PFR is desired and it didn't exist in the list (avoid duplicates).
      if (_pfrType == PFR_RECOVER)
      {
         _ds->AddElement(this);
      }
   }
}



//
// Register method
//
template <class dataType> void BaseElement<dataType>::Register (DataStore *ds, PfrType pfr, const dataType &initValue)
{
   bool created; 

   ElementType::Register(ds, pfr);

   BindItem(ds, &_handle, ITEM_BASE_ELEMENT_SYMBOL_CONTAINER, created);

   if (created)
   {
      // Create the items in the symbol table entry
      CreateSymbolTableEntry();

      // Only add the element if PFR is desired and it didn't exist in the list (avoid duplicates).
      if (_pfrType == PFR_RECOVER)
      {
         _ds->AddElement(this);
      }

      // First datastore initializes the element
      // TBD.  This may need to be semaphore protected.
      *_handle->_data = initValue;
      //Set(initValue);
   }
}



//
// Get method
//
template <class dataType> void BaseElement<dataType>::Get(dataType *item) const
{
   if (_ds == 0)
   {
      _FATAL_ERROR(__FILE__, __LINE__, "Element failed to register");
   }

   // If calling instance is spoofer or no spoof has been registered, return real value
   if( *_handle->_fp == NULL || _ds->GetRole() == ROLE_SPOOFER )
   {
       Access(LockAccess);
       *item = *_handle->_data;
       Access(UnlockAccess);
   }
   else // spoofer callback returns spoofed value
   {
       Access(LockAccess);
       pair< dataType*, const dataType* > toFrom( item, _handle->_data );
       (*(*_handle->_fp))( &toFrom );  // runs spoofer callback
       Access(UnlockAccess);
   }
}


//
// Get method
//
template <class dataType> dataType BaseElement<dataType>::Get() const
{

   if (_ds == 0)
   {
      _FATAL_ERROR(__FILE__, __LINE__, "Element failed to register");
   }

   // If calling instance is spoofer or no spoof has been registered, return real value
   if ( *_handle->_fp == NULL || _ds->GetRole() == ROLE_SPOOFER ) 
   {
      Access(LockAccess);
      dataType temp = *_handle->_data;
      Access(UnlockAccess);
      return temp;
   }
   else // spoofer callback returns spoofed value
   {
      dataType temp;
      pair< dataType*, const dataType* > toFrom( &temp, _handle->_data );

      Access(LockAccess);
      (*(*_handle->_fp))( &toFrom );  // runs spoofer callback
      Access(UnlockAccess);
      return temp;
   }
}

//
// Set method
//
template <class dataType> bool BaseElement<dataType>::Set(const dataType &data)
{
   if (_ds == 0)
   {
      _FATAL_ERROR(__FILE__, __LINE__, "Element failed to register");
   }

   if (_ds->GetRole() != ROLE_RO)
   {
      Access(LockAccess);
      _handle->_spooferCacheIsValid = false;
      *_handle->_data = data;
      Access(UnlockAccess);
   }
   else
   {
      // Log Fatal Error
      DataLog_Critical _fatal;
      DataLog(_fatal) << "BaseElement: Set Failed in CDS " << _ds->Name() << ".  Role is RO." << endmsg;
      _FATAL_ERROR(__FILE__, __LINE__, "BaseElement Set failed");
   }

   return true;
}




//
// ReadSelf method
//
template <class dataType> void BaseElement<dataType>::ReadSelf (ifstream &pfrfile)
{
   pfrfile.read(_handle->_data, sizeof(dataType));
   //pfrfile >> _data;
   
   //DataLog(_ds->*_debug) << "Reading value " << *_data << ", size " << sizeof(dataType) << endmsg;

   if (!pfrfile.good())
   {
      DataLog_Critical _fatal;
      DataLog(_fatal) << "ReadSelf failed in " << _ds->Name()
                              << ".  status is: " << pfrfile.rdstate() << endmsg;
      _FATAL_ERROR(__FILE__, __LINE__, "ReadSelf failed.");
   }
}
   


//
// WriteSelf method
//
template <class dataType> void BaseElement<dataType>::WriteSelf (ofstream &pfrfile)
{
   //DataLog(_ds->*_debug) << "Writing value " << *_data << ", size " << sizeof(dataType) << endmsg;

   pfrfile.write(_handle->_data, sizeof(dataType));
   //pfrfile << _data;

   if (!pfrfile.good())
   {
      DataLog_Critical _fatal;
      DataLog(_fatal) << "WriteSelf failed in " << _ds->Name()
                              << ".  status is: " << pfrfile.rdstate() << endmsg;
      _FATAL_ERROR(__FILE__, __LINE__, "WriteSelf failed.");
   }
}



//
// SetSpoof
//
template <class dataType> void BaseElement<dataType>::SetSpoof (const CallbackBase* fp)
{
    DataLog( log_level_cds_spoof_info ) << "SetSpoof: " << _ds->Name() << endmsg;
   
    _handle->_spooferCacheIsValid = false;
   *_handle->_fp  = fp;
}




//
// ClearSpoof
//
template <class dataType> void BaseElement<dataType>::ClearSpoof ()
{
    DataLog( log_level_cds_spoof_info ) << "ClearSpoof: " << _ds->Name() << endmsg;

   *_handle->_fp = NULL;
}



//
// CreateSymbolTableEntry
//
template <class dataType> void BaseElement<dataType>::CreateSymbolTableEntry()
{
   // Create the data element
   _handle->_data = new dataType;

   // Create the spoof fp 
   _handle->_fp = new (const CallbackBase *);
   *_handle->_fp = NULL;

   // Create the spoof cacheIsValid flag 
   _handle->_spooferCacheIsValid = false;
}



////////////////////////////////////////////////////////////////
// Ranged Element
////////////////////////////////////////////////////////////////

//
// Default Constructor
//
template <class dataType> RangedElement<dataType>::RangedElement()
{
}



//
// Base Destructor
//
template <class dataType> RangedElement<dataType>::~RangedElement()
{
}



//
// Register method
//
template <class dataType> void RangedElement<dataType>::Register (DataStore *ds, PfrType pfr, const dataType min, const dataType max)
{
   _min = min;
   _max = max;

   BaseElement<dataType>::Register(ds, pfr);
}



//
// Register method
//
template <class dataType> void RangedElement<dataType>::Register (DataStore *ds, PfrType pfr, const dataType min, const dataType max, const dataType &initValue)
{
   _min = min;
   _max = max;

   BaseElement<dataType>::Register(ds, pfr, initValue);
}



//
// Set Method
//
template <class dataType> bool RangedElement<dataType>::Set(const dataType &data)
{
   if ( (data >= _min) && 
        (data <= _max) )
   {
      return BaseElement<dataType>::Set(data);
   }
   else
   {
      // Log Error
      DataLog(log_level_cds_info) << "RangedElement: Set Failed in datastore " << _ds->Name() 
           << ".  Value is out of Range.  Value=" << data 
           << " Range=" << _min << "->" << _max << endmsg;
      return false;
   }
}

#endif /* ifndef _DATASTORE_PRIVATE_INCLUDE */

