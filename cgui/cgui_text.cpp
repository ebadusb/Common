/*
 * $Header: //bctquad3/home/BCT_Development/vxWorks/Common/cgui/rcs/cgui_text.cpp 1.45 2009/03/02 20:46:25Z adalusb Exp ms10234 $
 *
 * $Log: cgui_text.cpp $
 * Revision 1.35  2008/07/18 23:10:12Z  adalusb
 * Checked in changes for asian language text wrap. A new function getCharBasedToken() has been added.
 * Revision 1.34  2008/06/11 23:06:12Z  rm10919
 * Add handling of non-breaking spaces. IT 5377
 * Revision 1.33  2008/03/07 22:38:53Z  jl11312
 * - only update text on screen if it has changed (IT 3278)
 * Revision 1.32  2008/01/10 18:17:43Z  jl11312
 * - add support for embedded format commands
 * Revision 1.31  2007/06/04 22:04:20Z  wms10235
 * IT83 - Updates for the common GUI project to use the unicode string class
 * Revision 1.30  2007/04/14 18:05:19Z  jl11312
 * - handle deletion of objects referenced by an active screen (common IT 81)
 * Revision 1.29  2006/11/13 20:21:14Z  jd11007
 * IT 65 - Memory leak fixes.
 * Revision 1.28  2006/08/21 18:17:18Z  jl11312
 * - added code to trap displayed string IDs (SIMNT build only)
 * Revision 1.27  2006/07/12 23:36:07Z  rm10919
 * Updates from adding cguiListBox class.
 * Revision 1.26  2006/05/15 21:54:40Z  rm10919
 * Fix bug to handle latin char (accented vowels) as unsigned char to unsigned wide char correctly.
 * Revision 1.25  2005/11/22 00:34:42Z  rm10919
 * Get data item database to work with software layers.
 * Revision 1.24  2005/10/21 16:45:38Z  rm10919
 * Correct use of _stringSize and adding null string terminator.
 * Revision 1.23  2005/10/19 16:17:25Z  rm10919
 * Add Chris' textBlock addition to string size in variable sub. function.
 * Revision 1.22  2005/09/30 22:40:52Z  rm10919
 * Get the variable database working!
 * Revision 1.21  2005/08/21 19:59:48Z  cf10242
 * TAOS It 841 - fix getString to actually work
 * Revision 1.20  2005/08/05 22:55:14Z  cf10242
 * remove append text function
 * Revision 1.19  2005/08/01 23:31:38Z  cf10242
 * Revision 1.18  2005/06/20 14:49:02Z  rm10919
 * Fix bug in creating region from text item.
 * Revision 1.17  2005/06/06 18:21:57Z  rm10919
 * Changed constructor not to use the region the
 * styling record passed in unless the styling record exsists.
 * Revision 1.16  2005/05/16 22:49:26Z  cf10242
 * add appendText
 * Revision 1.15  2005/04/26 23:16:48Z  rm10919
 * Made changes to cgui_text and cgui_text_item, plus added
 * classes for variable substitution in text strings.
 * Revision 1.14  2005/03/18 16:42:07Z  rm10919
 * Fix getText method to actually do something.
 * Revision 1.13  2005/03/15 00:21:35Z  rm10919
 * Change CGUIText to not add object to window object list of parent in constructor.
 * Revision 1.12  2005/02/21 17:17:12Z  cf10242
 * IT 133 - delete all allocated memory to avoid
 * unrecovered memory
 * Revision 1.11  2005/02/07 18:53:55Z  rm10919
 * Fix screen size references to be generic.
 * Revision 1.10  2005/01/17 17:58:59Z  cf10242
 * Clean up some pointer references where the
 * pointer existence is not checked before reference
 * Revision 1.9  2005/01/03 23:49:50Z  cf10242
 * add a setRegion to cguiText to invalidate
 * region even if region size did not change
 * Revision 1.8  2005/01/03 20:40:52Z  cf10242
 * add defensive coding to catch gui crashes
 * Revision 1.7  2004/12/10 23:29:20Z  cf10242
 * Correct issues with length of text in UGL.
 * Revision 1.6  2004/11/30 16:43:21Z  rm10919
 * Assigning value for production compile.
 * Revision 1.5  2004/11/04 20:19:08Z  rm10919
 * Common updates and changes.
 * Revision 1.4  2004/11/01 17:27:23Z  cf10242
 * Change TextItem to CGUITextItem
 * Revision 1.3  2004/10/29 15:11:14Z  rm10919
 * Revision 1.2  2004/10/22 20:14:34Z  rm10919
 * CGUIButton updates and changes.
 * Revision 1.1  2004/09/20 18:18:08Z  rm10919
 * Initial revision
 *
 */

#include <vxWorks.h>

#include "cgui_text.h"
#include "cgui_window.h"
#include "datalog_levels.h"

#if CPU==SIMNT
# include <string.h>

bool captureTextID = false;
string captureTextIDStr;
const char * captureScreenName = NULL;

#endif /* if CPU==SIMNT */

CGUIVariableDatabaseContainer CGUIText::_variableDictionary;
unsigned short CGUIText::_tabSpaceCount = 5;

static StringChar newline_char = '\n';
static StringChar space_char = ' ';
static StringChar	tab_char = '\t';
static StringChar null_char = '\0';
static StringChar decimal_point = '.';
static StringChar decimal_comma = ',';
static UnicodeString	paragraph_format_start("#![PG");
static UnicodeString paragraph_format_end("]");
static const UnicodeString nonBreakingSpace(" ");  // unicode hex or utf8 format = 0xC2A0
static const UnicodeString regularLatinSpace(" "); // unicode hex = 0x0020
CGUIText::TokenSplitMethod CGUIText::_tokenSplitMethod = CGUIText::WordBased;

bool CGUIText::_forbiddenStartCharsAvailable = false;
bool CGUIText::_forbiddenEndCharsAvailable = false;
bool CGUIText::_forbiddenCharsInitialized=false;

UnicodeString CGUIText::_forbiddenStartCharList;
UnicodeString CGUIText::_forbiddenEndCharList;

int currentLanguage = 0;

CGUIText::CGUIText(CGUIDisplay & display)
	: CGUIWindowObject(display)
{
	initializeData(NULL, NULL);
}

CGUIText::CGUIText(CGUIDisplay & display, CGUITextItem * textItem, StylingRecord * stylingRecord = NULL)
   : CGUIWindowObject(display)
{
	initializeData(textItem, stylingRecord);
}

CGUIText::CGUIText(CGUIDisplay & display, CGUITextItem * textItem, CGUIColor backgroundColor, StylingRecord * stylingRecord = NULL)
   : CGUIWindowObject(display)
{
	initializeData(textItem, stylingRecord);
	setBackgroundColor(backgroundColor);
}

CGUIText::~CGUIText()
{
}

void CGUIText::initializeData(CGUITextItem * textItem, StylingRecord * stylingRecord)
{
	_textItem =  textItem;
	_forceCompute = false;
	_captureBackgroundColor = false;
	_backgroundColorSet = false;
	_languageSetByApp = false;

	initializeForbiddenChars();
	
	if (_textItem)
	{
		if (_textItem->isInitialized())
		{
			//  If styling record from constructor is null, set the the cgui_text._stylingRecord to
			//  the cgui_text_item._stylingRecord by default.
			//
			if (!stylingRecord)
			{
				_stylingRecord = _textItem->getStylingRecord();
			}
			else
			{
				_stylingRecord = *stylingRecord;
			}

			// Set the requested region.
			_requestedRegion = _stylingRecord.region;
			_textString = _textItem->getTextObj(_textItem->getLanguageId());

			if( _textString.getLength() == 0 )
				setRegion(CGUIRegion(0, 0, 0, 0));

			_forceCompute = true;
			computeTextRegion();
		}
	}

	_languageSetByApp = false;
}

void CGUIText::setAttributes(unsigned int attributes)
{
	if ( _stylingRecord.attributes != attributes )
	{
		_stylingRecord.attributes = attributes;
		_forceCompute = true;
		computeTextRegion();
	}
}

void CGUIText::setBackgroundColor(CGUIColor color)
{
	if ( !_backgroundColorSet ||
		  _backgroundColor != color )
	{
		_backgroundColorSet = true;
		_backgroundColor = color;
		if ( _owner ) _owner->invalidateObjectRegion(this);
	}
}

void CGUIText::setColor(CGUIColor color)
{
	if ( _stylingRecord.color != color )
	{
		_stylingRecord.color = color;
		if ( _owner ) _owner->invalidateObjectRegion(this);
	}
}

void CGUIText::setColor(int red, int green, int blue)
{
	CGUIColor color = MakeCGUIColor(red, green, blue);
	setColor(color);
}

void CGUIText::setFontId(CGUIFontId fontId)
{
	// Internally, fontId is a pointer, so unlike the others, we don't
	// check for a change in this update function.
	//
	_stylingRecord.fontId = fontId;
	_forceCompute = true;
	computeTextRegion();
}

void CGUIText::setFontSize(int fontSize)
{
	if ( _stylingRecord.fontSize != fontSize )
	{
		_stylingRecord.fontSize = fontSize;
		_forceCompute = true;
		computeTextRegion();
	}
}

void CGUIText::setLanguage(LanguageId configLanguage)
{
	//
	// This animal will look different as the
	// language module is developed.
	//
	_configLanguage = configLanguage;
	if( _textItem )
		_textItem->setLanguageId(configLanguage);

	_languageSetByApp = true;
}

void CGUIText::setStylingRecord (StylingRecord * stylingRecord)
{
	_stylingRecord = * stylingRecord;
	_requestedRegion = stylingRecord->region;
	_forceCompute = true;
	computeTextRegion();
}

void CGUIText::setText(CGUITextItem * textItem)
{
	if (textItem)
	{
		_textItem =  textItem;

		if( _textItem && _textItem->isInitialized() )
		{
			_textString = _textItem->getTextObj(_textItem->getLanguageId());

			if( _textString.getLength() == 0 )
				setRegion(CGUIRegion(0, 0, 0, 0));

			computeTextRegion();
		}
	}
}

void CGUIText::setText(void)
{
	if( _textItem )
	{
		_textString = _textItem->getTextObj(_textItem->getLanguageId());
	}
	else
	{
		_textString.empty();
	}

	if( _textString.getLength() == 0 )
		setRegion(CGUIRegion(0, 0, 0, 0));

	computeTextRegion();
}

void CGUIText::setText(const StringChar * text)
{
	if (text)
	{
		_textString = text;
		_textItem = NULL;
	}
	else
	{
		setRegion(CGUIRegion(0, 0, 0, 0));
	}

	computeTextRegion();
}

void CGUIText::setText(const UnicodeString & text)
{
	_textString = text;
	_textItem = NULL;

	if (text.getLength() == 0)
	{
		setRegion(CGUIRegion(0, 0, 0, 0));
	}

	computeTextRegion();
}

void CGUIText::setText(const char * text)
{
	if (text)
	{
		_textString = text;
		_textItem = NULL;
	}
	else
	{
		setRegion(CGUIRegion(0, 0, 0, 0));
	}

	computeTextRegion();
}

const StringChar * CGUIText::getText(void)
{
	return _textString.getString();
}

const UnicodeString& CGUIText::getTextObj(void)
{
	handleVariableSubstitution();
	return _textString;
}

int CGUIText::getLength(void) const
{
	int retVal = 0;

	if( _textItem )
	{
		retVal = _textItem->getLength();
	}
	else
	{
		retVal = _textString.getLength();
	}

	return retVal;
}

void CGUIText::getSize(CGUIRegion & region, int startIndex, int length)
{
	if ( _stylingRecord.fontId == UGL_NULL_ID ||
  		  !_textString.getLength() ||
		  startIndex >= _textString.getLength() )
	{
		region = CGUIRegion(0, 0, 0, 0);
	}
	else
	{
		UGL_SIZE uglStartIndex = startIndex;
		UGL_SIZE uglLength = length;

		if ( uglStartIndex < 0 )
		{
			uglStartIndex = 0;
		}

		if ( uglLength < 0 ||
			  uglStartIndex + uglLength > _textString.getLength() )
		{
			uglLength = _textString.getLength() - startIndex;
		}

		UGL_SIZE width = 0;
		UGL_SIZE height = 0;

		if ( _stylingRecord.attributes & BOLD )
		{
			UGL_ORD option = 0;
			uglFontInfo(_stylingRecord.fontId, UGL_FONT_WEIGHT_SET, &option);
		}
		const StringChar * textStr = _textString.getString();
		uglTextSizeGetW(_stylingRecord.fontId, &width, &height, uglLength, &textStr[uglStartIndex]);
		region = CGUIRegion(0, 0, width, height);
	}
}

void CGUIText::getPixelSize(CGUIRegion & pixelRegion)
{
	getSize(pixelRegion, 0, _textString.getLength());
}

CGUIText::GetTokenResult CGUIText::getCharBasedToken(int start_index, bool start_of_line, int & length)
{
	bool token_ended = false;
	int current_index = start_index;
	length = 0;
	GetTokenResult	result = EndOfString;

	// Check for format command.  These are only allowed at the start of a line.
	//
	if ( start_of_line &&
		  current_index < _textString.getLength() - paragraph_format_start.getLength() )
	{
		if ( _textString[current_index] == paragraph_format_start[0] &&
			  _textString.mid(current_index, paragraph_format_start.getLength()) == paragraph_format_start )
		{
			int	endOfFormatToken = _textString.find(paragraph_format_end, current_index+paragraph_format_start.getLength());
			if ( endOfFormatToken >= 0 )
			{
				endOfFormatToken += paragraph_format_end.getLength();
				length = endOfFormatToken - current_index;
				token_ended = true;
				result = FormatToken;
			}
		}
	}

	// keep moving characters from the text string to the token string until
	// a trailing blank or EOS is found.
	//

	int tokenWidth=0;
	bool englishToken = false;
	bool forbiddenStartToken = false;
	bool forbiddenEndToken = false;

	while ( current_index < _textString.getLength() &&
			  !token_ended )
	{
		if( _textString[current_index] == newline_char )
		{
			token_ended = true;
			if ( current_index == start_index )
			{
				length = 1;
			}
		}
		else if( checkIfForbiddenStart(current_index+1) ) // Asian Forbidden Start char
		{
			if( englishToken && !checkIfEnglish(current_index) && !forbiddenStartToken )
			{
				token_ended=true;
			}
			else
			{
				length+=1;
				current_index+=1; 
				forbiddenStartToken=true;  
			}
		}
		else if( checkIfForbiddenEnd(current_index) ) // Asian Forbidden End Char
		{
			if( englishToken && !forbiddenStartToken && !forbiddenEndToken )
			{
				token_ended=true;
			}
			else
			{
				length+=1;
				current_index+=1;
				forbiddenEndToken=true;
			}  
		}
		else if( checkIfEnglish(current_index) ) // English letters or numbers
		{
			englishToken = true;
			length++;
			current_index++;
		}
		else if ( _textString[current_index] == tab_char )
		{
			token_ended = true;
			if ( current_index == start_index )
			{
				length = 1;
			}
		}
		else if( _textString[current_index] == space_char)
		{
			if( englishToken )
			{
				token_ended=true;
			}
			else
			{
				length+=1;
				current_index+=1;
			}
		}
		else   
		{
			// Floating point numbers
			if( ( _textString[current_index] == decimal_point || _textString[current_index] == decimal_comma )
				 && checkIfArabicNumeral(current_index-1) && checkIfArabicNumeral(current_index+1) )
			{
				length+=1;
				current_index+=1;
			}
			else // All other characters
			{
				if( forbiddenStartToken || forbiddenEndToken || !englishToken )
				{
					length+=1;
					current_index+=1;
	
					if( forbiddenStartToken ) forbiddenStartToken=false;
					if( forbiddenEndToken )   forbiddenEndToken=false;
				}
	
				if( englishToken )
				{
					englishToken=false;
				}
	
				token_ended = true; 
			}
		}
	}

	if ( result != FormatToken )
	{
		result = (length <= 0) ? EndOfString : NormalToken;
	}

	return result;
}

bool CGUIText::checkIfArabicNumeral(int index)
{
	bool result=false;
	UnicodeString zero('0');
	UnicodeString nine('9');

	if( index < _textString.getLength() && index >= 0 )
	{
		StringChar charPos = _textString[index];

		if( charPos >= zero[0] && charPos <= nine[0] )
		{
			result = true;
		}
	}

	return result;

}

bool CGUIText::checkIfEnglish(int index)
{
	bool result = false;
	UnicodeString smallA('a');
	UnicodeString bigA('A');
	UnicodeString smallZ('z');
	UnicodeString bigZ('Z');

	UnicodeString zero('0');
	UnicodeString nine('9');

	if( index < _textString.getLength() && index >= 0 )
	{
		StringChar charPos = _textString[index];

		if( ( charPos >= smallA[0] && charPos <= smallZ[0] ) || 
			 ( charPos >= bigA[0] && charPos <= bigZ[0] ) ||
			 ( charPos >= zero[0] && charPos <= nine[0] ) )
		{
			result = true;
		}
	}

	return result;
}

bool CGUIText::checkIfForbiddenStart(int index)
{
	bool result = false;

	if( _forbiddenStartCharsAvailable )
	{	
		if( index < _textString.getLength() && index >= 0 )
		{
			if( _forbiddenStartCharList.find(_textString[index],0) != -1 )
			{
				result = true;
			}
		}
	}

	return result;
}

bool CGUIText::checkIfForbiddenEnd(int index)
{
	bool result = false;

	if( _forbiddenEndCharsAvailable )
	{
		if( index < _textString.getLength() && index >= 0 )
		{
			if( _forbiddenEndCharList.find(_textString[index],0) != -1 )
			{
				result = true;
			}
		}
	}

	return result;
}


void CGUIText::initializeForbiddenChars()
{
	if( !_forbiddenCharsInitialized )
	{
		CGUITextItem* forbiddenStartCharList = CGUITextItem::_textMap.findString("forbiddenStartCharList");
		if( forbiddenStartCharList != NULL )
		{
			_forbiddenStartCharList = forbiddenStartCharList->getTextObj();
			_forbiddenStartCharsAvailable=true;
		}
		else
		{
			DataLog( log_level_cgui_info ) << "Couldn't find forbiddenStartCharList in string.info files" << endmsg;
		}
	
		CGUITextItem* forbiddenEndCharList = CGUITextItem::_textMap.findString("forbiddenEndCharList");
		if( forbiddenEndCharList != NULL )
		{
			_forbiddenEndCharList = forbiddenEndCharList->getTextObj();
			_forbiddenEndCharsAvailable=true;
		}
		else
		{
			DataLog( log_level_cgui_info ) << "Couldn't find forbiddenEndCharList in string.info files" << endmsg;
		}

		_forbiddenCharsInitialized=true;
	}
}

CGUIText::GetTokenResult CGUIText::getToken(int start_index, bool start_of_line, int & length)
{
	// this flag is false before encountering non-blank token characters
	// and true after encountering non-blank token characters. This allows
	// leading blanks to be included in the token, and the first trailing
	// blank to delimit the token.
	//
	bool look_for_trailing_blank = false;
	bool token_ended = false;

	int current_index = start_index;
	length = 0;

	GetTokenResult	result = EndOfString;

	// Check for format command.  These are only allowed at the start of a line.
	//
	if ( start_of_line &&
		  current_index < _textString.getLength() - paragraph_format_start.getLength() )
	{
		if ( _textString[current_index] == paragraph_format_start[0] &&
			  _textString.mid(current_index, paragraph_format_start.getLength()) == paragraph_format_start )
		{
			int	endOfFormatToken = _textString.find(paragraph_format_end, current_index+paragraph_format_start.getLength());
			if ( endOfFormatToken >= 0 )
			{
				endOfFormatToken += paragraph_format_end.getLength();
				length = endOfFormatToken - current_index;
				token_ended = true;
				result = FormatToken;
			}
		}
	}

	// keep moving characters from the text string to the token string until
	// a trailing blank or EOS is found.
	//
	while ( current_index < _textString.getLength() &&
			  !token_ended )
	{
		if ( _textString[current_index] == newline_char ||
			  _textString[current_index] == tab_char )
		{
			// If this character is an explicit newline or tab and the token buffer is empty,
			// return the character.  Otherwise return the token parsed so far, and this character
			// will be returned on the next call to getToken().
			//
			token_ended = true;
			if ( current_index == start_index )
			{
				// nothing has been added to the token buffer, just return the single character
				length = 1;
			}
		}
		else if ( _textString[current_index] == space_char &&
					 look_for_trailing_blank )
		{
			// if this is the delimiting trailing blank, leave the loop and return the token
			token_ended = true;
		}
		else
		{
			if ( _textString[current_index] != space_char )
			{
				look_for_trailing_blank = true;
			}

			// add this character to the token
			length++;
			current_index++;
		}
	}

	if ( result != FormatToken )
	{
		result = (length <= 0) ? EndOfString : NormalToken;
	}

	return result;
}

void CGUIText::convertTextToMultiline(CGUIRegion & region)
{
	bool	start_of_line = true;
	int	current_index = 0;
	int	current_line = 0;
	int	space_pixel_size = 0;
	int	single_line_height = 0;
	int	x_min = 0;
	int	x_max = 0;
	int	y_max = 0;

	_lineData.clear();
	_formatData.firstLineIndent = 0;
	_formatData.secondLineIndent = 0;

	if ( _stylingRecord.fontId != UGL_NULL_ID )
	{
		UGL_SIZE width = 0;
		UGL_SIZE height = 0;

		uglTextSizeGet(_stylingRecord.fontId, &width, &height, 1, " ");
		space_pixel_size = width;
		single_line_height = height;
	}

	int	line_start_x = 0;
	int	line_start_y = 0;

	while ( current_index < _textString.getLength() )
	{
		int	current_segment_pixel_width = 0;
		int	line_start_index = current_index;
		int	line_byte_count = 0;
		bool  line_done = false;
		bool	line_ended_by_tab = false;

		CGUIRegion line_size;

		while ( !line_done )
		{
			int	token_char_count;
			if (start_of_line) line_start_x = _formatData.firstLineIndent * _tabSpaceCount * space_pixel_size;

			// get the next blank delimited token

			GetTokenResult tokenResult = EndOfString;

			switch( _tokenSplitMethod )
			{
			case WordBased:
				tokenResult = getToken(current_index, start_of_line, token_char_count);
				break;

			case CharBased:
				tokenResult = getCharBasedToken(current_index, start_of_line, token_char_count);
				break;
			}

			UnicodeString ustr = _textString.mid(current_index,token_char_count);

			if ( tokenResult == EndOfString )
			{
				// reached end of string, so this is the last line
				line_done = true;
				current_index = _textString.getLength();
			}
			else if ( tokenResult == FormatToken )
			{
				int	parameter_index = current_index + paragraph_format_start.getLength();
				int	first_line_indent = -1;
				int	second_line_indent = -1;
				while ( parameter_index < current_index + token_char_count &&
						  second_line_indent < 0 )
				{
					if ( _textString[parameter_index] >= (StringChar)'0' &&
						  _textString[parameter_index] <= (StringChar)'9' )
					{
						if ( first_line_indent < 0 )
						{
							first_line_indent = _textString[parameter_index] - (StringChar)'0';
						}
						else
						{
							second_line_indent = _textString[parameter_index] - (StringChar)'0';
						}
					}

					parameter_index += 1;
				}

				if ( first_line_indent >= 0 &&
					  second_line_indent >= 0 )
				{
					_formatData.firstLineIndent = first_line_indent;
					_formatData.secondLineIndent = second_line_indent;
				}

				current_index += token_char_count;
				line_start_index = current_index;
			}
			else if ( _textString[current_index] == newline_char )
			{
				// newline forced a line break
				start_of_line = true;
				current_index += token_char_count;

				if ( line_byte_count )
				{
					line_done = true;
				}
				else
				{
					line_start_index = current_index;
					line_start_y += 3*single_line_height/4;
				}
			}
			else if ( _textString[current_index] == tab_char )
			{
				start_of_line = false;
				current_index += token_char_count;

				if ( line_byte_count )
				{
					line_done = true;
					line_ended_by_tab = true;
				}
				else
				{
					line_start_x += _tabSpaceCount * space_pixel_size;
					line_start_index = current_index;
				}
			}
			else
			{
				start_of_line = false;

				CGUIRegion token_size;
				getSize(token_size, current_index, token_char_count);

				if ( convertLinePosition(current_segment_pixel_width+token_size.width, token_size.height, line_start_x, line_size) )
				{
					// new token fits on line
					line_byte_count += token_char_count;
					current_index   += token_char_count;
					current_segment_pixel_width += token_size.width;
				}
				else
				{
					line_done = true;
					if ( line_byte_count <= 0 )
					{
						// must put token on line
						line_byte_count += token_char_count;
						current_index   += token_char_count;
						current_segment_pixel_width += token_size.width;
					}
					else
					{
						convertLinePosition(current_segment_pixel_width, token_size.height, line_start_x, line_size);
					}
				}
			}
		}

		// Create line_data struct
		//
		if ( line_byte_count > 0 )
		{
			LineData 	current_line_data;

			current_line_data.x = line_size.x;
			current_line_data.y = line_start_y;

			current_line_data.index = line_start_index;
			current_line_data.textLength = line_byte_count;
			if ( _textString[line_start_index] == space_char )
			{
				current_line_data.index++;
				current_line_data.textLength--;
			}

			_lineData.push_back(current_line_data);

			x_max = (line_size.x + line_size.width - 1 > x_max) ? line_size.x + line_size.width - 1 : x_max;
			if ( current_line <= 0 )
			{
				x_min = line_size.x;
			}
			else
			{
				x_min = (line_size.x < x_min) ? line_size.x : x_min;
			}

			y_max = _lineData.back().y + line_size.height + 1;
			current_line++;
		}

		if ( !line_ended_by_tab )
		{
			line_start_x = _formatData.secondLineIndent * _tabSpaceCount * space_pixel_size;
			line_start_y += single_line_height;
		}
		else
		{
			int tab_column = ( line_size.x + line_size.width - 1 - _requestedRegion.x ) / ( _tabSpaceCount * space_pixel_size ) + 1;
			line_start_x = tab_column * _tabSpaceCount * space_pixel_size;
		}
	}

	// Setup final region required by the text
	//
	region.x = x_min;
	region.y = 0;
	region.width = x_max - x_min + 1;
	region.height = y_max + 1;

	// Adjust line horizontal positions to left side of region
	//
	list<LineData>::iterator line;
	for ( line=_lineData.begin(); line != _lineData.end(); ++line )
	{
		(*line).x -= region.x;
	}
}


bool CGUIText::convertLinePosition(int width, int height, int indent_pixels, CGUIRegion & region)
{
   int  available_pixel_width = 0;

   // Given alignment specification and text size information, determine the region
   // in which the text should be drawn
   if (_stylingRecord.attributes & HJUSTIFY_CENTER)
   {
      // CENTER alignment
      region.x = _requestedRegion.x + (_requestedRegion.width - width)/2;
      available_pixel_width = _requestedRegion.width;
   }
   else if (_stylingRecord.attributes & HJUSTIFY_RIGHT)
   {
      // RIGHT alignment: text will end at _requestedRegion.x.  Text with RIGHT
      // alignment is not wrapped, so available_pixel_width is always set to the
      // text size.
      region.x = _requestedRegion.x + _requestedRegion.width - 1 - width;
      available_pixel_width = width;
   }
   else
   {
      // LEFT alignment: text will start at _requestedRegion.x plus the current indent
      region.x = _requestedRegion.x + indent_pixels;
      available_pixel_width = ( _requestedRegion.width == 0 ) ? _display.width()-region.x : _requestedRegion.width-indent_pixels;
   }

   region.y = 0;
   region.width = width;
   region.height = height;

   return( width <= available_pixel_width );
} // END convert_line_position


void CGUIText::computeTextRegion(void)
{
   if ( _textString.getLength() > 0 ) handleVariableSubstitution();
   if ( _forceCompute ||
		  _textString != _lastTextString )
	{
		_forceCompute = false;
		_lastTextString = _textString;

		CGUIRegion    text_region;
		convertTextToMultiline(text_region);
	
		// find vertical region to place text
		if (_stylingRecord.attributes & VJUSTIFY_CENTER)
		{
			// center vertically within specified region
			if (text_region.height <= _requestedRegion.height)
			{
				text_region.vertShift(_requestedRegion.y + (_requestedRegion.height-text_region.height)/2);
			}
			else
			{
				text_region.vertShift(_requestedRegion.y);
			}
		}
		else if (_stylingRecord.attributes & VJUSTIFY_BOTTOM)
		{
			// justify at bottom of requested region
			if (text_region.height <= _requestedRegion.height)
			{
				text_region.vertShift(_requestedRegion.y + _requestedRegion.height - text_region.height);
			}
			else
			{
				text_region.vertShift(_requestedRegion.y);
			}
		}
		else
		{
			// justify at top of requested region
			text_region.vertShift(_requestedRegion.y);
		}
	
		setRegion(text_region);
	}
}

void CGUIText::setRegion(const CGUIRegion & newRegion)
{
   if ( _owner )
	{
      _owner->setObjectRegion(this, newRegion);
	}
   else
	{
      _region = newRegion;
	}
}

void CGUIText::draw(UGL_GC_ID gc)
{
   if (!_textString.getLength() ||
       _stylingRecord.fontId == UGL_NULL ||
       _lineData.empty() ||
       _region.width == 0 ||
       _region.height == 0)
   {
      return;
   }

   //
   // Need true coordinates (not viewport relative) on screen so that
   // we can get correct background color for text.
   //
   UGL_POS vp_left, vp_top, vp_right, vp_bottom;
   uglViewPortGet(gc, &vp_left, &vp_top, &vp_right, &vp_bottom);

   CGUIColor  backgroundColor;
   {
      //
      // Average the color of the four corner pixels for the text region.
      //
      int   pixelCount = 0;
      UGL_ARGB   red = 0;
      int   green = 0;
      int   blue = 0;

      //
      // Get the pixel from each corner of text region and average the color pixel.
      //
      for (int x = _region.x; x <= _region.x + _region.width; x += _region.width)
      {
         for (int y = _region.y; y <= _region.y + _region.height; y += _region.height)
         {
            if (x+vp_left >= 0 && x+vp_left < _display.width() &&
                y+vp_top >= 0 && y+vp_top < _display.height())
            {
               UGL_ARGB    pixelARGB;
               uglPixelGet(gc, x, y, &pixelARGB);

               red += (pixelARGB & 0xf800) >> 8;
               green += (pixelARGB & 0x07e0) >> 3;
               blue += (pixelARGB & 0x001f) << 3;
               pixelCount += 1;
            }
         }
      }

      if (pixelCount > 1)
      {
         red /= pixelCount;
         green /= pixelCount;
         blue /= pixelCount;
      }

      backgroundColor = MakeCGUIColor(red, green, blue);

#if CPU==SIMNT
      if (_captureBackgroundColor)
      {
         _captureBackgroundColor = false;
         _backgroundColorSet = true;
         _backgroundColor = backgroundColor;
      }
#endif
   }

#if CPU==SIMNT
	if ( captureTextID &&
		  _textItem )
	{
		captureTextIDStr += captureScreenName;
		captureTextIDStr += " ";
		captureTextIDStr += _textItem->getId();
		captureTextIDStr += "\n";
	}
#endif /* if CPU==SIMNT */

	// make copy of text string to draw.
	UnicodeString copyTextString( _textString );
	copyTextString.replace( nonBreakingSpace, regularLatinSpace );
	
	//  let's write this out
   uglBackgroundColorSet(gc, backgroundColor);
   uglForegroundColorSet(gc, _stylingRecord.color);
   uglFontSet(gc, _stylingRecord.fontId);

   list<LineData>::const_iterator   lineIter = _lineData.begin();
   while( lineIter != _lineData.end() )
   {
      uglTextDrawW(gc, (*lineIter).x + _region.x, (*lineIter).y + _region.y, (*lineIter).textLength, &copyTextString[(*lineIter).index]);

      ++lineIter;
		
   } /* while lineIter != _lineData.end() */
} // END draw

void CGUIText::handleVariableSubstitution(void)
{
	if( _textItem )
	{
		_textString = _textItem->getTextObj();
	}
}

