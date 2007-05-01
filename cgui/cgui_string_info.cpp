/*
 *	Copyright (c) 2006 by Gambro BCT, Inc.  All rights reserved.
 *
 * Derived from cgui_string_data.cpp revision 1.7  2006/07/25 15:42:37  cf10242
 * $Header: K:/BCT_Development/vxWorks/Common/cgui/rcs/cgui_string_info.cpp 1.7 2008/12/16 22:01:41Z rm10919 Exp wms10235 $
 * $Log: cgui_string_info.cpp $
 * Revision 1.3  2007/02/08 19:28:05Z  rm10919
 * Updates to add languages to string data.
 * Revision 1.2  2006/11/29 17:57:38Z  pn02526
 * Fix bugs found integrating with CGUIStringData.
 * Revision 1.1  2006/11/27 15:26:23  pn02526
 * Initial revision
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
bool CGUIStringInfo::get( const char * stringKey, const CGUIFontId * fontId, CGUITextItem & result, int fontIndex = 0)
{
//   DataLog( log_level_cgui_debug ) << "CGUIStringInfo::get(stringKey=\"" << stringKey << "\" fontId=" << (void *)fontId << " result=" << (void *)&result << ")" << endmsg;
    if( stringKey == NULL )
        return false;
    else
        return get( fontId, result, stringKey, fontIndex ); 
}

// main get routine called by all other gets.
bool CGUIStringInfo::get ( const CGUIFontId * fontId, CGUITextItem & result, const char * stringKey, int fontIndex = 0 )
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
         return parseLine( p, fontId, result, fontIndex);
      }
   }

   return false;
}

bool CGUIStringInfo::getQuotedString(char *&data, StringChar *& wString)
{
   wString = new StringChar[strlen(data)];
	bool 	started = false;
	bool 	done = false;

	int	writeIdx = 0;
	while ( !done )
	{
		if ( *data == '\0' ||
			  ( started && *data == '"' ))
		{
			done = true;
		}
		else if ( !started )
		{
			if ( *data == '"' ) started = true;
			data++;
		}
		else if ( started )
		{
			if ( *data == '\\' )
			{
				data++;
				switch ( *data )
				{
				case '\0':
					break;

            case 'b':
					wString[writeIdx++] = '\b';
					data++;
					break;

            case 'n':
					wString[writeIdx++] = '\n';
					data++;
					break;

            case 'r':
					wString[writeIdx++] = '\r';
					data++;
					break;

            case 't':
					wString[writeIdx++] = '\t';
					data++;
					break;

            case '"':
					wString[writeIdx++] = '"';
					data++;
					break;
  
            case 'x':
               char unicode[5];
               int  l;

               data++;
               unicode[0] = (*data != '\0') ? *data++ : '\0';
               unicode[1] = (*data != '\0') ? *data++ : '\0';
               unicode[2] = (*data != '\0') ? *data++ : '\0';
               unicode[3] = (*data != '\0') ? *data++ : '\0';
               unicode[4] = '\0';

               sscanf(unicode, "%x", &l);
               wString[writeIdx++] = (StringChar)l;
               break;

				default:
					wString[writeIdx++] = (StringChar)*data++;
					break;
				}
			}
			else
         {
            // not a backslash sequence, check for UTF8 sequence and convert.
            StringChar * nextWord = (StringChar *)data;
            if ( (*nextWord & 0x80C0) == 0x80C0 )
            {
					wString[writeIdx++] = (unsigned char)UTF8ToUnicode(*nextWord);
               data += 2;
            }
            else
				{
					// not UTF8, just copy the character.
               wString[writeIdx++] = (StringChar)*data++;
				}
			}
		}
	}

	bool	result = false;
	if ( *data == '"' )
	{
		result = true;
		data++;
	}
	else
	{
		DataLog( log_level_cgui_error ) << "line " << _line << ": unterminated string - " << _filename << endmsg;
	}

	return result;
}

// Parse the line and populate the given CGUITextItem.
bool CGUIStringInfo::parseLine ( char * p, const CGUIFontId * fontId, CGUITextItem & result, int fontIndex = 0)
{
	bool status = true;
   StringChar * wString = NULL;
	status &= getQuotedString(p, wString);

	int	red, green, blue;
	unsigned int attributes;
	int	x, y, width, height, fontSize;

	if ( status &&
		  sscanf(p, "%d %d %d %x %d %d %d %d %d", &red, &green, &blue, &attributes, &x, &y, &width, &height, &fontSize) != 9 )
	{
		status = false;
		DataLog( log_level_cgui_error ) << "line " << _line << ": missing parameters - " << _filename << endmsg;
	}

   // Create Styling Record from line information.
   //
   if ( status )
	{
		StylingRecord stylingRecord;
		stylingRecord.color = MakeCGUIColor(red, green, blue);
		stylingRecord.attributes = attributes;
		stylingRecord.region = CGUIRegion(x, y, width, height);
		stylingRecord.fontSize = fontSize;
		stylingRecord.fontId = fontId[fontSize + fontIndex*50];

		// Populate the CGUITextItem class.
		//
		result.setText(wString); // setText copies the string into its own string buffer.
		result.setStylingRecord(stylingRecord); // setStylingRecord copies the styling record into its own styling record.
	}

	if ( wString ) delete[] wString;
   return status;
}
