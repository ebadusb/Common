
/************************> TRIMA CONFIG Conversion <****************************

*     Copyright(c) 1999 COBE BCT, Inc. All rights reserved

*     Author:       Terry Wahl

*     Class name:   NEW_drvr_collect

*     File name:    new_drvr_collect.hpp

*     Contents:     See prototype header descriptions below...

*     Description:

         This class manages all the NEW_drvr_elements. It will
         populate of all the driver elements in the collection and cause
         all the driver elements to create a corresponding printable config
         record.

*************************< TRIMA CONFIG Conversion >****************************

$Header: //bctquad3/HOME/BCT_Development/Common/convert/rcs/new_drvr_collect.hpp 1.1 2001/08/16 22:40:20 ms10234 Exp ms10234 $
$Log: new_drvr_collect.hpp $
Revision 1.1  2001/08/16 13:50:14  jl11312
Initial revision
Revision 1.1  1999/12/10 14:36:58  BS04481
Initial revision

*/



// Single include define guardian
#ifndef NEWDRVCOL_HPP
#define NEWDRVCOL_HPP

#include "common_defs.h"

#include "old_cfg_collect.hpp"
#include "enum_hit_collect.cpp"
#include "new_drvr_element.hpp"


#include "tablecol.hpp"         // include the base template declaration

// System #INCLUDES

#include <stdlib.h>
#include <string.h>




class NEW_drvr_collect : public Table_Collection<NEW_drvr_element>
{
   private:

   ENUM_hit_collect  enum_trans_hit_collection ;


   protected:



   public:

   //CONSTRUCTOR
   NEW_drvr_collect ()
   {
   } ;


   //DESTRUCTOR
   virtual ~NEW_drvr_collect ()
   {
   } ;


   //LOAD_DRIVER_ELEMENTS
   // This method will contain all the generated code that adds the driver
   // elements to the collection.
   void load_driver_elements () ;


   //GENERATE_NEW_CONFIG_FILE
   // This method will iterate through all the driver elements and write
   // the generated config text records to the appropriate file.
   void generate_new_config_file (char * file_name, OLD_cfg_collect * old_config_collection) ;



};  // END of NEW_drvr_collect Class Declaration


// End of single include define guardian
#endif


