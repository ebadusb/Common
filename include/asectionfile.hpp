/*
 * Copyright (c) 1995 by Cobe BCT, Inc.  All rights reserved.
 *
 * TITLE:      aSectionfile.hpp
 * PURPOSE:    Focussed system prediction functions - wrapper for
 *             data file reader abstractions.
 * CHANGELOG:
 *   $Header: Z:/BCT_Development/Common/INCLUDE/rcs/ASECTIONFILE.HPP 1.2 1999/12/08 00:39:14 BD10648 Exp MS10234 $
 *   $Log: ASECTIONFILE.HPP $
 *   Revision 1.10  1999/04/20 00:40:31  TD07711
 *     In crope's less_than function, changed args from crope objects to
 *     const crope& to avoid unecessary copy constructiing and destruction.
 *     This should help performance problem associated with IT3650.
 *   1.1 - 1.9 Tate Moore
 */

#ifndef ASECTIONFILE_HPP
#define ASECTIONFILE_HPP

#include <Rope.h>
#include <List.h>
#include <map.h>

#include "aSectionfile_element.hpp"

typedef list<aSectionfile_element> elementListType;

// Must define a class with a operator () that is a compare function for the 
// map type used below.  STL requirement.
struct crope_less_then
{  
   bool operator()(const crope& s1, const crope& s2) const  
   {
      return s1.compare(s2) < 0;
   }
};

typedef map<crope, void*, crope_less_then> elementMapType;

class aSectionfile
{

   //
   //
   //   The following methods are available for general use.
   //
   public:

      // Constructor
      aSectionfile();

      // Destructor
      virtual ~aSectionfile();

      //
      //
      //  Once I have been constucted I must be initialized.  I return
      //  zero on success.
      //
      virtual const int initialize(const char *filename, const char *pathname);

      //
      //
      //  Get/set my pathname.
      //
      virtual const char *pathname() const;
      virtual void pathname(const char *path);

      //
      //
      //  You can ask me to write my data to a file.  If you do not specify
      //  a pathname I use the pathname I currently have.  If you rewrite me
      //  using a filename then the filename given replaces any previous path.
      //  I return zero on success.
      //
      virtual const int write(const char *filename, const char *path = NULL);
      virtual const int writeSection(const char *section, const char *path = NULL);
      virtual const int writeCurrentSection(const char *path = NULL);
      virtual const int writeAllSections(const char *path = NULL);


      // Used to walk thru the section list. 
      virtual const int getFirstSection(crope &first_sections_name);
      virtual const int getNextSection(crope &next_sections_name);
      virtual const int getNextSectionElement(crope &next_sections_name, 
                                              crope &next_variables_name);

      // Used to CRC the sectionfile.
      virtual unsigned long calculateCRC();

      //
      //
      //  You can get or set any piece of data in my file.  (Changes are only
      //  internal until you ask me to write).  Generally the variable name
      //  is unique so you do not need to specify the section name.  If I
      //  cannot find the variable name or section while getting data then I
      //  return zero for the value and a nonzero result.  I return a zero
      //  status if everything was successful.  If the variable name specified
      //  is not unique and the section name is not specified then I set or
      //  return the value for the first matching variable found.
      //
	  virtual const int get(const char *section,
						         const char *variable,
							      aSectionfile_element &return_element);
      virtual const int set(const aSectionfile_element &new_element);

      const int seekZero();

   //
   //
   //   These methods are for internal use only.
   //
   protected:
      virtual elementListType::iterator *find_element(const char *section, 
                                                 const char *variable);

      const int aSectionfile::seek( const char *section = NULL, const char *variable = NULL);

      //
      //
      //   Copy constructor, op= are not valid for this class
      //
      aSectionfile(const aSectionfile &orig);
      aSectionfile &operator=(const aSectionfile &orig);
//
//
//-----------------------------------------------------------------------------
//                      Member data
//
   protected:
      crope _pathname;                                   // my pathname
      elementListType _elements;                         // internal list of data file elements
      elementMapType _elementsMap;                       // a map of the elements for fast lookup.
      elementListType::iterator _current_pos_iterator;   // an iterator for the list.
      elementListType::iterator _end_pos_iterator;       // another iterator for the list.
};

#endif
