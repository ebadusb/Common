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
   //char nameKey[SYM_NAME_SIZE];
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

      //DataLog(*(ds->_debug)) << "Attaching item " << nameKey.c_str() << ", address " << dataPtr << ", " << *dataPtr << endmsg;
   }
   else
   {
      // create a new data element
      *dataPtr = new T; 

      // add to symbol table
      status = symAdd(DataStore::getTable(), (char *)nameKey.c_str(), (char *)*dataPtr, N_DATA, item);

      //DataLog(*(ds->_debug)) << "Creating item " << nameKey.c_str() << ", address " << dataPtr << ", " << *dataPtr << endmsg;

      if (status == ERROR)
      {
         // Log Fatal Error
         DataLog(*(ds->_fatal)) << "BindItem: symAdd failed in CDS " << ds->Name() << "." << endmsg;
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
   BindItem(ds, &_data, ITEM_DATA, created);

   // Only add the element if PFR is desired and it didn't exist in the list (avoid duplicates).
   if ( created && (_pfrType == PFR_RECOVER) )
   {
      _ds->AddElement(this);
   }

   // Bind the spoof fp to the DataStore symbol table entry.
   BindItem(ds, &_fp, ITEM_SPOOF, created);
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
   BindItem(ds, &_data, ITEM_DATA, created);

   if (created)
   {
      // Only add the element if PFR is desired and it didn't exist in the list (avoid duplicates).
      if (_pfrType == PFR_RECOVER)
      {
         _ds->AddElement(this);
      }

      // First datastore initializes the element
      // TBD.  This may need to be semaphore protected.
      *_data = initValue;
      //Set(initValue);
   }

   // Bind the spoof fp to the DataStore symbol table entry.
   BindItem(ds, &_fp, ITEM_SPOOF, created);
   *_fp = NULL;
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
      DataLog(*(_ds->_fatal)) << "BaseElement: Set Failed in CDS " << _ds->Name() << ".  Role is RO." << endmsg;
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
   //DataLog(_ds->*_debug) << "Reading value " << *_data << ", size " << sizeof(dataType) << endmsg;
}
   


//
// WriteSelf method
//
template <class dataType> void BaseElement<dataType>::WriteSelf (ofstream &pfrfile)
{
   //DataLog(_ds->*_debug) << "Writing value " << *_data << ", size " << sizeof(dataType) << endmsg;
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
   _min = min;
   _max = max;

   BaseElement::Register(ds, role, pfr);
}



//
// Register method
//
template <class dataType> void RangedElement<dataType>::Register (DataStore *ds, Role role, PfrType pfr, const dataType min, const dataType max, const dataType &initValue)
{
   _min = min;
   _max = max;

   BaseElement::Register(ds, role, pfr, initValue);
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
      DataLog(*(_ds->_debug)) << "RangedElement: Set Failed in datastore " << _ds->Name() 
           << ".  Value is out of Range.  Value=" << data 
           << " Range=" << _min << "->" << _max << endmsg;
      return false;
   }
}



//
// DataStore
//

const unsigned int SYM_NAME_SIZE = 200;


const char DATASTORE_DATA_NAME[]         = "%s_ref_%d";
const char DATASTORE_SPOOF_NAME[]        = "%s_spoof_%d";
const char DATASTORE_LIST_NAME[]         = "_DataStore_%s_list";
const char DATASTORE_MUTEX_SEM[]         = "_DataStore_%s_mutexsem";
const char DATASTORE_READ_SEM[]          = "_DataStore_%s_readsem";
const char DATASTORE_WRITE_SEM[]         = "_DataStore_%s_writesem";
const char DATASTORE_SIGNAL_READ[]       = "_DataStore_%s_signalread";
const char DATASTORE_SIGNAL_WRITE[]      = "_DataStore_%s_signalwrite";
const char DATASTORE_READ_COUNT[]        = "_DataStore_%s_readcount";
const char DATASTORE_WRITER_DECLARED[]   = "_DataStore_%s_writer_declared";

const int DATASTORE_SYMTBL_SIZE = 9;


/*
//
// Debug routine to dump symbol table entry
//
BOOL DumpEntry(char *name, int val, SYM_TYPE type, int arg, UINT16 group)
{
   cout << "Symbol " << name << ", val " << val << ", type " << type
        << ", group " << group << endl;

   return true;
}


void dump_datastore_table ()
{
   symEach (DataStore::getTable(), DumpEntry, 0);
}
*/
