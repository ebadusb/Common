/*
 *  Copyright(c) 2007 by Gambro BCT, Inc. All rights reserved.
 *
 * $Header$
 *
 *	Common wide character string functions. These functions have
 * the same functionality as the ANSI functions except take StringChar
 * type strings for UGL compatibility.
 *
 * $Log$
 *
 */

#include "ucs_string.h"

StringChar * ucscat(StringChar * destination, const StringChar * append)
{
	StringChar *save = destination;

	while ( *destination++ != 0 )		 /* find end of string */
		;

	destination--;

	while ( (*destination++ = *append++) != 0 )
		;

	return(save);
}

StringChar * ucschr(const StringChar * s, int c)
{
	const StringChar *r = s;

	while ( *r != (StringChar) c )	 /* search loop */
	{
		if ( *r++ == 0 )	  /* end of string */
			return(NULL);
	}

	return((StringChar*)r);
}

int ucscmp(const StringChar * s1, const StringChar * s2)
{
	while ( *s1++ == *s2++ )
		if ( s1 [-1] == 0 )
			return(0);

	return((s1 [-1]) - (s2 [-1]));
}

StringChar * ucscpy(StringChar * s1, const StringChar * s2)
{
	StringChar *save = s1;

	while ( (*s1++ = *s2++) != 0 )
		;

	return(save);
}

size_t ucscspn(const StringChar * s1, const StringChar * s2)
{
	const StringChar *save;
	const StringChar *p;
	StringChar    c1;
	StringChar    c2;

	for ( save = s1 + 1; (c1 = *s1++) != 0; )	/* search for EOS */
		for ( p = s2; (c2 = *p++) != 0; )	 /* search for first occurance */
		{
			if ( c1 == c2 )
				return(s1 - save);			  /* return index of substring */
		}

	return(s1 - save);
}


size_t ucslen(const StringChar * s)
{
	const StringChar *save = s + 1;

	while ( *s++ != 0 )
		;

	return(s - save);
}

StringChar * ucsncat(StringChar * dst, const StringChar * src, size_t n)
{
	if ( n != 0 )
	{
		StringChar *d = dst;

		while ( *d++ != 0 )		  /* find end of string */
			;

		d--;					/* rewind back of EOS */

		while ( ((*d++ = *src++) != 0) && (--n > 0) )
			;

		if ( n == 0 )
			*d = 0;			  /* NULL terminate string */
	}

	return(dst);
}

int ucsncmp(const StringChar * s1, const StringChar * s2, size_t n)
{
	if ( n == 0 )
		return(0);

	while ( *s1++ == *s2++ )
	{
		if ( (s1 [-1] == 0) || (--n == 0) )
			return(0);
	}

	return((s1 [-1]) - (s2 [-1]));
}

StringChar * ucsncpy(StringChar * s1, const StringChar *s2, size_t n)
{
	register StringChar *d = s1;

	if ( n != 0 )
	{
		while ( (*d++ = *s2++) != 0 )	  /* copy <s2>, checking size <n> */
		{
			if ( --n == 0 )
				return(s1);
		}

		while ( --n > 0 )
			*d++ = 0;			  /* NULL terminate string */
	}

	return(s1);
}

StringChar * ucsstr(const StringChar * s, const StringChar * find)
{
	const StringChar *t1;
	const StringChar *t2;
	StringChar c;
	StringChar c2;

	if ( (c = *find++) == 0 )	 /* <find> an empty string */
		return((StringChar*)(s));

	for (;;)
	{
		while ( ((c2 = *s++) != 0) && (c2 != c) )
			;

		if ( c2 == 0 )
			return(NULL);

		t1 = s;
		t2 = find;

		while ( ((c2 = *t2++) != 0) && (*t1++ == c2) )
			;

		if ( c2 == 0 )
			return((StringChar*)(s - 1));
	}
}

