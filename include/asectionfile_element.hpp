/*
 * Copyright (c) 1995 by Cobe BCT, Inc.  All rights reserved.
 *
 * TITLE:      aSectionfile_element.hpp
 *
 * A section file element contains three character strings.
 * These are the section name, variable name and value.
 * Each line of the data file is parsed into a section file
 * element.  Three types of elements can exist:
 *
 * 1. A section header line: [section_name].  In this case,
 *    the section file element contains the section_name
 *    and the variable and value strings are null.
 * 2. A value line: VariableName=ValueString.  In this case,
 *    all three strings are non-null.  The section is set to
 *    the current section for this line.  The variable name is
 *    set to VariableName.  The Value string is set to ValueString.
 * 3. A comment or whitespace line.  In this case, the section and
 *    value strings are NULL and the variable name string
 *    contains a copy of the line.
 *
 * Each file line is limited to <= 255 characters by design.
 */
 
#ifndef aSectionfile_element_HPP
#define aSectionfile_element_HPP

#include <fstream.h>
#include "ap2agent.h"

class aSectionfile_element
{
   public:

      aSectionfile_element();
      ~aSectionfile_element();

      //
      //  Once I have been constucted I must be initialized.  I return
      //  zero on success.  The section and line sent is assumed to be \0 terminated.
      //  Use this init method if you know the section and have a line from a section 
	  //  file.  
	  //
      int initialize(const char *section, const char *line);

      // send to the tracelog
      void Log( void ) const;
      void Log( const char * ) const;

      //
      //  Once constucted, must be initialized.  Return
      //  zero on success.  The section, variable and value are assumed to be \0 terminated.
      //  Use this init method if all three parts of the section file element, 
	  //  section, variable name, and a value are known.
	  //
      int initialize(const char *section, const char *variable, const char *value);
      int initialize(const char *section, const char *variable, const int value);
      int initialize(const char *section, const char *variable, const float value);

      //
      //   output to a file stream
      //
      void put(ostream &str) const;

      //
      //
      //   return nonzero if a match and zero if do not match.
      //
      int match(const char *section, const char *variable) const;
      int matchSectionOnly(const char *section) const;

      //
      // Each of these return 1 if the value is valid 0 otherwise.
      int getValue(float& float_value) const;
      int getValue(int& int_value) const; 

      // get the string representing the value
      const char *getValue_charptr() const;
	  void setValue(const float value);
	  void setValue(const int value);
      void setValue(const char *value);

      //
      //
      //   Access for section and variable
      //
      const char *getVariable() const;
      const char *getSection() const;

      //
      //
      //   Copy constructor, op= are valid for this class
      //
      aSectionfile_element(const aSectionfile_element &orig);
      aSectionfile_element &operator=(const aSectionfile_element &orig);
      int operator==(const aSectionfile_element &orig) const;      
	  int operator<(const aSectionfile_element &orig) const;

   protected:

      void cleanup( void );
      void EraseValue( void );
      void copyover(const aSectionfile_element &orig);
      int SafeCompare( const char *p1 , const char *p2 ) const;
      void CleanStr( char * str );

   private:
      char * m_pVar;
      char * m_pSct;
      char * m_pVal;

};

#endif
