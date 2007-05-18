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

#ifndef _UCS_STRING_INCLUDE_
#define _UCS_STRING_INCLUDE_

#include "unicode_string/unicode_string_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Wide character strcat - equivalent to ANSI wcscat */
StringChar * ucscat(StringChar * destination, const StringChar * append);

/* Wide character strchr - equivalent to ANSI wcschr */
StringChar * ucschr(const StringChar * s, int c);

/* Wide character strcmp - equivalent to ANSI wcscmp */
int ucscmp(const StringChar * s1, const StringChar * s2);

/* Wide character strcpy - equivalent to ANSI wcscpy */
StringChar * ucscpy(StringChar * s1, const StringChar * s2);

/* Wide character strcspn - equivalent to ANSI wcsspn */
size_t ucscspn(const StringChar * s1, const StringChar * s2);

/* Wide character strlen - equivalent to ANSI wcslen */
size_t ucslen(const StringChar * s);

/* Wide character strncat - equivalent to ANSI wcscat */
StringChar * ucsncat(StringChar * dst, const StringChar * src, size_t n);

/* Wide character strncmp - equivalent to ANSI wcsncmp */
int ucsncmp(const StringChar * s1, const StringChar * s2, size_t n);

/* Wide character strncpy - equivalent to ANSI wcsncpy */
StringChar * ucsncpy(StringChar * s1, const StringChar *s2, size_t n);

/* Wide character strstr - equivalent to ANSI wcsstr */
StringChar * ucsstr(const StringChar * s, const StringChar * find);

#ifdef __cplusplus
}
#endif

#endif /* ifndef _UCS_STRING_INCLUDE_ */
