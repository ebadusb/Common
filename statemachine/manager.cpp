/*
 * Copyright (c) 2000 by Gambro BCT, Inc.  All rights reserved.
 *
 * TITLE:      manager.cpp
 *
 */

#include "datalog_levels.h"
#include "manager.h"

#include <stdio.h>
#include <stdlib.h>


Manager::Manager( ) :
_CurrentState( 0 ),
_SubstateList( 0 ),
_LastState( 0 ),
_NextState( )
{
}

Manager::~Manager()
{
   cleanup();
}

int Manager :: init( list< StateAbs* > &substateList )
{
   //
   // The passed in list will be the list of 
   //  states that I will manage.
   //
   _SubstateList = &substateList;

   //
   // Determine whether or not this state manager
   //  will have any processing to do ...
   //
   return determineStatus();

}

int Manager :: process()
{
   //
   // Make sure this state manager has been initialized
   //  and everything is set up OK ...
   //
   if ( _CurrentState == 0 )
   {
      //
      // No current state was set, find the first state
      //
      findFirstState();

      //
      // If we still don't have a substate
      //  
      if ( !_CurrentState )
         if ( _SubstateList->size() == 0 )
         { 
            return NO_SUBSTATES;
         }
         else
         {
            return TRANSITION_NOSTATE;
         }

   }

   //
   // Determine whether or not to call enter on the 
   //  current state ...
   //
   if ( callEnter() != NORMAL )
   {
      // Something went wrong with the state
      return PROCESSING_FAILED;
   }

   DataLog( log_level_state_machine_debug ) << "State Manager -> Current State: " << _CurrentState->stateName() << endmsg;

   //
   // Call the process function for the current state
   // 
   int processingStatus = _CurrentState->process();

   DataLog( log_level_state_machine_debug ) << "State Manager -> Current State Done: " << _CurrentState->stateName()
        << " (code = " << processingStatus << ")" << endmsg;

   if ( processingStatus == PROCESSING_ERROR )
   {
      // Something went wrong with the state
      return PROCESSING_FAILED;
   }

   //
   // Check for transitions 
   //
   int transStatus = checkForTransition( processingStatus );
   DataLog( log_level_state_machine_debug ) << "State Manager -> Check For Trans Done: " << _LastState->stateName()
        << " (code = " << transStatus << ")" << endmsg;

   if ( transStatus == TRANSITION || transStatus == TRANSITION_NOSTATE )
   {
      if ( _LastState->exit() != NORMAL )
      {
         // Something went wrong with the last state
         return PROCESSING_FAILED;
      }

      //
      // Guarantee the enter function executes even if we 
      //  transition to the same state ...
      //  
      _LastState = 0;

      //
      // Call the enter function on the next state ( new _CurrentState )
      //
      if ( callEnter() != NORMAL )
      {
         // Something went wrong with the state
         return PROCESSING_FAILED;
      }
   
   }

   return transStatus;
}

int Manager :: checkForTransition( int status )
{

   if ( !_CurrentState ) 
   {
      return TRANSITION_NOSTATE;
   }

   DataLog( log_level_state_machine_debug ) << "State Manager -> Check for Transition: " << _CurrentState->stateName() << endmsg;

   //
   // Make sure we are initialized ...
   //
   if ( _SubstateList == 0 )
   {
      // We have not been initialized
      return NOT_INITIALIZED;
   }

   //
   // Check the curren state for transitions
   //
   if ( _CurrentState->checkForTransition( status ) == true )
   {
      //
      // If the transition check returned true,
      //  find the new state and transition ...
      //
      string newState = _CurrentState->nextState( status );

      //
      // Initialize the next state ...
      //
      _NextState.erase();

      //
      // Find the next state ...
      //
      _CurrentState = findNextState( newState, _CurrentState);

      //
      // We should transition, but we don't know where to go ...
      //
      if ( _CurrentState == 0 ) 
      {
         //
         // If the name is a real state ...
         //
         if (    !newState.empty()
              && newState !=  "__null__" )
         {
            //
            // Save the name of the next state we are going to 
            //  transition into ...
            //
            DataLog( log_level_state_machine_debug ) << "State Manager -> Done Check for Transition: newState=" << newState << endmsg; 
            _NextState = newState;
         }
         //
         // Transition allowed, but no state to transition to ...
         //
         return TRANSITION_NOSTATE;
      }

      // 
      // Transition occurred ...
      //
      DataLog( log_level_state_machine_debug ) << "State Manager -> Transition " << _CurrentState->stateName();
      if ( _LastState ) DataLog( log_level_state_machine_debug ) << " from " << _LastState->stateName();
         DataLog( log_level_state_machine_debug ) << " ( transition status = false )" << endmsg;
      return TRANSITION;
   }
   DataLog( log_level_state_machine_debug ) << "State Manager -> Done Check for Transition: " << _CurrentState->stateName() 
        << " ( transition status = false )" << endmsg;
   //
   // No transition
   //
   return NO_TRANSITION;
}

StateAbs *Manager :: findNextState( const string &newState, StateAbs const *currentState )
{

   //
   // Use the newState name to find the next state ...
   //
   DataLog( log_level_state_machine_debug ) << "State Manager -> Find next state: ";
   if ( currentState ) DataLog( log_level_state_machine_debug ) << "CurrentState = " << currentState->stateName();
   DataLog( log_level_state_machine_debug ) << " ReqState = ";
   if ( newState.empty() ) DataLog( log_level_state_machine_debug ) << "__unspecified__"; else DataLog( log_level_state_machine_debug ) << newState;
   DataLog( log_level_state_machine_debug ) << endmsg;

   //
   // Find the new state 
   //
   int takeNextState=0;
   StateAbs *prevStatePtr=0;
   list< StateAbs* >::iterator state;
   for ( state = _SubstateList->begin() ;
         state != _SubstateList->end()  ;
         ++state )
   {
      DataLog( log_level_state_machine_debug ) << "     SubstateList ->  " << (*state)->stateName() << endmsg;

      //
      // If we have seen the current state ...
      //
      if ( prevStatePtr==currentState )
      {
         //
         // Any of the next states are further in the list and can
         //  be transitioned into ...
         //
         takeNextState=1;
      }

      //
      // If this state is a valid one to transition into ...
      //
      if ( //
           // This state is the specified transition state ...
           //
           ( (*state)->stateName() == newState ) ||
           //
           // Or ...
           //
           ( //
             // No transitions were specified (__null__) or 
             //  the state was not specified ( ie. the string was empty )
             //
             (    newState.empty()
                  || newState == "__null__" ) && 
             //
             // And this state is after the current state in the list ...
             takeNextState &&
             //
             // And the state has something to do when we transition into it ...
             // 
             ( (*state)->transitionStatus() == NO_TRANSITION ) 
           )
         )
      {
         //
         // Return the new state...
         //
         return(*state);
      }
      // 
      // Assign this state to be held as previous state
      //  in the list ...
      //
      prevStatePtr = (*state);
   }
   //
   // The new state was not found ...
   //
   return 0;
}

int Manager :: callEnter()
{
   //
   // If the last state and the current state are
   //  different, this is the first time to enter
   //  the current state ...
   //
   if ( _CurrentState &&
        _CurrentState != _LastState )
   {
      //
      // Now the last state is the current state
      //
      _LastState = _CurrentState;
      
      //
      // Call enter on the current state
      //
      if ( _CurrentState->enter() != NORMAL )
      {
         // Something went wrong with the state
         return PROCESSING_FAILED;
      }
   }
   return NORMAL;
}

int Manager :: reset()
{
   //
   // Reset the next state ...
   //
   _NextState.erase();

   //
   // Exit the current substate ...
   //
   if ( _CurrentState )
   {
      _CurrentState->exit();
   }

   //
   // Find the first state we can go into ...
   //
   findFirstState();

   //
   // Determine whether or not we have substates ...
   //
   return determineStatus();
}


void Manager :: findFirstState()
{
   //
   // The current state will be the first one in the list
   //
   //  Find the current state
   //
   string unspecifiedState;
   _CurrentState = findNextState( unspecifiedState, 0 ); 
   _LastState = 0;
}

int Manager :: determineStatus()
{
   if ( _SubstateList->size() == 0 )
   {
      //
      // No current state was found
      //
      return NO_SUBSTATES;
   }

   DataLog( log_level_state_machine_debug ) << "Substates found " << _SubstateList->size() << endmsg;

   //
   // Everything initialized OK
   //
   return NORMAL;
}

const char *Manager :: currentState() const
{
   if ( _SubstateList->size() == 0 || 
        _CurrentState == 0 )
   {
      return "__null__";
   }
   return _CurrentState->currentState();

}

void Manager :: cleanup()
{

}

