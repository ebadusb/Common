

/************************> TRIMA CONFIG Conversion <****************************

*     Copyright(c) 1999 COBE BCT, Inc. All rights reserved

*     Author:       Terry Wahl

*     Class name:   OLD_cfg_collect

*     File name:    old_cfg_collect.hpp

*     Contents:     See prototype header descriptions below...

*     Description:

         This class manages all config element in a collection. It will
         read in the config file and parse out all the section names and
         keyword/value pairs. Once it finds a keyword/value pair, it will
         create an OLD_cfg_element to store these values and store the new
         element. After this initialization phase, it will allow queries
         searching on the section name / keyword and, if found, return the
         associated config value as a CONFIG_DATA_TYPE.


*************************< TRIMA CONFIG Conversion >****************************

$Header: //bctquad3/home/BCT_Development/Common/disk_build/install_source/cfgconv_common/rcs/old_cfg_collect.hpp 1.1 2001/08/16 13:50:20 jl11312 Exp $
$Log: old_cfg_collect.hpp $
Revision 1.1  2001/08/16 13:50:20  jl11312
Initial revision
Revision 1.1  1999/12/10 14:37:16  BS04481
Initial revision

*/



// Single include define guardian
#ifndef OLDCFGCOL_HPP
#define OLDCFGCOL_HPP



#include "tablecol.hpp"         // include the base template declaration
#include "old_cfg_element.hpp"  // include the config element class

#include "common_defs.h"


class OLD_cfg_collect : public Table_Collection<OLD_cfg_element>
{

  private:

       //Current section name
       char current_section_name[MAX_SECTION_KEYWORD_STRING] ;


       //PARM_PARSE
       // This method parses a string for tokens as delimited  by the delim_string.
       // Any whitespace_string characters will be considered non-existent while
       // extracting the token (this may be a NULL string). Once the delimiting
       // character has been encountered (or end of line, '\0') it will return the
       // matching delimiter character (or '\0').
       char parm_parse (char * parse_string, int & parsing_index, char * delim_string, char * whitespace_string, char * token_string) ;


       //PARSE_AND_STORE_CONFIG_RECORD
       // This method will parse out the section name or keyword and config value.
       // If it is a section name, it will place it in the section name data member.
       // If it is a keyword / value pair it will create an OLD_cfg_element and
       // add it to the collection. It will ignore blank lines and comment lines.
       // Valid section name, keyword / value pairs, blank lines, comment lines
       // will cause this to return a 1, any others will cause this to return
       // a 0.
       int parse_and_store_config_record (char * config_record) ;



   protected:



   public:


       //CONSTRUCTOR
       OLD_cfg_collect()
       {
          current_section_name[0] = CHAR_ZERO ;
       };




       //DESTRUCTOR
       virtual ~OLD_cfg_collect()
       {
       };


       //LOAD_OLD_CONFIG_COLLECTION
       // This method is passed the name of the config file. It will open the
       // file and read all the records and attempt to store all the config
       // values.
       void load_old_config_collection (char * filename) ;


       //GET_SECTION_KEYWORD_VALUE
       // This method will be passed a section and keyword. It will search the config
       // collection for an OLD_cfg_element that matches the section name and keyword
       // name. If found, it will return its corresponding config value and set the
       // return code to SUCCESS. If it is not found, it will set the return code to FAILURE.
       int get_section_keyword_value (char * section_name, char * keyword_name, CONFIG_DATA_TYPE & config_value) ;



};  // END of OLD_cfg_collect Class Declaration


// End of single include define guardian
#endif


