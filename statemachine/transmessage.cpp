/*
 * Copyright (c) 2000 by Gambro BCT, Inc.  All rights reserved.
 *
 * TITLE:      transmessage.cpp
 *
 */

#include "callback.h"
#include "datalog_levels.h"
#include "objdictionary.h"
#include "messagebase.h"
#include "transmessage.h"

TransMessage :: TransMessage() :
TransAbs(),
_CanTransition( 0 ),
_MessageName( ),
_TransitionMessagePtr( 0 )
{
}

TransMessage :: TransMessage( const TransMessage &trans ) :
TransAbs( (TransAbs&) trans ),
_CanTransition( trans._CanTransition ),
_MessageName( trans._MessageName ),
_TransitionMessagePtr( 0 )
{
   copyOver( trans );
}

TransMessage :: ~TransMessage()
{
   cleanup();
}

int TransMessage :: init( const char *name )
{
   //
   // Save the int message name, but don't create the message
   //
   if ( !ObjDictionary::valid( name ) )
   {
      return PROCESSING_ERROR;
   }
   _MessageName = name;
   DataLog( log_level_state_machine_debug ) << "Transition Message " << "- create transition for message " << _MessageName.c_str() << endmsg;

   return TransAbs::init( );
}

void TransMessage :: notify()
{ 
   DataLog( log_level_state_machine_debug ) << "Transition Message " << _MessageName.c_str() << endmsg;
   _CanTransition=1; 
}

void TransMessage :: activate()
{
   //
   // Create the message and set up the notify function
   //  to be called by the message
   //
   _TransitionMessagePtr = (MessageBase*)ObjDictionary::create( _MessageName.c_str() );
   if ( _TransitionMessagePtr ) 
      _TransitionMessagePtr->init( Callback< TransMessage >( this, &TransMessage::notify ) );      

   DataLog( log_level_state_machine_debug ) << "Transition Message " << "- activated transition for message " << _MessageName.c_str() << endmsg;
   _CanTransition=0; 
}

void TransMessage :: deactivate()
{
   //
   // remove the message
   //
   cleanup();

   DataLog( log_level_state_machine_debug ) << "Transition Message " << "- deactivated transition for message " << _MessageName.c_str() << endmsg;
   _CanTransition=0; 
}

TransMessage &TransMessage :: operator=( const TransMessage &trans ) 
{    
   if ( &trans != this ) 
   {
      TransAbs::operator=( trans );
      copyOver( trans );
      _CanTransition = trans._CanTransition;
      _MessageName = trans._MessageName;
   }
   return *this;
}
   
int TransMessage :: operator==( const TransMessage &trans ) 
{ 
   return compare( trans )==0 ? 1 : 0; 
}
   
int TransMessage :: operator<( const TransMessage &trans ) 
{ 
   return compare( trans )<0 ? 1 : 0; 
}

int TransMessage :: compare( const TransMessage &trans ) const
{

   int status = TransAbs::compare( (const TransAbs&)trans );

   if ( status == 0 )
   {
      if ( _CanTransition < trans._CanTransition )
      {
         return -1;
      }
      else if ( _CanTransition > trans._CanTransition )
      {
         return 1;
      }

      if ( _MessageName < trans._MessageName ) 
      {
         return -1;
      }
      else if ( _MessageName > trans._MessageName ) 
      {
         return 1;
      }
   }
   return status;
}

int TransMessage :: checkStatus( int )
{
   if ( _CanTransition )
   {
      DataLog( log_level_state_machine_debug ) << "Transition Check " << TRANSITION  << " " << transitionState() << endmsg;
      return TRANSITION;
   }
   DataLog( log_level_state_machine_debug ) << "Transition Check " << NO_TRANSITION << endmsg;
   return NO_TRANSITION;
}

void TransMessage :: copyOver( const TransMessage & )
{
}

void TransMessage :: cleanup()
{
   //
   // Delete the dynamically allocated message ...
   //
   if ( _TransitionMessagePtr )
   {
      delete _TransitionMessagePtr;
      _TransitionMessagePtr = 0;
   }
}
