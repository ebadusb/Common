/*
 * Copyright (c) 2000 by Gambro BCT, Inc.  All rights reserved.
 *
 * TITLE:      transconditional.cpp
 *
 */

#include "datalog_levels.h"
#include "transconditional.h"

TransConditional :: TransConditional() :
TransAbs(),
_StatusFlag( 0 )
{
}

TransConditional :: TransConditional( const TransConditional &trans ) :
TransAbs( (TransAbs&) trans ),
_StatusFlag( trans._StatusFlag )
{
   copyOver( trans );
}

TransConditional :: ~TransConditional()
{
   cleanup();
}

int TransConditional :: init( int flag ) 
{
   //
   // Status flag which will trigger this transition when it
   //  becomes equal to the current processing status ...
   //
   _StatusFlag = flag;
   
   return TransAbs::init( );
}

TransConditional &TransConditional :: operator=( const TransConditional &trans ) 
{    
   if ( &trans != this ) 
   {
      TransAbs::operator=( trans );
      copyOver( trans );
      _StatusFlag = trans._StatusFlag;
   }
   return *this;
}
   
int TransConditional :: operator==( const TransConditional &trans ) 
{ 
   return compare( trans )==0 ? 1 : 0; 
}
   
int TransConditional :: operator<( const TransConditional &trans ) 
{ 
   return compare( trans )<0 ? 1 : 0; 
}

int TransConditional :: compare( const TransConditional &trans ) const
{

   int status = TransAbs::compare( (const TransAbs&)trans );

   if ( status == 0 )
   {

      // 
      // Compare the status flag
      //
      if ( _StatusFlag < trans._StatusFlag )
      {
         return -1;
      }
      else if ( _StatusFlag > trans._StatusFlag )
      {
         return 1;
      }
   }
   return status;
}

int TransConditional :: checkStatus( int status )
{
   DataLog( log_level_state_machine_debug ) << "Transition Check status: " << status << " ";
   //
   // Check the current processing status to see
   //  if this transition has been triggered ...
   //
   if ( _StatusFlag == status )
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
   DataLog( log_level_state_machine_debug ) << "Exit status: " << TRANSITION << endmsg; 
   return NO_TRANSITION;
}

void TransConditional :: copyOver( const TransConditional & )
{
}

void TransConditional :: cleanup()
{
}
