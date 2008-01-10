/*
 *  Copyright(c) 2007 by Gambro BCT, Inc. All rights reserved.
 *
 * $Header: H:/BCT_Development/vxWorks/Common/unicode_string/rcs/unicode_string.cpp 1.2 2007/06/04 22:05:17Z wms10235 Exp wms10235 $
 *
 *	Wide character string class
 *
 * $Log: unicode_string.cpp $
 * Revision 1.2  2007/06/04 22:05:17Z  wms10235
 * IT83 - Bug fix for unicode string class
 * Revision 1.1  2007/05/18 16:19:19Z  wms10235
 * Initial revision
 *
 */

#include <vxWorks.h>
#include "ucs_string.h"
#include "unicode_conversion.h"
#include "unicode_string.h"

// Minimum number of characters to allocate or to grow string by.
static const unsigned int minAllocationSize = 32;

UnicodeString::UnicodeString(void) :
	_data(NULL),
	_length(0),
	_allocatedSize(0)
{
}

UnicodeString::~UnicodeString()
{
	deallocate();
}

UnicodeString::UnicodeString(const StringChar a)
{
	_data = NULL;
	_length = _allocatedSize = 0;

	allocate( 1 );

	if( a == 0 )
		_length = 0;

	_data[0] = a;
	_data[1] = 0;
}

UnicodeString::UnicodeString(const StringChar c, unsigned int n)
{
	_data = NULL;
	_length = _allocatedSize = 0;

	if( c != 0 && n > 0 )
	{
		allocate( n );

		for (unsigned int i=0; i<n; i++ ) _data[i]=c;
		_data[n]=0;
	}
}

UnicodeString::UnicodeString(const StringChar * ucsStr)
{
	_data = NULL;
	_length = _allocatedSize = 0;

	if( ucsStr )
	{
		allocate( ucslen( ucsStr ) );

		memcpy( _data, ucsStr, (_length + 1) * sizeof(StringChar) );
	}
}

UnicodeString::UnicodeString(const UnicodeString& other)
{
	_data = NULL;
	_length = _allocatedSize = 0;

	if( other._data != NULL && other._length > 0 )
	{
		allocate( other._length );

		memcpy( _data, other._data, other._length * sizeof(StringChar) );
	}
}

UnicodeString::UnicodeString(const char *utf8Str)
{
	_data = NULL;
	_length = _allocatedSize = 0;

	if( utf8Str )
	{
		UnicodeConversion conv;

		if( conv.utf8ToUcs( utf8Str ) == UnicodeConversion::Success )
		{
			*this = conv.getUcsString();
		}
	}
}

bool UnicodeString::allocate(const unsigned int nsize)
{
	bool retVal = true;

	if( nsize >= _allocatedSize )
	{
		unsigned int blockSize = minAllocationSize * ( (nsize + 1) / minAllocationSize + 1 );

		if( _data )
		{
			_data = (StringChar *)realloc( _data, blockSize * sizeof(StringChar) );
		}
		else
		{
			_data = (StringChar *)malloc( blockSize * sizeof(StringChar) );
		}

		if( _data )
		{
			memset( _data, 0, blockSize * sizeof(StringChar) );
			_allocatedSize = blockSize;
		}
		else
		{
			_length = _allocatedSize = 0;
			retVal = false;
		}
	}
	else
	{
		memset( _data, 0, _allocatedSize * sizeof(StringChar) );
	}

	_length = nsize;

	return retVal;
}

void UnicodeString::deallocate(void)
{
	if( _data ) free( _data );
	_data = NULL;
	_length = _allocatedSize = 0;
}

unsigned int UnicodeString::indexCheck(int index) const
{
	unsigned int retVal = 0;

	if( index > 0 )
	{
		retVal = (unsigned int)index;

		if( retVal > _length )
			retVal = _length;
	}

	return retVal;
}

int UnicodeString::insert(const StringChar * ins, int index)
{
	if( ins )
	{
		size_t nsize = ucslen( ins );

		if( _data == NULL )
		{
			allocate( nsize );
			memcpy( _data, ins, nsize * sizeof(StringChar) );
		}
		else if( nsize > 0 )
		{
			unsigned int pos = indexCheck(index);
			unsigned int newSize = _length + nsize + 1;
			unsigned int i;

			if( _allocatedSize >= newSize )
			{
				for(i=0; i<_length - pos + 1; i++)
				{
					_data[_length + nsize - i] = _data[_length - i];
				}

				for(i=0; i<nsize; i++)
				{
					_data[pos + i] = ins[i];
				}
			}
			else
			{
				unsigned int blockSize = minAllocationSize * ( newSize / minAllocationSize + 1 );

				StringChar * holder = (StringChar*)malloc( blockSize * sizeof(StringChar) );

				_allocatedSize = blockSize;

				if ( pos > 0 )
				{
					// put first piece
					ucsncpy(holder, _data, pos);
					holder[pos] = 0;
					// append other one
					ucsncat(holder, ins, nsize);
				}
				else
					ucscpy( holder, ins );

				// append remainder
				ucscat(holder, &_data[pos]);

				free(_data);
				_data = holder;
			}

			_length += nsize;
		}
	}

	return (int)_length;
}

int UnicodeString::insert(const UnicodeString& ucsStr, int index=0)
{
	size_t nsize = ucsStr._length;

	if( _data == NULL )
	{
		*this = ucsStr;
	}
	else if( nsize > 0 )
	{
		unsigned int pos = indexCheck(index);
		unsigned int newSize = _length + nsize + 1;
		unsigned int i;

		if( _allocatedSize >= newSize )
		{
			for(i=0; i<_length - pos + 1; i++)
			{
				_data[_length + nsize - i] = _data[_length - i];
			}

			for(i=0; i<nsize; i++)
			{
				_data[pos + i] = ucsStr._data[i];
			}
		}
		else
		{
			unsigned int blockSize = minAllocationSize * ( newSize / minAllocationSize + 1 );

			StringChar * holder = (StringChar*)malloc( blockSize * sizeof(StringChar) );

			_allocatedSize = blockSize;

			if ( pos > 0 )
			{
				// put first piece
				ucsncpy(holder, _data, pos);
				holder[pos] = 0;
				// append other one
				ucsncat(holder, ucsStr._data, nsize);
			}
			else
				ucscpy( holder, ucsStr._data );

			// append remainder
			ucscat(holder, &_data[pos]);

			free(_data);
			_data = holder;
		}

		_length += nsize;
	}

	return _length;
}

UnicodeString& UnicodeString::operator=(const StringChar a)
{
	deallocate();

	allocate(1);

	if( a == 0 )
		_length = 0;

	_data[0]=a;
	_data[1]=0;

	return *this;
}

UnicodeString& UnicodeString::operator=(const StringChar * ucsStr)
{
	if( _data != ucsStr )
	{
		deallocate();

		if( ucsStr )
		{
			allocate( ucslen(ucsStr) );
			memcpy( _data, ucsStr, _length * sizeof(StringChar) );
		}
	}

	return *this;
}

UnicodeString& UnicodeString::operator=(const char* utf8Str)
{
	UnicodeConversion conv;
	if( conv.utf8ToUcs(utf8Str) == UnicodeConversion::Success )
	{
		*this = conv.getUcsString();
	}

	return *this;
}

UnicodeString& UnicodeString::operator=(const UnicodeString &second)
{
	if( this != &second )
	{
		*this = second._data;
	}

	return *this;
}

UnicodeString& UnicodeString::operator+=(const UnicodeString &second)
{
	insert( second._data, _length );
	return *this;
}
UnicodeString& UnicodeString::operator+=(const StringChar * str)
{
	insert( str, _length );
	return *this;
}

UnicodeString& UnicodeString::operator+=(const char* utf8Str)
{
	UnicodeConversion conv;
	if( conv.utf8ToUcs(utf8Str) == UnicodeConversion::Success )
	{
		*this += conv.getUcsString();
	}

	return *this;
}

UnicodeString UnicodeString::operator+(const UnicodeString &second)
{
	UnicodeString temp = *this;
	if( second._length > 0 )
		temp += second;
	return temp;
}

UnicodeString UnicodeString::operator+(const UnicodeString &second) const
{
	UnicodeString temp = *this;
	if( second._length > 0 )
		temp += second;
	return temp;
}

UnicodeString UnicodeString::operator+(const StringChar * str)
{
	UnicodeString temp = *this;

	if( str && ucslen(str) > 0 )
		temp += str;

	return temp;
}

UnicodeString UnicodeString::operator+(const StringChar * str) const
{
	UnicodeString temp = *this;

	if( str && ucslen(str) > 0 )
		temp += str;

	return temp;
}

bool UnicodeString::operator==(const StringChar * other) const
{
	bool retVal = false;

	if( _data && other )
	{
		retVal = ucscmp( _data, other ) == 0;
	}
	else if( other == NULL && _length == 0 )
	{
		retVal = true;
	}

	return retVal;
}

bool UnicodeString::operator==(const UnicodeString& other) const
{
	return operator==(other._data);
}

bool UnicodeString::operator!=(const UnicodeString& other) const
{
	return !operator==(other._data);
}

bool UnicodeString::operator<( const UnicodeString& other ) const
{
	bool retVal = false;

	if( other._data && _data )
	{
		retVal = ucscmp( _data, other._data ) < 0;
	}

	return retVal;
}

bool UnicodeString::operator>(const UnicodeString& other) const
{
	bool retVal = false;

	if( other._data && _data )
	{
		retVal = ucscmp( _data, other._data ) > 0;
	}

	return retVal;
}

bool UnicodeString::operator<=(const UnicodeString& other) const
{
	bool retVal = false;

	if( other._data && _data )
	{
		retVal = ucscmp( _data, other._data ) <= 0;
	}
	else if( other._length == 0 && _length == 0 )
	{
		retVal = true;
	}

	return retVal;
}

bool UnicodeString::operator>=(const UnicodeString& other) const
{
	bool retVal = false;

	if( other._data && _data )
	{
		retVal = ucscmp( _data, other._data ) >= 0;
	}
	else if( other._length == 0 && _length == 0 )
	{
		retVal = true;
	}

	return retVal;
}

StringChar& UnicodeString::operator[](int i)
{
	if( _data == NULL )
	{
		allocate(1);
		_length = 0;
	}

	unsigned int index = indexCheck(i);

	if( index == _length && _length > 0 )
		index = _length - 1;

	if( _length == 0 )
		_data[1] = 0;

	return _data[index];
}

const StringChar UnicodeString::operator[](int i) const
{
	StringChar retVal = 0;

	if( _data )
	{
		unsigned int index = i;
		// Allow the NULL terminator to be read.
		if( index > _length )
			index = _length;
		retVal = _data[index];
	}

	return retVal;
}

int UnicodeString::find(const StringChar *ucsStr, int index) const
{
	int retVal = -1;

	if( _data && ucsStr )
	{
		if( index >= 0 && index < _length )
		{
			StringChar *ptr = ucsstr( &_data[index], ucsStr );

			if( ptr )
			{
				retVal = (int)(ptr - _data);
			}
		}
	}

	return retVal;
}

int UnicodeString::find(const UnicodeString& ucsStr, int index) const
{
	return find(ucsStr._data, index);
}

int UnicodeString::find(const StringChar a, int index) const
{
	int retVal = -1;

	if( _data )
	{
		if( index >= 0 && index < _length )
		{
			for( int i=index; i<_length; i++ )
			{
				if( a == _data[i] )
				{
					retVal = i;
					break;
				}
			}
		}
	}

	return retVal;
}

int UnicodeString::reverseFind(const StringChar a, int index) const
{
	int retVal = -1;

	if( _data )
	{
		int nIndex = index;

		if( nIndex == -1 )
		{
			nIndex = (int)_length - 1;
		}

		if( nIndex >= 0 && nIndex < _length )
		{
			for( int i=nIndex; i>=0; i-- )
			{
				if( a == _data[i] )
				{
					retVal = i;
					break;
				}
			}
		}
	}

	return retVal;
}

int UnicodeString::reverseFind(const StringChar *ucsStr, int index) const
{
	int retVal = -1;

	if( _data && ucsStr )
	{
		int len = (int)_length;
		int nIndex = index;
		int findStrLen = ucslen( ucsStr );

		if( nIndex == -1 )
		{
			nIndex = len - findStrLen;
		}

		if( nIndex > len - findStrLen )
		{
			nIndex = len - findStrLen;
		}

		if( nIndex >= 0 && nIndex < len )
		{
			for(int i=nIndex; i>=0; i--)
			{
				if( ucscmp( &_data[i], ucsStr ) == 0 )
				{
					retVal = i;
					break;
				}
			}
		}
	}

	return retVal;
}

int UnicodeString::reverseFind(const UnicodeString& ucsStr, int index) const
{
	int retVal = -1;

	if( _data && ucsStr._data && ucsStr._length > 0 )
	{
		int len = (int)_length;
		int nIndex = index;
		int findStrLen = (int)ucsStr._length;

		if( nIndex == -1 )
		{
			nIndex = len - findStrLen;
		}

		if( nIndex > len - findStrLen )
		{
			nIndex = len - findStrLen;
		}

		if( nIndex >= 0 && nIndex < len )
		{
			for(int i=nIndex; i>=0; i--)
			{
				if( ucsStr == &_data[i] )
				{
					retVal = i;
					break;
				}
			}
		}
	}

	return retVal;
}

int UnicodeString::deleteChar(int index, int count)
{
	int retVal = -1;

	if( _data )
	{
		int len = (int)_length;
		int nCount = count;

		if( nCount == -1 )
		{
			nCount = len - index;
		}

		if( index + nCount > len )
			nCount = len - index;

		if( index >= 0 && index < len && nCount > 0 )
		{
			for(int i=index; i<len; i++)
			{
				if( i + nCount >= len )
				{
					_data[i] = 0;
				}
				else
				{
					_data[i] = _data[i+nCount];
					_data[i+nCount] = 0;
				}
			}

			retVal = nCount;
			_length -= (unsigned int)nCount;
		}
	}

	return retVal;
}

int UnicodeString::removeString(const StringChar *ucsStr, int index)
{
	int retVal = find( ucsStr, index );

	if( retVal >= 0 )
	{
		size_t len = ucslen( ucsStr );

		deleteChar( retVal, len );
	}

	return retVal;
}

int UnicodeString::removeString(const UnicodeString& ucsStr, int index)
{
	int retVal = find( ucsStr, index );

	if( retVal >= 0 )
	{
		deleteChar( retVal, ucsStr._length );
	}

	return retVal;
}

UnicodeString UnicodeString::left(int count) const
{
	UnicodeString retVal;

	if( _length > 0 && count > 0 && _data )
	{
		if( (unsigned int)count >= _length )
		{
			retVal = *this;
		}
		else
		{
			retVal.allocate( count );
			ucsncpy( retVal._data, _data, count );
		}
	}

	return retVal;
}

UnicodeString UnicodeString::right(int count) const
{
	UnicodeString retVal;

	if( _length > 0 && count > 0 && _data )
	{
		if( (unsigned int)count >= _length )
		{
			retVal = *this;
		}
		else
		{
			retVal.allocate( count );
			ucscpy( retVal._data, &_data[_length - count] );
		}
	}

	return retVal;
}

UnicodeString UnicodeString::mid(int index, int count) const
{
	UnicodeString retVal;

	if( _length > 0 && _data )
	{
		if( index >= 0 && index < _length )
		{
			int nCount = count;

			if( nCount < 0 || index + nCount > _length )
			{
				nCount = (int)_length - index;
			}

			retVal.allocate( nCount );
			ucsncpy( retVal._data, &_data[index], nCount );
		}
	}

	return retVal;
}

UnicodeString UnicodeString::rightPos(int index) const
{
	UnicodeString retVal;

	if( _length > 0 && _data )
	{
		if( index >= 0 && index < _length )
		{
			retVal.allocate( _length - index );
			ucscpy( retVal._data, &_data[index] );
		}
	}

	return retVal;
}

void UnicodeString::trimLeadingChars(const StringChar *trim)
{
	if( _data && trim && _length > 0 )
	{
		int count = 0;

		for(unsigned int i=0; i<_length; i++)
		{
			if( ucschr( trim, _data[i] ) != NULL )
			{
				count++;
			}
			else
			{
				break;
			}
		}

		if( count > 0 )
			deleteChar( 0, count );
	}
}

void UnicodeString::trimTrailingChars(const StringChar *trim)
{
	if( _data && trim && _length > 0 )
	{
		int count = 0;
		int index = _length - 1;

		for(int i=index; i>=0; i--)
		{
			if( ucschr( trim, _data[i] ) != NULL )
			{
				index = i;
				count++;
			}
			else
			{
				break;
			}
		}

		if( count > 0 )
			deleteChar( index, count );
	}
}

void UnicodeString::trimLeadingTrailingChars(const StringChar *trim)
{
	trimLeadingChars(trim);
	trimTrailingChars(trim);
}

const StringChar * UnicodeString::getString(void) const
{
	if( _data == NULL )
	{
		_data = (StringChar*)malloc( minAllocationSize * sizeof(StringChar) );
		_allocatedSize = minAllocationSize;
		_length = 0;
		memset( _data, 0, minAllocationSize * sizeof(StringChar) );
	}

	return _data;
}

string UnicodeString::getUTF8(void) const
{
	string retVal;

	if( _data && _length > 0 )
	{
		UnicodeConversion conv;

		if( conv.ucsToUtf8( *this ) == UnicodeConversion::Success )
		{
			retVal = conv.getUtf8String();
		}
	}

	return retVal;
}

void UnicodeString::empty(void)
{
	deallocate();
}

bool UnicodeString::getAt(int index, StringChar& ucsChar) const
{
	bool retVal = false;

	if( index >= 0 && index < _length )
	{
		ucsChar = _data[index];
		retVal = true;
	}

	return retVal;
}

bool UnicodeString::setAt(int index, StringChar ucsChar)
{
	bool retVal = false;

	if( index >= 0 && index < _length )
	{
		_data[index] = ucsChar;
		retVal = true;
	}

	return retVal;
}

int UnicodeString::replace( StringChar chOld, StringChar chNew )
{
	int nCount = 0;

	if( _data && _length > 0 )
	{
		for(unsigned int i=0; i<_length; i++)
		{
			if( _data[i] == chOld )
			{
				_data[i] = chNew;
				nCount++;
			}
		}

		_length = ucslen( _data );
	}

	return nCount;
}

int UnicodeString::replace(const StringChar * oldStr, const StringChar * newStr)
{
	int nCount = 0;
	int posF = 0, pos = 0;

	if( _data && oldStr && newStr )
	{
		posF = find(oldStr, pos);

		if( posF >= 0 )
		{
			int oldLen = (int)ucslen(oldStr);
			int newLen = (int)ucslen(newStr);

			while( posF >= 0 )
			{
				deleteChar( posF, oldLen );
				insert( newStr, posF );

				pos = posF + newLen;

				nCount++;

				posF = find(oldStr, pos);
			}
		}
	}

	return nCount;
}

int UnicodeString::replace(const UnicodeString& oldStr, const UnicodeString& newStr)
{
	int nCount = 0;
	int posF = 0, pos = 0;

	if( _data )
	{
		posF = find(oldStr, pos);

		if( posF >= 0 )
		{
			int oldLen = (int)oldStr.getLength();
			int newLen = (int)newStr.getLength();

			while( posF >= 0 )
			{
				deleteChar( posF, oldLen );
				insert( newStr, posF );

				pos = posF + newLen;

				nCount++;

				posF = find(oldStr, pos);
			}
		}
	}

	return nCount;
}
