/*
 * Copyright (c) 2000 by Gambro BCT, Inc.  All rights reserved.
 *
 * TITLE:      callback.cpp
 *
 *
 */


#include <callback.h>

#include <stdarg.h>


Callback :: Callback(FncPtr ff)
{
   //
   // Set the function pointer variable
   //
   _Ptr = 0;
   _FncPtr = ff;
};

Callback :: Callback(void *pp, ...)
{
   //
   // Recurse the arguments to get the object pointer
   //  and the member function pointer
   //
   va_list ap; va_start(ap,pp);
   _Ptr=(Callback*)pp;
   _MemFncPtr = va_arg(ap,MemFncPtr);
   _FncPtr = 0;
};

Callback :: ~Callback()
{
}

Callback :: Callback( const Callback &cb ) :
   _Ptr(cb._Ptr),
   _MemFncPtr(cb._MemFncPtr),
   _FncPtr(cb._FncPtr)
{
};

Callback &Callback :: operator=( const Callback &cb )
{
   _Ptr = cb._Ptr;
   _MemFncPtr = cb._MemFncPtr;
   _FncPtr = cb._FncPtr;

   return *this;
};

void Callback :: operator()()
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

