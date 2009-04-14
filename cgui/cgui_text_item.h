/*
 *	Copyright (c) 2005 by Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: K:/BCT_Development/vxWorks/Common/cgui/rcs/cgui_text_item.h 1.15 2009/04/13 22:34:00Z rm10919 Exp wms10235 $
 * $Log: cgui_text_item.h $
 * Revision 1.14  2007/06/04 22:04:21Z  wms10235
 * IT83 - Updates for the common GUI project to use the unicode string class
 * Revision 1.13  2007/04/30 21:18:28Z  wms10235
 * IT2354 - Updated the getTextItem() method to be a static
 * Revision 1.12  2007/04/26 16:47:20Z  wms10235
 * IT2354 - Updated the assignment operator and copy contructor
 * Revision 1.11  2007/02/21 21:06:53Z  rm10919
 * Add methods for variable substitution and stringLength.  Fix copy constructor and equals operator.
 * Revision 1.10  2006/07/13 20:31:51Z  cf10242
 * IT 1220: eliminate private copy contrstuctor
 * Revision 1.9  2006/07/12 23:36:08Z  rm10919
 * Updates from adding cguiListBox class.
 * Revision 1.8  2006/06/28 00:56:37Z  MS10234
 * - add include file for cgui_string_data_container.h
 * Revision 1.7  2005/11/22 00:34:43Z  rm10919
 * Get data item database to work with software layers.
 * Revision 1.6  2005/08/01 23:31:39Z  cf10242
 * Revision 1.5  2005/04/26 23:16:48Z  rm10919
 * Made changes to cgui_text and cgui_text_item, plus added
 * classes for variable substitution in text strings.
 * Revision 1.4  2005/04/04 18:03:53Z  rm10919
 * Add int getLength(void) method for text string.
 * Revision 1.3  2005/03/15 00:21:36Z  rm10919
 * Change CGUIText to not add object to window object list of parent in constructor.
 * Revision 1.2  2005/02/21 17:17:13Z  cf10242
 * IT 133 - delete all allocated memory to avoid unrecovered memory
 * Revision 1.1  2005/01/31 17:36:39Z  rm10919
 * Initial revision
 *
 */

#ifndef _CGUI_TEXT_ITEM_INCLUDE
#define _CGUI_TEXT_ITEM_INCLUDE

#include "unicode_string/unicode_string.h"
#include "cgui_graphics.h"
#include "cgui_string_data_container.h"

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
	CGUITextItem(void);
	CGUITextItem(const char * id, StylingRecord * stylingRecord = NULL);
	CGUITextItem(const CGUITextItem& textItem);

	CGUITextItem& operator= (const CGUITextItem& textItem);

	inline bool operator== (const CGUITextItem& textItem) {return (_id == textItem._id);}
	virtual ~CGUITextItem();

	// Set the string's database ID
	void setId(const char * id);
	const char * getId(void) const;

	// Returns font size for stylling record.
	int getFontSize( void ) { return _stylingRecord.fontSize;}

	// The method returns a pointer to the text string
	const StringChar * getText(LanguageId languageId = currentLanguage);

	// Returns a unicode string object of the string
	const UnicodeString& getTextObj(LanguageId languageId = currentLanguage);

	// Get a UTF8 (ASCII) representation of the string
	void getAscii(string& myString, LanguageId languageId = currentLanguage);

	// Helper function that looks up a CGUITextItem for the given ID.
	// Returns NULL if the item is not found.
	static CGUITextItem * getTextItem(const char * id, LanguageId languageId = currentLanguage);

	// Set the text for this CGUITextItem using a UTF8 string. The UTF8
	// string is converted to wide character unicode.
	void setText(const char * utf8String, LanguageId = currentLanguage);

	// Set the text for this CGUITextItem using a wide character string.
	void setText(const StringChar * texString, LanguageId = currentLanguage);

	// Get the language ID
	LanguageId getLanguageId(void) const { return _languageId;}

	// Set the language ID or default language ID
	void setLanguageId(LanguageId languageId){_languageId = languageId;}
	void setDefaultLanguage(LanguageId languageId){_defaultLanguageId = languageId;}

	// Returns true if the text item ID is set
	bool isInitialized(void) const;

	// Set the GUI styling record.
	void setStylingRecord(StylingRecord stylingRecord) {_stylingRecord = stylingRecord;}

	// Get the GUI styling record.
	StylingRecord getStylingRecord(void) const {return _stylingRecord;}

	// Returns the length of the string in characters
	int getLength(void) const;

	// Returns the length of the string in characters and
	// performs variable substitution.
	int getStringCharVariableLength(void);

	// Perform variable substitution in the text.
	void handleVariableSubstitution(void);

	static CGUIStringDataContainer _textMap; // String database

private:
   string			_id;						// Non-translated internal string ID
   UnicodeString	_string;					// String with variables substituted
   UnicodeString	_template;				// String template with variable placeholders
   LanguageId		_languageId;			// Language ID
   StylingRecord	_stylingRecord;		// GUI styling information
   bool				_hasVariables;			// Indicates that the template cantains variables

   static int		_defaultLanguageId;
};

#endif /* #ifndef _CGUI_TEXT_ITEM_INCLUDE */
