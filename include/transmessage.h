/*
 * Copyright (c) 2000 by Gambro BCT, Inc.  All rights reserved.
 *
 * TITLE:      transmessage.h
 *
 * ABSTRACT:   This file contains the definition 
 *              for conditional transitions used by State classes.
 *             
 */

#ifndef TRANSMESSAGE_H
#define TRANSMESSAGE_H

#include <string>
#include <transabs.h>

class MessageBase;

class TransMessage : public TransAbs
{
public:

   // Constructor
   TransMessage();

   // Copy constructor
   TransMessage( const TransMessage &trans );

   // Destructor
   virtual ~TransMessage();

   //
   // Initialize the transition with an int message
   //
   virtual int init( const char *msgname );

   //
   // Function used to activate the message 
   //
   virtual void activate();

   //
   // Function used to deactivate the message 
   //
   virtual void deactivate();

   //
   // Basic operations needed for using standard template
   //  classes
   //

   //
   // Assignment operator
   //
   virtual TransMessage &operator=( const TransMessage &trans ); 
   
   //
   // Equality operator
   //
   virtual int operator==( const TransMessage &trans ); 
   
   //
   // Comparison operator
   //
   virtual int operator<( const TransMessage &trans ); 

   //
   // Compare function
   //
   //   return 1 -- I am greater than the passed in transition
   //   return 0 -- I am equal to the passed in transition
   //   return -1 - I am less than the passed in transition
   //
   int compare( const TransMessage &trans ) const;


   //
   // When this function is called, the transition
   //  can occur
   //
   void notify();
 
   //
   // Create a duplicate of myself
   //
   virtual TransAbs *clone() const { return (new TransMessage( *this )); };


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
   void copyOver( const TransMessage &trans );

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
   // Message identifier name ...
   //
   string _MessageName;

   //
   // Pointer to the transition message ...
   //
   MessageBase *_TransitionMessagePtr;

};

#endif