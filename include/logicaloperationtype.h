/*
 * Copyright (c) 2000 by Gambro BCT, Inc.  All rights reserved.
 *
 * TITLE:      logicaloperationtype.h
 *
 * ABSTRACT:   This class model enumerated type of logical operation
 *              types
 *
 */

#ifndef LOGICALOPERATIONTYPE_H                  
#define LOGICALOPERATIONTYPE_H

class LogicalOperationType
{
public:

   //
   // Logical operation type enum
   //
   // enum Type { null=-1, or, and, end };
   enum Type { nullOperation=-1, orOperation, andOperation, endOperation };

   //
   // Function to return enumerated type given a string
   //
   static Type type( const char *t );

   //
   // Function to return a string given an enumerated type 
   //
   static const char *type( const Type t );

   
private:

   static const char* _TypeString[];

};

#endif
