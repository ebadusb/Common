/*
 * Copyright (c) 1996 by Cobe BCT, Inc.  All rights reserved.
 *
 * $Header: I:/trima_project/source/router/rcs/new.cpp 1.3 1996/07/24 19:49:52 SS03309 Exp $
 * $Log: new.cpp $
 * Revision 1.3  1996/07/24 19:49:52  SS03309
 * fix MKS 
 * Revision 1.2  1996/07/22 14:43:30  SS03309
 * Revision 1.1  1996/07/02 19:01:22  SS03309
 * Initial revision
 *
 * TITLE:      new.cpp, Focussed System - overloaded new/delete
 *
 * ABSTRACT:   Overloaded new and delete.  The "new" operator
 *             calls calloc() to clear memory.  The "delete"
 *             operator is here for completeness.  If memory
 *             can't be allocated, new calls FATAL_ERROR.
 *
 * DOCUMENTS
 * Requirements:
 * Test:
 *
 * PUBLIC FUNCTIONS:
 *  new
 *  delete
 */

#include <malloc.h>
#include "error.h"

// SPECIFICATION:    overloaded new operator
//
// ERROR HANDLING:   FATAL_ERROR.

void *operator new( size_t size)
{
   void* rtn = calloc( 1, size);
   if (rtn == NULL)
   {
      _FATAL_ERROR( __FILE__, __LINE__, TRACE_TEST1, 0, "new failed");
   }
   return rtn;
}

// SPECIFICATION:    overloaded delete operator
//
// ERROR HANDLING:   none.

void operator delete( void *type)
{
   if (type != NULL)
   {
      free( type);
   }
};


