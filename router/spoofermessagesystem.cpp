/*
 * Copyright (c) 2002 by Gambro BCT, Inc.  All rights reserved.
 *
 * TITLE:      spoofermessagesystem.cpp
 *
 */

#include <vxWorks.h>

#include "datalog.h"
#include "error.h"
#include "spooferdispatcher.h"
#include "spoofermessagesystem.h"

SpooferMessageSystem::SpooferMessageSystem() :
   MessageSystem()
{
   DataLog_Level slog( "Spoofer" );
   slog( __FILE__, __LINE__ ) << "Creating Spoofer message system for task " << hex << taskIdSelf() << endmsg;
}

SpooferMessageSystem::~SpooferMessageSystem()
{
   DataLog_Level slog( "Spoofer" );
   slog( __FILE__, __LINE__ ) << "Spoofer message system for task " << hex << taskIdSelf() 
                              << " has been deleted." << endmsg;
}

void SpooferMessageSystem::createDispatcher()
{
   if ( _Dispatcher )
      delete _Dispatcher;

   _Dispatcher = new SpooferDispatcher;

   if ( !_Dispatcher )
      _FATAL_ERROR( __FILE__, __LINE__, "SpooferDispatcher create failed" );
}

