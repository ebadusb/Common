/*
 * Copyright (c) 2000 by Gambro BCT, Inc.  All rights reserved.
 *
 * TITLE:      stateabs.h
 *
 */


#ifndef STATEABS_H  //  Prevents multiple inclusions.
#define STATEABS_H

#include <vxWorks.h>
#include <list> 
#include <string>

#include "monitorbase.h"
#include "transcomplex.h"
#include "statedictionary.h"


class Manager;

// Class Declaration
class StateAbs 
{
public:

   // Default constructor
   StateAbs( StateAbs *parentState=0 );
   
   // Copy constructor
   StateAbs( const StateAbs &state );

   // Default destructor
   virtual ~StateAbs();

   // Initialize the object
   //
   //   return PROCESSING_ERROR -- Memory allocation failed
   //   return TRANSITION_NOSTATE -- State Manager failed to find substate
   //   return NORMAL -- Initialized OK
   //
   //  Intializes the transition list and then initialized the 
   //   state manager
   //
   virtual int init( const TransComplex &t );
   //
   //  Initializes the State manager ...
   //
   virtual int init();

   //
   // Return the current state ...
   //
   virtual const char *currentState() const;

   //
   // Entrance fuction (override the preEnter function to 
   //  add state specific entry code
   //
   virtual int enter(); 
   //
   // Main control function after state is entered ...
   //  (doesn't need to be overriden by derived classes --
   //   override the 'pre' and 'post' process functions below)
   //
   //   should return (0,99] -- processing completion status
   //   should return NO_TRANSITION -- processing not complete 
   //   should return PROCESSING_ERROR -- error
   //
   virtual int process();
   //
   // Exit function (override the postExit function to 
   //  add state specific exit code
   //
   virtual int exit();

   //
   // Function to test whether this state has met its requirements
   //  for termination ...
   //
   //   return (0,99] -- exit code
   //   return NO_TRANSITION -- state is not finished processing
   //
   virtual int transitionStatus() { return NO_TRANSITION; };

   //
   // Function to test whether this state has met its requirements
   //  for termination or exit given current processing status
   //
   bool checkForTransition(  const int status );
   const string &nextState( const int status=0 );

   //
   // Assignment operator
   //
   StateAbs &operator=( const StateAbs &state );

   //
   // Equality operator
   //
   int operator==( const StateAbs &state ) const;
   int operator==( const string &stateName ) const;
   int operator==( const char *stateName ) const;

   //
   // Comparison operator
   //
   int operator<( const StateAbs &state ) const;
   int operator<( const string &state ) const;
   int operator<( const char *state ) const;

   // 
   // Compare function
   //
   //   return 1 -- I am greater than the passed in StateAbs object
   //   return 0 -- I am equal to the passed in StateAbs object
   //   return -1 - I am less than the passed in StateAbs object
   //
   int compare( const StateAbs &state) const;
   int compare( const string &state) const;
   int compare( const char *state) const;

   //
   // Add a substate to the substate list
   //
   void addSubstate( StateAbs *state );

   //
   // This function adds monitors to the monitor list ...
   //
   int addMonitor( MonitorBase *mon );

   // Set/Get for state name
   void stateName( const string &n ) {_StateName = n; };
   void stateName( const char *n ) {_StateName = n; };
   const char* stateName() const { return _StateName.c_str(); };

   // Get substate list
   list< StateAbs* > &substates() { return _SubstateList; };

   // Set/Get for complex transition 
   void transition( TransComplex &t ) { _Transition = t; };
   TransComplex *transition() { return &_Transition; };

   // Set/Get for parent state 
   void parentState( StateAbs *s ) { _ParentState = s; };
   const StateAbs *parentState() const { return _ParentState; };

   // Set/Get for maintain history flag
   void maintainHistory( int f ) { _MaintainHistoryFlag = f; };
   const int maintainHistory() const { return _MaintainHistoryFlag; };

   virtual StateAbs *clone() const = 0;

protected:

   //
   // Virtual function which creates and initializes the monitors,
   //  if any ...
   //
   //   should return NORMAL -- initialized normally 
   //   should return PROCESSING_ERROR -- error
   //
   virtual int initMonitors();

   // 
   // Pure virtual functions which should be overridden in the 
   //  base derived class
   //
   //   should return NORMAL -- processing completed normally 
   //   should return PROCESSING_ERROR -- processing error
   //
   virtual int preProcess() = 0;
   virtual int postProcess() = 0;

   // 
   // These functions may be overridden.  They will only be
   //  called upon entrance to and exit from the state
   //
   virtual int preEnter() { return NORMAL; };
   virtual int postExit() { return NORMAL; };

   //
   // These functions enable and disable monitors from the monitors list 
   //  based on the state ...
   //
   void enableMonitors();
   void disableMonitors();

   //
   // This function checks monitors from the monitors list to
   //  find any and all alarm conditions ...
   //
   virtual void checkMonitors();

   //
   // Resets the state to the initial starting configuration ...
   //
   virtual void reset() {};
   //
   // Perform a deep copy of the items in this object
   //
   virtual void copyOver( const StateAbs &s );
   
   //
   // Dispose of any dynamically allocated memory 
   //
   virtual void cleanup();

protected:

   //
   // Controller for the state machine hierarchy
   //
   Manager                  *_StateManager;

   //
   // List of substates that this state encompasses
   //
   list< StateAbs* >        _SubstateList;

   // 
   // The 'key' for the state object.  This is a character
   //  name for the state.
   //
   string                   _StateName;

   //
   // Transition which holds one or many transitions joined
   //  by a logical operation
   //
   TransComplex             _Transition;

   //
   // This is the state which maintains ownership of this
   //  state
   //
   StateAbs                 *_ParentState;

   //
   // Flag used to signal this state to save its
   //  current position and return to it if the state
   //  is entered again.
   //
   int                          _MaintainHistoryFlag;

   //
   // When all the substates that this state contains have
   //  finished processing and there are no more in which to 
   //  transition, and this state has no processing left to do,
   //  set this flag.
   //
   int                          _ProcessingDone;

   //
   // List of Monitors to check at the end of processing 
   //
   list< MonitorBase* >         _MonitorsList;

};


#endif
