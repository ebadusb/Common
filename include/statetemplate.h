/*
 * Copyright (c) 2000 by Gambro BCT, Inc.  All rights reserved.
 *
 * TITLE:      statetemplate.h
 *
 * ABSTRACT:   Description of the actions and purpose of this state ...
 *
 * To use this state, add the class name in the 'states' file.  All functionality
 *  needed to make this state complete should be coded into the substate.  The transitions
 *  in the states file should only be used for system specification of where to go 
 *  next ...
 *
 */


#ifndef STATETEMPLATE_H  //  Prevents multiple inclusions.
#define STATETEMPLATE_H

#include <stateabs.h>

class StateTemplate : public StateAbs
{
   DECLARE_STATE( StateTemplate );
public:

   // Default constructor
   StateTemplate( );
   
   // Copy constructor
   StateTemplate( const StateTemplate &state );

   // Default destructor
   virtual ~StateTemplate();

   //
   // Transition determination function ...
   //
   virtual int transitionStatus();

   // Clone this state ...
   virtual StateAbs *clone() const { return ( new StateTemplate( *this )); };

protected:

   //
   // Functions to do this state's processing ...
   //
   virtual int preProcess();
   virtual int postProcess();

   // 
   // Function called upon first entrance to this state
   //
   virtual int preEnter();

   // 
   // Function called upon transition from this state
   //
   virtual int postExit();
   
   //
   // Resets the state upon termination
   //
   virtual void reset();

   //
   // Perform a deep copy of the items in this object
   //
   virtual void copyOver( const StateTemplate &s );
   
   //
   // Dispose of any dynamically allocated memory 
   //
   virtual void cleanup();

protected:

   //
   // Declare data members here ...
   //
};


#endif
