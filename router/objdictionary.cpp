/*
 * Copyright (c) 2000 by Gambro BCT, Inc.  All rights reserved.
 *
 * TITLE:      objdictionary.cpp
 *
 */

#include <vxWorks.h>

#include "datalog.h"
#include "datalog_levels.h"
#include "error.h"
#include "objdictionary.h"

#include <stdio.h>
#include <iostream.h>


list< ObjDictionaryEntry* > ObjDictionary::_ObjEntries = list< ObjDictionaryEntry* >();


ObjDictionaryEntry :: ObjDictionaryEntry( const string &n, NewObjFcnPtr fcnPtr ) :
_ClassName( n ),
_NewObjFcnPtr( fcnPtr )
{
   ObjDictionary::add( this );
};

ObjDictionaryEntry :: ObjDictionaryEntry( const char* n, NewObjFcnPtr fcnPtr ) :
_ClassName( n ),
_NewObjFcnPtr( fcnPtr )
{
   ObjDictionary::add( this );
};

ObjDictionaryEntry :: ObjDictionaryEntry( const ObjDictionaryEntry &d ) :
_ClassName( d._ClassName )
{
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
      _ClassName = d.className();
   }
   return *this;
};

void *ObjDictionaryEntry :: newObject()
{
   return (*_NewObjFcnPtr)();
}

int ObjDictionaryEntry :: operator==( const ObjDictionaryEntry &d )
{
   return operator==( d._ClassName );
};

int ObjDictionaryEntry :: operator==( const string &n )
{
   return ( compare( n )==0 ? 1 : 0 );
};

int ObjDictionaryEntry :: operator==( const char *n )
{
   return ( compare( n )==0 ? 1 : 0 );
};

int ObjDictionaryEntry :: operator<( const ObjDictionaryEntry &d )
{
   return operator<( d._ClassName );
};

int ObjDictionaryEntry :: operator<( const string &n )
{
   return ( compare( n )<0 ? 1 : 0 );
};

int ObjDictionaryEntry :: operator<( const char *n )
{
   return ( compare( n )<0 ? 1 : 0 );
};

int ObjDictionaryEntry :: compare( const string &n )
{
   return ( _ClassName.compare( n ) );
};

int ObjDictionaryEntry :: compare( const char *n )
{
   return ( _ClassName.compare( n ) );
};

void ObjDictionaryEntry :: cleanup()
{
   _ClassName.erase();
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
      DataLog( log_level_message_system_error ) << "Object not found -> " << n << endmsg;
      return 0;
   }
   return e->newObject( );
}

int ObjDictionary :: valid( const char *n )
{
   
   list< ObjDictionaryEntry* >::iterator entry;
   for ( entry = _ObjEntries.begin();
         entry != _ObjEntries.end() ;
         ++entry ) 
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
   outs << "ObjDictionary content dump:" << endmsg;
   list< ObjDictionaryEntry* >::iterator entry;
   for ( entry = _ObjEntries.begin();
         entry != _ObjEntries.end() ;
         ++entry ) 
   {
      outs << " dictionary entry: " <<  (*entry)->className() << endmsg;
   }

}

ObjDictionaryEntry *ObjDictionary :: findEntry( const char *n )
{
   
   list< ObjDictionaryEntry* >::iterator entry;
   for ( entry = _ObjEntries.begin();
         entry != _ObjEntries.end() ;
         ++entry ) 
   {
      if ( *(*entry) == n )
      {
         return (*entry);
      }
   }
   return 0;
}

