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


class CallbackBase
{
public:
   // pointer to a member function of CallbackBase class
   // that takes no arguments
   typedef void (CallbackBase::*MemFncPtr)();

   // pointer to a non-member function that takes no
   // arguments
   typedef void (*FncPtr)();

   // constructor that takes a pointer to a non member
   // function
   CallbackBase( FncPtr ff=0 ) :
      _Ptr( 0 ),
      _FncPtr( ff )
   {
   };

   // destructor
   virtual ~CallbackBase() {};

   // copy constructor
   CallbackBase( const CallbackBase &cb ) :
      _Ptr(cb._Ptr),
      _MemFncPtr(cb._MemFncPtr),
      _FncPtr(cb._FncPtr)
   {
   };

   // operator =
   CallbackBase &operator=( const CallbackBase &cb )
   {
      _Ptr = cb._Ptr;
      _MemFncPtr = cb._MemFncPtr;
      _FncPtr = cb._FncPtr;
   
      return *this;
   };

   // function dispatch
   void operator()()
   {
      //
      // If the object pointer variable is set ...
      //
      if (_Ptr)
      {
         // Call the member function ...
         (_Ptr->*_MemFncPtr)();
      }
      //
      // If the function pointer is set ...
      //
      else if (_FncPtr)
      {
         // Call the function ...
         _FncPtr();
      }
      //
      // If nothing has been set, then do nothing ...
      //
   };

protected:

   //
   // Cannot be used outside of this class and its descendants...
   CallbackBase( CallbackBase* ptr, MemFncPtr fptr ) : _Ptr( ptr ), _MemFncPtr( fptr ) {};

private:

   CallbackBase *_Ptr;
   union { MemFncPtr _MemFncPtr; FncPtr _FncPtr; };
                                           
};


template < class CallbackClass > class Callback : public CallbackBase
{
private:

   //
   // Default Constructor
   Callback() : CallbackBase() { };

public:
   // pointer to a member function of Callback class
   // that takes no arguments
   typedef void (CallbackClass::*TMemFncPtr)();

   // constructor that takes a pointer to member function
   Callback(CallbackClass *pp, TMemFncPtr ptr) : CallbackBase( (CallbackBase*)pp, (MemFncPtr)ptr ) { };
     
   // destructor
   ~Callback() {};

   // copy constructor
   Callback( const Callback &cb ) : CallbackBase( cb ) { };

   // operator =
   Callback &operator=( const Callback &cb )
   {
      return CallbackBase::operator =( cb );
   };

};

#endif
