
/************************> TRIMA CONFIG Conversion <****************************

*     Copyright(c) 1999 COBE BCT, Inc. All rights reserved

*     Author:      Terry Wahl

*     Class name:  OLD_cfg_collect

*     File name:   old_cfg_collect.cpp

*     Contents:    Implementations of class methods

*     Description: See descriptions in corresponding .hpp file.

*************************< TRIMA CONFIG Conversion >****************************

$Header: //bctquad3/home/BCT_Development/Common/disk_build/install_source/cfgconv_common/rcs/old_cfg_collect.cpp 1.1 2001/08/16 13:50:18 jl11312 Exp $
$Log: old_cfg_collect.cpp $
Revision 1.1  2001/08/16 13:50:18  jl11312
Initial revision
Revision 1.1  1999/12/10 14:37:11  BS04481
Initial revision
*/




// EXTERNAL REFERENCES

#include "old_cfg_element.hpp"
#include "old_cfg_collect.hpp"

#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

// System #INCLUDES

#include <stdlib.h>
#include <string.h>




////////////////////////////////////////////////////////////////////////////////
//PARM_PARSE

char
OLD_cfg_collect::parm_parse (char * parse_string,
                             int &  parsing_index,
                             char * delim_string,
                             char * whitespace_string,
                             char * token_string)
{
   // Init the current char string to null with a null eos.
   char current_char[2] = "\0" ;

   // Init token string to null
   token_string[0] = CHAR_ZERO ;

   // Reserve a char pointer for the delimiter check
   char * delim_ptr ;


   while (1)
   {
      current_char[0] = parse_string[parsing_index++] ;

      // End of string, so return
      if (current_char[0] == CHAR_ZERO)
      {
        return (CHAR_ZERO) ;
      }


      // See if this is a valid delimiter
      delim_ptr = strchr(delim_string, current_char[0]) ;

      if (delim_ptr != NULL)
      {
         // We hit a delimiter, so return
         return (*delim_ptr) ;
      }


      // See if this is white space to be ignored
      if (strchr(whitespace_string, current_char[0]) != NULL)
      {
         // it was whitespace so skip past this iteration
         continue;
      }

      // Anything else we move into the token buffer

      strcat(token_string, current_char) ;

   } // end of while (1)


} ;   // END of OLD_cfg_collect::parm_parse



////////////////////////////////////////////////////////////////////////////////
//PARSE_AND_STORE_CONFIG_RECORD

int
OLD_cfg_collect::parse_and_store_config_record (char * config_record)
{

   //parsing index
   int parse_index = 0 ;

   //Delimiter
   char delimiter ;

   //Big token buffer
   char token[MAX_TOKEN_SIZE] ;

   //Keyword name of the config value
   char keyword_name[MAX_SECTION_KEYWORD_STRING+1];

   //Config value
   double config_value_temp ;
   CONFIG_DATA_TYPE  config_value ;


   // Get the first token
   delimiter = parm_parse (config_record, parse_index, STRING_KEYWORD_EQUATE, " \n", token) ;


   // Ignore comments, blank lines
   if ( (strcmp(token, "") == 0 && delimiter == CHAR_ZERO) //blank
         ||
        (token[0] == CHAR_COMMENT) // comment
      )
   {
      return (SUCCESS) ; // no config value here but a valid blank or comment
   }


   // Check for a new Section header
   if (token[0] == CHAR_SECTION)
   {
      // we got a header -- move it to the Section header hold buffer.
      strncpy (current_section_name, token, MAX_SECTION_KEYWORD_STRING) ;
      current_section_name[MAX_SECTION_KEYWORD_STRING] = CHAR_ZERO ; // cauderize

      return(SUCCESS) ;  // valid section name
   }

   // If we're here, we must have a config keyword delimited by an equal(=) sign.
   if (delimiter != CHAR_KEYWORD_EQUATE)
   {
      return (FAILURE); // invalid keyword record.
   }

   // One must assume we have a valid keyword... move it into the hold buffer
   strncpy (keyword_name, token, MAX_SECTION_KEYWORD_STRING) ;
   keyword_name[MAX_SECTION_KEYWORD_STRING] = CHAR_ZERO ; // cauderize

   // Let's get the numeric value
   delimiter = parm_parse (config_record, parse_index, "", " \n", token) ;

   // Convert to a double -- the only floating point conversion available
   config_value_temp = atof(token) ;

   // Make this a CONFIG_DATA_TYPE value
   config_value = CONFIG_DATA_TYPE(config_value_temp) ;

   // Create and store the config element
   add_table_entry (new OLD_cfg_element(current_section_name, keyword_name, config_value)) ;

   /* DEBUG
      printf ("CURSEC=\"%s\",CURKEY=\"%s\",CONFIG_VALUE=\"%f\"\n", current_section_name, keyword_name, config_value) ;
   */

   // Our work here is done
   return(SUCCESS) ;

} ;  // END of OLD_cfg_collect::parse_and_store_config_record




////////////////////////////////////////////////////////////////////////////////
//LOAD_OLD_CONFIG_COLLECTION
 void
 OLD_cfg_collect::load_old_config_collection (char * filename)
 {

   // file handle
   FILE * filehandle ;

   // file buffer
   char filebuff[MAX_FILE_RECORD_SIZE] ;


   filehandle = fopen (filename, "r") ;

   if (filehandle == NULL)
   {
      printf("**** Unable to open OLD configuration file name = \"%s\" ****\n", filename) ;
      exit (BAD_EXIT) ;
   }


   // Read all the OLD config records in and attempt to create a collection record
   while (fgets (filebuff, sizeof(filebuff)-1, filehandle) != NULL)
   {
      int return_code = parse_and_store_config_record (filebuff) ;

      if (return_code != SUCCESS)
      {
         printf("**** Parsing error encountered in OLD configuration error. Record = \"%s\" ****\n", filebuff) ;
      }
   }

   // close file
   fclose (filehandle) ;

 } ;  // END of OLD_cfg_collect::load_old_config_collection




////////////////////////////////////////////////////////////////////////////////
//GET_SECTION_KEYWORD_VALUE

int
OLD_cfg_collect::get_section_keyword_value (char * parm_section_name, char * parm_keyword_name, CONFIG_DATA_TYPE & config_value)
{
   OLD_cfg_element * cfg_element_ptr ;

   CONFIG_DATA_TYPE ret_value ;

   // Set the collection cursor to the top of the colletion
   set_cursor_to_top () ;

   // Get the first entry
   cfg_element_ptr = get_next_entry () ;


   // Loop through all the config elements and compare the section name and keyname
   // looking for a match

   while (cfg_element_ptr != NULL)
   {
      // See if this is a match
      if (cfg_element_ptr -> compare_config_element (parm_section_name, parm_keyword_name, ret_value) == 1)
      {
         // we found a match: assign the value and return successfully
         config_value = ret_value ;
         return (SUCCESS) ;
      }

      // Get the next entry
      cfg_element_ptr = get_next_entry () ;

   } // End of while (cfg_element_ptr != NULL)

   // didn't find an entry. set value to zero and indicate failure
   config_value = 0.0f ;
   return (FAILURE) ;


} ;  // END of OLD_cfg_collect::get_section_keyword_value



