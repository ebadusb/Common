/*
 * Copyright (c) 1995 by Cobe BCT, Inc.  All rights reserved.
 *
 * TITLE:      aSectionfile_element.hpp
 *             Focussed system prediction functions - wrapper for
 *             data file reader abstractions.
 *
 * AUTHOR:     R.E. Edwards
 *
 * ABSTRACT:
 *
 * DOCUMENTS
 * Requirements:
 * Test:
 *
 * EXAMPLE:
 *
 *
 */

#ifndef aSectionfile_element_HPP
#define aSectionfile_element_HPP

#include <fstream.h>
#include <Rope.h>
#include "ap2agent.h"

class aSectionfile_element
{

   //
   //
   //   The following methods are available for general use.
   //
   public:

      // Constructor
      aSectionfile_element();

      // Destructor
      virtual ~aSectionfile_element();

      //
      //
      //  Once I have been constucted I must be initialized.  I return
      //  zero on success.  The section and line sent is assumed to be \0 terminated.
      //  Use this init method if you know the section and have a line from a section 
	  //  file.  
	  //
      virtual const int initialize(const char *section, const char *line);

      //
      //
      //  Once I have been constucted I must be initialized.  I return
      //  zero on success.  The section, variable and value are assumed to be \0 terminated.
      //  Use this init method if you have all three parts of the section file element, 
	  //  section, variable name, and a value for the section variable.
	  //
      virtual const int initialize(const char *section, const char *variable, const char *value);
      virtual const int initialize(const char *section, const char *variable, const int value);
      virtual const int initialize(const char *section, const char *variable, const float value);
      //
      //
      //   You can ask me to output myself to a file stream
      //
      void put(ostream &str) const;

      //
      //
      //   You can ask me if I match a given variable and section name.
      //   I return nonzero if I match and zero if I do not match.
      //
      virtual const int match(const char *section, const char *variable) const;
      virtual const int matchSectionOnly(const char *section) const;
      //
      //
      //  You can get or set my data value.
      //
      // Each of these return 1 if the value is valid 0 otherwise.
      virtual const int getValue(float& float_value) const;
      virtual const int getValue(int& int_value) const; 
	   
      virtual const char *getValue_charptr() const;
	   virtual void setValue(const float value);
	   virtual void setValue(const int value);
      virtual void setValue(const char *value);
      //
      //
      //   Accessors for my section and variable
      //
      virtual const char *getVariable() const;
      virtual const char *getSection() const;

      //
      //
      //   Copy constructor, op= are valid for this class
      //
      aSectionfile_element(const aSectionfile_element &orig);
      aSectionfile_element &operator=(const aSectionfile_element &orig);
      int operator==(const aSectionfile_element &orig) const;      
	  int operator<(const aSectionfile_element &orig) const;
   //
   //
   //   These methods are for internal use only.
   //
   protected:

      //
      //   methods to invoke internally for construction / destruction
      //   (note non-virtual)
      void cleanup();
      void copyover(const aSectionfile_element &orig);

//
//
//-----------------------------------------------------------------------------
//                      Member data
//
   protected:
      crope _variable;
      crope _section;
      crope _value;

};

#endif
