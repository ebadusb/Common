/*
 * Copyright (c) 2000 by Gambro BCT, Inc.  All rights reserved.
 *
 * TITLE:      callback.h
 *
 * ABSTRACT:   This class supports callback functions
 *
 */

#ifndef __CALLBACK_H__
#define __CALLBACK_H__


class Callback
{
public:
   // pointer to a member function of Callback class
   // that takes no arguments
   typedef void (Callback::*MemFncPtr)();

   // pointer to a non-member function that takes no
   // arguments
   typedef void (*FncPtr)();

   // constructor that takes a pointer to a non member
   // function
   Callback(FncPtr ff=0);

   // constructor that takes a pointer to member function
   //  ( The restriction on object pointers and function
   //    pointers being associated with a known class is 
   //    bypassed by using the elipsis notation )
   Callback(void *pp, ...);
     
   // destructor
   ~Callback();

   // copy constructor
   Callback( const Callback &cb );

   // operator =
   Callback &operator=( const Callback &cb );

   // function dispatch
   virtual void operator()();

private:

   Callback *_Ptr;
   union { MemFncPtr _MemFncPtr; FncPtr _FncPtr; };
                                           
};

#endif
