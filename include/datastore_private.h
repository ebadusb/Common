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

#include "datastore.h"

#include "error.h"
#include <a_out.h>  // Needed for symbol table



////////////////////////////////////////////////////////////////
// BindItem
////////////////////////////////////////////////////////////////

//template <class T> void DataStore::BindItem(T** dataPtr, BIND_ITEM_TYPE item, bool &created)
template <class T> void BindItem(DataStore *ds, T **dataPtr, BIND_ITEM_TYPE item, bool &created)
{
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

      if (ds->is_logging())
      {
         DataLog(ds->_debug) << "Attaching item " << nameKey.c_str() << ", address " << dataPtr << ", " << *dataPtr << endmsg;
      }
   }
   else
   {
      // create a new data element
      *dataPtr = new T; 

      // add to symbol table
      status = symAdd(DataStore::getTable(), (char *)nameKey.c_str(), (char *)*dataPtr, N_DATA, item);

      if (ds->is_logging())
      {
         DataLog(ds->_debug) << "Creating item " << nameKey.c_str() << ", address " << dataPtr << ", " << *dataPtr << endmsg;
      }

      if (status == ERROR)
      {
         // Log Fatal Error
         DataLog(ds->_fatal) << "BindItem: symAdd failed in CDS " << ds->Name() << "." << endmsg;
         _FATAL_ERROR(__FILE__, __LINE__, "FATAL ERROR");
      }

      created = true;
   }
}



////////////////////////////////////////////////////////////////
// BaseElement
////////////////////////////////////////////////////////////////


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
      _FATAL_ERROR(__FILE__, __LINE__, "FATAL ERROR.  Element failed to register");
   }

   // If calling instance is spoofer or no spoof has been registered, return real value
   if( *_handle->_fp == NULL )
   {
       _ds->Lock();
       *item = *_handle->_data;
       _ds->Unlock();
   }
   else if( _ds->GetRole() == ROLE_SPOOFER && _handle->_spooferCacheIsValid == false )
   {
       _ds->Lock();
       _handle->_spooferCacheIsValid = true;
       *item = *_handle->_data;
       _ds->Unlock();
   }
   else
   {
       _ds->Lock();
       pair< dataType*, const dataType* > toFrom( item, _handle->_data );
       (*(*_handle->_fp))( &toFrom );  // runs spoofer callback
       _ds->Unlock();
   }
}



#define NOLOCK_FASTGET(T)                                                                  \
template<> inline void BaseElement<T>::Get(T *item) const                                  \
{                                                                                          \
   if (_ds == 0)                                                                           \
   {                                                                                       \
      _FATAL_ERROR(__FILE__, __LINE__, "FATAL ERROR.  Element failed to register");        \
   }                                                                                       \
                                                                                           \
   /* If calling instance is spoofer or no spoof has been registered, return real value */ \
   if ( *_handle->_fp == NULL )                                                            \
   {                                                                                       \
      *item = *_handle->_data;                                                             \
   }                                                                                       \
   else if( _ds->GetRole() == ROLE_SPOOFER && _handle->_spooferCacheIsValid == false )     \
   {                                                                                       \
       _handle->_spooferCacheIsValid = true;                                               \
       *item = *_handle->_data;                                                            \
   }                                                                                       \
   else                                                                                    \
   {                                                                                       \
      pair< T*, const T* > toFrom( item, _handle->_data );                                 \
      (*(*_handle->_fp))( &toFrom );  /* runs spoofer callback */                          \
   }                                                                                       \
}



//
// Get Specializations.
// The following types are "safe" and do not require locking as a read operation is atomic.
//
NOLOCK_FASTGET(int);
NOLOCK_FASTGET(char);
NOLOCK_FASTGET(bool);
NOLOCK_FASTGET(float);
NOLOCK_FASTGET(double);



//
// Get method
//
template <class dataType> dataType BaseElement<dataType>::Get() const
{

   if (_ds == 0)
   {
      _FATAL_ERROR(__FILE__, __LINE__, "FATAL ERROR.  Element failed to register");
   }

   // If calling instance is spoofer or no spoof has been registered, return real value
   if ( *_handle->_fp == NULL ) 
   {
      _ds->Lock();
      dataType temp = *_handle->_data;
      _ds->Unlock();
      return temp;
   }
   else if ( (_ds->GetRole() == ROLE_SPOOFER) && (_handle->_spooferCacheIsValid == false) )
   {
       _ds->Lock();
       _handle->_spooferCacheIsValid = true;
       dataType temp = *_handle->_data;
       _ds->Unlock();
       return temp;
   }
   else
   {
      dataType temp;
      pair< dataType*, const dataType* > toFrom( &temp, _handle->_data );
      _ds->Lock();
 
      (*(*_handle->_fp))( &toFrom );  // runs spoofer callback
 
      _ds->Unlock();
      return temp;
   }
}



#define NOLOCK_GET(T)                                                                      \
template<> inline T BaseElement<T>::Get() const                                            \
{                                                                                          \
   if (_ds == 0)                                                                           \
   {                                                                                       \
      _FATAL_ERROR(__FILE__, __LINE__, "FATAL ERROR.  Element failed to register");        \
   }                                                                                       \
                                                                                           \
   /* If calling instance is spoofer or no spoof has been registered, return real value */ \
   if ( *_handle->_fp == NULL )                                                            \
   {                                                                                       \
      T temp = *_handle->_data;                                                            \
      return temp;                                                                         \
   }                                                                                       \
   else if ( (_ds->GetRole() == ROLE_SPOOFER) && (_handle->_spooferCacheIsValid == false) )\
   {                                                                                       \
      _handle->_spooferCacheIsValid = true;                                                \
      T temp = *_handle->_data;                                                            \
      return temp;                                                                         \
   }                                                                                       \
   else                                                                                    \
   {                                                                                       \
      T temp;                                                                              \
      pair< T*, const T* > toFrom( &temp, _handle->_data );                                \
      (*(*_handle->_fp))( &toFrom );  /* runs spoofer callback */                          \
      return temp;                                                                         \
   }                                                                                       \
}



//
// Get Specializations.
// The following types are "safe" and do not require locking as a read operation is atomic.
//
NOLOCK_GET(int);
NOLOCK_GET(char);
NOLOCK_GET(bool);
NOLOCK_GET(float);
NOLOCK_GET(double);



//
// Set method
//
template <class dataType> bool BaseElement<dataType>::Set(const dataType &data)
{
   if (_ds == 0)
   {
      _FATAL_ERROR(__FILE__, __LINE__, "FATAL ERROR.  Element failed to register");
   }

   if (_ds->GetRole() != ROLE_RO)
   {
      _ds->Lock();
      _handle->_spooferCacheIsValid = false;
      *_handle->_data = data;
      _ds->Unlock();

      return true;
   }
   else
   {
      // Log Fatal Error
      DataLog(_ds->_fatal) << "BaseElement: Set Failed in CDS " << _ds->Name() << ".  Role is RO." << endmsg;
      _FATAL_ERROR(__FILE__, __LINE__, "FATAL ERROR");
      return false;
   }
}



#define NOLOCK_SET(T)                                                                \
template<> inline bool BaseElement<T>::Set(const T &data)                            \
{                                                                                    \
   if (_ds == 0)                                                                     \
   {                                                                                 \
      _FATAL_ERROR(__FILE__, __LINE__, "FATAL ERROR.  Element failed to register");  \
   }                                                                                 \
                                                                                     \
   if (_ds->GetRole() != ROLE_RO)                                                    \
   {                                                                                 \
      _handle->_spooferCacheIsValid = false;                                         \
      *_handle->_data = data;                                                        \
                                                                                     \
      return true;                                                                   \
   }                                                                                 \
   else                                                                              \
   {                                                                                 \
      /* Log Fatal Error */                                                          \
      DataLog(_ds->_fatal) << "BaseElement: Set Failed in " << _ds->Name()           \
                              << ".  Role is RO." << endmsg;                         \
      _FATAL_ERROR(__FILE__, __LINE__, "FATAL ERROR");                               \
      return false;                                                                  \
   }                                                                                 \
}



//
// Set Specializations.
// The following types are "safe" and do not require locking as a read operation is atomic.
//
NOLOCK_SET(int);
NOLOCK_SET(char);
NOLOCK_SET(bool);
NOLOCK_SET(float);
NOLOCK_SET(double);



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
      DataLog(_ds->_fatal) << "ReadSelf failed in " << _ds->Name()
                              << ".  status is: " << pfrfile.rdstate() << endmsg;
      _FATAL_ERROR(__FILE__, __LINE__, "FATAL ERROR.  ReadSelf failed.");
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
      DataLog(_ds->_fatal) << "WriteSelf failed in " << _ds->Name()
                              << ".  status is: " << pfrfile.rdstate() << endmsg;
      _FATAL_ERROR(__FILE__, __LINE__, "FATAL ERROR.  WriteSelf failed.");
   }
}



//
// SetSpoof
//
template <class dataType> void BaseElement<dataType>::SetSpoof (const CallbackBase* fp)
{
   _handle->_spooferCacheIsValid = false;
   *_handle->_fp  = fp;
}




//
// ClearSpoof
//
template <class dataType> void BaseElement<dataType>::ClearSpoof ()
{
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
template <class dataType> RangedElement<dataType>::RangedElement() :
   BaseElement()
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

   BaseElement::Register(ds, pfr);
}



//
// Register method
//
template <class dataType> void RangedElement<dataType>::Register (DataStore *ds, PfrType pfr, const dataType min, const dataType max, const dataType &initValue)
{
   _min = min;
   _max = max;

   BaseElement::Register(ds, pfr, initValue);
}



//
// Set Method
//
template <class dataType> bool RangedElement<dataType>::Set(const dataType &data)
{
   if ( (data >= _min) && 
        (data <= _max) )
   {
      return BaseElement::Set(data);
   }
   else
   {
      // Log Error
      DataLog(_ds->_debug) << "RangedElement: Set Failed in datastore " << _ds->Name() 
           << ".  Value is out of Range.  Value=" << data 
           << " Range=" << _min << "->" << _max << endmsg;
      return false;
   }
}



//
// DataStore
//
const unsigned int SYM_NAME_SIZE = 200;

const char DATASTORE_SYMBOL_CONTAINER[]    = "_DataStore_%s_container";
const char BASE_ELEMENT_SYMBOL_CONTAINER[] = "_Base_Element_%s_ref_%d";

const int DATASTORE_SYMTBL_SIZE = 6;   // Create Max 64 datastore derived classes


