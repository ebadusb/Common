/*
 *  Copyright(c) 2007 by Gambro BCT, Inc. All rights reserved.
 *
 * $Header$
 *
 *	This class is responsible for unicode format conversions.
 * It can convert USC (16-bit unicode characters) to UTF8
 * and UTF8 to UCS.
 *
 * $Log$
 *
 */

#ifndef _UNICODE_CONVERSION_INCLUDE_
#define _UNICODE_CONVERSION_INCLUDE_

#include <string>
#include <unicode_string/unicode_string.h>

class UnicodeConversion
{
public:
	enum Result
	{
		Success,				/* conversion successful */
		SourceExhausted,	/* Abnormal end of string. */
		TargetExhausted,	/* insuff. room in target string for conversion */
		Invalid				/* Result is invalid */
	};

	UnicodeConversion(void);
	virtual ~UnicodeConversion();

	//	This function converts a null terminated UTF8 string to
	//	a UCS-2 wide character unicode string and returns it in
	//	a String object.  A null string is return if the result
	//	code is not 0.
	Result utf8ToUcs(const char *utf8Str);

	// Get the last unicode string converted by utf8ToUcs().
	// Note that a NULL is returned if the utf8ToUcs() method
	// was never called or failed.
	const StringChar * getUcsString(void) const { return _ucsStr; }

	//	This method converts a USC-2 unicode wide character
	//	string to a UTF8 encoded unicode string.  A pointer to
	//	the converted string is returned if the convertion is
	//	successful.  Otherwise, a null string is returned and
	//	the result code is set to indicate the error.  Note that
	//	the converted string store in the object is destroyed
	//	with this object should be copied to another string
	//	before this object goes out of scope or before the next
	//	call to this method.
	Result ucsToUtf8(const StringChar *ucsStr);
	Result ucsToUtf8(const UnicodeString& ucsStr);

	// Get the unicode string converted by ucsToUtf8().
	const char * getUtf8String(void) const { return _utf8Str; }

	//	Get the result code for the last conversion.
	Result getResult(void) const { return _result; }

protected:
	Result			_result;
	StringChar			*_ucsStr;
	char				*_utf8Str;

	// no value semantics
	UnicodeConversion& operator=(const UnicodeConversion& rhs);
	UnicodeConversion(const UnicodeConversion& rhs);
};
#endif /* ifndef _UNICODE_CONVERSION_INCLUDE_ */

