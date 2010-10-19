
/************************> TRIMA CONFIG Conversion <****************************

*     Copyright(c) 1999 COBE BCT, Inc. All rights reserved

*     Author:      Terry Wahl

*     Class name:  OLD_cfg_element

*     File name:   old_cfg_element.cpp

*     Contents:    Implementations of class methods

*     Description: See descriptions in corresponding .hpp file.

*************************< TRIMA CONFIG Conversion >****************************

$Header$
$Log: old_cfg_element.cpp $
Revision 1.1  2001/08/16 13:50:21  jl11312
Initial revision
Revision 1.1  1999/12/10 14:37:19  BS04481
Initial revision
*/




// EXTERNAL REFERENCES

#include "old_cfg_element.hpp"
#include <string.h>


////////////////////////////////////////////////////////////////////////////////
//COMPARE_CONFIG_ELEMENT

int
OLD_cfg_element::compare_config_element (char * parm_section_name, char * parm_keyword_name, CONFIG_DATA_TYPE & ret_value)
{
   if ((stricmp(parm_section_name, section_name) == 0) &&
       (stricmp(parm_keyword_name, keyword_name) == 0)
      )
   {
      // The section name and keyword name match
      ret_value = config_value ;
      return (1) ;
   }
   else
   {
      // No match so set value to zero and return a "no match" value
      ret_value = 0.0 ;
      return (0) ;
   }

};   // END of OLD_cfg_element::compare_config_element




