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

template < class T >
class Message;

class CallbackBase
{
public:
   // pointer to a member function of CallbackBase class
   // that takes no arguments
   typedef void (CallbackBase::*MemFncPtrVoid)();
   typedef void (CallbackBase::*MemFncPtrMsg)( Message< class T >& );

   // pointer to a non-member function that takes no
   // arguments
   typedef void (*FncPtrVoid)();
   typedef void (*FncPtrMsg)( Message< class T >& );

   // constructor that takes a pointer to a non member
   // void function
   CallbackBase( FncPtrVoid ff=0 ) :
      _Ptr( 0 ),
      _FncPtrVoid( ff )
   {
   };

   // constructor that takes a pointer to a non member
   // function taking a Message ref.
   CallbackBase( FncPtrMsg ff ) :
      _Ptr( 0 ),
      _FncPtrMsg( ff )
   {
   };

   // destructor
   virtual ~CallbackBase() {};

   // copy constructor
   CallbackBase( const CallbackBase &cb ) :
      _Ptr(cb._Ptr),
      _MemFncPtrVoid(cb._MemFncPtrVoid),
      _MemFncPtrMsg(cb._MemFncPtrMsg),
      _FncPtrVoid(cb._FncPtrVoid),
      _FncPtrMsg(cb._FncPtrMsg)
   {
   };

   // operator =
   CallbackBase &operator=( const CallbackBase &cb )
   {
      _Ptr = cb._Ptr;
      _MemFncPtrVoid = cb._MemFncPtrVoid;
      _MemFncPtrMsg = cb._MemFncPtrMsg;
      _FncPtrVoid = cb._FncPtrVoid;
      _FncPtrMsg = cb._FncPtrMsg;
   
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
         (_Ptr->*_MemFncPtrVoid)();
      }
      //
      // If the function pointer is set ...
      //
      else if (_FncPtrVoid)
      {
         // Call the function ...
         _FncPtrVoid();
      }
      //
      // If nothing has been set, then do nothing ...
      //
   };
   void operator()( Message< class T >& msg)
   {
      //
      // If the object pointer variable is set ...
      //
      if (_Ptr)
      {
         // Call the member function ...
         (_Ptr->*_MemFncPtrMsg)( msg );
      }
      //
      // If the function pointer is set ...
      //
      else if (_FncPtrMsg)
      {
         // Call the function ...
         _FncPtrMsg( msg );
      }
      //
      // If nothing has been set, then do nothing ...
      //
   };

protected:

   //
   // Cannot be used outside of this class and its descendants...
   CallbackBase( CallbackBase* ptr, MemFncPtrVoid fptr ) : _Ptr( ptr ), _MemFncPtrVoid( fptr ) {};
   CallbackBase( CallbackBase* ptr, MemFncPtrMsg fptr ) : _Ptr( ptr ), _MemFncPtrMsg( fptr ) {};

private:

   CallbackBase *_Ptr;
   union { MemFncPtrVoid _MemFncPtrVoid; MemFncPtrMsg _MemFncPtrMsg; 
           FncPtrVoid _FncPtrVoid; FncPtrMsg _FncPtrMsg; };
                                           
};


template < class CallbackClass, class T > class Callback : public CallbackBase
{
private:

   //
   // Default Constructor
   Callback() : CallbackBase() { };

public:
   // pointer to a member function of Callback class
   // that takes no arguments
   typedef void (CallbackClass::*TMemFncPtrVoid)();
   typedef void (CallbackClass::*TMemFncPtrMsg)( Message< class T >& );

   // constructor that takes a pointer to member function
   Callback(CallbackClass *pp, TMemFncPtrVoid ptr) : CallbackBase( (CallbackBase*)pp, (MemFncPtrVoid)ptr ){ };
   Callback(CallbackClass *pp, TMemFncPtrMsg ptr) : CallbackBase( (CallbackBase*)pp, (MemFncPtrMsg)ptr ) { };
     
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
