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

#ifndef __DP_ELEMENT
#define __DP_ELEMENT

#include <fstream.h>
#include <string>


enum PfrType {PFR_RECOVER, NO_PFR};
enum Role    {ROLE_RO,     ROLE_RW, ROLE_SPOOFER};




// Forward Reference
class DataStore;


//
// Encapsulation of an element.
//
class ElementType
{
   friend class DataStore;

// Class Methods
protected:
   ElementType ();  // Hide default constructor
   virtual ~ElementType();

   void Register (DataStore *ds, Role role, PfrType pfr);

   virtual void ReadSelf  (ifstream &pfrfile) = 0;
   virtual void WriteSelf (ofstream &pfrfile) = 0;

// Data Members
protected:
   Role       _role;
   PfrType    _pfrType;
   DataStore *_ds;
};




//
// Standard Element Class.  Contains base functions for creating and managing elements.
// Can be extended by derived classes to handle structs, etc.
//
template <class dataType> class BaseElement : public ElementType
{
// Class Methods
public:
   typedef const dataType & (*FP)(const ElementType &);

   BaseElement ();

   virtual ~BaseElement();

   virtual const dataType & Get() const;

   virtual bool     Set(const dataType &data);

   void SetSpoof   (FP fp);
   void ClearSpoof ();

   virtual void Register (DataStore *ds, Role role, PfrType pfr);
   virtual void Register (DataStore *ds, Role role, PfrType pfr, const dataType &initValue);

// Class Methods
protected:
   virtual void ReadSelf  (ifstream &pfrfile);
   virtual void WriteSelf (ofstream &pfrfile);

// Data Members
private:
   dataType *_data;  // Points to the Symbol Table entry
   FP       *_fp;    // Points to the Symbol Table entry
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

   virtual void Register (DataStore *ds, Role role, PfrType pfr, const dataType min, const dataType max);
   virtual void Register (DataStore *ds, Role role, PfrType pfr, const dataType min, const dataType max, const dataType &initValue);

// Data Members
private:
   dataType  _min;
   dataType  _max;
};



//
// Base CDS Container Class.  All CDS Containers are derived from this class.
//
#include <list>

#include <symLib.h>

#include "datalog.h"

// Forward Reference
class DataStore;

typedef list<ElementType *> ELEMENT_LISTTYPE;
typedef list<DataStore *>   DATASTORE_LISTTYPE;

enum BIND_ITEM_TYPE
{  ITEM_DATA,
   ITEM_SPOOF,
   ITEM_PFR_LIST,
   ITEM_READ_COUNT_SEMAPHORE,
   ITEM_WRITE_COUNT_SEMAPHORE,
   ITEM_SIGNAL_WRITE,
   ITEM_READ_COUNT,
   ITEM_WRITER_DECLARED
};

class DataStore
{
// Data Members
public:
   static DataLog_Level    *_debug;
   static DataLog_Critical *_fatal;


// Class Methods
public:
   friend class ElementType;

   static void SavePfrData (ofstream &pfrFile);
   static void RestorePfrData (ifstream &pfrFile);

   void Lock();
   void Unlock();

   const string & Name () const {return _name; }

   void GetSymbolName (string &s, const BIND_ITEM_TYPE item);

   void AddElement (ElementType *member);

   static const SYMTAB_ID & getTable() { return _datastoreTable; }
   
// Class Methods
protected:
   DataStore (char *name, Role role);
   virtual ~DataStore();

   void DeleteElement (ElementType *member);
   virtual void CheckForMultipleWriters();

// Class Methods
private:
   DataStore();    // Base Constructor not available


private:
// Data Members

   // instance vars
   Role _role;
   string _name;
   
   int _refCount;
   int _spoofCount;

   // instance vars connected to the global symbol table
   bool *_writerDeclared;
   
   // Mutex control flags
   int *_signalWrite;
   int *_readCount;

   // Mutex semaphores
   SEM_ID *_writeSemaphore;
   SEM_ID *_readCountSemaphore;

   // List of PFR elements
   ELEMENT_LISTTYPE *_pfrList;
   
   // static class vars
   static DATASTORE_LISTTYPE _datastoreList;
   static SYMTAB_ID          _datastoreTable;

};



class MultWriteDataStore : public DataStore
{
// Class Methods
protected:
   MultWriteDataStore (char *name, Role role);
   virtual ~MultWriteDataStore();

   virtual void CheckForMultipleWriters() { };

private:
   MultWriteDataStore();    // Base Constructor not available
};


#include "datastore_private.h"


#endif
