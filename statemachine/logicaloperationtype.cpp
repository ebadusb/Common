/*
 * Copyright (c) 2000 by Gambro BCT, Inc.  All rights reserved.
 *
 * TITLE:      logicaloperationtype.h
 *
 * ABSTRACT:   This class model enumerated type of logical operation
 *              types
 *
 */

#include "logicaloperationtype.h"
#include <string.h>

const char *LogicalOperationType::_TypeString[] = { "null", "or", "and" };

LogicalOperationType::Type LogicalOperationType :: type( const char *t )
{
   for ( int i=nullOperation ; i<endOperation ; i++ )
   {
      if ( strcmp( t , _TypeString[i+1] ) == 0 )
         return (Type)i;
   }
   return nullOperation;
}

const char *LogicalOperationType :: type( const LogicalOperationType::Type t )
{
   for ( int i=nullOperation+1 ; i<endOperation ; i++ )
   {
      if ( t == (Type)i )
         return _TypeString[i+1];
   }
   return _TypeString[0];
}
