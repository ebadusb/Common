/*
 * Copyright (c) 2000 by Gambro BCT, Inc.  All rights reserved.
 *
 * TITLE:      statedictionary.cpp
 *
 */

#include "datalog_levels.h"
#include <statedictionary.h>

StateDictionaryEntry :: StateDictionaryEntry( const char* n, NewObjFcnPtr fcnPtr ) :
ObjDictionaryEntry( n, fcnPtr )
{
};

StateDictionaryEntry :: StateDictionaryEntry( const StateDictionaryEntry &d ) :
ObjDictionaryEntry( d )
{
};

StateDictionaryEntry :: ~StateDictionaryEntry()
{
};

StateDictionaryEntry &StateDictionaryEntry :: operator=( const StateDictionaryEntry &d )
{
   if ( &d != this )
   {
      ObjDictionaryEntry::operator=( d );
   }
   return *this;
};

