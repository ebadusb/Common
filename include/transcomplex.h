/*
 * Copyright (c) 2000 by Gambro BCT, Inc.  All rights reserved.
 *
 * TITLE:      transcomplex.h
 *
 * ABSTRACT:   This file contains the definition 
 *              for complex transitions used by State classes.
 *              This class is a wrapper for several transition
 *              at once.
 *             
 */

#ifndef TRANSCOMPLEX_H
#define TRANSCOMPLEX_H


#include <vxWorks.h>
#include <list>
#include <string>

#include "logicaloperationtype.h"
#include "transabs.h"


class TransComplex : public TransAbs
{
public:

   // Constructor
   TransComplex();

   // Copy constructor
   TransComplex( const TransComplex &trans );

   // Destructor
   virtual ~TransComplex();

   //
   // Initialize the transition
   //
   virtual int init( const LogicalOperationType::Type type=LogicalOperationType::orOperation );

   //
   // Function which activates all transitions in the transition list
   //
   virtual void activate();

   //
   // Function which deactivates all transitions in the transition list
   //
   virtual void deactivate();

   //
   // Basic operations needed for using standard template
   //  classes
   //

   //
   // Assignment operator
   //
   virtual TransComplex &operator=( const TransComplex &trans ); 
   
   //
   // Equality operator
   //
   virtual int operator==( const TransComplex &trans ); 
   
   //
   // Comparison operator
   //
   virtual int operator<( const TransComplex &trans ); 

   //
   // Compare function
   //
   //   return 1 -- I am greater than the passed in transition
   //   return 0 -- I am equal to the passed in transition
   //   return -1 - I am less than the passed in transition
   //
   virtual int compare( const TransComplex &trans ) const;

   // Methods
   //
   // Function called to do determine if a transition has
   //  been achieved
   //
   virtual bool canTransition( const int status );
   //
   // Function used to add a transition
   //
   virtual void addTransition( TransAbs *trans );

   // Get transition list
   list< TransAbs* > &transitions() { return _TransitionList; };

   // Set/Get for condition type 
   void conditionType( LogicalOperationType::Type t ) { _ConditionType = t; };
   const LogicalOperationType::Type conditionType() const { return _ConditionType; };

   // Get for the name of the state in which to transition
   virtual void transitionState( const char *s ) { TransAbs::transitionState( s ); }
   virtual const string &transitionState() const;

   //
   // Create a duplicate of myself
   //
   virtual TransAbs *clone() const { return (new TransComplex( *this )); };


protected:

   //
   // Always returns TRUE for the complex transitions
   //
   virtual int checkStatus( int status );

   //
   // Function which does a deep copy on the elements within
   //  the transition list
   //
   void copyOver( const TransComplex &trans );

   //
   // Cleanup the dynamic memory associated with the application
   //
   void cleanup();
   
protected:

   //
   // List of transition controlled by this complex transition
   //
   list<TransAbs*> _TransitionList;

   //
   // Type of logical operation to perform
   //
   LogicalOperationType::Type  _ConditionType;

   //
   // Transition state determined from the child transitions
   string _ChildTransitionState;
};

#endif
