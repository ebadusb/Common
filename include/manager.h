/*
 * Copyright (c) 2000 by Gambro BCT, Inc.  All rights reserved.
 *
 * TITLE:      manager.h
 *
 * ABSTRACT:   This file contains the definitions for  Manager.  
 *             This will manage the state of proc.
 */

#ifndef MANAGER_H
#define MANAGER_H

// Include files
#include <vxWorks.h>
#include <list> 
#include <string> 

#include "stateabs.h"


class Manager
{
public:
   // Constructor
   Manager();

   // Destructor
   virtual ~Manager();

   //
   // Initialize the state manager
   //
   //   return NORMAL -- initialized and ready
   //   return NO_SUBSTATES -- no substates found
   //
   int init( list< StateAbs* > &substateList );

   // Methods
   //
   // Function called to do state manager processing ...
   //
   //   return TRANSITION_NOSTATE -- transition but no state specified
   //   return TRANSITION -- transitioned to new state 
   //   return NO_TRANSITION -- process went OK (no transition)
   //   return NOT_INITIALIZED -- not been initialized correctly
   //   return PROCESSING_FAILED -- substate processing failed
   int process();

   //
   // Reset the substates
   //
   //   return NORMAL -- initialized and ready
   //   return NO_SUBSTATES -- no substates found
   //
   int reset();

   //
   // Find the first state we can enter in the substate list.  This
   //  function is called upon entry into the parent state.
   //
   void findFirstState();

   //
   // This function will determine whether the state manager has
   //  anything the manage
   //
   //   return NORMAL -- Substates available
   //   return NO_SUBSTATES -- No substates to manage
   //
   int determineStatus();
 
   //
   // Return current state, either as a name ("__null__" is returned for a
   // null current state) or as a pointer to the StateAbs object.
   //
   StateAbs * currentStatePtr() const { return _CurrentState; }
   const char *currentState() const;


   // Set/Get for next state name
   void nextState( const char *n ) { if ( n ) _NextState = n; else _NextState.erase(); }
   void nextState( const string &n ) { _NextState = n; }
   const string &nextState() const { return _NextState; }

protected:

   //
   // Function called to check the transitions for the current state
   //  help by this state manager ...
   //
   //   return TRANSITION_NOSTATE -- transition but no state specified
   //   return TRANSITION -- transition found 
   //   return NO_TRANSITION -- no transition found
   //   return NOT_INITIALIZED -- not been initialized correctly
   //
   int checkForTransition( int status /* current processing status */ );

   //
   // Function which returns the next state in the state manager's state
   //  list that should be transitioned into.  If newState is NULL, the
   //  returned state pointer will be NULL.  If newState is not found, the
   //  returned state pointer will be NULL.  
   //
   //   return NULL -- next state not found
   //   return {ptr} - found next state
   //
   StateAbs *findNextState( const string &newState, StateAbs const *currentState );

   //
   // Function called to determine whether the current state is a
   //  'new' current state and to call the 'enter' function for the
   //  current state ...
   //
   //   return NORMAL -- processing went OK (whether the enter was called or not)
   //   return PROCESSING_FAILED -- the new current state enter processing failed
   //
   int callEnter();

   //
   // Cleanup the dynamic memory associated with the application
   //
   void cleanup();

protected:

   //
   // Current state managed by this state manager
   //
   StateAbs *_CurrentState;

   //
   // List of substates this state manager will control
   //
   list< StateAbs* > *_SubstateList;

   //
   // Last state managed by this state manager
   //  
   StateAbs *_LastState;
   
   //
   // Next state name we need to transition into ...
   //
   string _NextState;
};

#endif
