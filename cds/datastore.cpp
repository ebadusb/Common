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
 * HISTORY:    Revision 1.16  2002/09/25 16:05:00Z  rm70006
 * HISTORY:    Fixed bug in fatal logging.
 * HISTORY:    Revision 1.15  2002/09/24 16:48:39Z  rm70006
 * HISTORY:    Add extra debugging ability
 * HISTORY:    Revision 1.14  2002/09/19 21:46:07Z  jl11312
 * HISTORY:    - added initialization to avoid compiler warning for BUILD_TYPE=PRODUCTION
 * HISTORY:    Revision 1.13  2002/09/18 22:13:57  rm70006
 * HISTORY:    Changed get and set to have built in locking.
 * HISTORY:    Revision 1.12  2002/09/13 20:09:23Z  rm70006
 * HISTORY:    Fix bug with lock/unlock.
 * HISTORY:    Revision 1.10  2002/09/04 18:33:24Z  rm70006
 * HISTORY:    Added missing destructor for SingleWriteDataStore.
 * HISTORY:    Added code to SingleWriteDataStore to allow writer to be created and deleted as many times as needed.  Check now ensures only one at a time, not one ever.
 * HISTORY:    Revision 1.9  2002/09/03 14:36:19Z  rm70006
 * HISTORY:    Added new SingleWrite class.
 * HISTORY:    Revision 1.8  2002/08/23 14:55:08Z  rm70006
 * HISTORY:    Changed binditem to work with 486 compiler bug
 * HISTORY:    Revision 1.7  2002/07/17 14:03:22Z  sb07663
 * HISTORY:    latest latest fix for multiple writer check
 * HISTORY:    Revision 1.6  2002/07/16 22:22:13  rm70006
 * HISTORY:    Fix problem with uninitialized variable.
 * HISTORY:    Revision 1.5  2002/07/16 21:02:44Z  rm70006
 * HISTORY:    Fix bug in check for multiple writers.
 * HISTORY:    Revision 1.4  2002/07/05 16:36:46Z  rm70006
 * HISTORY:    Added destructor.
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
#include "errnoLib.h"   // Needed for errnoGet
#include "usrLib.h"     // Needed for printErrno
#include "wvLib.h"      // Needed for Event Timing


////////////////////////////////////////////////////////////////
// ElementType
////////////////////////////////////////////////////////////////


//
// Default Constructor
//
ElementType::ElementType()
{
   _ds      = NULL;
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
void ElementType::Register(DataStore *ds, PfrType pfr)
{
   _ds      = ds;
   _pfrType = pfr;
}



////////////////////////////////////////////////////////////////
// DataStore
////////////////////////////////////////////////////////////////
#include <memLib.h>


DATASTORE_LISTTYPE DataStore::_datastoreList;

DataLog_Level    *DataStore::_debug = 0;
DataLog_Critical *DataStore::_fatal = 0;

SYMTAB_ID DataStore::_datastoreTable = NULL;

bool DataStore::_logging = false;

//
// Default Constructor
//
DataStore::DataStore()
{
   DataLog_Critical _fatal;

   // Assert an error.
   DataLog(_fatal) << "Datastore default constructor was called." << endmsg;
   _FATAL_ERROR(__FILE__, __LINE__, "Datastore default constructor called.");
}



//
// DataStore Constructor
//
// This constructor creates the following resources:
// 1) allocates the _writerDeclared and environment variable
// 1) allocates the _pfrList and environment variable
// 2) allocates the _mutexSemaphore and environment variable
// 3) allocates the _readSemaphore and environment variable
// 3) allocates the _writeSemaphore and environment variable
// 4) allocates the _signalWrite and environment variable
// 4) allocates the _signalRead and environment variable
// 6) allocates the _readCount and environment variable
//
//
DataStore::DataStore(char *name, Role role) :
   _role(role),
   _name(name),
   _refCount(0),
   _pfrList(NULL),
   _spoofCount(0),
   _mutexSemaphore(NULL),
   _readSemaphore(NULL),
   _writeSemaphore(NULL),
   _signalRead(NULL),
   _signalWrite(NULL),
   _readCount(NULL),
   _writerDeclared(NULL)
{
   const int MUTEX_SEM_FLAGS = SEM_Q_PRIORITY | SEM_DELETE_SAFE | SEM_INVERSION_SAFE;
   bool created;

   if (_debug == 0)
   {
      _debug = new DataLog_Level(LOG_DATASTORE);
      _fatal = new DataLog_Critical();
   }

   // Create the Symbol table.
   if (_datastoreTable == NULL)
   {
      _datastoreTable = symTblCreate(DATASTORE_SYMTBL_SIZE, false, memSysPartId);   // Create a symbol table that does NOT allow duplicate items.
   }

   //
   // Keep track of how many writers there are.  There should only be 1.
   //
   BindItem(this, &_writerDeclared, ITEM_WRITER_DECLARED, created);

   // If the symbol doesn't exist, initialize it (first time).
   if (created)
   {
      *_writerDeclared = false;
   }

   //
   // Create list symbol name and assign it's value to the member variable.
   //
   BindItem(this, &_pfrList, ITEM_PFR_LIST, created);


   // If the symbol doesn't exist, add it to the master list.
   if (created)
   {
      _datastoreList.push_back(this);
   }


   //
   // Create env var names for mutex semaphores and assign their values to the member variables.
   //
   BindItem(this, &_mutexSemaphore, ITEM_MUTEX_SEMAPHORE, created);

   // If the symbol doesn't exist, create it (first time).
   if (created)
   {
      // Create the semaphore.
      *_mutexSemaphore = semMCreate(MUTEX_SEM_FLAGS);
      
      DataLog(*_debug) << "_mutexSemaphore value(" << hex << *_mutexSemaphore << dec << ")." << endmsg;

      if (*_mutexSemaphore == NULL)
      {
         // Fatal Error
         DataLog(*_fatal) << "_mutexSemaphore could not be created for " << _name 
                          << ".  Errno " << errnoGet() << ".  " << endmsg;
         printErrno(errnoGet());
         _FATAL_ERROR(__FILE__, __LINE__, "_mutexSemaphore could not be created.");
      }
   }

   BindItem(this, &_readSemaphore, ITEM_READ_SEMAPHORE, created);

   // If the env var doesn't exist, create it (first time).
   if (created)
   {
      // Create the semaphore.
      *_readSemaphore = semBCreate(SEM_Q_PRIORITY, SEM_FULL);

      // Fatal if _writeSemaphore = NULL
      if (*_readSemaphore == NULL)
      {
         DataLog(*_fatal) << "_readSemaphore could not be created for " << _name 
                          << ".  Errno " << errnoGet() << ".  " << endmsg;
         printErrno(errnoGet());
         _FATAL_ERROR(__FILE__, __LINE__, "_readSemaphore could not be created.");
      }
      else
         DataLog(*_debug) << "_readSemaphore value(" << hex << *_readSemaphore << dec << ")." << endmsg;
   }

   BindItem(this, &_writeSemaphore, ITEM_WRITE_SEMAPHORE, created);

   // If the env var doesn't exist, create it (first time).
   if (created)
   {
      // Create the semaphore.
      *_writeSemaphore = semBCreate(SEM_Q_PRIORITY, SEM_FULL);

      // Fatal if _writeSemaphore = NULL
      if (*_writeSemaphore == NULL)
      {
         DataLog(*_fatal) << "_writeSemaphore could not be created for " << _name 
                          << ".  Errno " << errnoGet() << ".  " << endmsg;
         printErrno(errnoGet());
         _FATAL_ERROR(__FILE__, __LINE__, "_writeSemaphore could not be created.");
      }
      else
         DataLog(*_debug) << "_writeSemaphore value(" << hex << *_writeSemaphore << dec << ")." << endmsg;

   }

   //
   // Create symbol names for mutex control flags and assign their values to the member variables.
   //
   BindItem(this, &_signalRead, ITEM_SIGNAL_READ, created);

   // If the symbol doesn't exist, initialize it (first time).
   if (created)
   {
      *_signalRead = false;
   }

   BindItem(this, &_signalWrite, ITEM_SIGNAL_WRITE, created);
   
   // If the symbol doesn't exist, initialize it (first time).
   if (created)
   {
      *_signalWrite = false;
   }

   BindItem(this, &_readCount, ITEM_READ_COUNT, created);
   
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
      DataLog(*_debug) << "saving datastore " << (*datastoreIterator)->_name << endmsg;

      // Check for existence
      if ((*datastoreIterator)->_pfrList == NULL)
      {
         DataLog(*_fatal) << "SavePfrData _pfrList is NULL.  Datastore " << (*datastoreIterator)->_name << " is invalid" << endmsg; 
         _FATAL_ERROR(__FILE__, __LINE__, "Datastore SavePfrData _pfrList is NULL");
      }
      else
      {
         int count = 0;

         for (ELEMENT_LISTTYPE::iterator pfrListIterator = (*datastoreIterator)->_pfrList->begin(); pfrListIterator != (*datastoreIterator)->_pfrList->end(); ++pfrListIterator)
         {
            if ((*pfrListIterator)->_pfrType == PFR_RECOVER)
            {
               (*pfrListIterator)->WriteSelf(pfrFile);
               count++;

               if (_logging)
               {
                  DataLog(*_debug) << "saving element " << count << "in " << (*datastoreIterator)->_name << " to PFR File." << endmsg;
               }
            }
         }

         DataLog(*_debug) << "saved " << count << " elements in " << (*datastoreIterator)->_name << "." << endmsg;
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
      DataLog(*_debug) << "restoring datastore " << (*datastoreIterator)->_name << endmsg;

      // Check for existence
      if ((*datastoreIterator)->_pfrList == NULL)
      {
         DataLog(*_fatal) << "RestorePfrData _pfrList is NULL.  Datastore " << (*datastoreIterator)->_name << " is invalid" << endmsg;
         _FATAL_ERROR(__FILE__, __LINE__, "Datastore RestorePfrData _pfrList is NULL");
      }
      else
      {
         int count = 0;

         for (ELEMENT_LISTTYPE::iterator pfrListIterator = (*datastoreIterator)->_pfrList->begin(); pfrListIterator != (*datastoreIterator)->_pfrList->end(); ++pfrListIterator)
         {
            if ((*pfrListIterator)->_pfrType == PFR_RECOVER)
            {
               (*pfrListIterator)->ReadSelf(pfrFile);
               count++;

               if (_logging)
               {
                  DataLog(*_debug) << "restoring element " << count << "in " << (*datastoreIterator)->_name << " to PFR File." << endmsg;
               }
            }
         }

         DataLog(*_debug) << "saved " << count << " elements in " << (*datastoreIterator)->_name << "." << endmsg;
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
      DataLog(*_fatal) << "AddElement _pfrList is null for " << _name << endmsg;
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
      DataLog(*_fatal) << "DeleteElement _pfrList is null for " << _name << endmsg;
      _FATAL_ERROR(__FILE__, __LINE__, "DeleteElement _pfrList is null");
   }
   else
   {
      _pfrList->remove (member);
   }
}



//
// Semaphore Take macro
//
#define SEM_TAKE(sem, wait)                                                           \
{                                                                                     \
   STATUS status = semTake(sem, wait);                                                \
   if (status != OK)                                                                  \
   {                                                                                  \
      DataLog(*_fatal) << "SemTake failed.  Errno " << errnoGet() << ".  " << endmsg; \
      printErrno(errnoGet());                                                         \
      _FATAL_ERROR(__FILE__, __LINE__, "semTake failed");                             \
   }                                                                                  \
}



//
// Semaphore Give macro
//
#define SEM_GIVE(sem)                                                                 \
{                                                                                     \
   STATUS status = semGive(sem);                                                      \
   if (status != OK)                                                                  \
   {                                                                                  \
      DataLog(*_fatal) << "SemGive failed.  Errno " << errnoGet() << ".  " << endmsg; \
      printErrno(errnoGet());                                                         \
      _FATAL_ERROR(__FILE__, __LINE__, "semGive failed");                             \
   }                                                                                  \
}



//
// Semaphore Flush macro
//
#define SEM_FLUSH(sem)                                                                 \
{                                                                                      \
   STATUS status = semFlush(sem);                                                      \
   if (status != OK)                                                                   \
   {                                                                                   \
      DataLog(*_fatal) << "SemFlush failed.  Errno " << errnoGet() << ".  " << endmsg; \
      printErrno(errnoGet());                                                          \
      _FATAL_ERROR(__FILE__, __LINE__, "semFlush failed");                             \
   }                                                                                   \
}



//
// Begin Critical Section macro
//
//#define BEGIN_CRITICAL_SECTION() SEM_TAKE(*_mutexSemaphore, WAIT_FOREVER);
#define BEGIN_CRITICAL_SECTION() ;



//
// End Critical Section macro
//
//#define END_CRITICAL_SECTION() SEM_GIVE(*_mutexSemaphore);
#define END_CRITICAL_SECTION() ;



//
// Lock
//
void DataStore::Lock()
{
   bool crit_section_released = false;
   int event_type = 0;
   static char temp[20];

   // If instance is RO, perform RO semaphore lock
   if (_role == ROLE_RO)
   {
      if (_logging)
      {
         event_type = 0x10;
         wvEvent(event_type, temp, 20);
      }

      BEGIN_CRITICAL_SECTION();

      // If a writer has signaled to write, readers should block until after 
      // writer releases semaphore.
      if (*_signalWrite)
      {

         if (_logging)
         {
            DataLog(*_debug) << "reader " << taskName(taskIdSelf()) << " RBOW in " << _name << ".\t"
                             << "WF(" << *_signalWrite << ")\t"
                             << "RF(" << *_signalRead << ")." << endmsg;
         }
         
         END_CRITICAL_SECTION();

         crit_section_released = true;

         // Block waiting for writer
         SEM_TAKE(*_readSemaphore, WAIT_FOREVER);

         if (_logging)
         {
            DataLog(*_debug) << "reader " << taskName(taskIdSelf()) 
                             << " unblocked by writer, continuing in " << _name << endmsg;
         }
         
         // Reset semaphore for next writer
         SEM_GIVE(*_readSemaphore);

         event_type += 0x1;
      }
      
      if (crit_section_released)
      {
         BEGIN_CRITICAL_SECTION();
      }

      // Increment the read count
      ++(*_readCount);

      if (*_signalRead == false)
      {
         // Signal Writers to block for read
         *_signalRead = true;

         if (_logging)
         {
            DataLog(*_debug) << "reader " << taskName(taskIdSelf()) << " first reader in " 
                             << _name << "(" << *_readCount << ").  Blocking future writers." << endmsg;
         }

         SEM_TAKE(*_writeSemaphore, WAIT_FOREVER);   // Non-blocking

         event_type += 0x2;
      }
      else
      {
         if (_logging)
         {
            DataLog(*_debug) << "reader " << taskName(taskIdSelf()) << " free pass (" 
                             << *_readCount << ") in " << _name << "." << endmsg;

            event_type += 0x4;
         }
      }

      END_CRITICAL_SECTION();
   }
   else  // Do RW semaphore lock
   {
      if (_logging)
      {
         event_type = 0x20;
         wvEvent(event_type, temp, 20);
      }

      BEGIN_CRITICAL_SECTION();

      // Set write count.
      *_signalWrite = true;

      if (*_signalRead)
      {
         if (_logging)
         {
            event_type += 0x1;

            DataLog(*_debug) << "writer " << taskName(taskIdSelf()) 
                             << " blocking future readers in " << _name << "." << endmsg;
         }

         // Block future readers (RBOW).  At this point, we shouldn't block.
         SEM_TAKE(*_readSemaphore, WAIT_FOREVER);    // Non-blocking

         if (_logging)
         {
            DataLog(*_debug) << "writer " << taskName(taskIdSelf()) << " WBOR in " << _name << ".\t"
                             << "WF(" << *_signalWrite << ")\t"
                             << "RF(" << *_signalRead << ")." << endmsg;
         }

         END_CRITICAL_SECTION();

         // Block on the Read semaphore (WBOR)
         SEM_TAKE(*_writeSemaphore, WAIT_FOREVER);    // Blocking

         BEGIN_CRITICAL_SECTION();

         if (_logging)
         {
            DataLog(*_debug) << "writer " << taskName(taskIdSelf()) << " lock in " 
                             << _name << "." << endmsg;
         }

         // Give back read semaphore.
         SEM_GIVE(*_writeSemaphore);    // Reset for next reader.

         END_CRITICAL_SECTION();
      }
      else
      {
         if (_logging)
         {
            event_type += 0x2;

            DataLog(*_debug) << "writer " << taskName(taskIdSelf()) << " lock in " 
                             << _name << "." << endmsg;
         }

         // Block future writers (RBOW).  At this point, we shouldn't block.
         SEM_TAKE(*_readSemaphore, WAIT_FOREVER);    // Non-blocking

         END_CRITICAL_SECTION();
      }
   }

   if (_logging)
   {
      wvEvent(event_type, temp, 20);
   }
}



//
// Unlock
//
void DataStore::Unlock()
{
   static char temp[20];
   int event_type = 0;

   // If instance is RO, perform RO semaphore lock
   if (_role == ROLE_RO)
   {
      if (_logging)
      {
         event_type = 0x30;
         wvEvent(event_type, temp, 20);
      }

      BEGIN_CRITICAL_SECTION();

      if (*_readCount > 0)
      {
         --(*_readCount);
      }

      if ( (*_signalWrite) && (*_signalRead) )   // Only do the first time.
      {
         event_type += 0x1;

         // Unlock the writer
         SEM_GIVE(*_writeSemaphore);

         // Clear/Reset read flag
         *_signalRead = false;

         if (_logging)
         {
            DataLog(*_debug) << "reader " << taskName(taskIdSelf()) << " released SEMWRITE in " << _name 
                             << ". WF(" << *_signalWrite << ") "
                             << "RC(" << *_readCount << ")." << endmsg;
         }
      }
      else if (*_readCount == 0)   // No more readers.
      {
         event_type += 0x2;

         // Unlock the writer
         SEM_GIVE(*_writeSemaphore);

         // Clear/Reset read flag
         *_signalRead = false;

         if (_logging)
         {
            DataLog(*_debug) << "reader " << taskName(taskIdSelf()) << " released SEMWRITE in " << _name 
                             << ". No more readers.  WF(" << *_signalWrite << ")." << endmsg;
         }
      }
      else
      {
         event_type += 0x4;

         if (_logging)
         {
            DataLog(*_debug) << "reader " << taskName(taskIdSelf()) << " free out in " << _name << ".  "
                             << "RF(" << *_signalRead << ") "
                             << "RC(" << *_readCount << ")." << endmsg;
         }
      }

      END_CRITICAL_SECTION();
   }
   else  // Do RW semaphore lock
   {
      if (_logging)
      {
         event_type = 0x40;
         wvEvent(event_type, temp, 20);
      }

      BEGIN_CRITICAL_SECTION();

      if (_logging)
      {
         DataLog(*_debug) << "writer " << taskName(taskIdSelf()) << " releasing SEMREAD in " << _name 
                          << ". WF(" << *_signalWrite << ") "
                          << "RF(" << *_signalRead << ")." << endmsg;
      }

      // Clear the Write signal
      *_signalWrite = false;

      END_CRITICAL_SECTION();

      // Unlock the semaphore
      SEM_GIVE(*_readSemaphore);
      SEM_FLUSH(*_readSemaphore);

      event_type += 0x1;
   }

   if (_logging)
   {
      wvEvent(event_type, temp, 20);
   }
}



//
// symbolName
//
void DataStore::GetSymbolName(string &s, const BIND_ITEM_TYPE item)
{
   int size = 0;
   const int s_len = s.size();

   // Create the Symbol name to search for.
   switch (item)
   {
   case ITEM_DATA:
      size = sprintf((char *)s.c_str(), DATASTORE_DATA_NAME, _name.c_str(), _refCount);
      ++_refCount;
      break;

   case ITEM_SPOOF:
      size = sprintf((char *)s.c_str(), DATASTORE_SPOOF_NAME, _name.c_str(), _spoofCount);
      ++_spoofCount;
      break;

   case ITEM_PFR_LIST:
      size = sprintf((char *)s.c_str(), DATASTORE_LIST_NAME, _name.c_str());
      break;

   case ITEM_MUTEX_SEMAPHORE:
      size = sprintf((char *)s.c_str(), DATASTORE_MUTEX_SEM, _name.c_str());
      break;

   case ITEM_READ_SEMAPHORE:
      size = sprintf((char *)s.c_str(), DATASTORE_READ_SEM, _name.c_str());
      break;

   case ITEM_WRITE_SEMAPHORE:
      size = sprintf((char *)s.c_str(), DATASTORE_WRITE_SEM, _name.c_str());
      break;

   case ITEM_SIGNAL_READ:
      size = sprintf((char *)s.c_str(), DATASTORE_SIGNAL_READ, _name.c_str());
      break;

   case ITEM_SIGNAL_WRITE:
      size = sprintf((char *)s.c_str(), DATASTORE_SIGNAL_WRITE, _name.c_str());
      break;

   case ITEM_READ_COUNT:
      size = sprintf((char *)s.c_str(), DATASTORE_READ_COUNT, _name.c_str());
      break;

   case ITEM_WRITER_DECLARED:
      size = sprintf((char *)s.c_str(), DATASTORE_WRITER_DECLARED, _name.c_str());
      break;

   default:
      DataLog(*_fatal) << "Unknown symbol type(" << item << ")" << "in " << Name() << "." << endmsg;
      _FATAL_ERROR(__FILE__, __LINE__, "Unknown symbol type.");
   }

   if (size > s_len)
   {
      // Dude.  You just wiped out something.
      DataLog(*_fatal) << "datastore item(" << item << ") name size mismatch in " << Name() 
                       << ".  Generated size is " << size << ".  Available size is " << s_len << "." << endmsg;
      _FATAL_ERROR(__FILE__, __LINE__, "Unknown symbol type.");
   }

}



//
// SingleWriteDataStore
//



//
// Base Constructor
// 
SingleWriteDataStore::SingleWriteDataStore(char * name, Role role) :
   DataStore (name, role)
{
   // Ensure no multiple writers
   CheckForMultipleWriters();
}



//
// Class Destructor
//
SingleWriteDataStore::~SingleWriteDataStore()
{
   if (GetRole() == ROLE_RW)
   {
      *_writerDeclared = false;
      DataLog(*_debug) << "Writer exited for CDS " << Name() << "." << endmsg;
   }
}



//
// CheckForMultipleWriters
//
void SingleWriteDataStore::CheckForMultipleWriters()
{
   if (GetRole() == ROLE_RW)
   {
      // The base implementation of DataStore fatal errors when multiple writers are declared.
      if (*_writerDeclared)
      {
         // This is an error.
         DataLog(*_fatal) << "Error.  Multiple Writers Declared for CDS " << Name() << ".  Abort!!!!!!" << endmsg;
         _FATAL_ERROR(__FILE__, __LINE__, "Datastore multiple writers");
         return;
      }
      else
         *_writerDeclared = true;
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
   // Empty check.  Included for completeness.
   CheckForMultipleWriters();
}



//
// Class Destructor
// 
MultWriteDataStore::~MultWriteDataStore()
{
}



//
// DynamicSingleWriteDataStrore
//



//
// Base Constructor
// 
DynamicSingleWriteDataStore::DynamicSingleWriteDataStore(char * name, Role role) :
   SingleWriteDataStore (name, role)
{
}



//
// Class Destructor
// 
DynamicSingleWriteDataStore::~DynamicSingleWriteDataStore()
{
}



//
// SetRead
//
void DynamicSingleWriteDataStore::SetRead()
{
   // If you are currently the writer, then unset the _writerDeclared flag
   if (_role == ROLE_RW)
   {
      *_writerDeclared = false;
   }

   _role = ROLE_RO;
}



//
// SetWrite
//
void DynamicSingleWriteDataStore::SetWrite()
{
   _role = ROLE_RW;

   CheckForMultipleWriters();
}



int dump_table_choice;

//
// Debug routine to dump symbol table entry
//
BOOL DumpEntry(char *name, int val, SYM_TYPE type, int arg, UINT16 group)
{
   int *data = (int *)val;

   switch (group)
   {
   case ITEM_DATA:
      cout << "DataItem " << name << ",\tvalue " << *data << endl;
      break;

   case ITEM_SPOOF:
      if (dump_table_choice > 0)
         cout << "SpoofItem " << name << ",\tvalue " << *data << endl;

      break;

   case ITEM_PFR_LIST:
      if (dump_table_choice > 0)
         cout << "PFRList " << name << ",\tptr " << data << endl;

      break;

   case ITEM_MUTEX_SEMAPHORE:
      if (dump_table_choice > 0)
         cout << "MUTEX " << name << ",\tptr " << data << endl;

      break;

   case ITEM_READ_SEMAPHORE:
      if (dump_table_choice > 0)
         cout << "READSEM " << name << ",\tptr " << data << endl;

      break;

   case ITEM_WRITE_SEMAPHORE:
      if (dump_table_choice > 0)
         cout << "WRITESEM " << name << ",\tptr " << data << endl;

      break;

   case ITEM_SIGNAL_READ:
      if (dump_table_choice > 0)
         cout << "SigRead " << name << ",\tvalue " << *data << endl;

      break;

   case ITEM_SIGNAL_WRITE:
      if (dump_table_choice > 0)
         cout << "SigWrite " << name << ",\tvalue " << *data << endl;

      break;

   case ITEM_READ_COUNT:
      if (dump_table_choice > 0)
         cout << "ReadCount " << name << ",\tvalue " << *data << endl;

      break;

   case ITEM_WRITER_DECLARED:
      if (dump_table_choice > 0)
         cout << "WriterDeclared " << name << ",\tvalue " << *data << endl;

      break;

   default:
      cout << "Symbol " << name << ", val " << data << ", type " << (int)type
           << ", group " << group << endl;
   }

   return true;
}


extern "C" void cds_dump (int choice = 0)
{
   dump_table_choice = choice;

   symEach (DataStore::getTable(), (FUNCPTR)DumpEntry, 0);
}

