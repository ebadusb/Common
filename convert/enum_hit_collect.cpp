
/************************> TRIMA CONFIG Conversion <****************************

*     Copyright(c) 1999 COBE BCT, Inc. All rights reserved

*     Author:       Terry Wahl

*     Class names:   ENUM_hit_collect
*                    ENUM_hit_element

*     File name:    enum_hit_collect.cpp

*     Contents:     See prototype header descriptions below...

*     Description:

         This class manages a collection of objects that record the
         creation of NEW config file records for ENUM_translation. These
         records permit the NEW_drvr_element to know whether or not one of
         the ENUM translation driver records was hit and therefore does not
         need to create a config file record from an ENUM DEFAULT driver record.

*************************< TRIMA CONFIG Conversion >****************************

$Header: //bctquad3/home/BCT_Development/Common/disk_build/install_source/cfgconv_common/rcs/enum_hit_collect.cpp 1.1 2001/08/16 13:50:11 jl11312 Exp $
$Log: enum_hit_collect.cpp $
Revision 1.1  2001/08/16 13:50:11  jl11312
Initial revision
Revision 1.1  1999/12/10 14:36:37  BS04481
Initial revision

*/



// Single include define guardian
#ifndef ENMHITCOL_HPP
#define ENMHITCOL_HPP

#include "common_defs.h"

#include "tablecol.hpp"         // include the base template declaration

// System #INCLUDES

#include <stdlib.h>
#include <string.h>



/// Declare / define the ENUM_hit_element
class ENUM_hit_element
{
   private:

      char section_name[MAX_SECTION_KEYWORD_STRING] ;

      char keyword_name[MAX_SECTION_KEYWORD_STRING] ;

   protected:


   public:

   //CONSTRUCTOR

   ENUM_hit_element (char * p_section_name, char * p_keyword_name)
   {
      strcpy (section_name, p_section_name) ;
      strcpy (keyword_name, p_keyword_name) ;
   };

   //GET_SECTION
   char * get_section ()
   {
      return (section_name) ;
   };

   //GET_KEYWORD
   char * get_keyword ()
   {
      return (keyword_name) ;
   };


   //DESTRUCTOR
   virtual ~ENUM_hit_element ()
   {
   } ;


}; // END of ENUM_hit_element




class ENUM_hit_collect : public Table_Collection<ENUM_hit_element>
{
   private:




   protected:



   public:

      //CONSTRUCTOR
      ENUM_hit_collect ()
      {
      } ;


      //DESTRUCTOR
      virtual ~ENUM_hit_collect ()
      {
      } ;


      //EXISTS_ENUM_TRANSLATION_HIT
      // This method searches the collection of ENUM_hit_elements to find
      // one containing the section and keyword names passed as parameters.
      // If it finds one it returns SUCCESS, if not, FAILURE
      int exists_enum_translation_hit (char * NEW_section_name, char * NEW_keyword_name)
      {

         // collection element pointer
         ENUM_hit_element * ENUM_hit_element_ptr ;

         // Set the collection cursor to the top of the collection
         set_cursor_to_top () ;

         // Get the first entry
         ENUM_hit_element_ptr = get_next_entry () ;

         // Loop through all the driver elements and have them generate a new config reocrd

         while (ENUM_hit_element_ptr != NULL)
         {

            if (strcmp(ENUM_hit_element_ptr->get_section(), NEW_section_name) == 0
                &&
                strcmp(ENUM_hit_element_ptr->get_keyword(), NEW_keyword_name) == 0
               )
            {
               return (SUCCESS) ;
            }

            // Get the next entry
            ENUM_hit_element_ptr = get_next_entry () ;

         } // End of while (ENUM_hit_element_ptr != NULL)

         return (FAILURE) ;

      }; // END of exists_enum_translation_hit



      //POST_ENUM_TRANSLATION_HIT
      // This method creates a new ENUM_hit_element and adds it to the
      // collection.
      void post_enum_translation_hit (char * NEW_section_name, char * NEW_keyword_name)
      {
         add_table_entry (new ENUM_hit_element(NEW_section_name, NEW_keyword_name)) ;
      };


};  // END of ENUM_hit_collect Class Declaration



// End of single include define guardian
#endif



