/*
 *	Copyright (c) 2006 by Gambro BCT, Inc.  All rights reserved.
 *
 * Derived from cgui_string_data.h revision 1.2  2006/07/25 15:42:37  cf10242
 * $Header: K:/BCT_Development/vxWorks/Common/cgui/rcs/cgui_string_info.h 1.6 2008/12/16 06:03:02Z rm10919 Exp wms10235 $
 * $Log: cgui_string_info.h $
 * Revision 1.1  2006/11/27 15:27:50  pn02526
 * Initial revision
 *
 */

#ifndef _CGUI_STRING_INFO_INCLUDE
#define _CGUI_STRING_INFO_INCLUDE
                    
#include <stdio.h>

#include "cgui_text_item.h"
#include "datalog_levels.h"

class CGUIStringInfo
{
public:
   CGUIStringInfo(const char * filename);
   CGUIStringInfo();
   virtual ~CGUIStringInfo(void);

   void open(const char * filename);

   inline void open(const string & filename) { CGUIStringInfo::open(filename.c_str()); };

   void close();

   bool get( const char * stringKey, const CGUIFontId * fontId, CGUITextItem & result );

   bool get( const CGUIFontId * fontId, CGUITextItem & result, const char * stringKey=NULL );

   inline bool get( const string & stringKey, const CGUIFontId * fontId, CGUITextItem & result )
   {
//      DataLog( log_level_cgui_debug ) << "CGUIStringInfo::get(stringKey=\"" << stringKey << "\" fontId=" << (void *)fontId << " result=" << (void *)&result << ")" << endmsg;
      return CGUIStringInfo::get( (const char *)stringKey.c_str(), fontId, result );
   };

   bool get( const string & filename, const string & stringKey, const CGUIFontId * fontId, CGUITextItem & result )
   {
      DataLog( log_level_cgui_info ) << "CGUIStringInfo::get(filename=\"" << filename << " stringKey=\"" << stringKey << "\" fontId=" << (void *)fontId << " result=" << (void *)&result << ")" << endmsg;
      CGUIStringInfo::open(filename);
      bool retval = CGUIStringInfo::get( (const char *)stringKey.c_str(), fontId, result );
      CGUIStringInfo::close();
      return retval;
   };

   int line() { return _line; };

   bool endOfFile(void) { return !_readingFileTable; }  // If false after a get, no more records, or searched for key and not found.

private:

   enum
   {
      LineBufferSize = 65000
   };

    char * _filename;
    FILE * _stringInfo;
    char * _lineBuffer;
    unsigned int _line;
    bool _readingFileTable;

   void initialize();

   StringChar UTF8ToUnicode(StringChar utf8Char);

//   bool fetchTextItem ( const CGUIFontId * fontId, CGUITextItem & result, char * stringLabel=NULL );

   bool parseLine ( char * p, const CGUIFontId * fontId, CGUITextItem & result);
};
#endif //_CGUI_STRING_DATA_INCLUDE