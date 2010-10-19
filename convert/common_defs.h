
/************************> TRIMA CONFIG Conversion <****************************

*     Copyright(c) 1999 COBE BCT, Inc. All rights reserved

*     Author:       Terry Wahl

*     Class name:   common config conversion definitions

*     File name:    common_defs.h

*     Contents:     See prototype header descriptions below...

*     Description:

         This file contains common defines used throughout the
         application.

*************************< TRIMA CONFIG Conversion >****************************

$Header$
$Log: common_defs.h $
Revision 1.1  2001/08/16 13:50:24  jl11312
Initial revision
Revision 1.1  1999/12/10 14:36:29  BS04481
Initial revision

*/


// Single include define guardian
#ifndef COMMONDEFS_HPP
#define COMMONDEFS_HPP

#define CONFIG_DATA_TYPE double

// string size for the allocation of keywords and section names
#define MAX_SECTION_KEYWORD_STRING 100

// max expected record size from the old configuration file
#define MAX_FILE_RECORD_SIZE 400

// max expected token size to be parsed from the old configuration file
#define MAX_TOKEN_SIZE  400

// max config data buffer size
#define MAX_DATA_BUFFER 100

// A null or zero character
#define CHAR_ZERO '\0'

// The beginning character in a section heading
#define CHAR_SECTION '['

// The comment line character for configuratin files
#define CHAR_COMMENT '#'

// The keyword to value delimiter (= sign)
#define STRING_KEYWORD_EQUATE "="
#define CHAR_KEYWORD_EQUATE '='

// define success and failure
#define SUCCESS 1
#define FAILURE 0
#define BAD_EXIT 666

// define changed and nochange
#define CHANGED 1
#define NOCHANGE 0

// define record generation status
#define WRITE_REC 2
#define NO_REC    3


// End of single include define guardian
#endif


