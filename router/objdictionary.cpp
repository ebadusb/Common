/*
 * Copyright (c) 2000 by Gambro BCT, Inc.  All rights reserved.
 *
 * TITLE:      objdictionary.cpp
 *
 */

#include <vxWorks.h>

#include "datalog.h"
#include "error.h"
#include "objdictionary.h"

#include <stdio.h>
#include <iostream.h>


list< ObjDictionaryEntry* > ObjDictionary::_ObjEntries = list< ObjDictionaryEntry* >();


ObjDictionaryEntry :: ObjDictionaryEntry( const char* n, NewObjFcnPtr fcnPtr ) :
_ClassName( 0 ),
_NewObjFcnPtr( fcnPtr )
{
   ObjDictionary::add( this );
   strcpy(_ClassName,n);
};

ObjDictionaryEntry :: ObjDictionaryEntry( const ObjDictionaryEntry &d ) :
_ClassName( 0 )
{
   strcpy(_ClassName,d.className());
   ObjDictionary::add( this );
};

ObjDictionaryEntry :: ~ObjDictionaryEntry()
{
   cleanup();
};

ObjDictionaryEntry &ObjDictionaryEntry :: operator=( const ObjDictionaryEntry &d )
{
   if ( &d != this )
   {
      className( d.className() );
   }
   return *this;
};

void *ObjDictionaryEntry :: newObject()
{
   return (*_NewObjFcnPtr)( _ClassName );
}

int ObjDictionaryEntry :: operator==( const ObjDictionaryEntry &d )
{
   return operator==( d.className() );
};

int ObjDictionaryEntry :: operator==( const char *n )
{
   return ( compare( n )==0 ? 1 : 0 );
};

int ObjDictionaryEntry :: operator<( const ObjDictionaryEntry &d )
{
   return operator<( d.className() );
};

int ObjDictionaryEntry :: operator<( const char *n )
{
   return ( compare( n )<0 ? 1 : 0 );
};

int ObjDictionaryEntry :: compare( const char *n )
{
   return strcmp( _ClassName, n );
};

void ObjDictionaryEntry :: cleanup()
{
   className( 0 );
};



void ObjDictionary :: add( ObjDictionaryEntry *d )
{
   _ObjEntries.push_back( d );
}

void *ObjDictionary :: create( const char *n)
{
   ObjDictionaryEntry *e = findEntry( n );
   if ( !e )
   {
      DataLog_Level log(LOG_ERROR);
      DataLog(log) << "Object not found -> " << n << endmsg;
      return 0;
   }
   return e->newObject( );
}

int ObjDictionary :: valid( const char *n )
{
   
   list< ObjDictionaryEntry* >::iterator entry;
   for ( entry = _ObjEntries.begin();
         entry != _ObjEntries.end() ;
         entry++ ) 
   {
      if ( *(*entry) == n )
      {
         return 1;
      }
   }
   return 0;
}

void ObjDictionary :: dump( ostream &outs )
{
   list< ObjDictionaryEntry* >::iterator entry;
   for ( entry = _ObjEntries.begin();
         entry != _ObjEntries.end() ;
         entry++ ) 
   {
      outs << "objdictionary obj: " <<  (char *)(*entry)->className() << endmsg;
   }

}

ObjDictionaryEntry *ObjDictionary :: findEntry( const char *n )
{
   
   list< ObjDictionaryEntry* >::iterator entry;
   for ( entry = _ObjEntries.begin();
         entry != _ObjEntries.end() ;
         entry++ ) 
   {
      if ( *(*entry) == n )
      {
         return (*entry);
      }
   }
   return 0;
}

