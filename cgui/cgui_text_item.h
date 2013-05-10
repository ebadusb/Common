/*
 *	Copyright (c) 2005 by Gambro BCT, Inc.  All rights reserved.
 *
 * $Header$
 */

#ifndef _CGUI_TEXT_ITEM_INCLUDE
#define _CGUI_TEXT_ITEM_INCLUDE

#include "unicode_string/unicode_string.h"
#include "cgui_graphics.h"
#include "cgui_string_data_container.h"

struct StylingRecord
{
	CGUIColor     color;       ///< color of the text as applied to whole string
	unsigned int  attributes;  ///< bits are defined in cgui_text.h
	CGUIRegion    region;      ///< placement in window or _owner relative
	CGUIFontId    fontId;      ///< font used, this should use the _owner default value
	int           fontSize;    ///< size of font most of the time
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
	void setText(const char * utf8String, LanguageId languageId = currentLanguage);

	// Set the text for this CGUITextItem using a wide character string.
	void setText(const StringChar * texString, LanguageId languageId = currentLanguage);

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
