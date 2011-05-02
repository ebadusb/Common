/*
 *	Copyright (c) 2006 by Gambro BCT, Inc.  All rights reserved.
 *
 * Derived from cgui_string_data.cpp revision 1.7  2006/07/25 15:42:37  cf10242
 * $Header$
 * $Log: cgui_string_info.cpp $
 * Revision 1.7  2008/12/16 22:01:41Z  rm10919
 * Correct include file name.
 * Revision 1.6  2008/12/16 06:03:02Z  rm10919
 * Add the ablility for combined fonts in text. IT 6562
 * Revision 1.5  2007/05/08 21:57:32Z  rm10919
 * Changed UFT8ToUnicode to handle 3 byte UTF8 characters. IT-3294
 * Revision 1.4  2007/04/30 18:26:07Z  jl11312
 * - additional error checking when reading string info files (Taos IT 3102)
 * Revision 1.3  2007/02/08 19:28:05Z  rm10919
 * Updates to add languages to string data.
 * Revision 1.2  2006/11/29 17:57:38Z  pn02526
 * Fix bugs found integrating with CGUIStringData.
 * Revision 1.1  2006/11/27 15:26:23  pn02526
 * Initial revision
 *
 */

#include <vxWorks.h>
#include "cgui_graphics.h"
#include "cgui_string_info.h"
#include "cgui_text.h"
#include "font_table.h"
#include "datalog_levels.h"
#include "error.h"

//extern int getFontIndex( const char *fontName );

// constructor that accepts a file name, opens it, and prepares for gets
//
CGUIStringInfo::CGUIStringInfo(const char * filename)
{
	DataLog( log_level_cgui_info ) << "CGUIStringInfo::CGUIStringInfo( \"" << filename << "\" )" << endmsg;
   CGUIStringInfo::initialize();
   CGUIStringInfo::open(filename);
}

// constructor that just sets up the class members, but does not require a file name to open
//
CGUIStringInfo::CGUIStringInfo()
{
   CGUIStringInfo::initialize();
}

CGUIStringInfo::~CGUIStringInfo(void)
{
    CGUIStringInfo::close();
}

// open the given string.info file
//
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

void CGUIStringInfo::open(const string & filename)
{
	CGUIStringInfo::open(filename.c_str());
}

// close the string.info file
//
void CGUIStringInfo::close()
{
    if( _filename != NULL ) delete[] _filename;
    if( _stringInfo != NULL ) fclose(_stringInfo);
    if( _lineBuffer != NULL ) delete[] _lineBuffer;
    CGUIStringInfo::initialize();
}

// initialize class members
//
void CGUIStringInfo::initialize()
{
    _filename = NULL;
    _stringInfo = NULL;
    _lineBuffer = NULL;
    _readingFileTable = false;
    _readingFontRanges = false;
    _line = 0;
}

// convert UTF8 encoding to Unicode
//
void CGUIStringInfo::UTF8ToUnicode( char *&data, StringChar * wString, int &writeIdx )
{
   if ( (*data & 0xf0) == 0xe0 &&
        (*(data + 1) & 0xc0) == 0x80 &&
        (*(data + 2) & 0xc0) == 0x80 )
   {
      StringChar ch1 = *data++ & 0x0f;
      StringChar ch2 = *data++ & 0x3f;
      StringChar ch3 = *data++ & 0x3f;

      wString[writeIdx++] = ( ch1 << 12 ) | ( ch2 << 6 ) | ch3;
   }
   else if ( (*data & 0xe0) == 0xc0 &&
               (*(data + 1) & 0xc0) == 0x80 )
   {
      StringChar ch1 = *data++ & 0x1f;
      StringChar ch2 = *data++ & 0x3f;

      wString[writeIdx++] = ( ch1 << 6 ) | ch2;
   }
   else
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
         wString[writeIdx++] = (StringChar)*data++;
      }
   }
}

bool CGUIStringInfo::get( const string & stringKey, const CGUIFontId * fontId, CGUITextItem & result, int fontIndex = 0 )
{
	return get( (const char *)stringKey.c_str(), fontId, result, fontIndex );
}

bool CGUIStringInfo::get( const string & filename, const string & stringKey, const CGUIFontId * fontId, CGUITextItem & result, int fontIndex = 0 )
{
	DataLog( log_level_cgui_info ) << "CGUIStringInfo::get(filename=\"" << filename << " stringKey=\"" << stringKey << /*"\" fontId=" << (void *)fontId << */" result=" << (void *)&result << ")" << endmsg;
	CGUIStringInfo::open(filename);
	bool retval = get( (const char *)stringKey.c_str(), fontId, result, fontIndex );
	CGUIStringInfo::close();
	return retval;
}

// get that returns false if caller provides a null pointer or string as the key.
//
bool CGUIStringInfo::get( const char * stringKey, const CGUIFontId * fontId, CGUITextItem & result, int fontIndex = 0)
{
    if( stringKey == NULL )
        return false;
    else
        return get( fontId, result, stringKey, fontIndex );
}

// main get routine called by all other gets.
//
bool CGUIStringInfo::get ( const CGUIFontId * fontId, CGUITextItem & result, const char * stringKey, int fontIndex = 0 )
{
   if(_stringInfo != NULL) while (fgets(_lineBuffer, LineBufferSize, _stringInfo) != NULL)
   {
      char * p = NULL;
      _line += 1;
      char * firstToken = strtok_r(_lineBuffer, " \t\n\r\"", &p);

      // skip comments
      if (!firstToken || firstToken[0] == '#'|| firstToken[0] == '[')
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

		else if( strcmp( firstToken, "FONT_RANGE_TABLE_START" ) == 0 )
		{
			if (_readingFileTable)
         {
            DataLog( log_level_cgui_error ) << "line " << _line << ": unexpected FONT_RANGE_TABLE_START - " << _filename << endmsg;
            break;
         }
			else
			{
				_readingFontRanges = true;
			}
		}

		else if( strcmp(firstToken, "FONT_RANGE_TABLE_END") == 0 )
		{
			if ( _readingFileTable || !_readingFontRanges )
			{
				DataLog( log_level_cgui_error ) << "line " << _line << ": unexpected FONT_RANGE_TABLE_START - " << _filename << endmsg;
				break;
			}
			else
			{
            _readingFontRanges = false;
			}
		}

      else if( _readingFileTable && ( stringKey == NULL || ( strcmp( firstToken, stringKey ) == 0 )))
      {
         result.setId(firstToken);
         return parseLine( p, fontId, result, fontIndex);
      }
		else if( _readingFontRanges )
		{
//			parseRange( p );
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
			// Added 4/29/2011: Strip out the segmentation tags as we load the strings
            // D.S. (x4664)
            if (data[0] == '#' && data[1] == '!' && data[2] == '[' &&
				((data[3] == 'B' && data[4] == 'e' && data[5] == 'g' && data[6] == 'i' && data[7] == 'n' && data[8] == 'S' && data[9] == 'e' && data[10] == 'g') ||
				 (data[3] == 'E' && data[4] == 'n' && data[5] == 'd' && data[6] == 'S' && data[7] == 'e' && data[8] == 'g')))
			{
				// Keep moving forward until we find the end of the tag,
				// or the end of the buffer, or the end of the quoted section.
				while (*data != ']' && *data != '\0' && *data != '"') 
					data++;
				
				// Bump it one more, as long as this isn't the end of string.
				if (*data == ']') data++;
				else DataLog (log_level_cgui_error) << "Unterminated markup tag at line " << _line << endmsg;
			}
			// This is the normal case.  Deal with a standard character.
			else 
			UTF8ToUnicode(data, wString, writeIdx);
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
//
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
		int index = fontSize + fontIndex*50;

		if( index >= MAX_FONTS || index < 0 )
		{
			_FATAL_ERROR(__FILE__, __LINE__, "Font size and index are outside font table boundries.");
		}

		stylingRecord.fontId = fontId[index];

		// Populate the CGUITextItem class.
		//
		result.setText(wString); // setText copies the string into its own string buffer.
		result.setStylingRecord(stylingRecord); // setStylingRecord copies the styling record into its own styling record.
	}

	if ( wString ) delete[] wString;
   return status;
}

bool CGUIStringInfo::parseRange( char * p )
{
	bool result = false;
	unsigned int startRange, endRange;

	if( sscanf( p, "%x %x", &startRange, &endRange ) == 2 )
	{
      char * q1 = strstr( (const char *)p, "\"");
      char * q2 = strrchr( (const char *)p, '"');

      if( q1 != q2 )
		{
			q1++;
         int length = q2 - q1;

			char * fontName = new char[length+1];
         strncpy( fontName, q1, length );
			fontName[length] = '\0';

			int index = getFontIndex( fontName );

			if( index < MAX_FONTS || index >= 0 )
			{
				FontRange *fontRange = new FontRange;

				fontRange->startIndex = startRange;
				fontRange->endIndex = endRange;
				fontRange->fontName = fontName;
				fontRange->fontIndex = index;

				result = CGUIText::addFontRange( fontRange );
			}
			else
			{
				DataLog( log_level_cgui_error ) << "line " << _line << ": wrong parameters from Font Range Table - " << _filename << endmsg;
			}
		}
		else
		{
			DataLog( log_level_cgui_error ) << "line " << _line << ": wrong parameters from Font Range Table - " << _filename << endmsg;
		}
   }
	else
	{
		DataLog( log_level_cgui_error ) << "line " << _line << ": missing parameters from Font Range Table - " << _filename << endmsg;
	}

	return result;
}

bool CGUIStringInfo::setFontRange ( const string & filename )
{
	bool result = false;
	CGUIStringInfo::open( filename );

   if(_stringInfo != NULL) while (fgets(_lineBuffer, LineBufferSize, _stringInfo) != NULL)
   {
      _line += 1;
      char * p = NULL;
		char * p2 =  new char[strlen(_lineBuffer)+1];
		strcpy( p2, _lineBuffer );
      char * firstToken = strtok_r(_lineBuffer, " \t\n\r\"", &p);

      // skip comments
      if( !firstToken || firstToken[0] == '#' )
      {
         continue;
      }

		if( strcmp( firstToken, "FONT_RANGE_TABLE_START" ) == 0 )
		{
				_readingFontRanges = true;
		}

		else if( strcmp(firstToken, "FONT_RANGE_TABLE_END") == 0 )
		{
			if ( _readingFileTable || !_readingFontRanges )
			{
				DataLog( log_level_cgui_error ) << "line " << _line << ": unexpected FONT_RANGE_TABLE_START - " << _filename << endmsg;
				break;
			}
			else
			{
				//	All finished, so let's quit.
            _readingFontRanges = false;
				result = true;
				break;
			}
		}

		else if( _readingFontRanges && firstToken[0] == '0' )
		{
			parseRange( p2 );
		}
		delete p2;
   }
	CGUIStringInfo::close();

	return result;
}

