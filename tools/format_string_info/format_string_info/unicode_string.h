/*
 *  Copyright(c) 2007 by Gambro BCT, Inc. All rights reserved.
 *
 * $Header: K:/BCT_Development/vxWorks/Common/unicode_string/rcs/unicode_string.h 1.1 2007/05/18 16:19:19Z wms10235 Exp $
 *
 *	Wide character string class
 *
 * $Log: unicode_string.h $
 * Revision 1.1  2007/05/18 16:19:19Z  wms10235
 * Initial revision
 *
 */

#ifndef _UNICODE_STRING_INCLUDE
#define _UNICODE_STRING_INCLUDE

#include "unicode_string_types.h"
#include <string>

class UnicodeString
{
public:
	UnicodeString(void);
	UnicodeString(const StringChar c);
	UnicodeString(const StringChar c, unsigned int n);
	UnicodeString(const StringChar *ucsStr);
	UnicodeString(const UnicodeString& other);
	UnicodeString(const char *utf8Str);
	virtual ~UnicodeString();

	StringChar& operator[](int i);
	const StringChar operator[](int i) const;

	UnicodeString& operator=(const StringChar ucsChar);
	UnicodeString& operator=(const StringChar *ucsStr);

	UnicodeString& operator=(const char* utf8Str);
	UnicodeString& operator+=(const char* utf8Str);
	UnicodeString& operator=(const UnicodeString& ucsStr);
	UnicodeString& operator+=(const UnicodeString& ucsStr);
	UnicodeString& operator+=(const StringChar * ucsStr);
	UnicodeString operator+(const UnicodeString& ucsStr);
	UnicodeString operator+(const UnicodeString& ucsStr) const;
	UnicodeString operator+(const StringChar * ucsStr);
	UnicodeString operator+(const StringChar * ucsStr) const;

	bool operator==(const StringChar * other) const;
	bool operator==(const UnicodeString& other) const;
	bool operator!=(const UnicodeString& other) const;
	bool operator<(const UnicodeString& other) const;
	bool operator>(const UnicodeString& other) const;
	bool operator<=(const UnicodeString& other) const;
	bool operator>=(const UnicodeString& other) const;

	unsigned int getLength() const { return _length; }
	unsigned int getBytes() const { return _length * sizeof(StringChar); }

	unsigned int getAllocatedLength() const { return _allocatedSize; }
	unsigned int getAllocatedBytes() const { return _allocatedSize * sizeof(StringChar); }

	// Find a substring in this string. If the substring is
	// found, the zero based index of where the substring started
	// is returned. Otherwise, -1 is returned. The "index" parameter
	// is the zero based index from where the search begins.
	int find(const StringChar *ucsStr, int index = 0) const;
	int find(const UnicodeString& ucsStr, int index = 0) const;
	int find(const StringChar a, int index = 0) const;

	// Find a substring starting from index and search toward the beginning
	// of the string. An index value of -1 indicates the starting point
	// is the end of the string.
	int reverseFind(const StringChar *ucsStr, int index = -1) const;
	int reverseFind(const UnicodeString& ucsStr, int index = -1) const;
	int reverseFind(const StringChar a, int index = -1) const;

	// Insert a string where index is the 0 based index into the string.
	// If index is greater than the string length, then uscStr is
	// concatinated to the end of the current string. If index is zero,
	// ucsStr is inserted in front of the current string. Returns the
	// length of the string after the insert.
	int insert(const StringChar * ucsStr, int index=0);
	int insert(const UnicodeString& ucsStr, int index=0);

	// Locate and remove a substring from this string starting
	// at index. If the substring is found, the zero based index
	// of where the substring started is returned. Otherwise, -1
	// is returned.
	int removeString(const StringChar *ucsStr, int index=0);
	int removeString(const UnicodeString& ucsStr, int index=0);

	// Delete a given number of characters from within the string starting
	// at index. If -1 is used for the count, characters are deleted
	// to the end of the string. Returns number of characters removed or -1
	// if the index is not within the string.
	int deleteChar(int index, int count = -1);

	// Return an object that contains a copy of the first
	// (ie leftmost) count of characters.
	UnicodeString left(int count) const;

	// Return an object that contains a copy of the last
	// (ie rightmost) count of characters.
	UnicodeString right(int count) const;

	// Copy up to "count" number of characters starting from the zero based
	// index into a new string that is returned. If -1 is used for the count,
	// characters are copied from index to the end of the string.
	UnicodeString mid(int index, int count = -1) const;

	// copy characters from index to end of string into a new string
	UnicodeString rightPos(int index) const;

	// Trim any character in the trim string from the this string.
	void trimLeadingChars(const StringChar * trim);
	void trimTrailingChars(const StringChar * trim);
	void trimLeadingTrailingChars(const StringChar * trim);

	// Returns the internal string.
	const StringChar * getString(void) const;

	// Returns a UTF8 encoded string
	std::string getUTF8(void) const;

	bool isEmpty(void) const { return ( _length < 1 ); }

	// Empty the string
	void empty(void);

	// Set a character or get a character. Returns false
	// if the zero based index is out of bounds.
	bool getAt(int index, StringChar& ucsChar) const;
	bool setAt(int index, StringChar ucsChar);

	// Replace a character with another, return the number of replacements
	// or zero if no changes.
	int replace( StringChar chOld, StringChar chNew );

	// Replace a substring with another, return the number of replacements
	int replace(const StringChar * oldStr, const StringChar * newStr);
	int replace(const UnicodeString& oldStr, const UnicodeString& newStr);

protected:

	bool allocate(const unsigned int nsize);
	void deallocate(void);
	unsigned int indexCheck(int index) const;

	mutable StringChar	*_data;					// Pointer to the unicode string
	mutable unsigned int	_length;			// Number of characters in the string not including NULL terminator
	mutable unsigned int	_allocatedSize;		// Allocated size of the _data buffer in characters
};

#endif /* ifndef _UNICODE_STRING_INCLUDE */
