/*
 * Copyright (c) 2002 by Gambro BCT, Inc.  All rights reserved.
 *
 * TITLE:      spoofermessagesystem.cpp
 *
 */

#include <vxWorks.h>

#include "error.h"
#include "spooferdispatcher.h"
#include "spoofermessagesystem.h"

SpooferMessageSystem::SpooferMessageSystem() :
   MessageSystem()
{
}

SpooferMessageSystem::~SpooferMessageSystem()
{
}

void SpooferMessageSystem::createDispatcher()
{
   if ( _Dispatcher )
      delete _Dispatcher;

   _Dispatcher = new SpooferDispatcher;

   if ( !_Dispatcher )
      _FATAL_ERROR( __FILE__, __LINE__, "SpooferDispatcher create failed" );
}

