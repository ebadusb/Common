/*
 * Copyright (c) 2000 by Gambro BCT, Inc.  All rights reserved.
 *
 * TITLE:      transcomplex.cpp
 *
 */

#include <transcomplex.h>
#include <datalog_levels.h>

TransComplex :: TransComplex() :
TransAbs(),
_TransitionList(),
_ConditionType( LogicalOperationType::orOperation ),
_ChildTransitionState()
{
}

TransComplex :: TransComplex( const TransComplex &trans ) :
TransAbs( (TransAbs&) trans ),
_TransitionList(),
_ConditionType( trans._ConditionType ),
_ChildTransitionState( trans._ChildTransitionState )
{
   copyOver( trans );
}

TransComplex :: ~TransComplex()
{
   cleanup();
}

int TransComplex :: init( const LogicalOperationType::Type t )
{
   //
   //  This state may or may not change the default condition
   //   type from "or"
   //
   _ConditionType = t;
   
   //
   // This state may or may not specify a transition
   //  state or message...
   //
   
   return TransAbs::init( );
}

void TransComplex :: activate()
{
   //
   // Activate all transitions in the list
   //
   list< TransAbs* >::iterator trans;
   for ( trans = _TransitionList.begin();
         trans != _TransitionList.end() ;
         ++trans ) 
   {
      (*trans)->activate();
   }
}

void TransComplex :: deactivate()
{
   //
   // Activate all transitions in the list
   //
   list< TransAbs* >::iterator trans;
   for ( trans = _TransitionList.begin();
         trans != _TransitionList.end() ;
         ++trans ) 
   {
      (*trans)->deactivate();
   }
}

TransComplex &TransComplex :: operator=( const TransComplex &trans ) 
{    
   if ( &trans != this ) 
   {
      TransAbs::operator=( trans );
      copyOver( trans );
      _ConditionType = trans._ConditionType;
      _ChildTransitionState = trans._ChildTransitionState;
   }
   return *this;
}
   
int TransComplex :: operator==( const TransComplex &trans ) 
{ 
   return compare( trans )==0 ? 1 : 0; 
}
   
int TransComplex :: operator<( const TransComplex &trans ) 
{ 
   return compare( trans )<0 ? 1 : 0; 
}


int TransComplex :: compare( const TransComplex &trans ) const
{

   int status = TransAbs::compare( (const TransAbs&)trans );

   if ( status == 0 )
   {
      // 
      // Compare the owning state's pointer
      if ( _ConditionType < trans._ConditionType )
         return -1;
      else if ( _ConditionType > trans._ConditionType )
         return 1;

      //
      // Compare the childrens transition state
      if ( _ChildTransitionState < trans._ChildTransitionState )
         return -1;
      else if ( _ChildTransitionState > trans._ChildTransitionState )
         return 1;

      //
      //  compare the transition list
      if ( _TransitionList < trans._TransitionList )
         return -1;
      else if (_TransitionList > trans._TransitionList )
         return 1;
   }
   return status;
}

bool TransComplex :: canTransition( const int status )
{
   DataLog( log_level_state_machine_debug ) << "Transition Check status: " << status << endmsg; 

   bool transStatus = false;
   _ChildTransitionState.erase();
  
   //
   // If the transiton list is empty, return "__null__"
   //
   if ( _TransitionList.size() == 0 )
   {
      DataLog( log_level_state_machine_debug ) << "Transition Exit status: " << "__null__" << endmsg; 
      _ChildTransitionState = "__null__";
      return true;
   }
   
   list< TransAbs* >::iterator trans;
   for ( trans = _TransitionList.begin();
         trans != _TransitionList.end() ;
         ++trans ) 
   {
      transStatus = (*trans)->canTransition( status );
      
      if ( transStatus == true )
      {
         _ChildTransitionState = (*trans)->transitionState();

         // If the complex transition is "or'd" together
         //  then we are done and should break out of the
         //  loop
         if ( _ConditionType == LogicalOperationType::orOperation )
         {
            break;
         }
         // If the complex transition is "and'd" together
         //  then we don't know if we are finished and 
         //  must continue through all transitions ...
         else if ( _ConditionType == LogicalOperationType::andOperation )
         {
            continue;
         }
      }
      else
      {
         //
         // If the complex transition is "and'd" together
         //  then we are done and cannot transition
         //
         if ( _ConditionType == LogicalOperationType::andOperation )
         {
            DataLog( log_level_state_machine_debug ) << "Transition Exit status: " << 0 << endmsg; 
            return false;
         }
         // If the complex transition is 'or'd" together
         //  then we don't know if we are done and should
         //  continue to process more transitions in the list
         else if ( _ConditionType == LogicalOperationType::orOperation )
         {
            continue;
         }
      }
   }
   //
   // Transition found
   //
   if ( transStatus == true )
   {
      DataLog( log_level_state_machine_debug ) << "Transition Exit status: " << transitionState() << endmsg; 
      return true;
   }

   //
   // Transition not found
   //
   DataLog( log_level_state_machine_debug ) << "Transition Exit status: " << 0 << endmsg; 
   return false;

}

const string &TransComplex :: transitionState() const
{
   if ( !_ChildTransitionState.empty() && _TransitionState.empty() ) 
      return _ChildTransitionState;
   else
      return _TransitionState;
}

int TransComplex :: checkStatus( const int )
{
   return TRANSITION;
}

void TransComplex :: addTransition( TransAbs *trans )
{
   _TransitionList.push_back( trans );
}

void TransComplex :: copyOver( const TransComplex &trans )
{
   //
   // Cleanup all items in the list
   //
   cleanup();

   //
   //  Deep copy the transition list
   //
   list< TransAbs* >::iterator transiter;
   for ( transiter = ( (TransComplex&)trans )._TransitionList.begin() ;
         transiter != ( (TransComplex&)trans )._TransitionList.end()  ;
         ++transiter 
       )
   {
      _TransitionList.push_back( (*transiter)->clone() );
   }
}

void TransComplex :: cleanup()
{
   // 
   // Cleanup memory owned by the lists, but
   //  we don't own the memory in these lists
   //  we just have pointers.
   //
   _TransitionList.erase( _TransitionList.begin(),
                          _TransitionList.end()    );
}