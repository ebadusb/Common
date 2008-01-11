// format_string_info.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "string_info.h"
#include "unicode_string.h"
#include "unicode_conversion.h"

UnicodeString insertBackslashChars(UnicodeString str)
{
	UnicodeString result;

	for ( unsigned int i=0; i<str.getLength(); i++ )
	{
		switch ( str[i] )
		{
		case (StringChar)'\b':
			result += "\\b";
			break;

		case (StringChar)'\n':
			result += "\\n";
			break;

		case (StringChar)'\r':
			result += "\\r";
			break;

		case (StringChar)'\t':
			result += "\\t";
			break;

		case (StringChar)'"':
			result += "\\\"";
			break;

		default:
			result += str.mid(i, 1);
			break;
		}
	}

	return result;
}

unsigned int checkForListStart(const UnicodeString & wholeString, unsigned int startIdx, unsigned int stringEndIdx, LineInfo & lineInfo, unsigned int attributes)
{
	unsigned int idx = startIdx;

	while ( ( wholeString[idx] == (StringChar)' ' ||
				 wholeString[idx] == (StringChar)'\t' ) &&
			  idx < stringEndIdx )
	{
		idx += 1;
	}

	if ( idx < stringEndIdx - 4 &&
		  (attributes & 0x30) == 0x00 )
	{
		if ( wholeString[idx] >= (StringChar)'1' &&
			  wholeString[idx] <= (StringChar)'9' &&
			  wholeString[idx+1] == (StringChar)'.' )
		{
			lineInfo._type = LineInfo::LT_Numeric;
			lineInfo._itemNumber = wholeString[idx]-(StringChar)'0';
			idx += 2;
		}
		else if ( wholeString[idx] >= (StringChar)'a' &&
					 wholeString[idx] <= (StringChar)'z' &&
					 wholeString[idx+1] == (StringChar)'.' )
		{
			lineInfo._type = LineInfo::LT_LowerCase;
			lineInfo._itemNumber = wholeString[idx]-(StringChar)'a'+1;
			idx += 2;
		}
		else if ( wholeString[idx] >= (StringChar)'A' &&
					 wholeString[idx] <= (StringChar)'Z' &&
					 wholeString[idx+1] == (StringChar)'.' )
		{
			lineInfo._type = LineInfo::LT_LowerCase;
			lineInfo._itemNumber = wholeString[idx]-(StringChar)'A'+1;
			idx += 2;
		}
		else if ( wholeString[idx] == (StringChar)0xb7 )
		{
			lineInfo._type = LineInfo::LT_Bullet;
			lineInfo._itemNumber = 0;
			idx += 1;
		}
		else if ( ( wholeString[idx] == (StringChar)'-' ||
					   wholeString[idx] == (StringChar)0x2012 ||
					   wholeString[idx] == (StringChar)0x2013 ) &&
                wholeString[idx+1] == (StringChar)' ' )
		{
			lineInfo._type = LineInfo::LT_Dash;
			lineInfo._itemNumber = 0;
			idx += 2;
		}
		else
		{
			lineInfo._type = LineInfo::LT_None;
		} 
	}
	else
	{
		lineInfo._type = LineInfo::LT_None;
	}

	return idx;
}

unsigned int parseLine(UnicodeString & wholeString, unsigned int startIdx, unsigned int & stringEndIdx, std::vector<LineInfo> & lineInfo, int lineCount, unsigned int attributes)
{
	unsigned int idx = startIdx;

	lineInfo[lineCount]._startIdx = idx;
	idx = checkForListStart(wholeString, idx, stringEndIdx, lineInfo[lineCount], attributes);

	if ( lineInfo[lineCount]._type != LineInfo::LT_None )
	{
		if ( lineCount == 0 ||
			  lineInfo[lineCount-1]._type == LineInfo::LT_None )
		{
			lineInfo[lineCount]._indent = 0;
		}
		else if ( lineInfo[lineCount-1]._type == lineInfo[lineCount]._type )
		{
			lineInfo[lineCount]._indent = lineInfo[lineCount-1]._indent;
		}
		else
		{
			bool	indentAssigned = false;
			for ( int i=lineCount-2; i>=0; i-- )
			{
				if ( lineInfo[i]._type == lineInfo[lineCount]._type &&
					  lineInfo[i]._indent < lineInfo[lineCount-1]._indent )
				{
					lineInfo[lineCount]._indent = lineInfo[i]._indent;
					indentAssigned = true;
					break;
				}
			}

			if ( !indentAssigned )
			{
				lineInfo[lineCount]._indent = lineInfo[lineCount-1]._indent + 1;
			}
		}

		lineInfo[lineCount]._startIndentIdx = idx;
		while ( ( wholeString[idx] == (StringChar)' ' ||
					 wholeString[idx] == (StringChar)'\t' ) &&
				  idx < stringEndIdx )
		{
			idx += 1;
		}

		lineInfo[lineCount]._endIndentIdx = idx;
		lineInfo[lineCount]._hasNewLine = false;

		bool	done = false;
		while ( !done &&
				  idx < stringEndIdx )
		{
			if ( wholeString[idx] == (StringChar)'\n' )
			{
				lineInfo[lineCount]._hasNewLine = true;

				idx += 1;
				while ( wholeString[idx] == (StringChar)'\n' &&
						  idx < stringEndIdx )
				{
					idx += 1;
				}

				LineInfo	nextLineInfo;
				checkForListStart(wholeString, idx, stringEndIdx, nextLineInfo, attributes);

				if ( nextLineInfo._type != LineInfo::LT_None ||
					  wholeString[idx] != (StringChar)' ' )
				{
					done = true;
				}
			}
			else
			{
				idx += 1;
			}
		}

		lineInfo[lineCount]._endIdx = idx;
	}
	else
	{
		lineInfo[lineCount]._indent = 0;
		lineInfo[lineCount]._startIndentIdx = 0;
		lineInfo[lineCount]._endIndentIdx = 0;
		lineInfo[lineCount]._hasNewLine = false;

		bool	done = false;
		while ( !done &&
				  idx < stringEndIdx )
		{
			if ( wholeString[idx] == (StringChar)'\n' )
			{
				done = true;
			}

			idx += 1;
		}

		lineInfo[lineCount]._endIdx = idx;
	}

	return idx;
}

void convertString(UnicodeString & wholeString)
{
	char  name[256];
	unsigned int	idx;

	name[0] = (char)wholeString[0];
	for ( idx=1; idx<wholeString.getLength() && idx<256; idx++ )
	{
		if ( wholeString[idx] != (StringChar)' ' &&
			  wholeString[idx] != (StringChar)'\t' ) 
		{
			name[idx] = (char)wholeString[idx];
		}
		else
		{
			name[idx] = '\0';
			break;
		}
	}

	while ( ( wholeString[idx] == (StringChar)' ' ||
				 wholeString[idx] == (StringChar)'\t' ) &&
			  idx < wholeString.getLength() )
	{
		idx += 1;
	}

	if ( wholeString[idx] != (StringChar)'"' )
	{
		fprintf(stderr, "No opening quote for string %s\n", name);
		exit(1);
	}

	idx += 1;
	unsigned int	stringStartIdx = idx;
	unsigned int	stringEndIdx;
 
	bool  done = false;
	while ( !done &&
			  idx < wholeString.getLength() )
	{
		if ( wholeString[idx] == (StringChar)'"' )
		{
			stringEndIdx = idx;
			done = true;
		}
		else if ( wholeString[idx] == (StringChar)'\\' )
		{
			wholeString.deleteChar(idx, 1);
			switch ( wholeString[idx] )
			{
			case (StringChar)'b':
				wholeString[idx] = (StringChar)'\b';
				break;

			case 'n':
				wholeString[idx] = '\n';
				break;

			case 'r':
				wholeString[idx] = '\r';
				break;

			case 't':
				wholeString[idx] = '\t';
				break;

			case '"':
				wholeString[idx] = '"';
				break;

			case 'x':
				char hexString[5];
				unsigned int  l;

				if ( idx + 5 >= wholeString.getLength() )
				{
					fprintf(stderr, "incomplete \\x sequence for string %s\n", name);
					exit(1);
				}

				hexString[0] = (char)wholeString[idx+1];
				hexString[1] = (char)wholeString[idx+2];
				hexString[2] = (char)wholeString[idx+3];
				hexString[3] = (char)wholeString[idx+4];
				hexString[4] = '\0';

				wholeString.deleteChar(idx+1, 4);
				sscanf(hexString, "%x", &l);
				wholeString[idx] = (StringChar)l;
				break;

			default:
				break;
			}
		}

		idx += 1;
	}

	if ( !done )
	{
		fprintf(stderr, "No closing quote for string %s\n", name);
		exit(1);
	}

	UnicodeString	args(wholeString.mid(stringEndIdx+1, wholeString.getLength()-stringEndIdx-1));
	UnicodeConversion argsConv;
	argsConv.ucsToUtf8(args);

	unsigned int	red, green, blue, attributes;
	if ( sscanf(argsConv.getUtf8String(), "%u %u %u %x", &red, &green, &blue, &attributes) != 4 )
	{
		fprintf(stderr, "No attributes for string %s\n", name);
		exit(1);
	}

	int	listItemCount = 0;
	int	lineCount = 0;
	std::vector<LineInfo> lineInfo;

	idx = stringStartIdx;
	while ( idx < stringEndIdx )
	{
		lineInfo.resize(lineCount+1);

		idx = parseLine(wholeString, idx, stringEndIdx, lineInfo, lineCount, attributes);
		printf("%s: lineCount=%d, type=%d lineIdx=%u %u indentIdx=%u %u indent=%u item=%u\n", name, lineCount,
			(int)lineInfo[lineCount]._type, lineInfo[lineCount]._startIdx, lineInfo[lineCount]._endIdx,
			lineInfo[lineCount]._startIndentIdx, lineInfo[lineCount]._endIndentIdx, lineInfo[lineCount]._indent, lineInfo[lineCount]._itemNumber);

		if ( lineInfo[lineCount]._type != LineInfo::LT_None )
		{
			listItemCount += 1;
		}

		lineCount += 1;
	}

	if ( lineCount == 1 &&
		 lineInfo[0]._type == LineInfo::LT_Dash )
	{
		// This avoids treating leading minus signs as a list element
		//
		lineInfo[0]._type = LineInfo::LT_None;
	}

	UnicodeString	outputString(wholeString.left(lineInfo[0]._startIdx));
	idx = lineInfo[0]._startIdx;
	int	currentIndent = -1;

	for ( int line=0; line<lineCount; line++ )
	{
		if ( lineInfo[line]._type != LineInfo::LT_None &&
			  currentIndent != lineInfo[line]._indent )
		{
			currentIndent = lineInfo[line]._indent;

			char	formatCmd[32];
			sprintf(formatCmd, "#![PG %d %d]", currentIndent, currentIndent+1);
			outputString += formatCmd;
		}
		else if ( lineInfo[line]._type == LineInfo::LT_None &&
					 currentIndent != -1 )
		{
			currentIndent = -1;
			outputString += "#![PG 0 0]";
		}

		if ( lineInfo[line]._type != LineInfo::LT_None )
		{
			idx = lineInfo[line]._startIdx;
			while ( idx < lineInfo[line]._endIndentIdx )
			{
				if ( wholeString[idx] != ' ' &&
					  wholeString[idx] != '\t' )
			   {
					outputString += insertBackslashChars(wholeString.mid(idx, 1));
				}

				idx += 1;
			}

			outputString += "\\t";
		}

		bool	skippingSpaceAfterNewLine = false;
		while (idx < lineInfo[line]._endIdx )
		{
			if ( wholeString[idx] == (StringChar)'\n' &&
				  idx != lineInfo[line]._endIdx - 1 )
			{
				if ( wholeString[idx+1] != (StringChar)'\n' &&
				     !skippingSpaceAfterNewLine )
				{
					outputString += " ";
					skippingSpaceAfterNewLine = true;
				}
			}
			else if ( wholeString[idx] != (StringChar)' ' &&
						 wholeString[idx] != (StringChar)'\t' )
			{
				skippingSpaceAfterNewLine = false;
			}

			if ( !skippingSpaceAfterNewLine ||
				  ( wholeString[idx] != (StringChar)' ' &&
					 wholeString[idx] != (StringChar)'\t' &&
					 wholeString[idx] != '\n' ))
			{
				outputString += insertBackslashChars(wholeString.mid(idx, 1));
			}

			idx += 1;
		}
	}

	outputString += wholeString.mid(idx, wholeString.getLength()-idx);
	wholeString = outputString;
}

int _tmain(int argc, _TCHAR* argv[])
{
	if ( argc != 3 )
	{
		fprintf(stderr, "must specify source and destination file names\n");
		return 1;
	}
 
	FILE  * fpIn = fopen(argv[1], "rb");
	if ( !fpIn )
	{
		perror((const char *)argv[1]);
		return 1;
	}

	FILE * fpOut = fopen(argv[2], "wb");
	if ( !fpOut )
	{
		perror((const char *)argv[1]);
		fclose(fpIn);
		return 1;
	}

	static char data[16000];
	static char stringDataStarted = false;

	while ( fgets(data, 16000, fpIn) )
	{
		UnicodeConversion conv;

		if ( conv.utf8ToUcs(data) != UnicodeConversion::Success )
		{
			fprintf(stderr, "Conversion to UCS failed for string: %s\n", data);
			break;
		}

		UnicodeString  wholeString(conv.getUcsString());

		static StringChar trimChars[] = { (StringChar)' ', (StringChar)'\t', (StringChar)0 };
		wholeString.trimLeadingTrailingChars(trimChars);
		wholeString.replace(UnicodeString("\r"), UnicodeString(""));

		if ( wholeString[0] == (StringChar)'#' ||
			  wholeString[0] == (StringChar)'\n' )
		{
			// Blank string or comment
			//
			fprintf(fpOut, "%s", data);
		}
		else if ( wholeString == UnicodeString("STRING_FILE_TABLE_START\n") )
		{
			// Start of string data
			//
			stringDataStarted = true;
			fprintf(fpOut, "%s", data);
		}
		else if ( wholeString == UnicodeString("STRING_FILE_TABLE_END\n") ||
					 wholeString == UnicodeString("STRING_FILE_TABLE_END") )
		{
			// End of string data
			//
			stringDataStarted = false;
			fprintf(fpOut, "%s", data);
		}
		else if ( stringDataStarted )
		{
			convertString(wholeString);
			if ( conv.ucsToUtf8(wholeString) != UnicodeConversion::Success )
			{
				fprintf(stderr, "Conversion to UTF8 failed for string: %s\n", data);
				break;
			}
   
			fprintf(fpOut, "%s", conv.getUtf8String());
		}
		else
		{
			// Unrecognized line - just copy to output
			//
			fprintf(fpOut, "%s", data);
		}
	}

	fclose(fpIn);
	fclose(fpOut);
	return 0;
}

