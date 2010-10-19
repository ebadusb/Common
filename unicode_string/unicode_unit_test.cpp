/*
 *  Copyright(c) 2007 by Gambro BCT, Inc. All rights reserved.
 *
 * $Header$
 *
 * Unicode string library unit test functions
 *
 * $Log: unicode_unit_test.cpp $
 * Revision 1.1  2007/05/18 16:19:17Z  wms10235
 * Initial revision
 *
 */

#include <stdio.h>
#include "ucs_string.h"
#include "unicode_conversion.h"
#include "unicode_string.h"

static void dumpWideCharString(const StringChar *str);


void ucsFunctionsUT(void)
{
	const StringChar *testStr1 = (const StringChar*)L"ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
	const StringChar *testStr2 = (const StringChar*)L"abcdefghijklmnopqrstuvwxyz0123456789";
	const StringChar *nullStr = (const StringChar*)L"";

	do
	{
		size_t size = sizeof(StringChar);

		size_t len = ucslen( testStr1 );

		if( len != 36 )
		{
			printf("ucslen failed. Returned %d\n", len );
			break;
		}

		int cmp = ucscmp( testStr1, testStr2 );

		if( cmp == 0 )
		{
			printf("ucscmp failed.\n");
			break;
		}

		StringChar *testStr3 = (StringChar*)malloc( (len * 2 + 1) * sizeof(StringChar));
		memset( testStr3, 0x21, (len * 2 + 1) * sizeof(StringChar) );

		StringChar *strRet = ucscpy( testStr3, testStr1 );

		if( ucscmp( testStr1, testStr3 ) != 0 )
		{
			printf("ucscpy failed.\n");
			break;
		}

		strRet = ucscat(testStr3, testStr2);

		strRet = ucschr(testStr3, 0x39);

		cmp = ucscmp( testStr1, testStr3 );

		if( cmp == 0 )
		{
			printf("ucscpy failed.\n");
			break;
		}


		len = ucscspn(testStr3, (const StringChar*)L"0123");

		ucscpy( testStr3, nullStr );

		strRet = ucsncat( testStr3, testStr1, 4 );

		cmp = ucsncmp( testStr1, testStr3, 4 );

		strRet = ucsncpy(testStr3, testStr2, 8 );

		strRet = ucsstr(testStr1, (const StringChar*)L"012");

		int diff = (int)(strRet - testStr1);

	} while(false);
}

void unicodeStringUT(void)
{
	const StringChar *testStr1 = (const StringChar*)L"ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
	const StringChar *testStr2 = (const StringChar*)L"abcdefghijklmnopqrstuvwxyz0123456789";
	const StringChar *nullStr = (const StringChar*)L"";
	const StringChar *otherStr = (const StringChar*)L"@$%*";
	const char *utf8Str = "Hello world!";
	StringChar wideChar = 0;

	UnicodeString * pTestStr3 = NULL;

	UnicodeConversion convert;

	convert.utf8ToUcs(utf8Str);

	if( convert.getResult() == UnicodeConversion::Success )
	{
		printf("UCS conversion of 'Hello world!'\n");
		dumpWideCharString( convert.getUcsString() );
	}
	else
	{
		printf("Error decoding UTF8 string.\n");
	}

	UnicodeString uTestStr1 = testStr1;
	UnicodeString uTestStr2 = testStr2;

	printf("UTF8 conversion of testStr1\n");
	string utf8 = uTestStr1.getUTF8();

	printf("%s\n", utf8.c_str() );

	pTestStr3 = new UnicodeString;

	do
	{
		int find;
		printf("Testing Unicode string operator==():");
		if( uTestStr1 == uTestStr2 )
		{
			printf("failed\n");
			break;
		}

		if( uTestStr1 != (const StringChar*)L"ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789" )
		{
			printf("failed\n");
			break;
		}

		if( uTestStr1 == nullStr )
		{
			printf("failed\n");
			break;
		}
		else
		{
			printf("passed\n");
		}

		*pTestStr3 = uTestStr1 + uTestStr2;

		printf("Testing Unicode string operator+():");

		if( pTestStr3->getLength() != 72 )
		{
			printf("failed\n");
			break;
		}
		else
		{
			printf("passed\n");
		}

		uTestStr1 += utf8Str;
		printf("Testing Unicode string operator+=():");

		if( uTestStr1.getLength() != 48 )
		{
			printf("failed\n");
			break;
		}
		else
		{
			printf("passed\n");
		}

		find = uTestStr1.find( otherStr );

		printf("Testing Unicode string find():");

		if( find > 0 )
		{
			printf("failed\n");
			break;
		}

		find = uTestStr1.find( (const StringChar*)L"E" );

		uTestStr1.getAt( find, wideChar );

		if( wideChar != L'E' )
		{
			printf("failed\n");
			break;
		}
		else
		{
			printf("passed\n");
		}

		printf("Testing Unicode string inert():");
		uTestStr1.insert( otherStr );

		find = uTestStr1.find( otherStr );
		if( find != 0 )
		{
			printf("failed\n");
			break;
		}

		uTestStr1.insert( otherStr, 10 );
		find = uTestStr1.find( otherStr, 6 );
		if( find != 10 )
		{
			printf("failed\n");
			break;
		}

		unsigned int len = uTestStr1.getLength();

		uTestStr1.insert( uTestStr2, uTestStr1.getLength() );
		find = uTestStr1.find( uTestStr2 );
		if( find != len )
		{
			printf("failed\n");
			break;
		}
		else
		{
			printf("passed\n");
		}

		printf("Testing Unicode string removeString():");

		len = uTestStr1.getLength();

		uTestStr1.removeString( (const StringChar*)L"abcdefghijklmnopqrstuvwxyz" );

		if( ucslen( uTestStr1.getString() ) != len - 26 )
		{
			printf("failed\n");
			break;
		}
		else
		{
			printf("passed\n");
		}

		printf("Testing Unicode string left():");

		if( uTestStr2.left(10) != (const StringChar*)L"abcdefghij" )
		{
			printf("failed\n");
			break;
		}
		else
		{
			printf("passed\n");
		}

		printf("Testing Unicode string right():");

		if( uTestStr2.right(10) != (const StringChar*)L"0123456789" )
		{
			printf("failed\n");
			break;
		}
		else
		{
			printf("passed\n");
		}

		printf("Testing Unicode string mid():");

		if( uTestStr2.mid(10, 10) != (const StringChar*)L"klmnopqrst" )
		{
			printf("failed\n");
			break;
		}

		if( uTestStr2.mid(17, 30) != (const StringChar*)L"rstuvwxyz0123456789" )
		{
			printf("failed\n");
			break;
		}

		if( uTestStr2.mid(17) != (const StringChar*)L"rstuvwxyz0123456789" )
		{
			printf("failed\n");
			break;
		}
		else
		{
			printf("passed\n");
		}

		printf("Testing Unicode string replace():");

		if( uTestStr2.replace( (const StringChar*)L"0123456789", (const StringChar*)L"9876543210" ) != 1 )
		{
			printf("failed\n");
			break;
		}

		if( uTestStr2 != (const StringChar*)L"abcdefghijklmnopqrstuvwxyz9876543210" )
		{
			printf("failed\n");
			break;
		}
		else
		{
			printf("passed\n");
		}

	} while(false);

	if( pTestStr3 )
		delete pTestStr3;
}

static void dumpWideCharString(const StringChar *str)
{
	size_t len = ucslen(str);

	for(size_t i=0; i<len; i++)
	{
		printf("%04X ", str[i] );
	}

	printf("\n");
}
