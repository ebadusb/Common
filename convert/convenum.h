
/************************> TRIMA CONFIG Conversion <****************************

*     Copyright(c) 1999 COBE BCT, Inc. All rights reserved

*     Author:       Terry Wahl

*     Class name:   utility enumerations: TYPE_FORMAT, CONV_ACTION

*     File name:    convenum.h

*     Contents:     See prototype header descriptions below...

*     Description:

         These enumerations define: the type of format for the
         NEW configuration file data values; the actions to be
         taken to create the NEW configuration file entries.


*************************< TRIMA CONFIG Conversion >****************************

$Header: //bctquad3/home/BCT_Development/Common/disk_build/install_source/cfgconv_common/rcs/convenum.h 1.1 2001/08/16 13:50:10 jl11312 Exp $
$Log: convenum.h $
Revision 1.1  2001/08/16 13:50:10  jl11312
Initial revision
Revision 1.1  1999/12/10 14:36:33  BS04481
Initial revision

*/


// Single include define guardian
#ifndef CONVENUM_HPP
#define CONVENUM_HPP


////////////////////////////////////////////////////////////////////////////
/// This enumeration defines the output configuration file data formats

enum TYPE_FORMAT
{
   FLOAT1,      // *nnn.n
   FLOAT2,      // *nnn.nn
   FLOAT3,      // *nnn.nnn
   FLOAT4,      // *nnn.nnnn
   INTEGER,     // *nnn
   NA_TYPE      // *nnn
};


////////////////////////////////////////////////////////////////////////////
/// This enumeration defines the actions to be taken to create the NEW
/// configuration entry.

enum CONV_ACTION
{
   OLD_VALUE,         // Use the value found in the OLD configuration file for this entry.
   NEW_VALUE,         // Use the default value found in the NEW configuration template file for this entry.
   COMMENT_LINE,      // This is a comment line found in the NEW Section name of the NEW_Driver_element -- inject it into the file as-is.
   ENUM_TRANS_OLD,    // See if the OLD config value is equal to the OLD operand value in the NEW_Driver_element. If it is, use the corresponding NEW default value for this entry.
   ENUM_DEFAULT_OLD,  // If this config entry hasn't been "translated" yet (see above), unconditionally use the NEW default value in the NEW template file.
   OFFSET_OLD,        // Add the OLD operand value to the OLD config value and use this value as the NEW configuration value. If the old cannot be found, the NEW default value will be used.
   SCALE_OLD,         // Multiply the OLD config value by the OLD operand value and use this value as the NEW configuration value. If the old cannot be found, the NEW default value will be used.
   HIGH_LIMIT,        // If old value is greater than new default value, use new default value
   LOW_LIMIT,         // If old value is less than new default value, use new default value
   LIMIT_DEFAULT_OLD  // Use old value if not translated by HIGH_LIMIT, LOW_LIMIT
};


// End of single include define guardian
#endif


