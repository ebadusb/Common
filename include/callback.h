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

#include <stdlib.h>

class CallbackBase
{
public:
   // pointer to a member function of CallbackBase class
   // that takes no arguments
   typedef void (CallbackBase::*MemFncPtrVoid)();
   //  ... function that takes one void* argument
   typedef void (CallbackBase::*MemFncPtrData)( void* );

   // pointer to a non-member function that takes no
   // arguments
   typedef void (*FncPtrVoid)();
   //  ... function that takes one void* argument
   typedef void (*FncPtrData)( void* );

   // constructor that takes a pointer to a non member
   // void function
   CallbackBase( FncPtrVoid ff=0 ) :
      _Ptr( 0 ),
      _FncPtrVoid( ff ),
      _FncPtrData( 0 ),
      _Data( 0 )
   {
		_Assigned = (ff != NULL);
   };

   // constructor that takes a pointer to a non member
   // function taking a void* and the data to be sent.
   CallbackBase( FncPtrData ff, void *data=0 ) :
      _Ptr( 0 ),
      _FncPtrVoid( 0 ),
      _FncPtrData( ff ),
      _Data( data )
   {
		_Assigned = (ff != NULL);
   };

   // destructor
   virtual ~CallbackBase() {};

   // copy constructor
   CallbackBase( const CallbackBase &cb ) :
      _Ptr(cb._Ptr),
      _FncPtrVoid(cb._FncPtrVoid),
      _MemFncPtrVoid(cb._MemFncPtrVoid),
      _FncPtrData(cb._FncPtrData),
      _MemFncPtrData(cb._MemFncPtrData),
      _Data(cb._Data),
      _Assigned(cb._Assigned)
   {
   };

   // operator =
   CallbackBase &operator=( const CallbackBase &cb )
   {
      _Ptr = cb._Ptr;
      _FncPtrVoid = cb._FncPtrVoid;
      _MemFncPtrVoid = cb._MemFncPtrVoid;
      _FncPtrData = cb._FncPtrData;
      _MemFncPtrData = cb._MemFncPtrData;
      _Data = cb._Data;
      _Assigned = cb._Assigned;
   
      return *this;
   };

	bool assigned(void) { return _Assigned; }

   // function dispatch
   void operator()() const
   {
      //
      // If the object pointer variable is set ...
      //
      if (_Ptr)
      {
         // Call the member function ...
         if ( _MemFncPtrVoid )
         {
            (_Ptr->*_MemFncPtrVoid)();
         }
         else if ( _MemFncPtrData )
         {
            (_Ptr->*_MemFncPtrData)( _Data );
         }
      }
      //
      // If the function pointer is set ...
      //
      else if (_FncPtrVoid)
      {
         // Call the function ...
         _FncPtrVoid();
      }
      else if (_FncPtrData)
      {
         // Call the function ...
         _FncPtrData( _Data );
      }
      //
      // If nothing has been set, then do nothing ...
      //
   };
   // ...call the function with the specified data.
   void operator()( void* d ) const 
   {
      //
      // If the object pointer variable is set ...
      //
      if (_Ptr)
      {
         // Call the member function ...
         if ( _MemFncPtrData )
         {
            (_Ptr->*_MemFncPtrData)( d );
         }
      }
      //
      // If the function pointer is set ...
      //
      else if (_FncPtrData)
      {
         // Call the function ...
         _FncPtrData( d );
      }
      //
      // If nothing has been set, then do nothing ...
      //
   };

protected:

   //
   // Cannot be used outside of this class and its descendants...
   CallbackBase( CallbackBase* ptr, MemFncPtrVoid fptr ) 
   : _Ptr( ptr ), _MemFncPtrVoid( fptr ), _MemFncPtrData( 0 ), _Data( 0 ) { _Assigned = (fptr != NULL); }
   CallbackBase( CallbackBase* ptr, MemFncPtrData fptr, void *data ) 
   : _Ptr( ptr ), _MemFncPtrVoid( 0 ), _MemFncPtrData( fptr ), _Data( data ) { _Assigned = (fptr != NULL); }

private:

   CallbackBase *_Ptr;
   union { MemFncPtrVoid _MemFncPtrVoid; FncPtrVoid _FncPtrVoid; };
   union { MemFncPtrData _MemFncPtrData; FncPtrData _FncPtrData; };
   void *_Data;
   bool _Assigned;
                                           
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
   typedef void (CallbackClass::*TMemFncPtrVoid)();
   typedef void (CallbackClass::*TMemFncPtrData)( void* );

   // pointer to a static member function of Callback class
   // that takes no arguments
   typedef void ( *TMemFncPtrVoidS)();
   typedef void ( *TMemFncPtrDataS)( void* );
   
   // constructor that takes a pointer to member function that takes no arguments
   Callback(CallbackClass *pp, TMemFncPtrVoid ptr) 
   : CallbackBase( (CallbackBase*)pp, (MemFncPtrVoid)ptr ){ };
   // constructor that takes a pointer to member function that takes a void* argument
   Callback(CallbackClass *pp, TMemFncPtrData ptr, void *data=0 ) 
   : CallbackBase( (CallbackBase*)pp, (MemFncPtrData)ptr, data ) { };
     
   // constructor that takes a pointer to a static member function that takes no arguments
   Callback( TMemFncPtrVoidS ptr) 
   : CallbackBase( (FncPtrVoid)ptr ){ };
   // constructor that takes a pointer to a static member function that takes a void* argument
   Callback( TMemFncPtrDataS ptr, void *data=0 ) 
   : CallbackBase( (FncPtrData)ptr, data ) { };
   
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
