
/************************> TRIMA CONFIG Conversion <****************************

*     Copyright(c) 1999 COBE BCT, Inc. All rights reserved

*     Author:      Terry Wahl

*     Class name:  NEW_drvr_collect

*     File name:   NEW_drvr_collect.cpp

*     Contents:    Implementations of class methods

*     Description: See descriptions in corresponding .hpp file.

*************************< TRIMA CONFIG Conversion >****************************

$Header$
$Log: new_drvr_collect.cpp $
Revision 1.1  2001/08/16 13:50:13  jl11312
Initial revision
Revision 1.2  2001/06/29 15:50:54  jl11312
- moved #include to column zero so make depend works properly
Revision 1.1  1999/12/10 14:36:53  BS04481
Initial revision
*/


#include "new_drvr_collect.hpp"

#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

// System #INCLUDES

#include <stdlib.h>
#include <string.h>



////////////////////////////////////////////////////////////////////////////////
//GENERATE_NEW_CONFIG_FILE
void
NEW_drvr_collect::generate_new_config_file (char * file_name, OLD_cfg_collect * old_config_collection)
{

   // collection element pointer
   NEW_drvr_element * NEW_drvr_element_ptr ;

   // file handle
   FILE * filehandle ;

   // file buffer
   char filebuff[MAX_FILE_RECORD_SIZE] ;


   // Set the collection cursor to the top of the colletion
   set_cursor_to_top () ;


   // Open the configuration file for output

   filehandle = fopen (file_name, "w") ;

   if (filehandle == NULL)
   {
      printf("**** Unable to open NEW configuration file name = \"%s\" ****\n", file_name) ;
      exit (BAD_EXIT) ;
   }



   // Get the first entry
   NEW_drvr_element_ptr = get_next_entry () ;

   // Loop through all the driver elements and have them generate a new config reocrd

   while (NEW_drvr_element_ptr != NULL)
   {

      if (NEW_drvr_element_ptr -> create_config_text_from_driver_record (old_config_collection, &enum_trans_hit_collection,  filebuff) == WRITE_REC)
      {
         // add final newline to this record (maybe embedded newlines from last call)
         strcat (filebuff, "\n") ;

         if (fputs (filebuff, filehandle) == EOF)
            {
                printf("**** Error while writing NEW configuration record = \"%s\" ****\n", filebuff) ;
                exit (BAD_EXIT) ;
            }
      }


      // Get the next entry
      NEW_drvr_element_ptr = get_next_entry () ;

   } // End of while (NEW_drvr_element_ptr != NULL)


   // close file
   fclose (filehandle) ;


};  // END of NEW_drvr_collect::generate_new_config_file



