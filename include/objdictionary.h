/*
 * Copyright (c) 2000 by Gambro BCT, Inc.  All rights reserved.
 *
 * TITLE:      objdictionary.h
 *
 * ABSTRACT:   
 *
 */

#ifndef _OBJDICTIONARY_H
#define _OBJDICTIONARY_H

#include <list>
#include <string>
#include "datalog.h"

#ifdef WIN32
using namespace std;
#endif


typedef void *(*NewObjFcnPtr)();

class ObjDictionaryEntry
{
public:

   ObjDictionaryEntry( const string &n, NewObjFcnPtr fcnPtr );
   ObjDictionaryEntry( const char* n, NewObjFcnPtr fcnPtr );

   ObjDictionaryEntry( const ObjDictionaryEntry &d );

   virtual ~ObjDictionaryEntry();

   ObjDictionaryEntry &operator=( const ObjDictionaryEntry &d );

   virtual void *newObject();
   
   const string &className() const { return _ClassName; }
   void className( const string &n ) { _ClassName = n; }
   void className( const char* n ) { _ClassName = n; }

   int operator==( const ObjDictionaryEntry &d );

   int operator==( const string &n );
   int operator==( const char *n );

   int operator<( const ObjDictionaryEntry &d );

   int operator<( const string &n );
   int operator<( const char *n );

   int compare( const string &n );
   int compare( const char *n );
   
protected:

   //
   // Delete the memory 
   //
   void cleanup();

protected:

   string _ClassName;

   NewObjFcnPtr _NewObjFcnPtr;
   
};


class ObjDictionary
{
public:

   static void add( ObjDictionaryEntry* d );

   static void *create( const char *n );

   static int valid( const char *n );

   static void dump( DataLog_Stream &outs );

protected:

   static ObjDictionaryEntry *findEntry( const char *n );

private:

   static list< ObjDictionaryEntry* > _ObjEntries;

};

#define DECLARE_OBJ( __objclassname__ ) \
   \
public: \
   \
   static void *newObject() \
   {  \
      return new __objclassname__( ); \
   }; \
   \
// End of DECLARE_OBJ macro


#define DEFINE_OBJ( __objclassname__ ) \
   \
   static ObjDictionaryEntry entry##__objclassname__\
                    ( #__objclassname__, &__objclassname__::newObject ); \
   \
// End of DEFINE_OBJ macro


#endif
