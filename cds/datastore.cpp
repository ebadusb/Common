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
 * HISTORY:    Revision 1.30  2004/01/05 19:25:27Z  rm70006
 * HISTORY:    IT 6665.  Changed semaphore use to use mutex'es.
 * HISTORY:    Revision 1.29  2003/11/13 14:49:26Z  rm70006
 * HISTORY:    IT 6507.  Change Lock/Unlock scheme to solve deadlock issues.
 * HISTORY:    Revision 1.28  2003/11/06 17:12:35Z  rm70006
 * HISTORY:    IT 6507.  Put back in critical section code.
 * HISTORY:    Revision 1.27  2003/08/29 21:09:37Z  ms10234
 * HISTORY:    semaphore protected the datastore creation code.
 * HISTORY:    Revision 1.26  2003/07/11 21:21:37Z  ms10234
 * HISTORY:    5829 - Speed improvements for PFR
 * HISTORY:    Revision 1.25  2003/07/08 22:16:03Z  ms10234
 * HISTORY:    5829 - Changes for PFR
 * HISTORY:    Revision 1.24  2003/06/19 18:39:06Z  ms10234
 * HISTORY:    5829 - changes for PFR
 * HISTORY:    Revision 1.23  2003/04/07 14:21:21Z  rm70006
 * HISTORY:    IT 5818.
 * HISTORY:    
 * HISTORY:    Change logging levels.
 * HISTORY:    Revision 1.22  2002/12/16 18:31:30Z  jl11312
 * HISTORY:    - checked logging mode before logging semaphore information
 * HISTORY:    Revision 1.21  2002/11/18 18:29:10  jl11312
 * HISTORY:    - modifications to improve compile speed, reduce inline function sizes
 * HISTORY:    Revision 1.20  2002/11/06 15:43:56  rm70006
 * HISTORY:    Remove unnecessary new's.
 * HISTORY:    remove inline functions to fix compiler bug.
 * HISTORY:    fix bug in register call.
 * HISTORY:    Revision 1.19  2002/10/31 19:22:31Z  rm70006
 * HISTORY:    Changed internal structure to use less symbols which improved speed of datastore creation.
 * HISTORY:    Revision 1.18  2002/10/25 20:46:36Z  td07711
 * HISTORY:    support spoofer caching mechanism
 * HISTORY:    Revision 1.17  2002/10/18 19:58:14  rm70006
 * HISTORY:    Added new type of CDS for proc.
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

#include <vxWorks.h>
#include <stdio.h>
#include <wvLib.h>      // Needed for Event Timing

#include "datastore.h"
#include "error.h"
#include "common_datalog.h"
#include "crcgen.h"




#define DS_LOCK_EVENT      10
#define DS_UNLOCK_EVENT    30
#define DS_CREATE_EVENT    50
#define DS_PF_SAVE_GROUP   60
#define DS_PF_SAVE_ELEMENT 70


// Set to turn on event logging for timing info.
#define EVENT_TRACE 0

const char DATASTORE_SYMBOL_CONTAINER[]    = "_DataStore_%s_container";
const char BASE_ELEMENT_SYMBOL_CONTAINER[] = "_Base_Element_%s_ref_%d";
enum { DATASTORE_SYMTBL_SIZE = 6 };   // Create Max 64 datastore derived classes


////////////////////////////////////////////////////////////////
// ElementType
////////////////////////////////////////////////////////////////

ElementType::ElementType() :
_ds( NULL ),
_pfrType( PfrType( -1 ) )
{
}

ElementType::~ElementType()
{
}

bool ElementType::Register(DataStore *ds, PfrType pfr)
{
   _ds      = ds;
   _pfrType = pfr;

   return false;
}



////////////////////////////////////////////////////////////////
// DataStore
////////////////////////////////////////////////////////////////
#include <memLib.h>


DATASTORE_LISTTYPE DataStore::_pfrDataStoreList;
unsigned int DataStore::_pfCompleteLength=0;
unsigned char *DataStore::_pfCompleteDataPtr=0;
SYMTAB_ID DataStore::_datastoreTable = NULL;
bool DataStore::_logging = false;

DataStore::DataStore() :
   _handle( 0 ),
   _role( ROLE_RO ),
   _name( ),
   _refCount( 0 )
{
   DataLog_Critical _fatal;

   // Assert an error.
   DataLog(_fatal) << "Datastore default constructor was called." << endmsg;
   _FATAL_ERROR(__FILE__, __LINE__, "Datastore default constructor called.");
}

void DataStore::CreateSymbolTableEntry()
{
   const int MUTEX_SEM_FLAGS = SEM_Q_PRIORITY | SEM_DELETE_SAFE | SEM_INVERSION_SAFE;

   DataLog_Critical _fatal;

   //
   // Keep track of how many writers there are.  There should only be 1.
   //
   _handle->_writerDeclared = false;

   //
   // Create mutex semaphores
   //

   _handle->_semaphore = semMCreate(SEM_Q_PRIORITY | SEM_INVERSION_SAFE);

   // Fatal if _semaphore = NULL
   if (_handle->_semaphore == NULL)
   {
      DataLog(_fatal) << "_semaphore could not be created for " << _name 
         << ".  Errno " << errnoMsg << "." << endmsg;
      _FATAL_ERROR(__FILE__, __LINE__, "_semaphore could not be created.");
   }
   else
      DataLog(log_level_cds_debug) << "_semaphore value(" << hex << _handle->_semaphore << dec << ")." << endmsg;
}


static SEM_ID 	datastoreInitSem = semMCreate(SEM_Q_PRIORITY | SEM_INVERSION_SAFE);

DataStore::DataStore(const char *name, Role role) :
   _handle( 0 ),
   _role(role),
   _name( name ),
   _refCount(0)
{
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
   bool created;
   int event_type;

#if EVENT_TRACE == 1
   event_type = DS_CREATE_EVENT;
   wvEvent(event_type, (char *)_name.c_str(), _name.length());
#endif

   semTake( datastoreInitSem, WAIT_FOREVER );

   // Create the Symbol table.
   if (_datastoreTable == NULL)
   {
      _datastoreTable = symTblCreate(DATASTORE_SYMTBL_SIZE, false, memSysPartId);   // Create a symbol table that does NOT allow duplicate items.
   }

   //
   // Create the symbol table entry for this datastore instance
   //
   BindItem(this, &_handle, ITEM_DATASTORE_SYMBOL_CONTAINER, created);

   // If first time, populate handle
   if (created)
   {
      _handle->_name = name;
      _handle->_pfDataPtr = 0;
      _handle->_pfDataStoreLength = 0;
      _handle->_pfDataRestored = true;

      CreateSymbolTableEntry();

      // First derived datastore instance.  Save it in the list of datastores.
      // The list of datastores is used during by PFR to restore all datastores
      // (element values) to their last state.
      _pfrDataStoreList[ _name ] = _handle;

      DataLog(log_level_cds_debug) << "First instance of " << _name << " created.  Saving datastore." << endmsg;
   }

   semGive( datastoreInitSem );

#if EVENT_TRACE == 1
   wvEvent(++event_type, (char *)_name.c_str(), _name.length());
#endif
}

DataStore::~DataStore()
{
   // Don't delete anything.  The rest of the system is still using it.
}

unsigned int DataStore::SetAllPfData ( )
{
   //
   // Create the correct size of memory for the data
   //
   if ( PfMemoryBlock() == 0 )
   {
      DataLog( log_level_cds_error ) << "Datastore memory block creation failed." << endmsg;
      return 0;
   }

   unsigned int location=0;
   for (DATASTORE_LISTTYPE::iterator datastoreIterator = _pfrDataStoreList.begin(); datastoreIterator != _pfrDataStoreList.end(); ++datastoreIterator)
   {
#if EVENT_TRACE == 1
   int event_type = DS_PF_SAVE_GROUP;
   wvEvent(event_type, (char *)(*datastoreIterator).first.c_str(), (*datastoreIterator).first.length());
#endif

      if ( location+(*datastoreIterator).second->_pfDataStoreLength > _pfCompleteLength )
      {
         DataLog( log_level_cds_error ) << "PF Data size mismatch for " << (*datastoreIterator).first 
                                        << ".  expected->" << _pfCompleteLength << ", actual->" 
                                        << (location+(*datastoreIterator).second->_pfDataStoreLength) << endmsg;
         return location;
      }

      unsigned int actual = (*datastoreIterator).second->SetPfData( (unsigned char*)(_pfCompleteDataPtr+location) );
      if ( actual != (*datastoreIterator).second->_pfDataStoreLength )
      {
         DataLog( log_level_cds_error ) << "PF Data size mismatch for " << (*datastoreIterator).first
                                        << ".  expected->" << (*datastoreIterator).second->_pfDataStoreLength
                                        << ", actual->" << actual << endmsg;
         return location;
      }
      location += (*datastoreIterator).second->_pfDataStoreLength;
   }
   return location;
}

bool DataStore::RetrieveAllPfData ()
{
   //
   // Create the correct size of memory for the data
   //
   if ( PfMemoryBlock() == 0 )
   {
      DataLog( log_level_cds_error ) << "Datastore memory block creation failed." << endmsg;
      return false;
   }

   unsigned int location=0;
   for (DATASTORE_LISTTYPE::iterator datastoreIterator = _pfrDataStoreList.begin(); datastoreIterator != _pfrDataStoreList.end(); ++datastoreIterator)
   {
      DataLog(log_level_cds_debug) << "retrieving PF data for datastore " << (*datastoreIterator).first << " " << (*datastoreIterator).second << endmsg;
      if ( !( (*datastoreIterator).second->RetrievePfData( (unsigned char*)(_pfCompleteDataPtr+location) ) ) )
      {
         DataLog( log_level_cds_error ) << "PF data retrieve failed for datastore " << (*datastoreIterator).first << endmsg;
         return false;
      }
      location += (*datastoreIterator).second->_pfDataStoreLength;
   }
   return true;
}

bool DataStore::RestoreAllPfData()
{
   for (DATASTORE_LISTTYPE::iterator datastoreIterator = _pfrDataStoreList.begin(); datastoreIterator != _pfrDataStoreList.end(); ++datastoreIterator)
   {
      DataLog(log_level_cds_debug) << "restoring datastore " << (*datastoreIterator).first << endmsg;
      if ( !( (*datastoreIterator).second->RestorePfData() ) )
      {
         DataLog( log_level_cds_error ) << "PF data restore failed for datastore " << (*datastoreIterator).first << endmsg;
         return false;
      }
   }
   return true;
}

//
// Semaphore Take macro
//
#define SEM_TAKE(sem, wait)                                                           \
{                                                                                     \
   if ( !sem )                                                                        \
   {                                                                                  \
      _FATAL_ERROR(__FILE__, __LINE__, "SEM_TAKE on NULL semaphore ID");              \
   }                                                                                  \
                                                                                      \
   STATUS status = semTake(sem, wait);                                                \
   if (status != OK)                                                                  \
   {                                                                                  \
      DataLog_Critical _fatal;                                                        \
      DataLog(_fatal) << "SemTake failed.  Errno " << errnoMsg << ".  " << endmsg;    \
      _FATAL_ERROR(__FILE__, __LINE__, "semTake failed");                             \
   }                                                                                  \
}


//
// Semaphore Give macro
//
#define SEM_GIVE(sem)                                                                 \
{                                                                                     \
   if ( !sem )                                                                        \
   {                                                                                  \
      _FATAL_ERROR(__FILE__, __LINE__, "SEM_GIVE on NULL semaphore ID");              \
   }                                                                                  \
                                                                                      \
   STATUS status = semGive(sem);                                                      \
   if (status != OK)                                                                  \
   {                                                                                  \
      DataLog_Critical _fatal;                                                        \
      DataLog(_fatal) << "SemGive failed.  Errno " << errnoMsg << ".  " << endmsg;    \
      _FATAL_ERROR(__FILE__, __LINE__, "semGive failed");                             \
   }                                                                                  \
}


//
// Semaphore Flush macro
//
#define SEM_FLUSH(sem)                                                                \
{                                                                                     \
   if ( !sem )                                                                        \
   {                                                                                  \
      _FATAL_ERROR(__FILE__, __LINE__, "SEM_FLUSH on NULL semaphore ID");             \
   }                                                                                  \
                                                                                      \
   STATUS status = semFlush(sem);                                                     \
   if (status != OK)                                                                  \
   {                                                                                  \
      DataLog_Critical _fatal;                                                        \
      DataLog(_fatal) << "SemFlush failed.  Errno " << errnoMsg << ".  " << endmsg;   \
      _FATAL_ERROR(__FILE__, __LINE__, "semFlush failed");                            \
   }                                                                                  \
}



//
// Lock
//
void DataStoreSymbolContainer::Lock( Role role )
{
#if EVENT_TRACE == 1
   wvEvent(DS_LOCK_EVENT, (char *)_name.c_str(), _name.length());
#endif

   SEM_TAKE(_semaphore, WAIT_FOREVER);
}



//
// Unlock
//
void DataStoreSymbolContainer::Unlock( Role role )
{
#if EVENT_TRACE == 1
   wvEvent(DS_UNLOCK_EVENT, (char *)_name.c_str(), _name.length());
#endif

   SEM_GIVE(_semaphore);
}



//
// AddElement
//
void DataStore::AddElement (BaseElementSymbolContainerAbs *element, unsigned int size )
{ 
   DataLog( log_level_cds_debug ) << "Add element to datastore " << _name << endmsg;

   //
   // Add the crc to the size totals once, determined if this
   //  is the first time adding an element to the pfr list.
   //
   if ( _handle->_pfDataStoreLength == 0 )
   {
      //
      // Add on a CRC specific to this datastore and
      //  accumulate it in the total size of all datastores
      //
      _handle->_pfDataStoreLength = sizeof( unsigned long );  // Spacer at the beginning
      _handle->_pfDataStoreLength += _name.size();            // Add the CDS name
      _handle->_pfDataStoreLength += sizeof( unsigned long ); // Spacer in between the name and the data
      _handle->_pfDataStoreLength += sizeof( unsigned long ); // Spacer between the data and the CRC
      _handle->_pfDataStoreLength += sizeof( unsigned long ); // Add the CRC

      _pfCompleteLength += _handle->_pfDataStoreLength;       // Add this Datastore's overhead to the total size
   }

   //
   // Add the element ...
   //
   _handle->_pfrList.push_back(element); 

   //
   // Increment the size variables, both for the overall and this particular datastore
   //
   _handle->_pfDataStoreLength += size; 
   _pfCompleteLength += size; 
}

unsigned int DataStoreSymbolContainer::SetPfData( unsigned char *memBlock )
{
   //
   // No need to mess with a DataStore object with no PFR elements ...
   //
   if ( _pfDataStoreLength == 0 )
      return 0;

   unsigned int size=0;

   //
   // Make sure we have data to restore ...
   //
   if ( !memBlock )
   {
      DataLog( log_level_cds_error ) << "NULL memory block pointer for datastore " << _name << endmsg;
      return false;
   }

   unsigned long separator=0xAAAAAAAA;
   memcpy( (memBlock+size), &separator, sizeof( separator ) ); size += sizeof( separator );
   char namechar;
   for (int i=0 ; i<_name.size() ; i++ )
   {
      namechar = _name[i];
      memcpy( (memBlock+size), &namechar, sizeof( char ) ); size += sizeof( char );
   }
   separator=0xBBBBBBBB;
   memcpy( (memBlock+size), &separator, sizeof( separator ) ); size += sizeof( separator );

   int count = 0;
   unsigned int dataSize=0;
   Lock( ROLE_RO );
   for (ELEMENT_LISTTYPE::iterator pfrListIterator = _pfrList.begin(); pfrListIterator != _pfrList.end(); ++pfrListIterator)
   {

#if EVENT_TRACE == 1
   int event_type = DS_PF_SAVE_ELEMENT;
   wvEvent(event_type, (char *)_name.c_str(), _name.length());
#endif

      count++;
      dataSize = (*pfrListIterator)->Save( memBlock+size );
      size += dataSize;
   }
   Unlock( ROLE_RO );
   separator=0xCCCCCCCC;
   memcpy( (memBlock+size), &separator, sizeof( separator ) ); size += sizeof( separator );

   //
   // Generate the CRC for the memory block ...
   //
   unsigned long CRC=0;
   crcgen32( &CRC, memBlock, size );
   
   //
   // Set the CRC as the last entry in the block ...
   //
   memcpy( (memBlock+size), &CRC, sizeof( CRC ) );
   size += sizeof( CRC );

   //
   // Return the size of the block used for our data
   //
   return size;
}

bool DataStoreSymbolContainer::RetrievePfData( unsigned char *memBlock )
{
   //
   // No need to mess with a DataStore object with no PFR elements ...
   //
   if ( _pfDataStoreLength == 0 )
      return true;

   //
   // Make sure we have data to restore ...
   //
   if ( !memBlock )
   {
      DataLog( log_level_cds_error ) << "NULL memory block pointer for datastore " << _name << endmsg;
      return false;
   }

   //
   // Save the location of the data ...
   //
   _pfDataPtr = memBlock;
   _pfDataRestored = false;

   //
   // Check the CRC of the memory block ...
   //
   unsigned long actualCRC=0;
   crcgen32( &actualCRC, memBlock, _pfDataStoreLength - sizeof(unsigned long) );
   unsigned long storedCRC = *((unsigned long*)(memBlock+_pfDataStoreLength-sizeof(unsigned long)));
   if ( actualCRC != storedCRC )
   {
      DataLog( log_level_cds_error ) << "PF Data CRC mismatch for datastore " << _name << ".  expected->" << storedCRC 
                                     << ", actual->" << actualCRC << endmsg;
      return false;
   }

   return true;
}

bool DataStoreSymbolContainer::RestorePfData( )
{
   //
   // No need to mess with a DataStore object with no PFR elements ...
   //
   if ( _pfDataStoreLength == 0 )
   {
      DataLog( log_level_cds_debug ) << "Data length for datastore " << _name << " was 0." << endmsg;
      return true;
   }

   //
   // If we have already restored or reset the CDS, then we don't need to do anything,
   //  so just skip it ...
   //
   if ( _pfDataRestored )
   {
      DataLog( log_level_cds_info ) << "Data for datastore " << _name << " has already been restored." << endmsg;
      return true;
   }

   //
   // Make sure we have data to restore ...
   //
   if ( !_pfDataPtr )
   {
      DataLog( log_level_cds_error ) << "NULL PF Data pointer for datastore " << _name << endmsg;
      return false;
   }

   unsigned int pos = ( 2*sizeof( unsigned long ) ) + _name.size();
   int count = 0;
   Lock( ROLE_RW );
   for (ELEMENT_LISTTYPE::iterator pfrListIterator = _pfrList.begin(); pfrListIterator != _pfrList.end(); ++pfrListIterator)
   {
      count++;
      DataLog(log_level_cds_debug) << "restoring element " << count << " in " << _name << " to PFR File." << endmsg;
      pos += (*pfrListIterator)->Restore( _pfDataPtr+pos );
   }
   Unlock( ROLE_RW );

   DataLog(log_level_cds_info) << "restored " << count << " elements in " << _name << "." << endmsg;
   return true;
}

bool DataStoreSymbolContainer::ForgetPfData( )
{
   _pfDataRestored = true;
   _pfDataPtr = 0;
   return true;
}

void DataStore::GetSymbolName(string &s, const BIND_ITEM_TYPE item)
{
   int size = 0;
   const int s_len = s.size();
   DataLog_Critical _fatal;                                                        

   // Create the Symbol name to search for.
   switch (item)
   {
   case ITEM_DATASTORE_SYMBOL_CONTAINER:
      size = sprintf((char *)s.c_str(), DATASTORE_SYMBOL_CONTAINER, _name.c_str());
      break;

   case ITEM_BASE_ELEMENT_SYMBOL_CONTAINER:
      size = sprintf((char *)s.c_str(), BASE_ELEMENT_SYMBOL_CONTAINER, _name.c_str(), _refCount);
      ++_refCount;
      break;

   default:
      DataLog(_fatal) << "Unknown symbol type(" << item << ")" << "in " << Name() << "." << endmsg;
      _FATAL_ERROR(__FILE__, __LINE__, "Unknown symbol type.");
   }

   if (size > s_len)
   {
      // Dude.  You just wiped out something.
      DataLog(_fatal) << "datastore item(" << item << ") name size mismatch in " << Name() 
                       << ".  Generated size is " << size << ".  Available size is " << s_len << "." << endmsg;
      _FATAL_ERROR(__FILE__, __LINE__, "Unknown symbol type.");
   }

}


SingleWriteDataStore::SingleWriteDataStore(const char * name, Role role) :
   DataStore (name, role)
{
   // Ensure no multiple writers
   CheckForMultipleWriters();
}

SingleWriteDataStore::~SingleWriteDataStore()
{
   if (GetRole() == ROLE_RW)
   {
      _handle->_writerDeclared = false;
      DataLog(log_level_cds_info) << "Writer exited for CDS " << Name() << "." << endmsg;
   }
}


void SingleWriteDataStore::CheckForMultipleWriters()
{
   if (GetRole() == ROLE_RW)
   {
      // The base implementation of DataStore fatal errors when multiple writers are declared.
      if (_handle->_writerDeclared)
      {
         // This is an error.
         DataLog_Critical _fatal;                                                        \
         DataLog(_fatal) << "Error.  Multiple Writers Declared for CDS " << Name() << ".  Abort!!!!!!" << endmsg;
         _FATAL_ERROR(__FILE__, __LINE__, "Datastore multiple writers");
         return;
      }
      else
         _handle->_writerDeclared = true;
   }
}


MultWriteDataStore::MultWriteDataStore(const char * name, Role role) :
   DataStore (name, role)
{
   // Empty check.  Included for completeness.
   CheckForMultipleWriters();
}


MultWriteDataStore::~MultWriteDataStore()
{
}



DynamicSingleWriteDataStore::DynamicSingleWriteDataStore(const char * name, Role role) :
   SingleWriteDataStore (name, role)
{
}


DynamicSingleWriteDataStore::~DynamicSingleWriteDataStore()
{
}


void DynamicSingleWriteDataStore::SetRead()
{
   // If you are currently the writer, then unset the _writerDeclared flag
   if (_role == ROLE_RW)
   {
      _handle->_writerDeclared = false;
   }

   _role = ROLE_RO;
}

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
#if 0
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

   case ITEM_SPOOF_CACHE:
      if (dump_table_choice > 0)
         cout << "SpoofCache " << name << ",\tvalue " << *data << endl;
    
      break;
#endif

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

