/*
 * Copyright (c) 2000 by Gambro BCT, Inc.  All rights reserved.
 *
 * TITLE:      statedictionary.h
 *
 */

#ifndef _STATEDICTIONARY_H
#define _STATEDICTIONARY_H

#include <objdictionary.h>

class StateAbs;

class StateDictionaryEntry : public ObjDictionaryEntry
{
public:

   StateDictionaryEntry( const char* n, NewObjFcnPtr fcnPtr );

   StateDictionaryEntry( const StateDictionaryEntry &d );

   virtual ~StateDictionaryEntry();

   StateDictionaryEntry &operator=( const StateDictionaryEntry &d );

};


class StateDictionary : public ObjDictionary
{
};

#define DECLARE_STATE( __stateclassname__ ) DECLARE_OBJ( __stateclassname__ )
// End of DECLARE_STATE macro


#define DEFINE_STATE( __stateclassname__ ) DEFINE_OBJ( __stateclassname__ )
// End of DEFINE_STATE macro


#endif
