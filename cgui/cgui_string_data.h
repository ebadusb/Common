/*
 *	Copyright (c) 2004 by Gambro BCT, Inc.  All rights reserved.
 *
 * $Header$
 * $Log: cgui_string_data.h $
 * Revision 1.6  2007/05/03 16:19:14Z  jl11312
 * - added semaphore protection for map structures
 * Revision 1.5  2007/04/30 18:26:07Z  jl11312
 * - additional error checking when reading string info files (Taos IT 3102)
 * Revision 1.4  2007/02/08 19:28:05Z  rm10919
 * Updates to add languages to string data.
 * Revision 1.3  2006/11/28 20:42:55Z  pn02526
 * Use CGUIStringInfo class to read string.info files.
 * Revision 1.2  2006/07/25 15:42:37  cf10242
 * IT 54: decode UTF8 into unicode
 * Revision 1.1  2005/04/27 13:40:46Z  rm10919
 * Initial revision
 *
 */

#ifndef _CGUI_STRING_DATA_INCLUDE
#define _CGUI_STRING_DATA_INCLUDE

#include <semLib.h>
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

   bool readDatabaseFile (const char * filename, CGUIFontId * fontId, LanguageId languageId = currentLanguage, int fontIndex = 0);
   bool readDatabaseItem (CGUITextItem * LanguageName); //, Language language[0]);

protected:
	SEM_ID _lock;
   map<string, CGUITextItem *> _textMap;

private:
	CGUIStringData(void);
	CGUIStringData(const CGUIStringData &obj);
	CGUIStringData& operator=(const CGUIStringData &obj);
};
#endif //_CGUI_STRING_DATA_INCLUDE
