/*******************************************************************************
 *
 * Copyright (c) 2002 by Gambro BCT, Inc.  All rights reserved.
 *
 * TITLE:      carray.h
 *             Template for generic type array.
 *
 * AUTHOR:     Regis McGarry
 *
 * ABSTRACT:   This file defines the template class for a generic array that
 *             provides iterator support and all expected operators.  Safe for
 *             usage in a datastore.
 *
 * HISTORY:    $Log: carray.h $
 * HISTORY:    Revision 1.2  2002/05/30 14:21:43  rm70006
 * HISTORY:    Added file header
*******************************************************************************/

#ifndef __CARRAY_H
#define __CARRAY_H
#include <cstddef>


template < class T, std::size_t thesize>
class carray 
{
private:
   T v[thesize];   // fixed-size array of elements of type T

public:
   // type definitions
   typedef T value_type;
   typedef T* iterator;
   typedef const T* const_iterator;
   typedef T& reference;
   typedef const T& const_reference;
   typedef size_t size_type;
   typedef ptrdiff_t difference_type;

   // iterator support
   iterator begin() { return v; }
   const_iterator begin() const { return v; }
   iterator end() { return v + thesize; }
   const_iterator end() const { return v + thesize; }

   // direct element access
   reference operator[] (size_t i) { return v[i]; }
   const_reference operator[] (size_t i) const { return v[i]; }
   
   // size is constant
   size_type size() const { return thesize; }
   size_type max_size() const { return thesize; }

   // conversion to ordinary array
   T* as_array() { return v; }
};
#endif


