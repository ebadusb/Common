
/************************> TRIMA CONFIG Conversion <****************************

*     Copyright(c) 1999 COBE BCT, Inc. All rights reserved

*     Author:      Terry Wahl

*     Class name:  NEW_drvr_element

*     File name:   new_drvr_element.cpp

*     Contents:    Implementations of class methods

*     Description: See descriptions in corresponding .hpp file.

*************************< TRIMA CONFIG Conversion >****************************

$Header$
$Log: new_drvr_element.cpp $
Revision 1.1  2001/08/16 13:50:15  jl11312
Initial revision
Revision 1.1  1999/12/10 14:37:02  BS04481
Initial revision
*/



#include "new_drvr_element.hpp"
#include <string.h>
#include <stdio.h>


// Null the last section name buffer
char NEW_drvr_element::LAST_section_name[]="" ;



////////////////////////////////////////////////////////////////////////////////
//MAINTAIN_LAST_SECTION_NAME

static int
NEW_drvr_element::maintain_last_section_name (char * section_name)
{
   if (strcmp (NEW_drvr_element::LAST_section_name, section_name) == 0)
   {
      return (NOCHANGE) ;
   }
   else
   {
      strcpy  (NEW_drvr_element::LAST_section_name, section_name) ;
      return (CHANGED) ;
   }

} ;   // END of NEW_drvr_element::maintain_last_section_name





////////////////////////////////////////////////////////////////////////////////
//CREATE_CONFIG_TEXT_FROM_DRIVER_RECORD

int
NEW_drvr_element::create_config_text_from_driver_record (OLD_cfg_collect * OLD_config_data,
                                                         ENUM_hit_collect * enum_hit_list,
                                                         char * return_text_record)
{
   //Data format buffer
   char data_buffer[MAX_DATA_BUFFER] ;

   //Return code
   int return_code = NO_REC ;

   //OLD config data buffer
   CONFIG_DATA_TYPE old_config_data ;

   //NULL the return record buffer
   return_text_record[0] = CHAR_ZERO ;


   //Determine the driver action from the conversion operator
   switch (conversion_operator)
   {

   case OLD_VALUE :

      prepare_new_keyword_record (return_text_record) ;

      if (OLD_config_data -> get_section_keyword_value (OLD_section_name, OLD_keyword_name, old_config_data) == SUCCESS)
      {
         strcat (return_text_record, format_config_data(old_config_data, NEW_type_format, data_buffer));
      }
      else
      {
         strcat (return_text_record, format_config_data(NEW_default_value, NEW_type_format, data_buffer));

         printf("**** Old config value not found. Using New template value for = \"%s\" ****\n", NEW_keyword_name) ;
      }

      return_code = WRITE_REC ;

      break;



   case NEW_VALUE :

        prepare_new_keyword_record (return_text_record) ;

        strcat (return_text_record, format_config_data(NEW_default_value, NEW_type_format, data_buffer));

        return_code = WRITE_REC ;

      break;



   case COMMENT_LINE :

        strcpy (return_text_record, NEW_section_name);  // Comments are held in the NEW_section_name field.

        return_code = WRITE_REC ;

      break;


   case ENUM_TRANS_OLD :

      if (OLD_config_data -> get_section_keyword_value (OLD_section_name, OLD_keyword_name, old_config_data) == SUCCESS)
      {
         if ((old_config_data == OLD_operand_value)
             &&
              //It hasn't already been translated, i.e. first translation wins.
             (enum_hit_list -> exists_enum_translation_hit (NEW_section_name, NEW_keyword_name) == FAILURE)
            )
         {
            // We've hit an enum translation; build the record

            prepare_new_keyword_record (return_text_record) ;

            strcat (return_text_record, format_config_data(NEW_default_value, NEW_type_format, data_buffer));

            enum_hit_list -> post_enum_translation_hit (NEW_section_name, NEW_keyword_name) ;

            return_code = WRITE_REC ;

         } // End of " if (old_config_data == OLD_operand_value)

      }
      else
      {
         printf("**** Old config value not found for enum translation: \"%s\" ****\n", OLD_keyword_name) ;
      }


      break;



   case SCALE_OLD :

      prepare_new_keyword_record (return_text_record) ;

      if (OLD_config_data -> get_section_keyword_value (OLD_section_name, OLD_keyword_name, old_config_data) == SUCCESS)
      {
            // We've found the old value; compute the scaled value from old value; build the record

            strcat (return_text_record, format_config_data((old_config_data * OLD_operand_value), NEW_type_format, data_buffer));

      }
      else
      {
         printf("**** Old config value not found for scaling -- new default value is being used: \"%s\" ****\n", OLD_keyword_name) ;

         strcat (return_text_record, format_config_data(NEW_default_value, NEW_type_format, data_buffer));
      }

      return_code = WRITE_REC ;

      break;




   case OFFSET_OLD :

      prepare_new_keyword_record (return_text_record) ;

      if (OLD_config_data -> get_section_keyword_value (OLD_section_name, OLD_keyword_name, old_config_data) == SUCCESS)
      {
            // We've found the old value; compute the scaled value from old value; build the record

            strcat (return_text_record, format_config_data((old_config_data + OLD_operand_value), NEW_type_format, data_buffer));

      }
      else
      {
         printf("**** Old config value not found for offsetting -- new default value is being used: \"%s\" ****\n", OLD_keyword_name) ;

         strcat (return_text_record, format_config_data(NEW_default_value, NEW_type_format, data_buffer));
      }

      return_code = WRITE_REC ;

      break;



   case HIGH_LIMIT :

      if (OLD_config_data -> get_section_keyword_value (OLD_section_name, OLD_keyword_name, old_config_data) == SUCCESS)
      {
         if ((old_config_data >= NEW_default_value)
             &&
              //It hasn't already been translated, i.e. first translation wins.
             (enum_hit_list -> exists_enum_translation_hit (NEW_section_name, NEW_keyword_name) == FAILURE)
            )
         {
            // We've hit a high limit; build the record

            prepare_new_keyword_record (return_text_record) ;

            strcat (return_text_record, format_config_data(NEW_default_value, NEW_type_format, data_buffer));

            enum_hit_list -> post_enum_translation_hit (NEW_section_name, NEW_keyword_name) ;

            return_code = WRITE_REC ;

         } // End of " if (old_config_data >= NEW_default_value)

      }
      else
      {
         printf("**** Old config value not found for high limit test: \"%s\" ****\n", OLD_keyword_name) ;
      }


      break;


   case LOW_LIMIT :

      if (OLD_config_data -> get_section_keyword_value (OLD_section_name, OLD_keyword_name, old_config_data) == SUCCESS)
      {
         if ((old_config_data <= NEW_default_value)
             &&
              //It hasn't already been translated, i.e. first translation wins.
             (enum_hit_list -> exists_enum_translation_hit (NEW_section_name, NEW_keyword_name) == FAILURE)
            )
         {
            // We've hit a low limit; build the record

            prepare_new_keyword_record (return_text_record) ;

            strcat (return_text_record, format_config_data(NEW_default_value, NEW_type_format, data_buffer));

            enum_hit_list -> post_enum_translation_hit (NEW_section_name, NEW_keyword_name) ;

            return_code = WRITE_REC ;

         } // End of " if (old_config_data <= NEW_default_value)

      }
      else
      {
         printf("**** Old config value not found for low limit test: \"%s\" ****\n", OLD_keyword_name) ;
      }


      break;


   case LIMIT_DEFAULT_OLD:


      if (enum_hit_list -> exists_enum_translation_hit (NEW_section_name, NEW_keyword_name) == FAILURE)
      {
         // This value has not been translated by the limit tests

         prepare_new_keyword_record (return_text_record) ;
         if (OLD_config_data -> get_section_keyword_value (OLD_section_name, OLD_keyword_name, old_config_data) == SUCCESS)
         {
            strcat (return_text_record, format_config_data(old_config_data, NEW_type_format, data_buffer));
         }
         else
         {
            strcat (return_text_record, format_config_data(NEW_default_value, NEW_type_format, data_buffer));
            printf("**** Old config value not found. Using New template value for = \"%s\" ****\n", NEW_keyword_name) ;
         }

         enum_hit_list -> post_enum_translation_hit (NEW_section_name, NEW_keyword_name) ;
         return_code = WRITE_REC ;
      }

      break;


   case ENUM_DEFAULT_OLD:


      if (enum_hit_list -> exists_enum_translation_hit (NEW_section_name, NEW_keyword_name) == FAILURE)
      {
         // This value has not been translated so use the default value

         prepare_new_keyword_record (return_text_record) ;

         strcat (return_text_record, format_config_data(NEW_default_value, NEW_type_format, data_buffer));

         enum_hit_list -> post_enum_translation_hit (NEW_section_name, NEW_keyword_name) ;

         return_code = WRITE_REC ;

         // Display a warning that the default value was used 'cause there was no match

         printf("**** Using the default value in an ENUM conversion for = \"%s\" ****\n", NEW_keyword_name) ;

      }

      break;


   default:

      printf("**** Encountered unknown conversion driver operator = \"%d\" ****\n", conversion_operator) ;
      exit (BAD_EXIT) ;

      return (NO_REC) ; // This is for compiler happiness

   } // END of "switch (conversion_operator)


   //Return with the action to be taken
   return (return_code) ;


} ;   // END of NEW_drvr_element::create_config_text_from_driver_record





////////////////////////////////////////////////////////////////////////////////
//PREPARE_NEW_KEYWORD_RECORD

void
NEW_drvr_element::prepare_new_keyword_record (char * return_text_record)

{
   // Clean the record buffer in case the caller forgot
   return_text_record[0] = CHAR_ZERO ;

   if (NEW_drvr_element::maintain_last_section_name(NEW_section_name) == CHANGED)
   {
    strcpy (return_text_record, NEW_section_name) ;
    strcat (return_text_record, "\n") ;
   }

   strcat (return_text_record, NEW_keyword_name);

   strcat (return_text_record, STRING_KEYWORD_EQUATE);

};   // END of NEW_drvr_element::prepare_new_keyword_record




////////////////////////////////////////////////////////////////////////////////
//FORMAT_CONFIG_DATA

char *
NEW_drvr_element::format_config_data(CONFIG_DATA_TYPE config_value,
                                     TYPE_FORMAT type_format,
                                     char * data_buffer)
{

   // Create a string from the config data

   switch (type_format)
   {
   case FLOAT1:
      sprintf (data_buffer, "%#1.1f", config_value) ;
      break;

   case FLOAT2:
      sprintf (data_buffer, "%#1.2f", config_value) ;
      break;

   case FLOAT3:
      sprintf (data_buffer, "%#1.3f", config_value) ;
      break;

   case FLOAT4:
      sprintf (data_buffer, "%#1.4f", config_value) ;
      break;

   case INTEGER:
      sprintf (data_buffer, "%1.0f", config_value) ;
      break;

   case NA_TYPE:
      sprintf (data_buffer, "%1.0f", config_value) ;
      break;

   default:
      printf("**** Encountered unknown config value format enumeration = \"%d\" ****\n", type_format) ;
      exit (BAD_EXIT) ;

   }


   return (data_buffer);


};   // END of NEW_drvr_element::format_config_data









