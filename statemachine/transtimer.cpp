/*
 * Copyright (c) 2000 by Gambro BCT, Inc.  All rights reserved.
 *
 * TITLE:      transtimer.cpp
 *
 */

#include "transtimer.h"
#include "datalog_levels.h"

TransTimer :: TransTimer() :
TransAbs(),
_CanTransition( 0 ),
_Timer( 0, Callback<TransTimer>( this, &TransTimer::timeout ), TimerMessage::DISARMED ), // Initialize the timer to 0 msecs
_MSecs( 0 )
{
}

TransTimer :: TransTimer( const TransTimer &trans ) :
TransAbs( (TransAbs&) trans ),
_CanTransition( trans._CanTransition ),
_Timer( 0, Callback<TransTimer>( this, &TransTimer::timeout ), TimerMessage::DISARMED ),
_MSecs( trans._MSecs )
{
   copyOver( trans );
}

TransTimer :: ~TransTimer()
{
   cleanup();
}

int TransTimer :: init( long msecs ) 
{
   //
   // Store the milliseconds ...
   //
   _MSecs = msecs;

   return TransAbs::init( );
}

void TransTimer :: timeout()
{ 
   DataLog( log_level_state_machine_debug ) << "Transition Timeout " << _MSecs << endmsg;
   _CanTransition=1; 
}

void TransTimer :: activate()
{
   //
   // Activate the timer
   //
   _Timer.interval( _MSecs );

   _CanTransition=0;
}

void TransTimer :: deactivate()
{
   //
   // Deactivate the timer
   //
   _Timer.interval( 0 );

   _CanTransition=0;
}

TransTimer &TransTimer :: operator=( const TransTimer &trans ) 
{    
   if ( &trans != this ) 
   {
      TransAbs::operator=( trans );
      _MSecs = trans._MSecs;
      copyOver( trans );
   }
   return *this;
}
   
int TransTimer :: operator==( const TransTimer &trans ) 
{ 
   return compare( trans )==0 ? 1 : 0; 
}
   
int TransTimer :: operator<( const TransTimer &trans ) 
{ 
   return compare( trans )<0 ? 1 : 0; 
}

int TransTimer :: compare( const TransTimer &trans ) const
{

   int status = TransAbs::compare( (const TransAbs&)trans );

   if ( status == 0 )
   {
      if ( _MSecs < trans._MSecs ) 
      {
         status = -1;
      }
      else if ( _MSecs > trans._MSecs )
      {
         status = 1;
      }
   }
   return status;
}

int TransTimer :: checkStatus( int )
{
   if ( _CanTransition )
   {
      DataLog( log_level_state_machine_debug ) << "Transition Check " << TRANSITION << endmsg;
      return TRANSITION;
   }
   DataLog( log_level_state_machine_debug ) << "Transition Check " << NO_TRANSITION << endmsg;
   return NO_TRANSITION;
}

void TransTimer :: copyOver( const TransTimer & )
{
}

void TransTimer :: cleanup()
{
}
