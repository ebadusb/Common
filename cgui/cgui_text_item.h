/*
 *	Copyright (c) 2005 by Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: //bctquad3/home/BCT_Development/vxWorks/Common/cgui/rcs/cgui_text_item.h 1.7 2005/11/22 00:34:43Z rm10919 Exp MS10234 $
 * $Log: cgui_text_item.h $
 *
 */

#ifndef _CGUI_TEXT_ITEM_INCLUDE
#define _CGUI_TEXT_ITEM_INCLUDE

#include "cgui_graphics.h"

struct StylingRecord
{
   CGUIColor     color;       // color of the text as applied to whole string
   unsigned int  attributes;  // attributes from the attribute enums above
   CGUIRegion    region;      // placement in window or _owner relative
   CGUIFontId    fontId;      // font used, this should use the _owner default value
   int           fontSize;    // size of font most of the time
};

//
// This class will access the strings from the database
// or the file that holds all of the string information
// (both string id and the actual string text).  The 
// string id is the key to accessing this information.
//

class CGUITextItem
{
public:
   CGUITextItem();
   CGUITextItem(const char * id, StylingRecord * stylingRecord = NULL);
   virtual ~ CGUITextItem();

   void setId(const char * id);
   //
   // The method will retrun the text string
   //
   //
   const StringChar * getText(LanguageId languageId = currentLanguage);
   char * getAscii(LanguageId languageId = currentLanguage);

   CGUITextItem * getTextItem(const char * id, LanguageId languageId);

   void setText(const char * string, LanguageId = currentLanguage);
   void setText(const StringChar * texString, LanguageId = currentLanguage);

   LanguageId getLanguageId(void){ return _languageId;}

   void setLanguageId(LanguageId languageId){_languageId = languageId;}
   void setDefaultLanguage(LanguageId languageId){_defaultLanguageId = languageId;}

   bool isInitialized(void);

   void setStylingRecord(StylingRecord stylingRecord){_stylingRecord = stylingRecord;}
   
   StylingRecord getStylingRecord(){return _stylingRecord;}


protected:

   unsigned short _stringLength;

private:
   //
   // All the methods and routines to manipulate getting the string id's and text go here.
   // These will be defined as the text string database is designed. A balance between
   // cacheing and reading the database will need to be developed. As this part is being 
   // developed a method to create dynamic string id's (for dymanic data, e.g. numeric
   // values) will need to be done.
   //
   const char * _id;
   StringChar * _string;
   LanguageId   _languageId;

   static int   _defaultLanguageId;

   StylingRecord _stylingRecord;
   // copy constructor and copy assignment are not legal for this class
   //CGUITextItem (CGUITextItem &);
   //CGUITextItem& operator= (const CGUITextItem&);
};

#endif /* #ifndef _CGUI_TEXT_ITEM_INCLUDE */
