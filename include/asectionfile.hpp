/*
 * Copyright (c) 1995 by Cobe BCT, Inc.  All rights reserved.
 *
 * TITLE:      aSectionfile.hpp
 * PURPOSE:    Focussed system prediction functions - wrapper for
 *             data file reader abstractions.
 * CHANGELOG:
 *   $Header: Z:/BCT_Development/Common/INCLUDE/rcs/ASECTIONFILE.HPP 1.2 1999/12/08 00:39:14 BD10648 Exp MS10234 $
 *   $Log: ASECTIONFILE.HPP $
 *   Revision 1.1  1999/05/24 23:26:12  TD10216
 *   Initial revision
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

#include "aSectionfile_element.hpp"

typedef list<aSectionfile_element> elementListType;


class aSectionfile
{
   public:

      aSectionfile();
      ~aSectionfile();

      // construct and then initialize before use.
      int initialize(const char *filename, const char *pathname);

      //  Get/set my pathname.
      const char *pathname() const;
      void pathname(const char *path);

      //  Write data to a file.  If pathname is not specified
      //  it uses the pathname it currently has.
      //  return zero on success.
      int write(const char *filename, const char *path = NULL);
      int writeSection(const char *section, const char *path = NULL);
      int writeAllSections(const char *path = NULL);

      // Used to walk thru the section list.
      // These were left using cropes for access to minimize changes
      // to source files that use it.  It is recommended to use the
      // FirstSection and NextSection member fns.
      int getFirstSection( crope &first_sections_name );
      int getNextSection( crope &next_sections_name );
      int getNextSectionElement(crope &next_sections_name, 
                                crope &next_variables_name);

      // used to walk through the list with char ptrs
      // return 0 if not found.
      const char * FirstSection( void );
      // provide char pointer of previous section
      // return 0 if no next section
      const char * NextSection( const char *ptr );

      // Used to CRC the sectionfile.
      unsigned long calculateCRC();

      // This goes through our section file element list
      // and if matches section and variable, it copies the
      // element and returns 0.  Returns non-zero if not found. 
	  int get(
          const char *section,
		  const char *variable,
		  aSectionfile_element &return_element
          );

      // return 0 on success
      //        1 on failure
      int set( const aSectionfile_element &new_element );

      // this is legacy, it does nothing now.
      int seekZero();

   protected:
       void Erase( void );
       void ErasePath( void );
       void SetPath( const char *pth );
       void MakeFileName( crope &nm , const char *fname );

      //   Copy constructor, op= are not valid for this class
      aSectionfile(const aSectionfile &orig);
      aSectionfile &operator=(const aSectionfile &orig);

   private:
      char * m_pPath; 
      elementListType _elements;
};

#endif
