/*
 * Copyright (c) 2000 by Gambro BCT, Inc.  All rights reserved.
 *
 * TITLE:      transtimer.h
 *
 * ABSTRACT:   This file contains the definition 
 *              for conditional transitions used by State classes.
 *             
 */

#ifndef TRANSTIMER_H
#define TRANSTIMER_H

#include "transabs.h"
#include "timermessage.h"

class TransTimer : public TransAbs
{
public:

   // Constructor
   TransTimer();

   // Copy constructor
   TransTimer( const TransTimer &trans );

   // Destructor
   virtual ~TransTimer();

   //
   // Initialize the transition
   //   
   //  The timer transition will go off in the specified number
   //  of milliseconds
   //
   virtual int init( long msecs );

   //
   // Function used to activate the timer
   //
   virtual void activate();

   //
   // Function used to deactivate the timer
   //
   virtual void deactivate();

   //
   // Basic operations needed for using standard template
   //  classes
   //

   //
   // Assignment operator
   //
   virtual TransTimer &operator=( const TransTimer &trans ); 
   
   //
   // Equality operator
   //
   virtual int operator==( const TransTimer &trans ); 
   
   //
   // Comparison operator
   //
   virtual int operator<( const TransTimer &trans ); 

   //
   // Compare function
   //
   //   return 1 -- I am greater than the passed in transition
   //   return 0 -- I am equal to the passed in transition
   //   return -1 - I am less than the passed in transition
   //
   int compare( const TransTimer &trans ) const;

   //
   // When this function is called, the transition
   //  can occur
   //
   void timeout();

   //
   // Create a duplicate of myself
   //
   virtual TransAbs *clone() const { return (new TransTimer( *this )); };


protected:

   // Methods
   //
   // Function called to do determine if a transition has
   //  been achieved given current processing status
   //
   //   return NO_TRANSITION -- Cannot transition
   //   return TRANSITION -- Transition allowed
   //
   virtual int checkStatus( int status );

   //
   // Function which does a deep copy on the elements within
   //  the transition list
   //
   void copyOver( const TransTimer &trans );

   //
   // Cleanup the dynamic memory associated with the application
   //
   void cleanup();
   
protected:

   //
   // Boolean flag used to hold the transition status
   //
   int _CanTransition;

   //
   // Timer message
   //
   TimerMessage _Timer;

   //
   // Milliseconds before timer goes off 
   //
   int _MSecs;
};

#endif /* ifndef TRANSTIMER_H */
