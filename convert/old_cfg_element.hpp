
/************************> TRIMA CONFIG Conversion <****************************

*     Copyright(c) 1999 COBE BCT, Inc. All rights reserved

*     Author:       Terry Wahl

*     Class name:   OLD_cfg_element

*     File name:    old_cfg_element.hpp

*     Contents:     See prototype header descriptions below...

*     Description:

         This class manages one config element. It contains
         the section name of the element; the keyword name of the element;
         the value of the element in a floating point format.

*************************< TRIMA CONFIG Conversion >****************************

$Header: //bctquad3/home/BCT_Development/Common/disk_build/install_source/cfgconv_common/rcs/old_cfg_element.hpp 1.1 2001/08/16 13:50:22 jl11312 Exp $
$Log: old_cfg_element.hpp $
Revision 1.1  2001/08/16 13:50:22  jl11312
Initial revision
Revision 1.1  1999/12/10 14:37:24  BS04481
Initial revision

*/



// Single include define guardian
#ifndef OLDCFGELE_HPP
#define OLDCFGELE_HPP

#include <string.h>
#include "common_defs.h"



class OLD_cfg_element
{
   private:

       // Disallow accidental constructions
       OLD_cfg_element() ;

       //Section name of the config value
       char section_name[MAX_SECTION_KEYWORD_STRING+1] ;

       //Keyword name of the config value
       char keyword_name[MAX_SECTION_KEYWORD_STRING+1] ;

       //Config value
       CONFIG_DATA_TYPE config_value ;


   protected:



   public:


       //CONSTRUCTOR
       OLD_cfg_element(char * parm_section_name, char * parm_keyword_name, CONFIG_DATA_TYPE parm_config_value) :
          config_value(parm_config_value)
       {

          strncpy (section_name, parm_section_name, MAX_SECTION_KEYWORD_STRING) ;

          strncpy (keyword_name, parm_keyword_name, MAX_SECTION_KEYWORD_STRING) ;

       };



       //DESTRUCTOR
       virtual ~OLD_cfg_element()
       {
       };



       //COMPARE_CONFIG_ELEMENT
       // This method will compare the section name and keyword name parameters with its
       // current values. If they are equal it will set the return value with the associated
       // value and set the return code to a SUCCESS. If they are not equal, it will set the return
       // value to 0 and the return code to FAILURE.

       int compare_config_element (char * parm_section_name, char * parm_keyword_name, CONFIG_DATA_TYPE & ret_value) ;


};  // END of OLD_cfg_element Class Declaration


// End of single include define guardian
#endif

