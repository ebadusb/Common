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
#include <string.h>

#ifdef WIN32
using namespace std;
#endif


typedef void *(*NewObjFcnPtr)( const char * );

class ObjDictionaryEntry
{
public:

   ObjDictionaryEntry( const char* n, NewObjFcnPtr fcnPtr );

   ObjDictionaryEntry( const ObjDictionaryEntry &d );

   virtual ~ObjDictionaryEntry();

   ObjDictionaryEntry &operator=( const ObjDictionaryEntry &d );

   virtual void *newObject();
   
   const char* className() const { return _ClassName; };
   void className( const char* n ) 
   {        
      if ( _ClassName != 0 )
      {
         free( _ClassName );
         _ClassName = 0;
      }
      if ( n != 0 )
         strcpy(_ClassName, n ); 
   };

   int operator==( const ObjDictionaryEntry &d );

   int operator==( const char *n );

   int operator<( const ObjDictionaryEntry &d );

   int operator<( const char *n );

   int compare( const char *n );
   
protected:

   //
   // Delete the memory 
   //
   void cleanup();

protected:

   char         *_ClassName;

   NewObjFcnPtr _NewObjFcnPtr;
   
};


class ObjDictionary
{
public:

   static void add( ObjDictionaryEntry* d );

   static void *create( const char *n );

   static int valid( const char *n );

   static void dump( ostream &outs );

protected:

   static ObjDictionaryEntry *findEntry( const char *n );

private:

   static list< ObjDictionaryEntry* > _ObjEntries;

};

#define DECLARE_OBJ( __objclassname__ ) \
   \
public: \
   static void *newObject( const char *name=0 ) \
   {  \
      return new __objclassname__##( name ); \
   }; \
   \
   static const char *className() { return (const char *)__ClassName; }; \
   \
private: \
   static char *__ClassName; \
// End of DECLARE_OBJ macro


#define DEFINE_OBJ( __objclassname__ ) \
   \
   static ObjDictionaryEntry entry##__objclassname__\
                    ( #__objclassname__, &__objclassname__##::newObject ); \
   char * __objclassname__##::__ClassName=#__objclassname__; \
   \
// End of DEFINE_OBJ macro


#endif
