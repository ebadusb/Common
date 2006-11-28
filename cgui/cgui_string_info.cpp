/*
 *	Copyright (c) 2006 by Gambro BCT, Inc.  All rights reserved.
 *
 * Derived from cgui_string_data.cpp revision 1.7  2006/07/25 15:42:37  cf10242
 * $Header: K:/BCT_Development/vxWorks/Common/cgui/rcs/cgui_string_info.cpp 1.7 2008/12/16 22:01:41Z rm10919 Exp wms10235 $
 * $Log: cgui_string_info.cpp $
 *
 */

#include <vxWorks.h>
#include "cgui_string_info.h"
#include "datalog_levels.h"

// constructor that accepts a file name, opens it, and prepares for gets
CGUIStringInfo::CGUIStringInfo(const char * filename)
{
	DataLog( log_level_cgui_info ) << "CGUIStringInfo::CGUIStringInfo( \"" << filename << "\" )" << endmsg;
   CGUIStringInfo::initialize();
   CGUIStringInfo::open(filename);
}

// constructor that just sets up the class members, but does not require a file name to open
CGUIStringInfo::CGUIStringInfo()
{
   CGUIStringInfo::initialize();
}

CGUIStringInfo::~CGUIStringInfo(void)
{
    CGUIStringInfo::close();
}

// open the given string.info file
void CGUIStringInfo::open(const char * filename)
{
    CGUIStringInfo::close();
    if( filename != NULL )
    {
        _filename = new char[strlen(filename)+1];
        strcpy(_filename,filename);
        _lineBuffer = new char[LineBufferSize];
        _stringInfo = fopen(_filename, "r");
    }
}

// close the string.info file
void CGUIStringInfo::close()
{
    if( _filename != NULL ) delete[] _filename;
    if( _stringInfo != NULL ) fclose(_stringInfo);
    if( _lineBuffer != NULL ) delete[] _lineBuffer;
    CGUIStringInfo::initialize();
}

// initialize class members
void CGUIStringInfo::initialize()
{
    _filename = NULL;
    _stringInfo = NULL;
    _lineBuffer = NULL;
    _readingFileTable = false;
    _line = 0;
}

// convert UTF8 encoding to Unicode
StringChar CGUIStringInfo::UTF8ToUnicode(StringChar utf8Char)
{
	StringChar unicodeChar = utf8Char;
	unsigned char firstByte = utf8Char & 0xff;
	unsigned char secondByte = (utf8Char & 0xff00) >> 8;
	// get number of bytes in UTF8 shows up in both bytes to make sure this is not
	// just an extended ASCII character
	unsigned char numBytes1 = firstByte & 0x60 ;
	unsigned char numBytes2 = secondByte & 0xc0 ;
	if (numBytes1 == 0x40 && numBytes2 == 0x80)  // corresponds to 2 bytes utf8 in both bytes
		unicodeChar = ((firstByte & 0x3) << 6) | (secondByte & 0x30) | (secondByte & 0xf);
	else
		DataLog( log_level_cgui_error ) << "Found possible non-2 byte UTF8 sequence in line " << _line << "   1st byte = " << hex << firstByte << "  second byte = " << secondByte << dec << "  continuing" << endmsg;
	return unicodeChar;
}

// get that returns false if caller provides a null pointer or string as the key.
bool CGUIStringInfo::get( const char * stringKey, const CGUIFontId * fontId, CGUITextItem & result)
{
//   DataLog( log_level_cgui_debug ) << "CGUIStringInfo::get(stringKey=\"" << stringKey << "\" fontId=" << (void *)fontId << " result=" << (void *)&result << ")" << endmsg;
    if( stringKey == NULL )
        return false;
    else
        return get( fontId, result, stringKey ); 
}

// main get routine called by all other gets.
bool CGUIStringInfo::get ( const CGUIFontId * fontId, CGUITextItem & result, const char * stringKey )
{
//   DataLog( log_level_cgui_debug ) << "CGUIStringInfo::get(fontId=" << (void *)fontId << " result=" << (void *)&result << " stringKey=\"" << stringKey << "\")" << endmsg;
   if(_stringInfo != NULL) while (fgets(_lineBuffer, LineBufferSize, _stringInfo) != NULL)
   {
      char * p = NULL;
      _line += 1;
      char * firstToken = strtok_r(_lineBuffer, " \t\n\r\"", &p);

      // skip comments
      if (!firstToken || firstToken[0] == '#')
      {
         continue;
      }

      if (strcmp(firstToken, "STRING_FILE_TABLE_START") == 0)
      {
         if (_readingFileTable)
         {
            DataLog( log_level_cgui_error ) << "line " << _line << ": unexpected STRING_FILE_TABLE_START - " << _filename << endmsg;
            break;
         }
         _readingFileTable = true;
      }

      else if (strcmp(firstToken, "STRING_FILE_TABLE_END") == 0)
      {
         if (!_readingFileTable)
         {
            DataLog( log_level_cgui_error ) << "line " << _line << ": unexpected STRING_FILE_TABLE_END - " << _filename << endmsg;
            break;
         }
         _readingFileTable = false;
         break;
      }

      else if (_readingFileTable && (stringKey == NULL || (strcmp(firstToken, stringKey) == 0) ) )
      {
         result.setId(firstToken);
//         DataLog( log_level_cgui_debug ) << "CGUIStringInfo::get got \"" << firstToken << "\"" << endmsg;
         return parseLine(p,fontId,result);
      }
   }
   return false;
}

// Parse the line and populate the given CGUITextItem.
bool CGUIStringInfo::parseLine ( char * p, const CGUIFontId * fontId, CGUITextItem & result)
{
         p = strchr(p, '"') + 1; // skip past first "
         char * text = strtok_r(p, "\"", &p);
         char * red = strtok_r(NULL," \t\n\r", &p);
         char * green = strtok_r(NULL," \t\n\r", &p);
         char * blue = strtok_r(NULL," \t\n\r", &p);
         char * attributes = strtok_r(NULL," \t\n\r", &p);
         char * x = strtok_r(NULL," \t\n\r", &p);
         char * y = strtok_r(NULL," \t\n\r", &p);
         char * width = strtok_r(NULL," \t\n\r", &p);
         char * height = strtok_r(NULL," \t\n\r", &p);
         char * fontSize = strtok_r(NULL," \t\n\r", &p);

         // Create Styling Record from line information.
         StylingRecord stylingRecord;
         stylingRecord.color = MakeCGUIColor( atoi(red), atoi(green), atoi(blue));
         sscanf(attributes, "%x", &stylingRecord.attributes);
         stylingRecord.region = CGUIRegion( atoi(x), atoi(y), atoi(width), atoi(height));
         stylingRecord.fontSize = atoi(fontSize);
         stylingRecord.fontId = fontId[atoi(fontSize)];               

         int writeIndex = 0;
         // Allocate to maximum possible length (may be less due to slash sequences)
         StringChar wString[strlen(text)+2];

         // Scan string, replacing slash sequences as necessary
         //
         while (*text != '\0')
         {
               // check for slash sequence.
               if (*text != '\\')
               {
                  // not slash sequence, check for UTF8 sequence and convert.
                  StringChar * nextWord = (StringChar *)text;
                  if( (*nextWord & 0x80C0) == 0x80C0 )
                  {
                     wString[writeIndex++] = (unsigned char)UTF8ToUnicode(*nextWord);
                     text+=2;
                  }
                  else
                      // not UTF8, just copy the character.
                      wString[writeIndex++] = (unsigned char)(UGL_WCHAR)*text++;

                } else
                {
                    text++;
                    switch (*text)
                    {
                    case '\0': break; // ignore slash at end of line
                    case 'b':wString[writeIndex++] = '\b'; break;  // backspace
                    case 'n':wString[writeIndex++] = '\n'; break;  // newline
                    case 'r':wString[writeIndex++] = '\r'; break;  // return
                    case 't':wString[writeIndex++] = '\t'; break;  // tab
                    case '"':wString[writeIndex++] = '"'; break;   // double quote
                    case 'x':                                      // hex
                          text += 2;  // get past the slash and the x 
                          char unicode[5] ;
                          int  l;

                          unicode[0] = (*text != '\0') ? *text++ : '\0';
                          unicode[1] = (*text != '\0') ? *text++ : '\0';
                          unicode[2] = (*text != '\0') ? *text++ : '\0';
                          unicode[3] = (*text != '\0') ? *text++ : '\0';
                          unicode[4] = '\0';

                          sscanf(unicode, "%x",&l);
                          wString[writeIndex++] = (StringChar)l;

                          break;

                    default:wString[writeIndex++] = (unsigned char)(UGL_WCHAR)*text;
                    }
                }
         }
         //
         // Null-terminate the new string.
         //
         wString[writeIndex] = '\0';

         //
         // Populate the CGUITextItem class.
         //            
         result.setText(wString); // setText copies the string into its own string buffer.
         result.setStylingRecord(stylingRecord); // setStylingRecord copies the styling record into its own styling record.
         return true;
}
