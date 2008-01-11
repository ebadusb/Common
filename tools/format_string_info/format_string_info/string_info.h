/*
 * $Header$
 *
 * $Log$
 *
 */

#ifndef _STRING_INFO_INCLUDE
#define _STRING_INFO_INCLUDE

#include <vector>
#include "unicode_string.h"

struct LineInfo
{
	enum ListType
	{
		LT_None,
		LT_Numeric,
		LT_LowerCase,
		LT_UpperCase,
		LT_Bullet,
		LT_Dash
	};

	ListType	_type;
	unsigned int	_itemNumber;
	unsigned int	_indent;

	unsigned int	_startIdx;
	unsigned int	_startIndentIdx;
	unsigned int	_endIndentIdx;
	unsigned int	_endIdx;
	unsigned int	_hasNewLine;

	LineInfo(void) :
		_type(LT_None), _itemNumber(0), _indent(0), _startIdx(0), _startIndentIdx(0), _endIndentIdx(0), _endIdx(0), _hasNewLine(false) { }
};

#endif /* ifndef _STRING_INFO_INCLUDE */
