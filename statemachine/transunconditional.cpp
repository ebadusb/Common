/*
 * Copyright (c) 2000 by Gambro BCT, Inc.  All rights reserved.
 *
 * TITLE:      transunconditional.cpp
 *
 */

#include "transunconditional.h"
#include "datalog_levels.h"

TransUnconditional :: TransUnconditional() :
TransAbs()
{
}

TransUnconditional :: TransUnconditional( const TransUnconditional &trans ) :
TransAbs( (TransAbs&) trans )
{
   copyOver( trans );
}

TransUnconditional :: ~TransUnconditional()
{
   cleanup();
}

int TransUnconditional :: init( ) 
{
   return TransAbs::init( );
}

TransUnconditional &TransUnconditional :: operator=( const TransUnconditional &trans ) 
{    
   if ( &trans != this ) 
   {
      TransAbs::operator=( trans );
      copyOver( trans );
   }
   return *this;
}
   
int TransUnconditional :: operator==( const TransUnconditional &trans ) 
{ 
   return compare( trans )==0 ? 1 : 0; 
}
   
int TransUnconditional :: operator<( const TransUnconditional &trans ) 
{ 
   return compare( trans )<0 ? 1 : 0; 
}

int TransUnconditional :: compare( const TransUnconditional &trans ) const
{
   return TransAbs::compare( (const TransAbs&)trans );
}

int TransUnconditional :: checkStatus( int status )
{
   DataLog( log_level_state_machine_debug ) << "Transition Check status: " << status << " ";
   //
   // Check the current processing status to see
   //  if this transition has been triggered ...
   //
   if ( NO_TRANSITION != status )
   {

      // 
      // Transiton allowed ...
      //
      DataLog( log_level_state_machine_debug ) << "Exit status: " << TRANSITION << endmsg; 
      return TRANSITION;
   }
      
   //
   // Cannot transition yet ...
   //
   DataLog( log_level_state_machine_debug ) << "Exit status: " << NO_TRANSITION << endmsg; 
   return NO_TRANSITION;
}

void TransUnconditional :: copyOver( const TransUnconditional & )
{
}

void TransUnconditional :: cleanup()
{
}