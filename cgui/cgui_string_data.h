/*
 *	Copyright (c) 2004 by Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: K:/BCT_Development/vxWorks/Common/cgui/rcs/cgui_string_data.h 1.6 2007/05/03 16:19:14Z jl11312 Exp wms10235 $
 * $Log: cgui_string_data.h $
 * Revision 1.2  2006/07/25 15:42:37  cf10242
 * IT 54: decode UTF8 into unicode
 * Revision 1.1  2005/04/27 13:40:46Z  rm10919
 * Initial revision
 *
 */

#ifndef _CGUI_STRING_DATA_INCLUDE
#define _CGUI_STRING_DATA_INCLUDE
                    
                    
#include "link_element.h"
#include "link_group.h"
#include "cgui_text_item.h"

class CGUIStringData: public LinkElement
{
public:
   CGUIStringData(unsigned int linkLevel);
   virtual ~CGUIStringData(void);

   CGUITextItem * findString(const char * id);

   void addTextItem(const char * name, CGUITextItem * textItem);
   void deleteTextItem (const char * name);

   void readDatabaseFile (const char * filename, CGUIFontId * fontId, LanguageId languageId = currentLanguage);
   bool readDatabaseItem (CGUITextItem * LanguageName); //, Language language[0]);

protected:
   map<string, CGUITextItem *> _textMap;

private:

};
#endif //_CGUI_STRING_DATA_INCLUDE
