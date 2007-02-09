/*
 *	Copyright (c) 2004 by Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: K:/BCT_Development/vxWorks/Common/cgui/rcs/cgui_string_data.cpp 1.12 2007/06/14 19:34:11Z wms10235 Exp wms10235 $
 * $Log: cgui_string_data.cpp $
 * Revision 1.8  2006/12/01 19:20:07Z  pn02526
 * Use CGUIStringInfo class to read string.info files.
 * Revision 1.7  2006/07/25 15:42:37  cf10242
 * IT 54: decode UTF8 into unicode
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
#include "cgui_string_info.h"
#include "datalog_levels.h"


CGUIStringData::CGUIStringData(unsigned int linkLevel): LinkElement()
{
   do_link((LinkGroup *)&CGUITextItem::_textMap, (LinkElement**)&CGUITextItem::_textMap._textTable, linkLevel, LT_Exclusive);
}

CGUIStringData::~CGUIStringData(void)
{
}

void CGUIStringData::readDatabaseFile (const char * filename, CGUIFontId * fontId, LanguageId languageId = currentLanguage, int fontIndex = 0)
{
    CGUITextItem textItem;
    // Open file
    CGUIStringInfo stringInfo( filename );

//   taskSuspend(taskIdSelf());
    DataLog( log_level_cgui_info ) << "Entering CGUIStringData::readDatabaseFile for " << filename << endmsg;

   // Loop reading string info records, converting them to CGUTextItems,
   // and putting them in their proper places in the text map.
   while ( stringInfo.get(fontId, textItem, NULL, fontIndex) )
   {
//            DataLog( log_level_cgui_info ) << "line " << stringInfo.line() << ": got record " << textItem.getId() << endmsg;

            // Define a pointer for a map entry's CGUITextItem.
            CGUITextItem * result;

            // Define an iterator for the text map.
            map<string, CGUITextItem *>::iterator iter;

            //
            // Do the lookup thing for the textItem.
            //
            iter = _textMap.find(textItem.getId());
//            DataLog( log_level_cgui_info ) << "line " << stringInfo.line() << ": found " << textItem.getId() << endmsg;
            result = iter->second;

            // If found the corresponding text item in map,
            // correct the formatting sequences in the text.
            if (iter != _textMap.end() && result)
            {
               //
               // put text item information in corresponding _textMap[id].
               //            
//               DataLog( log_level_cgui_info ) << "line " << stringInfo.line() << ": copying to captive text item at " << hex << (unsigned int)result << dec << endmsg;
               result->setText(textItem.getText());
               result->setStylingRecord(textItem.getStylingRecord());
               result->setLanguageId(currentLanguage);
            } else
            {
               DataLog( log_level_cgui_error ) << "line " << stringInfo.line() << ": Can't find string Id " << textItem.getId() << " in map!!!!! - " << filename << endmsg;
               printf("line %d: Can't find string Id %s in map!!!!!\n - %s", stringInfo.line(), textItem.getId(), filename);
            }
   }

   // Check for premature end.
   if( !stringInfo.endOfFile() )
   {
      DataLog( log_level_cgui_error ) << "line " << stringInfo.line() << ": bad entry in string info file - " << filename << endmsg;
      printf("line %d: bad entry in string info file\n - %s", stringInfo.line(), filename);
   }

   // Close file.
   DataLog( log_level_cgui_info ) << "line " << stringInfo.line() << ": closing string info file - " << filename << endmsg;
   stringInfo.close();

   return;
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


