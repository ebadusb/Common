/*
 * Copyright (c) 2002 by Gambro BCT, Inc.  All rights reserved.
 *
 * TITLE:      spoofermessagesystem.cpp
 *
 */

#include <vxWorks.h>

#include "datalog.h"
#include "datalog_levels.h"
#include "error.h"
#include "spooferdispatcher.h"
#include "spoofermessagesystem.h"

SpooferMessageSystem::SpooferMessageSystem() :
   MessageSystem()
{
   DataLog( log_level_message_spoof_info ) << "Creating Spoofer message system for task " << hex << taskIdSelf() << endmsg;
}

SpooferMessageSystem::~SpooferMessageSystem()
{
   DataLog( log_level_message_spoof_info ) << "Spoofer message system for task " << hex << taskIdSelf() 
                                           << " has been deleted." << endmsg;
}

void SpooferMessageSystem :: spoofMessage( MessageBase &mb )
{
   ((SpooferDispatcher*)_Dispatcher)->spoofMessage( mb );
}

void SpooferMessageSystem :: spoofMessage( MessageBase &mb, const CallbackBase &cb )
{
   ((SpooferDispatcher*)_Dispatcher)->spoofMessage( mb, cb );
}

void SpooferMessageSystem :: despoofMessage( MessageBase &mb )
{
   ((SpooferDispatcher*)_Dispatcher)->despoofMessage( mb );
}

SpooferDispatcher &SpooferMessageSystem::spooferDispatcher() 
{ 
   if ( !_Dispatcher )
      _FATAL_ERROR( __FILE__, __LINE__, "SpooferDispatcher not created" );

   return *((SpooferDispatcher*)_Dispatcher);
}

void SpooferMessageSystem::createDispatcher()
{
   if ( _Dispatcher )
      delete _Dispatcher;

   _Dispatcher = new SpooferDispatcher;

   if ( !_Dispatcher )
      _FATAL_ERROR( __FILE__, __LINE__, "SpooferDispatcher create failed" );
}

