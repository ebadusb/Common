/*******************************************************************************
 *
 * Copyright (c) 2002 by Gambro BCT, Inc.  All rights reserved.
 *
 * TITLE:      datastore.h
 *             Common Data Store Interface
 *
 * AUTHOR:     Regis McGarry
 *
 * ABSTRACT:   This file defines the Data Store primitives from which Common Data
 *             Stores are made.
 *
 * HISTORY:    $Log: datastore.h $
 * HISTORY:    Revision 1.26  2003/07/22 14:51:29Z  rm70006
 * HISTORY:    IT 6222.  Suspected bug in get and equal operator.
 * HISTORY:    Revision 1.25  2003/07/08 22:15:39Z  ms10234
 * HISTORY:    5829 - changes for PFR
 * HISTORY:    Revision 1.24  2003/06/19 18:38:29Z  ms10234
 * HISTORY:    5829 - Changes for PFR
 * HISTORY:    Revision 1.23  2003/05/19 17:11:06Z  ms10234
 * HISTORY:    5816 - changed Access function to be usable with const objects
 * HISTORY:    Revision 1.22  2003/04/07 14:46:59Z  rm70006
 * HISTORY:    IT 5818.
 * HISTORY:    
 * HISTORY:    Fix log levels.
 * HISTORY:    Revision 1.21  2002/11/18 18:29:28Z  jl11312
 * HISTORY:    - modifications to improve compile speed, reduce inline function sizes
 * HISTORY:    Revision 1.20  2002/11/07 00:11:35Z  td07711
 * HISTORY:    modified spoofer caching - added setSpooferCacheValid() and isSpooferCacheValid()
 * HISTORY:    Revision 1.19  2002/11/06 15:42:47  rm70006
 * HISTORY:    Removed unnecessary new's.
 * HISTORY:    Removed some inline functions to relieve compiler problems.
 * HISTORY:    fixed bug in register call.
 * HISTORY:    Revision 1.18  2002/10/31 19:25:58Z  rm70006
 * HISTORY:    Changed internal stucture to use less symbols which improved datastore creation speed.
 * HISTORY:    Revision 1.17  2002/10/25 20:45:21Z  td07711
 * HISTORY:    support spoofer caching mechanism
 * HISTORY:    Revision 1.16  2002/10/21 20:19:16  rm70006
 * HISTORY:    Added = operator for ranged element type.
 * HISTORY:    Revision 1.15  2002/10/18 23:16:01Z  td07711
 * HISTORY:    use CallbackBase class for spoofing callback
 * HISTORY:    Revision 1.14  2002/10/18 20:00:49  rm70006
 * HISTORY:    Add new cds type for proc.  this version allows dynamic roles.
 * HISTORY:    Revision 1.13  2002/10/17 20:14:47Z  rm70006
 * HISTORY:    Added = and access routines that provide implicit get and set.
 * HISTORY:    Revision 1.12  2002/09/25 16:03:16Z  rm70006
 * HISTORY:    Added GetRef call for Config DataStore
 * HISTORY:    Revision 1.11  2002/09/24 16:46:09Z  rm70006
 * HISTORY:    Add extra debugging ability.
 * HISTORY:    Revision 1.10  2002/09/19 16:04:10Z  rm70006
 * HISTORY:    Added fast get for large datastore items.
 * HISTORY:    Revision 1.9  2002/09/18 22:13:21Z  rm70006
 * HISTORY:    Change Get and Set to have built in locking.
 * HISTORY:    Revision 1.8  2002/09/13 20:09:04Z  rm70006
 * HISTORY:    Fix bug with lock/unlock.
 * HISTORY:    Revision 1.7  2002/09/04 18:31:28Z  rm70006
 * HISTORY:    Added accessor function for role.
 * HISTORY:    Revision 1.6  2002/09/03 14:36:33Z  rm70006
 * HISTORY:    Added new single write class.
 * HISTORY:    Revision 1.5  2002/08/23 14:53:21Z  rm70006
 * HISTORY:    Changed binditem to work with 486 compiler bug.
 * HISTORY:    Revision 1.4  2002/07/16 21:05:02Z  rm70006
 * HISTORY:    Fix bug in check for multiple writers.
 * HISTORY:    Revision 1.3  2002/07/02 19:29:37Z  rm70006
 * HISTORY:    Made register virtual function for overriding by derived classes.
 * HISTORY:    Revision 1.2  2002/07/02 16:03:44Z  rm70006
 * HISTORY:    Added new non-write-restrictive class to CDS.
 * HISTORY:    Revision 1.1  2002/06/24 19:30:44Z  rm70006
 * HISTORY:    Initial revision
 * HISTORY:    Revision 1.4  2002/06/17 18:40:41Z  rm70006
 * HISTORY:    Change design to make data store instance based instead of static.  Changed from taskVars to using the symbol table library.
 * HISTORY:    Revision 1.3  2002/06/04 18:52:50Z  rm70006
 * HISTORY:    Remove unneeded version of get
 * HISTORY:    Revision 1.2  2002/06/04 17:18:21Z  rm70006
 * HISTORY:    Added logging and new const ref get function.
 * HISTORY:    Revision 1.1  2002/05/28 19:00:26Z  rm70006
 * HISTORY:    Initial revision
*******************************************************************************/

#ifndef _DATASTORE_INCLUDE
#define _DATASTORE_INCLUDE


#include <fstream.h>
#include <string>

#include "callback.h"


enum PfrType {PFR_RECOVER, NO_PFR};
enum Role    {ROLE_RO,     ROLE_RW, ROLE_SPOOFER};




// Forward Reference
class DataStore;

//
// Encapsulation of an element.
//
class ElementType
{
public:
   friend class DataStore;

   PfrType pfrType() { return _pfrType; }

// Class Methods
protected:
   ElementType ();  // Hide default constructor
   virtual ~ElementType();

   bool Register (DataStore *ds, PfrType pfr);

private:

   // ElementType ( const ElementType &cElem );           // not implemented 
   // ElementType &operator =( const ElementType &cElem );  // not implemented

// Data Members
protected:

   PfrType    _pfrType;
   DataStore *_ds;

};

class BaseElementSymbolContainerAbs
{
public:

   virtual unsigned int Restore ( unsigned char *from ) = 0; // return size 
   virtual unsigned int Save    ( unsigned char *to ) = 0;   // return size

};

//
// data element instance vars connected to the global symbol table
//
template <class dataType> class BaseElementSymbolContainer : public BaseElementSymbolContainerAbs
{
public:

   inline BaseElementSymbolContainer<dataType>() : _data(0), _fp(0), _spooferCacheIsValid(false) {}

   virtual unsigned int Restore ( unsigned char *from ); // return size of dataType restored
   virtual unsigned int Save    ( unsigned char *to );   // return size of dataType saved

   dataType             *_data;                 // Pointer to data
   const CallbackBase  **_fp;                   // Points to the callback function
   bool                  _spooferCacheIsValid;  // if true, spoofer get() is bypassed to avoid unecessary copy 

};



//
// Standard Element Class.  Contains base functions for creating and managing elements.
// Can be extended by derived classes to handle structs, etc.
//
template <class dataType> class BaseElement : public ElementType
{
// Class Methods
public:

   BaseElement ();

   virtual ~BaseElement();

   virtual dataType Get() const;
   virtual void     Get(dataType *item) const;  // Faster version.  Better for large data items.

   inline operator const dataType () const { return Get(); } // Implicit get call.

   virtual bool Set(const dataType &data);

   inline dataType const operator = (const dataType &data) { Set(data); return Get();}  // Implicit Set call.

   void SetSpoof   (const CallbackBase* fp);
   void ClearSpoof ();

   // returns 'true' if it was created already
   virtual bool Register (DataStore *ds, PfrType pfr);
   virtual bool Register (DataStore *ds, PfrType pfr, const dataType &initValue);

   // these two functions for use by spoofer to avoid unnecessary data copies
   inline void setSpooferCacheValid() { _handle->_spooferCacheIsValid = true; };
   inline bool isSpooferCacheValid() { return _handle->_spooferCacheIsValid; };

// Class Methods
protected:
   inline const dataType & GetRef() const { return *_handle->_data; };

	enum AccessOp { LockAccess, UnlockAccess };
	void Access(AccessOp op) const;

// Class Methods
private:
   void CreateSymbolTableEntry();

   // BaseElement ( const BaseElement &cElem );                 // not implemented
   // BaseElement &operator =( const BaseElement &cElem ); // not implemented

// Data Members
private:

   BaseElementSymbolContainer<dataType> *_handle;
};



//
// Derived BaseElement class that handles int/float/double types that have defined 
// ranges that need to be checked.
//
template <class dataType> class RangedElement : public BaseElement <dataType>
{
// Class Methods
public:
   RangedElement();

   virtual ~RangedElement();

   virtual bool Set(const dataType &data);   // Perform range check on set call.

   virtual bool Register (DataStore *ds, PfrType pfr, const dataType min, const dataType max);
   virtual bool Register (DataStore *ds, PfrType pfr, const dataType min, const dataType max, const dataType &initValue);

   inline dataType operator = (const dataType &data) { Set(data); return Get();}  // Implicit Set call.

// Data Members
private:
   dataType  _min;
   dataType  _max;
};



//
// Base CDS Container Class.  All CDS Containers are derived from this class.
//
#include <list>
#include <map>
#include <symLib.h>


enum BIND_ITEM_TYPE
{  
   ITEM_DATASTORE_SYMBOL_CONTAINER,
   ITEM_BASE_ELEMENT_SYMBOL_CONTAINER
};

typedef list<BaseElementSymbolContainerAbs *> ELEMENT_LISTTYPE;

//
// datastore instance vars connected to the global symbol table
//
class DataStoreSymbolContainer
{
public:

   void Lock( Role role );
   void Unlock( Role role );

   unsigned int SetPfData(  unsigned char *memBlock ); // return length
   bool RetrievePfData( unsigned char *memBlock );
   bool RestorePfData();
   bool ForgetPfData();

   string _name;

   // List of PFR elements
   ELEMENT_LISTTYPE _pfrList;
   unsigned char *_pfDataPtr;
   unsigned int _pfDataStoreLength;
   bool _pfDataRestored;
      
   // Mutex semaphores
   SEM_ID _readSemaphore;
   SEM_ID _writeSemaphore;
      
   // Mutex control flags
   int  _readCount;

   // Flag tracking number of writers.
   bool _writerDeclared;
};

typedef map< string, DataStoreSymbolContainer *> DATASTORE_LISTTYPE;


//
// Base Datastore class
//
class DataStore
{
// Class Methods
public:
   friend class ElementType;

   //
   // Static PFR methods
   static unsigned int SetAllPfData( ); // return length
   static bool RetrieveAllPfData( );
   static bool RestoreAllPfData ( );
   static unsigned int PfCompleteLength() { return _pfCompleteLength; }
   static unsigned char *PfMemoryBlock() { if ( !_pfCompleteDataPtr ) _pfCompleteDataPtr = new unsigned char [ _pfCompleteLength ]; return _pfCompleteDataPtr; }

   void Lock() { _handle->Lock( _role ); }
   void Unlock() { _handle->Unlock( _role ); }

   void AddElement (BaseElementSymbolContainerAbs *element, unsigned int size );

   //
   // Non-static PFR method
   bool RestorePfData() { return _handle->RestorePfData(); }
   bool ForgetPfData() { return _handle->ForgetPfData(); }

   // Accessor functions
   static const SYMTAB_ID & getTable() { return _datastoreTable; }
   inline const string    & Name () const {return _name; }
   inline const Role      & GetRole () const { return _role; }
   
   void GetSymbolName (string &s, const BIND_ITEM_TYPE item);

// Class Methods
protected:
   DataStore (const char *name, Role role);
   virtual ~DataStore();

   virtual void CheckForMultipleWriters() = 0;

// Data Members
protected:

   // instance vars connected to the global symbol table
   DataStoreSymbolContainer *_handle;

   Role _role;


// Class Methods
private:
   DataStore();    // Base Constructor not available

   void CreateSymbolTableEntry();

// Data Members
private:

   // instance vars
   string _name;

   int _refCount;

   // static class vars
   static DATASTORE_LISTTYPE _pfrDataStoreList;
   static unsigned int       _pfCompleteLength;
   static unsigned char      *_pfCompleteDataPtr; // I own the allocated memory
   static SYMTAB_ID          _datastoreTable;

   static bool _logging;
};



//
// Single Write Datastore
// This Datastore type only allows one writer per derived datastore
//
class SingleWriteDataStore : public DataStore
{
// Class Methods
protected:
   SingleWriteDataStore (const char *name, Role role);
   virtual ~SingleWriteDataStore();

   virtual void CheckForMultipleWriters();

private:
   SingleWriteDataStore();    // Base Constructor not available
};



//
// Multi-Write Datastore
// This Datastore type allows any number of writers.
//
class MultWriteDataStore : public DataStore
{
// Class Methods
protected:
   MultWriteDataStore (const char *name, Role role);
   virtual ~MultWriteDataStore();

   inline virtual void CheckForMultipleWriters() { }

private:
   MultWriteDataStore();    // Base Constructor not available
};



//
// Dynamic-Single-Write Datastore
// This Datastore type allows datastore instances to change their role dynamically.
// The Datastore still only allows a single writer.
// Created instances default to READ-ONLY
//
class DynamicSingleWriteDataStore : public SingleWriteDataStore
{
// Class Methods
public:
   virtual void SetWrite();
   virtual void SetRead();

// Class Methods
protected:
   DynamicSingleWriteDataStore (const char *name, Role role);
   virtual ~DynamicSingleWriteDataStore();

private:
   DynamicSingleWriteDataStore();    // Base Constructor not available
};

#include "datastore_private.h"

#endif /* ifndef _DATASTORE_INCLUDE */

