
/************************> TRIMA CONFIG Conversion <****************************

*     Copyright(c) 1999 COBE BCT, Inc. All rights reserved

*     Author:       Terry Wahl

*     Class name:   NEW_drvr_element

*     File name:    new_drvr_element.hpp

*     Contents:     See prototype header descriptions below...

*     Description:

         This class manages one NEW driver element. A driver element
         reflects a single line in the spreadsheet that defines how an
         OLD config element is converted into a NEW config element. This
         class will contain all the elements for that conversion and the
         methods for executing the conversion. This class is dependent on
         the singleton instance of OLD_config_collection so that it can
         access the value of an OLD config value.

*************************< TRIMA CONFIG Conversion >****************************

$Header: //bctquad3/HOME/BCT_Development/Common/convert/rcs/new_drvr_element.hpp 1.1 2001/08/16 22:40:20 ms10234 Exp ms10234 $
$Log: new_drvr_element.hpp $
Revision 1.1  2001/08/16 13:50:17  jl11312
Initial revision
Revision 1.1  1999/12/10 14:37:07  BS04481
Initial revision

*/



// Single include define guardian
#ifndef NEWDRVELE_HPP
#define NEWDRVELE_HPP

#include "common_defs.h"
#include "convenum.h"

#include "old_cfg_collect.hpp"
#include "enum_hit_collect.cpp"


// System #INCLUDES

#include <stdlib.h>
#include <string.h>




class NEW_drvr_element
{
   private:


       // Disallow accidental constructions
       NEW_drvr_element() ;



       //MAINTAIN_LAST_SECTION_NAME
       // This STATIC method will update the "last" section name buffer. If the section
       // name changes then this method will return CHANGED otherwise NOCHANGE.
       static int maintain_last_section_name (char * section_name) ;


       //PREPARE_NEW_KEYWORD_RECORD
       // This method will prepare a record buffer with (optionally) the Section name
       // followed by a newline (if this is a new section segment) followed by the NEW
       // keyword and an equals sign.
       void prepare_new_keyword_record (char * record_buffer) ;


       //FORMAT_CONFIG_DATA
       // This method will take a floating point value and format it into a string
       // according to the type format parameter. It will then place that string in
       // the data buffer passed to the routine and return a pointer to that buffer.
       char * format_config_data(CONFIG_DATA_TYPE config_value, TYPE_FORMAT type_format, char * data_buffer) ;



       /// Keep track of last Section name
       static char LAST_section_name [(MAX_SECTION_KEYWORD_STRING*4)+1] ;



       /// Define all the data used to convert a single config element

       //NEW Section name -- allowing enough space to accommodate a long comment
       char NEW_section_name [(MAX_SECTION_KEYWORD_STRING*4)+1] ;

       //NEW Keyword name
       char NEW_keyword_name [MAX_SECTION_KEYWORD_STRING+1] ;

       //OLD Section name -- this never contains comments
       char OLD_section_name [MAX_SECTION_KEYWORD_STRING+1] ;

       //OLD Keyword name
       char OLD_keyword_name [MAX_SECTION_KEYWORD_STRING+1] ;

       //NEW value format
       TYPE_FORMAT NEW_type_format ;

       //NEW Default value (via the d:/Trima/templates/config.dat)
       CONFIG_DATA_TYPE NEW_default_value ;

       //OLD config value -- currently used only for enumeration conversions (see ENUM_TRANSLATION)
       CONFIG_DATA_TYPE OLD_operand_value ;

       //Conversion operation enum
       CONV_ACTION conversion_operator ;



   protected:



   public:


       //CONSTRUCTOR
       NEW_drvr_element(char *      p_NEW_section_name,
                        char *      p_NEW_keyword_name,
                        char *      p_OLD_section_name,
                        char *      p_OLD_keyword_name,
                        TYPE_FORMAT p_NEW_type_format,
                        CONFIG_DATA_TYPE       p_NEW_default_value,
                        CONFIG_DATA_TYPE       p_OLD_operand_value,
                        CONV_ACTION p_conversion_operator
                       )
       {

          strncpy (NEW_section_name, p_NEW_section_name, sizeof(NEW_section_name)-1) ;
          NEW_section_name[sizeof(NEW_section_name)-1] = CHAR_ZERO ; // cauderize

          strncpy (NEW_keyword_name, p_NEW_keyword_name, sizeof(NEW_keyword_name)-1) ;
          NEW_keyword_name[sizeof(NEW_keyword_name)-1] = CHAR_ZERO ; // cauderize

          strncpy (OLD_section_name, p_OLD_section_name, sizeof(OLD_section_name)-1) ;
          OLD_section_name[sizeof(OLD_section_name)-1] = CHAR_ZERO ; // cauderize

          strncpy (OLD_keyword_name, p_OLD_keyword_name, sizeof(OLD_keyword_name)-1) ;
          OLD_keyword_name[sizeof(OLD_keyword_name)-1] = CHAR_ZERO ; // cauderize

          NEW_type_format      = p_NEW_type_format ;

          NEW_default_value    = p_NEW_default_value ;

          OLD_operand_value    = p_OLD_operand_value ;

          conversion_operator  = p_conversion_operator ;

       };



       //DESTRUCTOR
       virtual ~NEW_drvr_element()
       {
       };


       //CREATE_CONFIG_TEXT_FROM_DRIVER_RECORD
       // This method will interpret the value of the conversion operator; create a
       // text record for the new configuration file; return the record to the
       // caller. The return code will reflect WRITE_REC NO_REC or FAILURE in attempting to
       // convert the driver record.
       int create_config_text_from_driver_record (OLD_cfg_collect * OLD_config_data,
                                                  ENUM_hit_collect * enum_hit_list,
                                                  char * return_text_record) ;


};  // END of NEW_drvr_element Class Declaration


// End of single include define guardian
#endif


