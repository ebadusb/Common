/*
 *	Copyright (c) 2004 by Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: K:/BCT_Development/vxWorks/Common/cgui/rcs/cgui_string_data.cpp 1.12 2007/06/14 19:34:11Z wms10235 Exp wms10235 $
 * $Log: cgui_string_data.cpp $
 * Revision 1.6  2006/07/12 23:37:57Z  rm10919
 * Update for reading in unicode strings.
 * Revision 1.5  2006/05/15 21:52:17Z  rm10919
 * Add debug lines.
 * Revision 1.4  2005/10/19 15:42:22Z  rm10919
 * Fix file read to not skip over text if only one space.
 * Revision 1.3  2005/06/09 15:00:12Z  rm10919
 * Fix the read data file routine.
 * Revision 1.2  2005/04/28 14:40:32Z  rm10919
 * Fixed bug in reading eol characters.
 * Revision 1.1  2005/04/27 13:40:45Z  rm10919
 * Initial revision
 *
 */

#include <vxWorks.h>
#include "cgui_string_data.h"
#include "cgui_string_data_container.h"
#include "datalog_levels.h"


CGUIStringData::CGUIStringData(unsigned int linkLevel): LinkElement()
{
   do_link((LinkGroup *)&CGUITextItem::_textMap, (LinkElement**)&CGUITextItem::_textMap._textTable, linkLevel, LT_Exclusive);
}

CGUIStringData::~CGUIStringData(void)
{
}

StringChar CGUIStringData::UTF8ToUnicode(StringChar utf8Char, int line)
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
		DataLog( log_level_cgui_error ) << "Found possible non-2 byte UTF8 sequence in line " << line << "   1st byte = " << hex << firstByte << "  second byte = " << secondByte << dec << endmsg;
	return unicodeChar;
}

bool CGUIStringData::readDatabaseFile (const char * filename, CGUIFontId * fontId, LanguageId languageId = currentLanguage)
{
   enum
   {
      LineBufferSize = 65000
   };

   struct StringDataEntry
   {
      char * id;
      char * text;
      char * red;
      char * green;
      char * blue;
      char * attributes;
      char * x;
      char * y;
      char * width;
      char * height;
      char * fontSize;
   }; 

   static char lineBuffer[LineBufferSize];
   bool  readingFileTable = false;

   FILE * stringInfo = fopen(filename, "r");

   int   line = 0;
   char * p = NULL;

//   taskSuspend(taskIdSelf());

   while (fgets(lineBuffer, LineBufferSize, stringInfo) != NULL)
   {
      line += 1;
      char * firstToken = strtok_r(lineBuffer, " \t\n\r\"", &p);

      if (!firstToken || firstToken[0] == '#')
      {
         continue;
      }

      if (strcmp(firstToken, "STRING_FILE_TABLE_START") == 0)
      {
         if (readingFileTable)
         {
            DataLog( log_level_cgui_error ) << "line " << line << ": unexpected STRING_FILE_TABLE_START - " << filename << endmsg;
            printf("line %d: unexpected STRING_FILE_TABLE_START\n - %s", line, filename);
            exit(1);
         }

         readingFileTable = true;
         continue;
      }

      if (strcmp(firstToken, "STRING_FILE_TABLE_END") == 0)
      {
         if (!readingFileTable)
         {
            DataLog( log_level_cgui_error ) << "line " << line << ": unexpected STRING_FILE_TABLE_END - " << filename << endmsg;
            printf("line %d: unexpected STRING_FILE_TABLE_END\n - %s", line, filename);
            exit(1);
         }
//         printf("\n\n\nline %d: STRING_FILE_TABLE_END\n - %s\n\n\n", line, filename);
         readingFileTable = false;
         continue;
      }

      if (readingFileTable)
      {
         StringDataEntry * entry = new StringDataEntry;
         char * hold = NULL;

         //
         // Parse entry from file.
         //
         entry->id = firstToken;

         if (*p != '"')
         {
            hold = strtok_r(NULL,"\"", &p);    // get to the first quote
         }
         entry->text = strtok_r(NULL, "\"", &p);
         entry->red = strtok_r(NULL," \t\n\r", &p);
         entry->green = strtok_r(NULL," \t\n\r", &p);
         entry->blue = strtok_r(NULL," \t\n\r", &p);
         entry->attributes = strtok_r(NULL," \t\n\r", &p);
         entry->x = strtok_r(NULL," \t\n\r", &p);
         entry->y = strtok_r(NULL," \t\n\r", &p);
         entry->width = strtok_r(NULL," \t\n\r", &p);
         entry->height = strtok_r(NULL," \t\n\r", &p);
         entry->fontSize = strtok_r(NULL," \t\n\r", &p);

//         printf("line %d: unexpected END\n - %s", line, filename);

         if (entry->id)
         {
            CGUITextItem * result = NULL;
            StylingRecord stylingRecord;

            map<string, CGUITextItem *>::iterator iter;

            //
            // Do the lookup thing for textItem.
            //
            iter = _textMap.find(entry->id);
            result = iter->second;

            // Create Styling Record from file information.
            stylingRecord.color = MakeCGUIColor( atoi(entry->red), atoi(entry->green), atoi(entry->blue));
            sscanf(entry->attributes, "%x", &stylingRecord.attributes);
            stylingRecord.region = CGUIRegion( atoi(entry->x), atoi(entry->y), atoi(entry->width), atoi(entry->height));
            stylingRecord.fontSize = atoi(entry->fontSize);
            stylingRecord.fontId = fontId[atoi(entry->fontSize)];               

            // Found the corresponding text item in map
            // so correct the formatting sequences in the text.
            // When the string.info file is generated by the 
            // database this read routine will need to be 
            // changed to update unicode instead of ASCII text.
            if (result)
            {
               int readIndex = 0;
               int writeIndex = 0;
               int length = strlen(entry->text);

               StringChar * wString = NULL;

               bool status = true;
               // Allocate to maximum possible length (may be less due to slash sequences)
               //
               wString =  new StringChar[length+2];

               // Scan string, replacing slash sequences as necessary
               //
               while (readIndex < length && status)
               {
                  if (entry->text[readIndex] != '\\')
                  {
					 // check for UTF8 character and convert
				     StringChar * nextWord = (StringChar *)&entry->text[readIndex];
					 if( (*nextWord & 0x80C0) == 0x80C0 )
					 {
						 wString[writeIndex++] = (unsigned char)UTF8ToUnicode(*nextWord, line);
						 readIndex+=2;
					 }
					 else
						 wString[writeIndex++] = (unsigned char)(UGL_WCHAR)entry->text[readIndex++];

                  } else
                  {
                     if (readIndex >= length - 1)
                     {
                        // Can't have \ as last character in string
                        //
                        status = false;
                     } else
                     {
                        switch (entry->text[readIndex+1])
                        {
                        case 'b':wString[writeIndex++] = '\b'; break;
                        case 'n':wString[writeIndex++] = '\n'; break;
                        case 'r':wString[writeIndex++] = '\r'; break;
                        case 't':wString[writeIndex++] = '\t'; break;
                        case '"':wString[writeIndex++] = '"'; break;
                        case 'x':
                           readIndex += 2;  // get past the slash and the x 
                           char unicode[5] ;
                           int  l;

                           unicode[0] = entry->text[readIndex++];
                           unicode[1] = entry->text[readIndex++];
                           unicode[2] = entry->text[readIndex];  //  will increment index at bottom of loop
                           unicode[3] = entry->text[readIndex+1];  // don't increment but look at next char
                           unicode[4] = '\0';

                           sscanf(unicode, "%x",&l);
                           wString[writeIndex++] = (StringChar)l;

                           break;

                        default:wString[writeIndex++] = (unsigned char)(UGL_WCHAR)entry->text[readIndex+1];
                        }
                        readIndex += 2;
                     }
                  }
               }
               //
               // Terminate the new string.
               //
               wString[writeIndex] = '\0';
//               }
//               printf("char string - %s\nwide char string - %s\n\n", reference, wString);
//               printf("char string - %s\nwide char string - %s\n\n", entry->text, wString);
               //
               // put text item information in corresponding _textMap[id].
               //            

               result->setText(wString);
               result->setStylingRecord(stylingRecord);
            } else
            {
               DataLog( log_level_cgui_error ) << "line " << line << ": Can't find string in map!!!!! - " << filename << endmsg;
               printf("line %d: Can't find string in map!!!!!\n - %s", line, filename);
               exit(1);
            }
         } else
         {
            DataLog( log_level_cgui_error ) << "line " << line << ": bad entry in string info file - " << filename << endmsg;
            printf("line %d: bad entry in string info file\n - %s", line, filename);
            exit(1);
         }
      }
   }
   // Close file.
   fclose(stringInfo);

   return readingFileTable;
}

bool CGUIStringData::readDatabaseItem (CGUITextItem * LanguageName) //, Language language[0]);
{
   return true;
}

CGUITextItem * CGUIStringData::findString(const char * id)
{
   CGUITextItem * result = NULL;

   map<string, CGUITextItem *>::iterator iter;

   //
   // Do the lookup thing for textItem.
   //
   iter = _textMap.find(id);

   if (iter != _textMap.end())
   {
      result = iter->second;

      if (strcmp(result->getId(), id) != 0)
      {
         result = NULL;
      }

   }

   // if can't find name return NULL.
   return result;
}

void CGUIStringData::addTextItem(const char * name, CGUITextItem * textItem)
{
   _textMap[name] = textItem;

}

void CGUIStringData::deleteTextItem(const char * name)
{

   _textMap.erase(name);
}


