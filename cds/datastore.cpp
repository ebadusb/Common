/*******************************************************************************
 *
 * Copyright (c) 2002 by Gambro BCT, Inc.  All rights reserved.
 *
 * TITLE:      datastore.cpp
 *             Common Data Store Interface
 *
 * AUTHOR:     Regis McGarry
 *
 * ABSTRACT:   This file defines the Data Store primitives from which Common Data
 *             Stores are made.
 *
 * HISTORY:    $Log: datastore.cpp $
 * HISTORY:    Revision 1.3  2002/07/02 16:00:20Z  rm70006
 * HISTORY:    Added new non-write-restrictive class to CDS.
 * HISTORY:    Revision 1.2  2002/07/01 16:38:41Z  sb07663
 * HISTORY:    More descriptive logs and fatal errors
 * HISTORY:    Revision 1.1  2002/06/24 20:33:48  rm70006
 * HISTORY:    Initial revision
 * HISTORY:    Revision 1.1  2002/06/24 19:24:25Z  rm70006
 * HISTORY:    Initial revision
 * HISTORY:    Revision 1.3  2002/06/17 18:40:39Z  rm70006
 * HISTORY:    Change design to make data store instance based instead of static.  Changed from taskVars to using the symbol table library.
 * HISTORY:    Revision 1.2  2002/06/04 17:18:17Z  rm70006
 * HISTORY:    Added logging and new const ref get function.
 * HISTORY:    Revision 1.1  2002/05/28 19:00:28Z  rm70006
 * HISTORY:    Initial revision
*******************************************************************************/

#include "datastore.h"
#include "common_datalog.h"


#include <stdio.h>




////////////////////////////////////////////////////////////////
// ElementType
////////////////////////////////////////////////////////////////


//
// Default Constructor
//
ElementType::ElementType()
{
   _ds      = NULL;
   _role    = Role(-1);
   _pfrType = PfrType(-1);
}



//
// Destructor
//
ElementType::~ElementType()
{
}



//
// Register
//
void ElementType::Register(DataStore *ds, Role role, PfrType pfr)
{
   _ds      = ds;
   _role    = role;
   _pfrType = pfr;

   STATUS status;

   // Only add the element if the Role is RW and PFR is desired (avoid duplicates).
   if ( (_role == ROLE_RW) && (_pfrType == PFR_RECOVER) )
   {
      _ds->AddElement(this);
   }
}



////////////////////////////////////////////////////////////////
// DataStore
////////////////////////////////////////////////////////////////
#include <memLib.h>


DATASTORE_LISTTYPE DataStore::_datastoreList;

DataLog_Level    DataStore::_debug(LOG_DATASTORE);
DataLog_Critical DataStore::_fatal;

SYMTAB_ID DataStore::_datastoreTable = NULL;


//
// Default Constructor
//
DataStore::DataStore()
{
   // Assert an error.
   DataLog(_fatal) << "Datastore Constructor Error: " << endmsg;
   _FATAL_ERROR(__FILE__, __LINE__, "Datastore default Constructor");
}



//
// DataStore Constructor
//
// This constructor creates the following resources:
// 1) allocates the _pfrList and environment variable
// 2) allocates the _readCountSemaphore and environment variable
// 3) allocates the _writeSemaphore and environment variable
// 4) allocates the _signalWrite and environment variable
// 5) allocates the _readCount and environment variable
//
//
DataStore::DataStore(char *name, Role role) :
   _role(role),
   _name(name),
   _refCount(0),
   _pfrList(NULL),
   _spoofCount(0),
   _readCountSemaphore(NULL),
   _writeSemaphore(NULL),
   _signalWrite(NULL),
   _readCount(NULL)
{
   const int SEM_FLAGS = SEM_Q_PRIORITY | SEM_DELETE_SAFE | SEM_INVERSION_SAFE;

   bool created;

   //
   // Keep track of how many writers there are.  There should only be 1.
   //
   if (role == ROLE_RW)
   {
      CheckForMultipleWriters();
   }

   // Create the Symbol table.
   if (_datastoreTable == NULL)
   {
      _datastoreTable = symTblCreate(DATASTORE_SYMTBL_SIZE, false, memSysPartId);   // Create a symbol table that does NOT allow duplicate items.
   }

   //
   // Create list symbol name and assign it's value to the member variable.
   //
   BindItem(&_pfrList, ITEM_PFR_LIST, created);


   // If the symbol doesn't exist, add it to the master list.
   if (created)
   {
      _datastoreList.push_back(this);
   }


   //
   // Create env var names for mutex semaphores and assign their values to the member variables.
   //
   BindItem(&_readCountSemaphore, ITEM_READ_COUNT_SEMAPHORE, created);

   // If the symbol doesn't exist, create it (first time).
   if (created)
   {
      // Create the semaphore.
      *_readCountSemaphore = semMCreate(SEM_FLAGS);

      if (*_readCountSemaphore == NULL)
      {
         // Fatal Error
         DataLog(_fatal) << "_readCountSemaphore could not be created for CDS " << _name << endmsg;
         _FATAL_ERROR(__FILE__, __LINE__, "_readCountSemaphore could not be created.");
      }
   }

   BindItem(&_writeSemaphore, ITEM_WRITE_COUNT_SEMAPHORE, created);

   // If the env var doesn't exist, create it (first time).
   if (created)
   {
      // Create the semaphore.
      *_writeSemaphore = semMCreate(SEM_FLAGS);

      // Fatal if _writeSemaphore = NULL
      if (*_writeSemaphore == NULL)
      {
         DataLog(_fatal) << "_writeSemaphoreSemaphore could not be createdfor CDS " << _name << endmsg;
         _FATAL_ERROR(__FILE__, __LINE__, "_writeSemaphoreSemaphore could not be created.");
      }
   }

   //
   // Create symbol names for mutex control flags and assign their values to the member variables.
   //
   BindItem(&_signalWrite, ITEM_SIGNAL_WRITE, created);

   // If the symbol doesn't exist, initialize it (first time).
   if (created)
   {
      *_signalWrite = 0;
   }

   BindItem(&_readCount, ITEM_READ_COUNT, created);
   
   // If the symbol doesn't exist, initialize it (first time).
   if (created)
   {
      *_readCount = 0;
   }
}



//
// Base Destructor
//
DataStore::~DataStore()
{
   // Don't delete anything.  The rest of the system is still using it.
}



//
// CheckForMultipleWriters
//
void DataStore::CheckForMultipleWriters()
{
   static bool _writerDeclared = false;
   
   // The base implementation of DataStore fatal errors when multiple writers are declared.
   if (_writerDeclared)
   {
      // This is an error.
      DataLog(_fatal) << "Error.  Multiple Writers Declared for CDS " << _name << ".  Abort!!!!!!" << endmsg;
      _FATAL_ERROR(__FILE__, __LINE__, "Datastore multiple writers");
      return;
   }
   else
      _writerDeclared = true;
}



//
// SavePfrData
//
void DataStore::SavePfrData (ofstream &pfrFile)
{
   // iterate the list.  Put out to the ofstream all elements that need to be
   // saved for PFR for every datastore created (duplicates were filtered out on create). 
   // Note:  Order is important only in that it needs to be consistent.
   //        List order is assigned by the container class Register calls.
   //        The restore function will put back in the same order.
   for (DATASTORE_LISTTYPE::iterator datastoreIterator = _datastoreList.begin(); datastoreIterator != _datastoreList.end(); ++datastoreIterator)
   {
      DataLog(_debug) << "saving datastore " << (*datastoreIterator)->_name << endmsg;

      // Check for existence
      if ((*datastoreIterator)->_pfrList == NULL)
      {
         DataLog(_fatal) << "SavePfrData _pfrList is NULL.  Datastore " << (*datastoreIterator)->_name << " is invalid" << endmsg;
         _FATAL_ERROR(__FILE__, __LINE__, "Datastore SavePfrData _pfrList is NULL");
      }
      else
      {
         for (ELEMENT_LISTTYPE::iterator pfrListIterator = (*datastoreIterator)->_pfrList->begin(); pfrListIterator != (*datastoreIterator)->_pfrList->end(); ++pfrListIterator)
         {
            if ((*pfrListIterator)->_pfrType == PFR_RECOVER)
            {
               (*pfrListIterator)->WriteSelf(pfrFile);
            }
         }
      }
   }
}



//
// RestorePfrData
//
void DataStore::RestorePfrData (ifstream &pfrFile)
{
   for (DATASTORE_LISTTYPE::iterator datastoreIterator = _datastoreList.begin(); datastoreIterator != _datastoreList.end(); ++datastoreIterator)
   {
      DataLog(_debug) << "restoring datastore " << (*datastoreIterator)->_name << endmsg;

      // Check for existence
      if ((*datastoreIterator)->_pfrList == NULL)
      {
         DataLog(_fatal) << "RestorePfrData _pfrList is NULL.  Datastore " << (*datastoreIterator)->_name << " is invalid" << endmsg;
         _FATAL_ERROR(__FILE__, __LINE__, "Datastore RestorePfrData _pfrList is NULL");
      }
      else
      {
         for (ELEMENT_LISTTYPE::iterator pfrListIterator = (*datastoreIterator)->_pfrList->begin(); pfrListIterator != (*datastoreIterator)->_pfrList->end(); ++pfrListIterator)
         {
            if ((*pfrListIterator)->_pfrType == PFR_RECOVER)
            {
               (*pfrListIterator)->ReadSelf(pfrFile);
            }
         }
      }
   }
}



//
// AddElement
//
void DataStore::AddElement (ElementType *member)
{
   if (_pfrList == NULL)
   {
      DataLog(_fatal) << "AddElement _pfrList is null for CDS " << _name << endmsg;
      _FATAL_ERROR(__FILE__, __LINE__, "AddElement _pfrList is null");
   }
   else
   {
      _pfrList->push_back(member);
   }
}



//
// DeleteElement
//
void DataStore::DeleteElement (ElementType *member)
{
   if (_pfrList == NULL)
   {
      DataLog(_fatal) << "DeleteElement _pfrList is null for CDS " << _name << endmsg;
      _FATAL_ERROR(__FILE__, __LINE__, "DeleteElement _pfrList is null");
   }
   else
   {
      _pfrList->remove (member);
   }
}



//
// Lock
//
void DataStore::Lock()
{
   if (_signalWrite == NULL)
   {
      DataLog(_fatal) << "_signalWrite is null for CDS " << _name << ".  DataStore writer must have exited." << endmsg;
      _FATAL_ERROR(__FILE__, __LINE__, "Datastore Lock _signalWrite is null.");
   }
   else
   {
      // If instance is RO, perform RO semaphore lock
      if (_role == ROLE_RO)
      {
         if (*_signalWrite)
         {
            semTake(*_writeSemaphore, -1);
         }

         semTake(*_readCountSemaphore, -1);

         if (++(*_readCount) == 1)
         {
            semTake(*_writeSemaphore, -1);
         }

         semGive(*_readCountSemaphore);
      }
      else  // Do RW semaphore lock
      {
         (*_signalWrite)++;

         semTake(*_writeSemaphore, -1);
      }
   }
}



//
// Unlock
//
void DataStore::Unlock()
{
   if (_readCountSemaphore == NULL)
   {
      // Log Error
   }
   else
   {
      // If instance is RO, perform RO semaphore lock
      if (_role == ROLE_RO)
      {
         semTake(*_readCountSemaphore, -1);

         if (-- (*_readCount) == 0)
         {
            semGive(*_writeSemaphore);
         }

         semGive(*_readCountSemaphore);
      }
      else  // Do RW semaphore lock
      {
         // Clear the Write signal
         *_signalWrite = 0;

         // Unlock the semaphore
         semGive(*_writeSemaphore);
      }
   }
}



//
// MultWriteDataStrore
//



//
// Base Constructor
// 
MultWriteDataStore::MultWriteDataStore(char * name, Role role) :
   DataStore (name, role)
{
}



//
// Base Destructor
// 
MultWriteDataStore::~MultWriteDataStore()
{
}



// Turn on for testing.  Remove when real one is developed
#if 0
#include <assert.h>
void _FATAL_ERROR(char* file, int line, char* eString)
{
   cout << "FATAL_ERROR: " << file << ":" << line << " - " << eString << endl;
   assert(0);
}
#endif
