/*
 * Copyright (c) 2000 by Gambro BCT, Inc.  All rights reserved.
 *
 * TITLE:      transconditional.h
 *
 * ABSTRACT:   This file contains the definition 
 *              for conditional transitions used by StateAbs classes.
 *             
 */

#ifndef TRANSCONDITIONAL_H
#define TRANSCONDITIONAL_H

#include <transabs.h>

class TransConditional : public TransAbs
{
public:

   // Constructor
   TransConditional();

   // Copy constructor
   TransConditional( const TransConditional &trans );

   // Destructor
   virtual ~TransConditional();

   //
   // Initialize the transition
   //
   virtual int init( int flag );

   //
   //
   // Basic operations needed for using standard template
   //  classes
   //

   //
   // Assignment operator
   //
   virtual TransConditional &operator=( const TransConditional &trans ); 
   
   //
   // Equality operator
   //
   virtual int operator==( const TransConditional &trans ); 
   
   //
   // Comparison operator
   //
   virtual int operator<( const TransConditional &trans ); 

   //
   // Compare function
   //
   //   return 1 -- I am greater than the passed in transition
   //   return 0 -- I am equal to the passed in transition
   //   return -1 - I am less than the passed in transition
   //
   int compare( const TransConditional &trans ) const;

   // Set/Get for status flag 
   void statusFlag( int f ) { _StatusFlag = f; }; 
   const int statusFlag() const { return _StatusFlag; };

   //
   // Create a duplicate of myself
   //
   virtual TransAbs *clone() const { return (new TransConditional( *this )); };


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
   void copyOver( const TransConditional &trans );

   //
   // Cleanup the dynamic memory associated with the application
   //
   void cleanup();
   
protected:

   //
   // Status flag which will trigger this transition when it
   //  matches the current processing status
   //
   int _StatusFlag;

};

#endif /* ifdef TRANSCONDITIONAL_H */
