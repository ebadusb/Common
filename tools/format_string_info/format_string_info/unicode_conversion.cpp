/*
 *  Copyright(c) 2007 by Gambro BCT, Inc. All rights reserved.
 *
 * $Header$
 *
 * Unicode conversion class
 *
 * $Log: unicode_conversion.cpp $
 * Revision 1.1  2007/05/18 16:19:18Z  wms10235
 * Initial revision
 *
 */

#include "stdafx.h"
#include <string.h>
#include "ucs_string.h"
#include "unicode_conversion.h"

static const char bytesFromUTF8[256] = {
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, 3,3,3,3,3,3,3,3,4,4,4,4,5,5,5,5};

static const unsigned char firstByteMark[7] = {0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC};

static const unsigned long offsetsFromUTF8[6] = {0x00000000UL, 0x00003080UL, 0x000E2080UL,
					 	 	 				0x03C82080UL, 0xFA082080UL, 0x82082080UL};

static const unsigned long kReplacementCharacter =	0x0000FFFDUL;
static const unsigned long kMaximumUCS2 =				0x0000FFFFUL;
static const unsigned long kMaximumUTF16 =			0x0010FFFFUL;
static const unsigned long kMaximumUCS4 =				0x7FFFFFFFUL;

static const int halfShift						= 10;
static const unsigned long halfBase			= 0x0010000UL;
static const unsigned long halfMask			= 0x3FFUL;
static const unsigned long kSurrogateHighStart	= 0xD800UL;
static const unsigned long kSurrogateHighEnd	= 0xDBFFUL;
static const unsigned long kSurrogateLowStart	= 0xDC00UL;
static const unsigned long kSurrogateLowEnd	= 0xDFFFUL;

UnicodeConversion::UnicodeConversion(void) :
	_result(UnicodeConversion::Invalid),
	_ucsStr(NULL),
	_utf8Str(NULL)
{
}

UnicodeConversion::~UnicodeConversion()
{
	if( _ucsStr )
		delete [] _ucsStr;
	if( _utf8Str )
		delete [] _utf8Str;
}

UnicodeConversion::Result UnicodeConversion::utf8ToUcs(const char *utf8Str)
{
	_result = UnicodeConversion::Invalid;

	if( utf8Str )
	{
		size_t	utf8Length = strlen(utf8Str);
		unsigned char	*source = (unsigned char*)utf8Str;
		unsigned char	*sourceEnd = source + utf8Length;

		// Since a UCS string must have at least the number of characters
		// in a UTF8 string, allocate the worst case amount for the UCS
		// string. Also, set the string to null.
		unsigned short	*targetStr = new unsigned short[utf8Length + 1];
		memset(targetStr, 0, (utf8Length + 1) * sizeof(unsigned short));

		// Set target to the begining of the target string.
		unsigned short	*target = targetStr;
		unsigned short	*targetEnd = targetStr + utf8Length + 1;

		_result = Success;

		if( _ucsStr )
		{
			delete [] _ucsStr;
			_ucsStr = NULL;
		}

		while(*source != 0)
		{
			register unsigned long ch = 0;
			register unsigned short extraBytesToWrite = bytesFromUTF8[*source];
			if (source + extraBytesToWrite > sourceEnd)
			{
				// Stop conversion. The UTF8 string is missing characters.
				_result = UnicodeConversion::SourceExhausted;
				break;
			}

			switch(extraBytesToWrite) 	/* note: code falls through cases! */
			{
				case 5:	ch += *source++; ch <<= 6;
				case 4:	ch += *source++; ch <<= 6;
				case 3:	ch += *source++; ch <<= 6;
				case 2:	ch += *source++; ch <<= 6;
				case 1:	ch += *source++; ch <<= 6;
				case 0:	ch += *source++;
			}
			ch -= offsetsFromUTF8[extraBytesToWrite];

			if (ch <= kMaximumUCS2)
			{
				*target++ = (unsigned short)ch;
			}
			else if (ch > kMaximumUTF16)
			{
				*target++ = kReplacementCharacter;
			}
			else
			{
				if (target + 1 >= targetEnd)
				{
					// The source overflowed the end of the target.
					_result = UnicodeConversion::TargetExhausted;
					break;
				}
				ch -= halfBase;
				*target++ = (unsigned short)((ch >> halfShift) + kSurrogateHighStart);
				*target++ = (unsigned short)((ch & halfMask) + kSurrogateLowStart);
			}
		}

		_ucsStr = (StringChar*)targetStr;
	}

	return _result;
}

UnicodeConversion::Result UnicodeConversion::ucsToUtf8(const UnicodeString& ucsStr)
{
	size_t ulen = ucsStr.getLength();

	register const unsigned short* source = (const unsigned short*)ucsStr.getString();
	const unsigned short* sourceEnd = source + ulen;

	// Initialize Error Flag to ok
	_result = Success;

	if( _utf8Str )
	{
		delete [] _utf8Str;
		_utf8Str = NULL;
	}

	// Allocate storage for the UTF8 string. Allocation should be
	// about 4X the length of the UCS string worst case. If an
	// old string is already allocated, delete it.
	_utf8Str = new char[(ulen * 4) + 1];

	register unsigned char* target = (unsigned char*)_utf8Str;
	unsigned char* targetEnd = target + (ulen * 4);

	while (*source != 0)
	{
		register unsigned long ch;
		register unsigned short bytesToWrite = 0;
		register const unsigned long byteMask = 0xBF;
		register const unsigned long byteMark = 0x80;
		ch = *source++;
		if (ch >= kSurrogateHighStart && ch <= kSurrogateHighEnd && source < sourceEnd)
		{
			register unsigned long ch2 = *source;
			if (ch2 >= kSurrogateLowStart && ch2 <= kSurrogateLowEnd)
			{
				ch = ((ch - kSurrogateHighStart) << halfShift)
					+ (ch2 - kSurrogateLowStart) + halfBase;
				++source;
			}
		}

		if (ch < 0x80)
		{
			bytesToWrite = 1;
		}
		else if (ch < 0x800)
		{
			bytesToWrite = 2;
		}
		else if (ch < 0x10000)
		{
			bytesToWrite = 3;
		}
		else if (ch < 0x200000)
		{
			bytesToWrite = 4;
		}
		else if (ch < 0x4000000)
		{
			bytesToWrite = 5;
		}
		else if (ch <= kMaximumUCS4)
		{
			bytesToWrite = 6;
		}
		else
		{
			bytesToWrite = 2;
			ch = kReplacementCharacter;
		}

		target += bytesToWrite;

		if (target > targetEnd)
		{
			target -= bytesToWrite;
			_result = UnicodeConversion::TargetExhausted;
			break;
		}

		switch (bytesToWrite) /* note: code falls through cases! */
		{
			case 6:	*--target = (unsigned char)((ch | byteMark) & byteMask); ch >>= 6;
			case 5:	*--target = (unsigned char)((ch | byteMark) & byteMask); ch >>= 6;
			case 4:	*--target = (unsigned char)((ch | byteMark) & byteMask); ch >>= 6;
			case 3:	*--target = (unsigned char)((ch | byteMark) & byteMask); ch >>= 6;
			case 2:	*--target = (unsigned char)((ch | byteMark) & byteMask); ch >>= 6;
			case 1:	*--target = (unsigned char)(ch | firstByteMark[bytesToWrite]);
		}
		target += bytesToWrite;
	}

	*target = 0;	// Null terminate the UTF8 string.

	return _result;
}

UnicodeConversion::Result UnicodeConversion::ucsToUtf8(const StringChar * ucsStr)
{
	_result = UnicodeConversion::Invalid;

	if( ucsStr )
	{
		size_t ulen = ucslen( ucsStr );

		register unsigned short* source = (unsigned short*)ucsStr;
		unsigned short* sourceEnd = source + ulen;

		// Initialize Error Flag to ok
		_result = Success;

		if( _utf8Str )
		{
			delete [] _utf8Str;
			_utf8Str = NULL;
		}

		// Allocate storage for the UTF8 string. Allocation should be
		// about 4X the length of the UCS string worst case. If an
		// old string is already allocated, delete it.
		_utf8Str = new char[(ulen * 4) + 1];

		register unsigned char* target = (unsigned char*)_utf8Str;
		unsigned char* targetEnd = target + (ulen * 4);

		while (*source != 0)
		{
			register unsigned long ch;
			register unsigned short bytesToWrite = 0;
			register const unsigned long byteMask = 0xBF;
			register const unsigned long byteMark = 0x80;
			ch = *source++;
			if (ch >= kSurrogateHighStart && ch <= kSurrogateHighEnd && source < sourceEnd)
			{
				register unsigned long ch2 = *source;
				if (ch2 >= kSurrogateLowStart && ch2 <= kSurrogateLowEnd)
				{
					ch = ((ch - kSurrogateHighStart) << halfShift)
						+ (ch2 - kSurrogateLowStart) + halfBase;
					++source;
				}
			}

			if (ch < 0x80)
			{
				bytesToWrite = 1;
			}
			else if (ch < 0x800)
			{
				bytesToWrite = 2;
			}
			else if (ch < 0x10000)
			{
				bytesToWrite = 3;
			}
			else if (ch < 0x200000)
			{
				bytesToWrite = 4;
			}
			else if (ch < 0x4000000)
			{
				bytesToWrite = 5;
			}
			else if (ch <= kMaximumUCS4)
			{
				bytesToWrite = 6;
			}
			else
			{
				bytesToWrite = 2;
				ch = kReplacementCharacter;
			}

			target += bytesToWrite;

			if (target > targetEnd)
			{
				target -= bytesToWrite;
				_result = UnicodeConversion::TargetExhausted;
				break;
			}

			switch (bytesToWrite) /* note: code falls through cases! */
			{
				case 6:	*--target = (unsigned char)((ch | byteMark) & byteMask); ch >>= 6;
				case 5:	*--target = (unsigned char)((ch | byteMark) & byteMask); ch >>= 6;
				case 4:	*--target = (unsigned char)((ch | byteMark) & byteMask); ch >>= 6;
				case 3:	*--target = (unsigned char)((ch | byteMark) & byteMask); ch >>= 6;
				case 2:	*--target = (unsigned char)((ch | byteMark) & byteMask); ch >>= 6;
				case 1:	*--target = (unsigned char)(ch | firstByteMark[bytesToWrite]);
			}
			target += bytesToWrite;
		}

		*target = 0;	// Null terminate the UTF8 string.
	}

	return _result;
}

