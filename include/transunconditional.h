/*
 * Copyright (c) 2000 by Gambro BCT, Inc.  All rights reserved.
 *
 * TITLE:      transunconditional.h
 *
 * ABSTRACT:   This file contains the definition 
 *              for unconditional transitions used by StateAbs classes.
 *             
 */

#ifndef TRANSUNCONDITIONAL_H
#define TRANSUNCONDITIONAL_H

#include <transabs.h>

class TransUnconditional : public TransAbs
{
public:

   // Constructor
   TransUnconditional();

   // Copy constructor
   TransUnconditional( const TransUnconditional &trans );

   // Destructor
   virtual ~TransUnconditional();

   //
   // Initialize the transition
   //
   virtual int init( );

   //
   //
   // Basic operations needed for using standard template
   //  classes
   //

   //
   // Assignment operator
   //
   virtual TransUnconditional &operator=( const TransUnconditional &trans ); 
   
   //
   // Equality operator
   //
   virtual int operator==( const TransUnconditional &trans ); 
   
   //
   // Comparison operator
   //
   virtual int operator<( const TransUnconditional &trans ); 

   //
   // Compare function
   //
   //   return 1 -- I am greater than the passed in transition
   //   return 0 -- I am equal to the passed in transition
   //   return -1 - I am less than the passed in transition
   //
   int compare( const TransUnconditional &trans ) const;

   //
   // Create a duplicate of myself
   //
   virtual TransAbs *clone() const { return (new TransUnconditional( *this )); };


protected:

   // Methods
   //
   // Function called to do determine if a transition has
   //  been achieved given current processing status.  Any
   //  status other than NO_TRANSITION will cause this function
   //  to return TRANSITION.
   //
   //   return NO_TRANSITION -- Cannot transition
   //   return TRANSITION -- Transition allowed
   //
   virtual int checkStatus( int status );

   //
   // Function which does a deep copy on the elements within
   //  the transition list
   //
   void copyOver( const TransUnconditional &trans );

   //
   // Cleanup the dynamic memory associated with the application
   //
   void cleanup();
   
protected:

};

#endif