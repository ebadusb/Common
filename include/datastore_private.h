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
#include <a_out.h>


////////////////////////////////////////////////////////////////
// BaseElement
////////////////////////////////////////////////////////////////


//
// Default Constructor (no default value)
//
template <class dataType> BaseElement<dataType>::BaseElement() :
   ElementType(),
   _fp(NULL),
   _data(NULL)
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
template <class dataType> void BaseElement<dataType>::Register (DataStore *ds, Role role, PfrType pfr)
{
   bool created;

   ElementType::Register(ds, role, pfr);

   // Bind the data element to the DataStore symbol table entry.
   ds->BindItem(&_data, ITEM_DATA, created);

   // Bind the spoof fp to the DataStore symbol table entry.
   ds->BindItem(&_fp, ITEM_SPOOF, created);
   *_fp = NULL;
}



//
// Register method
//
template <class dataType> void BaseElement<dataType>::Register (DataStore *ds, Role role, PfrType pfr, const dataType &initValue)
{
   bool created; 

   ElementType::Register(ds, role, pfr);

   // Bind the data element to the DataStore symbol table entry.
   ds->BindItem(&_data, ITEM_DATA, created);

   // Bind the spoof fp to the DataStore symbol table entry.
   ds->BindItem(&_fp, ITEM_SPOOF, created);
   *_fp = NULL;

   // Only initialize if Role is not RO
   if (_role != ROLE_RO)
   {
      *_data = initValue;
   }
}



//
// Get method
//
template <class dataType> const dataType & BaseElement<dataType>::Get() const
{
   // If calling instance is spoofer or no spoof has been registered, return real value
   if ( (_role == ROLE_SPOOFER) || (*_fp == NULL) )
   {
      return *_data;
   }
   else
   {
      FP fp = *_fp;
      return (fp(*this));   // Give control to spoofer to return "spoofed" value.
   }
}



//
// Set method
//
template <class dataType> bool BaseElement<dataType>::Set(const dataType &data)
{
   if (_role != ROLE_RO)
   {
      *_data = data;
      return true;
   }
   else
   {
      // Log Fatal Error
      DataLog(_ds->_fatal) << "BaseElement: Set Failed.  Role is RO." << endmsg;
      _FATAL_ERROR(__FILE__, __LINE__, "FATAL ERROR");
      return false;
   }
}



//
// ReadSelf method
//
template <class dataType> void BaseElement<dataType>::ReadSelf (ifstream &pfrfile)
{
   pfrfile.read(_data, sizeof(dataType));
   //pfrfile >> _data;
   //DataLog(_ds->_debug) << "Reading value " << *_data << ", size " << sizeof(dataType) << endmsg;
}
   


//
// WriteSelf method
//
template <class dataType> void BaseElement<dataType>::WriteSelf (ofstream &pfrfile)
{
   //DataLog(_ds->_debug) << "Writing value " << *_data << ", size " << sizeof(dataType) << endmsg;
   pfrfile.write(_data, sizeof(dataType));
   //pfrfile << _data;
}



//
// SetSpoof
//
template <class dataType> void BaseElement<dataType>::SetSpoof (FP fp)
{
   *_fp  = fp;
}




//
// ClearSpoof
//
template <class dataType> void BaseElement<dataType>::ClearSpoof ()
{
   *_fp = NULL;
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
template <class dataType> void RangedElement<dataType>::Register (DataStore *ds, Role role, PfrType pfr, const dataType min, const dataType max)
{
   bool created; 

   // Call the base class register
   ElementType::Register(ds, role, pfr);

   // Bind the data element to the DataStore symbol table entry.
   ds->BindItem(&_data, ITEM_DATA, created);

   // Bind the spoof fp to the DataStore symbol table entry.
   ds->BindItem(&_fp, ITEM_SPOOF, created);
   *_fp = NULL;

   _min = min;
   _max = max;
}



//
// Register method
//
template <class dataType> void RangedElement<dataType>::Register (DataStore *ds, Role role, PfrType pfr, const dataType min, const dataType max, const dataType &initValue)
{
   bool created;

   // Call the base class register
   ElementType::Register(ds, role, pfr);
   
   // Bind the data element to the DataStore symbol table entry.
   ds->BindItem(&_data, ITEM_DATA, created);

   // Bind the spoof fp to the DataStore symbol table entry.
   ds->BindItem(&_fp, ITEM_SPOOF, created);
   *_fp = NULL;

   _min = min;
   _max = max;

      // Only initialize if Role is not RO
   if (_role != ROLE_RO)
   {
      *_data = initValue;
   }
}



//
// Get method
//
template <class dataType> const dataType & RangedElement<dataType>::Get() const
{
   // If calling instance is spoofer or no spoof has been registered, return real value
   if ( (_role == ROLE_SPOOFER) || (*_fp == NULL) )
   {
      return *_data;
   }
   else
   {
      FP fp = *_fp;
      return (fp(*this));   // Give control to spoofer to return "spoofed" value.
   }
}



//
// Set Method
//
template <class dataType> bool RangedElement<dataType>::Set(const dataType &data)
{
   if ( (data >= _min) && 
        (data <= _max) )
   {
      if (_role != ROLE_RO)
      {
         *_data = data;
         return true;
      }
      else
      {
         // Log Fatal Error
         DataLog(_ds->_fatal) << "RangedElement: Set Failed.  Role is RO.  Value=" << data << endmsg;
         _FATAL_ERROR(__FILE__, __LINE__, "FATAL ERROR");
         return false;
      }
   }
   else
   {
      // Log Error
      DataLog(_ds->_debug) << "RangedElement: Set Failed.  Value is out of Range.  Value=" << data 
           << " Range=" << _min << "->" << _max << endmsg;
      return false;
   }
}



//
// ReadSelf method
//
template <class dataType> void RangedElement<dataType>::ReadSelf (ifstream &pfrfile)
{
   BaseElement::ReadSelf(pfrfile);
}



//
// WriteSelf method
//
template <class dataType> void RangedElement<dataType>::WriteSelf (ofstream &pfrfile)
{
   BaseElement::WriteSelf(pfrfile);
}



//
// DataStore
//

const unsigned int SYM_NAME_SIZE = 200;


const char DATASTORE_DATA_NAME[]       = "%s_ref_%d";
const char DATASTORE_SPOOF_NAME[]      = "%s_spoof_%d";
const char DATASTORE_LIST_NAME[]       = "_DataStore_%s_list";
const char DATASTORE_READ_SEM[]        = "_DataStore_%s_readsem";
const char DATASTORE_WRITE_SEM[]       = "_DataStore_%s_writesem";
const char DATASTORE_SIGNAL_WRITE[]    = "_DataStore_%s_signalwrite";
const char DATASTORE_READ_COUNT[]      = "_DataStore_%s_readcount";
const char DATASTORE_WRITER_DECLARED[] = "_DataStore_%s_writer_declared";

const int DATASTORE_SYMTBL_SIZE = 9;



//
// BindItem
//
template <class T> void DataStore::BindItem(T** dataPtr, BIND_ITEM_TYPE item, bool &created)
{
   char nameKey[SYM_NAME_SIZE];
   SYM_TYPE symbolType;
   char *valPtr;


   // Create the Symbol name to search for.
   switch (item)
   {
   case ITEM_DATA:
      sprintf(nameKey, DATASTORE_DATA_NAME, _name.c_str(), _refCount);
      ++_refCount;
      break;

   case ITEM_SPOOF:
      sprintf(nameKey, DATASTORE_SPOOF_NAME, _name.c_str(), _spoofCount);
      ++_spoofCount;
      break;

   case ITEM_PFR_LIST:
      sprintf(nameKey, DATASTORE_LIST_NAME, _name.c_str());
      break;

   case ITEM_READ_COUNT_SEMAPHORE:
      sprintf(nameKey, DATASTORE_READ_SEM, _name.c_str());
      break;

   case ITEM_WRITE_COUNT_SEMAPHORE:
      sprintf(nameKey, DATASTORE_WRITE_SEM, _name.c_str());
      break;

   case ITEM_SIGNAL_WRITE:
      sprintf(nameKey, DATASTORE_SIGNAL_WRITE, _name.c_str());
      break;

   case ITEM_READ_COUNT:
      sprintf(nameKey, DATASTORE_READ_COUNT, _name.c_str());
      break;
   
   case ITEM_WRITER_DECLARED:
      sprintf(nameKey, DATASTORE_WRITER_DECLARED, _name.c_str());
      break;
   }

   // Search for Symbol.
   STATUS status = symFindByName(_datastoreTable, nameKey, &valPtr, &symbolType);

   // If symbol exists
   if (status == OK)
   {
      // return a pointer to the value
      *dataPtr = (T *)valPtr;
      created = false;
   }
   else
   {
      // create a new data element (new char[size])
      *dataPtr = new T; 

      if (*dataPtr == NULL)
      {
         // Log Fatal Error
         DataLog(_fatal) << "new allocator failed" << endmsg;
         _FATAL_ERROR(__FILE__, __LINE__, "FATAL ERROR");
      }
      
      // add to symbol table
      status = symAdd(_datastoreTable, nameKey, (char *)*dataPtr, N_DATA, item);

      if (status == ERROR)
      {
         // Log Fatal Error
         DataLog(_fatal) << "symAdd failed" << endmsg;
         _FATAL_ERROR(__FILE__, __LINE__, "FATAL ERROR");
      }

      created = true;
   }
}
