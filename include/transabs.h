/*
 * Copyright (c) 2000 by Gambro BCT, Inc.  All rights reserved.
 *
 * TITLE:      transabs.h
 *
 * ABSTRACT:   This file contains the abstract definitions 
 *              for Transitions used by StateAbs classes.  
 *             
 */

#ifndef TRANSABS_H
#define TRANSABS_H

#include "returncodes.h"
#include <string>

class TransAbs
{
public:

   // Constructor
   TransAbs() :
      _TransitionState( ),
      _TransitionMessage( )
   {
   };

   // Copy Constructor
   TransAbs( const TransAbs &trans ) :
      _TransitionState( trans._TransitionState ),
      _TransitionMessage( trans._TransitionMessage )
   {
   };

   // Destructor
   virtual ~TransAbs()
   { 
      cleanup();
   };

   //
   // Function used to make the created transition
   //  active and functional
   //
   virtual void activate() {};
   
   //
   // Function used to make the created transition
   //  inactive 
   //
   virtual void deactivate() {};
   
   //
   // Basic operations needed for using standard template
   //  classes
   //

   //
   // Assignment operator
   //
   virtual TransAbs &operator=( const TransAbs &trans ) 
   {    
      if ( &trans != this ) 
      {
         _TransitionState = ( trans._TransitionState );   
         _TransitionMessage = ( trans._TransitionMessage );
      }
      return *this; 
   };
   
   //
   // Equality operator
   //
   //    return 1 -- I am equal to the passed in transition
   //    return 0 -- I am not equal to the passed in transition
   //
   virtual int operator==( const TransAbs &trans ) 
   {
      return ( compare( trans ) == 0 ? 1 : 0 );
   }

   //
   // Transition ordering function
   //   
   //    return 1 -- I am less than the passed in transition
   //    return 0 -- I am greater than or equal to the passed in transition
   //
   virtual int operator<( const TransAbs &trans ) 
   { 
      return ( compare( trans ) <0 ? 1 : 0 ); 
   };

   //
   // Compare function
   //
   //    return 1 -- I am greater than the passed in transition
   //    return 0 -- I am equal to the passed in transition
   //    return -1 - I am less than the passed in transition
   //
   virtual int compare( const TransAbs &trans ) const
   { 
      if ( _TransitionState < trans._TransitionState )
         return -1;
      if ( _TransitionState == trans._TransitionState )
         return 0;
      
      // then ( _TransitionState > trans._TransitionState )
         return 1;
   };
   
   // Methods
   //
   // Function called to determine if a transition has
   //  been achieved given processing status
   //
   virtual bool canTransition( const int status ) 
   {
      //
      // Log the transition message
      //

      if ( checkStatus( status ) == TRANSITION )
         return true;

      //
      // Can't transition return false
      //
      return false;
   };

   // Set/Get for state name
   virtual void transitionState( const char *s ){ if ( s ) _TransitionState = s; }
   virtual const string &transitionState() const { return _TransitionState; }

   // Set/Get for state name
   void transitionMessage( const char *m ) { if ( m ) _TransitionMessage =  m; };
   const string &transitionMessage() const { return _TransitionMessage; };

   //
   // Create a duplicate of myself
   //
   virtual TransAbs *clone() const = 0;


protected:

   
   //
   // Initialize the transition ( this should be called
   //  by the derived classes from their init functions )
   //
   virtual int init() 
   {
      return NORMAL; 
   };


   //
   // Method which checks for transition given current
   //  processing status (must be overidden)
   //
   //   return TRANSITION -- transition allowed
   //   return NO_TRANSITION -- transition not allowed
   //
   virtual int checkStatus( const int status ) = 0;



   //
   // Cleanup the dynamic memory associated with the application
   //
   void cleanup() 
   {
   };
   
protected:

   //
   // State to which this transition will point toward
   //
   string _TransitionState;

   //
   // Message to be logged when transition activates
   //
   string _TransitionMessage;

};

#endif
