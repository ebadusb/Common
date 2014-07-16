/*
 * $Header$
 *
 * $Log: cgui_text.cpp $
 * Revision 1.47  2009/05/29 23:11:49Z  ms10234
 * -remove wvEvent calls which were checked in by mistake
 * Revision 1.46  2009/05/26 20:30:28Z  ms10234
 * 6768 - change to only load font range once
 * Revision 1.45  2009/03/02 20:46:25Z  adalusb
 * IT 6701. Added a function to map a char to a font name for reports.
 * Revision 1.44  2009/01/07 16:52:12Z  adalusb
 * Tab width is now calculated using english language font instead of the styling record font.
 * Revision 1.43  2008/12/17 21:35:47Z  rm10919
 * Correction on text length for combined fonts. IT 6562
 * Revision 1.42  2008/12/17 19:16:01Z  rm10919
 * Fix global variable definition for build.
 * Revision 1.41  2008/12/17 16:46:37Z  rm10919
 * Correct global varibale.
 * Revision 1.40  2008/12/17 16:20:22Z  rm10919
 * Changes for combined fonts. IT6562
 * Revision 1.39  2008/12/16 22:01:42Z  rm10919
 * Correct include file name.
 * Revision 1.38  2008/12/16 06:03:03Z  rm10919
 * Add the ablility for combined fonts in text. IT 6562
 * Revision 1.37  2008/07/23 22:55:04Z  adalusb
 * Selection of the text wrapping algorithm based on the language loaded enabled.
 * Revision 1.36  2008/07/22 17:43:58Z  adalusb
 * Set the token algorithm to word based for now as strings have been frozen for MNC production ( English ). This will be changed for foreign languages.
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
#include <sstream>
#include <math.h>

#include "cgui_text.h"
#include "cgui_window.h"
#include "font_table.h"
#include "datalog_levels.h"
#include "error.h"

#if CPU==SIMNT
# include <string.h>

bool captureTextID = false;
string captureTextIDStr;
const char * captureScreenName = NULL;
#endif /* if CPU==SIMNT */


CGUIVariableDatabaseContainer CGUIText::_variableDictionary;
unsigned short CGUIText::_tabSpaceCount = 5;

static StringChar newlineChar = '\n';
static StringChar spaceChar = ' ';
static StringChar	tabChar = '\t';
static StringChar nullChar = '\0';
static StringChar decimalPoint = '.';
static StringChar decimalComma = ',';
static UnicodeString	paragraphFormatStart("#![PG");
static UnicodeString paragraphFormatEnd("]");
static const UnicodeString nonBreakingSpace(" ");  // unicode hex or utf8 format = 0xC2A0
static const UnicodeString regularLatinSpace(" "); // unicode hex = 0x0020
CGUIText::TokenSplitMethod CGUIText::_tokenSplitMethod = CGUIText::WordBased;

bool CGUIText::_forbiddenStartCharsAvailable = false;
bool CGUIText::_forbiddenEndCharsAvailable = false;
bool CGUIText::_forbiddenCharsInitialized=false;

UnicodeString CGUIText::_forbiddenStartCharList;
UnicodeString CGUIText::_forbiddenEndCharList;

bool CGUIText::_tokenSplitMethodSelected = false;

int currentLanguage = 0;
int CGUIText::_defaultFontIndex = 0;
int simTextBold = 0;

// Declare & initialize font range list.
list< FontRange * > CGUIText::_fontRange;
SEM_ID CGUIText::_fontRangeSem = semMCreate( SEM_Q_PRIORITY | SEM_INVERSION_SAFE );
bool CGUIText::_fontRangeIdsAdded = false;

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

	if( !_tokenSplitMethodSelected )
	{
		selectTokenSplitMethod();
	}

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

	// add any new fonts needed for text
	if( !_fontRangeIdsAdded )
	{
		addFontRangeFonts();
	}

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


bool CGUIText::addFontRange( FontRange *fontRange )
{
	semTake(_fontRangeSem, WAIT_FOREVER);

	// future add logic to insert in list in sorted order.
	_fontRange.push_front( fontRange );

	semGive( _fontRangeSem );

   return true;
}


CGUIFontId CGUIText::getFontId( int currentChar )
{
   CGUIFontId resultFontId = _stylingRecord.fontId;

	int fontIndex = checkInFontRange( currentChar );

   if( fontIndex != _defaultFontIndex )
	{
		int index = _stylingRecord.fontSize + fontIndex * 50;

		if( index >= MAX_FONTS || index < 0 )
		{
			_FATAL_ERROR(__FILE__, __LINE__, "Array bounds exceeded for font table.");
		}

      resultFontId = _display._font[index];
	}

	return resultFontId;
}

int CGUIText::checkInFontRange( int currentChar )
{
	int result = _defaultFontIndex;

	semTake(_fontRangeSem, WAIT_FOREVER);

	if( _fontRange.size() > 0 )
	{
		list< FontRange * >::iterator fontRangeIter = _fontRange.begin();
		FontRange * fontRange = NULL;

		while( fontRangeIter != _fontRange.end() )
		{
			fontRange = ( *fontRangeIter );
			if( currentChar >= fontRange->startIndex &&
				 currentChar <= fontRange->endIndex )
			{
				result = fontRange->fontIndex;
				break;
			}
			fontRangeIter++;
		} // while not end of _fontRange list do
	}

	semGive( _fontRangeSem );

	return result;
}

bool CGUIText::getFontNameForChar(unsigned short currentChar,string& fontName)
{
	bool result = false;

	semTake(_fontRangeSem, WAIT_FOREVER);

	if( _fontRange.size() > 0 )
	{
		list< FontRange * >::iterator fontRangeIter = _fontRange.begin();
		FontRange * fontRange = NULL;

		while( fontRangeIter != _fontRange.end() )
		{
			fontRange = ( *fontRangeIter );
			if( currentChar >= fontRange->startIndex &&
				 currentChar <= fontRange->endIndex )
			{
				fontName = fontRange->fontName;
				result = true;
				break;
			}

			fontRangeIter++;
		}
	}

	semGive( _fontRangeSem );

	return result;
}

void CGUIText::clearFontRange()
{
	semTake(_fontRangeSem, WAIT_FOREVER);

	if( _fontRange.size() > 0 )
	{
		list< FontRange * >::iterator fontRangeIter = _fontRange.begin();
		FontRange * fontRange = NULL;

		while( fontRangeIter != _fontRange.end() )
		{
         fontRange = ( *fontRangeIter );

			// go to next item in list
			fontRangeIter++;

			// delete pointer to fontRange struct
			delete( fontRange );
		} // while not end of _fontRange list do
	}

	_fontRange.clear();

	semGive( _fontRangeSem );
}

void CGUIText::addFontRangeFonts()
{
	semTake(_fontRangeSem, WAIT_FOREVER);

	if( _fontRange.size() > 0 )
	{
		list< FontRange * >::iterator fontRangeIter = _fontRange.begin();
		FontRange * fontRange = NULL;

		while( fontRangeIter != _fontRange.end() )
		{
			fontRange = ( *fontRangeIter );
//			if( _display._font[6 + fontRange->fontIndex* 50] == NULL )
//			{
				UGL_ORD option = UGL_FONT_ANTI_ALIASING_16;
				unsigned char bold = simTextBold;

				for (int f=6; f<=32; f++)
				{
					int index = f + fontRange->fontIndex * 50;

					if( index >= MAX_FONTS || index < 0 )
					{
						_FATAL_ERROR(__FILE__, __LINE__, "Array bounds exceeded for font table.");
					}

					_display._font[index] = _display.createFont( fontRange->fontName, f, bold);
					uglFontInfo(_display._font[index], UGL_FONT_ANTI_ALIASING_SET, &option);
	//				option = UGL_FONT_BOLD_LIGHT;
	//				uglFontInfo( _display->_font[f], UGL_FONT_WEIGHT_SET, &option );
				}
//			} // if font doesn't exsist

			fontRangeIter++;
		} // while not end of _fontRange list do
	} //	font range list greater than zero

	_fontRangeIdsAdded = true;

	semGive( _fontRangeSem );
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


void CGUIText::setNumericText(const double& value, int precision, bool commaDelim, const char* suffix )
{
   std::string displayText;
   ostringstream  textStream;
   textStream.setf( ios::fixed );
   textStream.precision( precision );
   double resultValue = value;

   if ( precision == 0 )
   {
      int displayValue = (int) (resultValue >= 0 ? resultValue + 0.5 : resultValue - 0.5);
      textStream << displayValue << ends;
   }
   else
   {
      double cutOff = (double)((5*pow(10, precision * -1))/10);
      if ( fabs(resultValue) < cutOff ) resultValue = 0.0;

      textStream << resultValue << ends;
   }

   if ( suffix )
   {
      textStream << suffix;
      textStream << ends;
   }
   displayText = textStream.str();

   if ( commaDelim )
   {
      // Changed to exactly mimic the previous implementation using c-strings
      //
      size_t found_pos = displayText.find_first_not_of("0123456789");

      if ( found_pos != string::npos &&
           found_pos >= 0 &&
           found_pos < displayText.length() &&
           displayText[found_pos] == '.' )
      {
         displayText[found_pos] = ',';
      }
   }
   setText( displayText.c_str() );
}


void CGUIText::setTextItem( CGUITextItem * textItem )
{
   setText( textItem );
   StylingRecord newStyle = textItem->getStylingRecord();

   setRegion( newStyle.region );

   setStylingRecord( &newStyle );

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

void CGUIText::getSize(CGUIRegion & region, int startIndex, int length, CGUIFontId fontId)
{
	if (( fontId == UGL_NULL_ID ||
			_stylingRecord.fontId == UGL_NULL_ID) ||
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
			uglFontInfo( fontId, UGL_FONT_WEIGHT_SET, &option );
		}
		const StringChar * textStr = _textString.getString();
		uglTextSizeGetW( fontId, &width, &height, uglLength, &textStr[uglStartIndex] );
		region = CGUIRegion( 0, 0, width, height );
	}
}

void CGUIText::getPixelSize(CGUIRegion & pixelRegion, CGUIFontId fontId )
{
	getSize(pixelRegion, 0, _textString.getLength(), fontId);
}


void CGUIText::getTokenSize( CGUIRegion & region, int startIndex = -1, int length = -1 )
{
	CGUIRegion resultRegion = CGUIRegion( 0, 0, 0, 0 );
	CGUIFontId currentFontId = getFontId( _textString[ startIndex ]);	//_stylingRecord.fontId;

	semTake(_fontRangeSem, WAIT_FOREVER);
	size_t listSize = _fontRange.size();
	semGive( _fontRangeSem );

	if( listSize > 0 )
	{
		int segmentLength = 0;

		if( startIndex >= 0 && length > 0 )
		{
			for( int i = startIndex; i <= ( startIndex + length ); i++ )
			{
				int currentChar = (int)_textString[i];
				CGUIFontId newFontId = getFontId( currentChar );

				if( newFontId == NULL )
					newFontId = _stylingRecord.fontId;

				if( newFontId != currentFontId || ( i == ( startIndex + length )))
				{

					//	update resultRegion to new values
					getSize( resultRegion,( i - segmentLength ), segmentLength, currentFontId );

					region.width += resultRegion.width;
					segmentLength = 0;	//	This will be automaticaly incremented when it drops out of the if-statement

					currentFontId = newFontId;
				}
				segmentLength++;

			}	// for text length
		}	 // if greater than zero

      region.height = resultRegion.height;
	}
	else
	{
		getSize( region, startIndex, length, currentFontId );
	}
}


CGUIText::GetTokenResult CGUIText::getCharBasedToken(int startIndex, bool startOfLine, int & length)
{
	bool tokenEnded = false;
	int currentIndex = startIndex;
	length = 0;
	GetTokenResult	result = EndOfString;

	// Check for format command.  These are only allowed at the start of a line.
	//
	if ( startOfLine &&
		  currentIndex < _textString.getLength() - paragraphFormatStart.getLength() )
	{
		if ( _textString[currentIndex] == paragraphFormatStart[0] &&
			  _textString.mid(currentIndex, paragraphFormatStart.getLength()) == paragraphFormatStart )
		{
			int	endOfFormatToken = _textString.find(paragraphFormatEnd, currentIndex+paragraphFormatStart.getLength());
			if ( endOfFormatToken >= 0 )
			{
				endOfFormatToken += paragraphFormatEnd.getLength();
				length = endOfFormatToken - currentIndex;
				tokenEnded = true;
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

	while ( currentIndex < _textString.getLength() &&
			  !tokenEnded )
	{
		if( _textString[currentIndex] == newlineChar )
		{
			tokenEnded = true;
			if ( currentIndex == startIndex )
			{
				length = 1;
			}
		}
		else if( checkIfForbiddenStart(currentIndex+1) ) // Asian Forbidden Start char
		{
			if( englishToken && !checkIfEnglish(currentIndex) && !forbiddenStartToken )
			{
				tokenEnded=true;
			}
			else
			{
				length+=1;
				currentIndex+=1;
				forbiddenStartToken=true;
			}
		}
		else if( checkIfForbiddenEnd(currentIndex) ) // Asian Forbidden End Char
		{
			if( englishToken && !forbiddenStartToken && !forbiddenEndToken )
			{
				tokenEnded=true;
			}
			else
			{
				length+=1;
				currentIndex+=1;
				forbiddenEndToken=true;
			}
		}
		else if( checkIfEnglish(currentIndex) ) // English letters or numbers
		{
			englishToken = true;
			length++;
			currentIndex++;
		}
		else if ( _textString[currentIndex] == tabChar )
		{
			tokenEnded = true;
			if ( currentIndex == startIndex )
			{
				length = 1;
			}
		}
		else if( _textString[currentIndex] == spaceChar)
		{
			if( englishToken )
			{
				tokenEnded=true;
			}
			else
			{
				length+=1;
				currentIndex+=1;
			}
		}
		else
		{
			// Floating point numbers
			if( ( _textString[currentIndex] == decimalPoint || _textString[currentIndex] == decimalComma )
				 && checkIfArabicNumeral(currentIndex-1) && checkIfArabicNumeral(currentIndex+1) )
			{
				length+=1;
				currentIndex+=1;
			}
			else // All other characters
			{
				if( forbiddenStartToken || forbiddenEndToken || !englishToken )
				{
					length+=1;
					currentIndex+=1;

					if( forbiddenStartToken ) forbiddenStartToken=false;
					if( forbiddenEndToken )   forbiddenEndToken=false;
				}

				if( englishToken )
				{
					englishToken=false;
				}

				tokenEnded = true;
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

void CGUIText::selectTokenSplitMethod()
{
	_tokenSplitMethod = CGUIText::WordBased;

	CGUITextItem* tokenSplitAlgorithm = CGUITextItem::_textMap.findString("languageWrappingAlgorithm");
	if( tokenSplitAlgorithm != NULL )
	{
		UnicodeString charBased("CharBased");
		if( tokenSplitAlgorithm->getTextObj().find(charBased) != -1 )
		{
			_tokenSplitMethod = CGUIText::CharBased;
			_forbiddenCharsInitialized = false;
			initializeForbiddenChars();
			DataLog( log_level_cgui_info ) << "CharBased token split algorithm selected" << endmsg;
		}
		else
		{
			DataLog( log_level_cgui_info ) << "WordBased token split algorithm selected" << endmsg;
		}
	}
	else
	{
		DataLog( log_level_cgui_info ) << "WordBased token split algorithm selected" << endmsg;
	}

	_tokenSplitMethodSelected = true;
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
			DataLog( log_level_cgui_info ) << "Found forbiddenStartCharList in string.info files" << endmsg;
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
			DataLog( log_level_cgui_info ) << "Found forbiddenEndCharList in string.info files" << endmsg;
		}
		else
		{
			DataLog( log_level_cgui_info ) << "Couldn't find forbiddenEndCharList in string.info files" << endmsg;
		}

		_forbiddenCharsInitialized=true;
	}
}

CGUIText::GetTokenResult CGUIText::getToken(int startIndex, bool startOfLine, int & length)
{
	// this flag is false before encountering non-blank token characters
	// and true after encountering non-blank token characters. This allows
	// leading blanks to be included in the token, and the first trailing
	// blank to delimit the token.
	//
	bool lookForTrailingBlank = false;
	bool tokenEnded = false;

	int currentIndex = startIndex;
	length = 0;

	GetTokenResult	result = EndOfString;

	// Check for format command.  These are only allowed at the start of a line.
	//
	if ( startOfLine &&
		  currentIndex < _textString.getLength() - paragraphFormatStart.getLength() )
	{
		if ( _textString[currentIndex] == paragraphFormatStart[0] &&
			  _textString.mid(currentIndex, paragraphFormatStart.getLength()) == paragraphFormatStart )
		{
			int	endOfFormatToken = _textString.find(paragraphFormatEnd, currentIndex+paragraphFormatStart.getLength());
			if ( endOfFormatToken >= 0 )
			{
				endOfFormatToken += paragraphFormatEnd.getLength();
				length = endOfFormatToken - currentIndex;
				tokenEnded = true;
				result = FormatToken;
			}
		}
	}

	// keep moving characters from the text string to the token string until
	// a trailing blank or EOS is found.
	//
	while ( currentIndex < _textString.getLength() &&
			  !tokenEnded )
	{
		if ( _textString[currentIndex] == newlineChar ||
			  _textString[currentIndex] == tabChar )
		{
			// If this character is an explicit newline or tab and the token buffer is empty,
			// return the character.  Otherwise return the token parsed so far, and this character
			// will be returned on the next call to getToken().
			//
			tokenEnded = true;
			if ( currentIndex == startIndex )
			{
				// nothing has been added to the token buffer, just return the single character
				length = 1;
			}
		}
		else if ( _textString[currentIndex] == spaceChar &&
					 lookForTrailingBlank )
		{
			// if this is the delimiting trailing blank, leave the loop and return the token
			tokenEnded = true;
		}
		else
		{
			if ( _textString[currentIndex] != spaceChar )
			{
				lookForTrailingBlank = true;
			}

			// add this character to the token
			length++;
			currentIndex++;
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
	bool	startOfLine = true;
	int	currentIndex = 0;
	int	currentLine = 0;
	int	spacePixelSize = 0;
	int	singleLineHeight = 0;
	int	xMin = 0;
	int	xMax = 0;
	int	yMax = 0;

	CGUIFontId currentFontId = _stylingRecord.fontId;

	_lineData.clear();
	_formatData.firstLineIndent = 0;
	_formatData.secondLineIndent = 0;

	if ( _stylingRecord.fontId != UGL_NULL_ID )
	{
		UGL_SIZE width = 0;
		UGL_SIZE height = 0;
		uglTextSizeGet(getFontId(0x0020), &width, &height, 1," ");
		spacePixelSize = width;
		singleLineHeight = height;
	}

	int	lineStartX = 0;
	int	lineStartY = 0;

	while ( currentIndex < _textString.getLength() )
	{
		int	currentSegmentPixelWidth = 0;
		int	lineStartIndex = currentIndex;
		int	lineByteCount = 0;
		bool  lineDone = false;
		bool	lineEndedByTab = false;

		CGUIRegion lineSize;

		while ( !lineDone )
		{
			int	tokenCharCount;
			if (startOfLine) lineStartX = _formatData.firstLineIndent * _tabSpaceCount * spacePixelSize;

			// get the next blank delimited token

			GetTokenResult tokenResult = EndOfString;

			switch( _tokenSplitMethod )
			{
				case WordBased:
					tokenResult = getToken( currentIndex, startOfLine, tokenCharCount );
					break;

				case CharBased:
					tokenResult = getCharBasedToken( currentIndex, startOfLine, tokenCharCount );
					break;
			}

//			UnicodeString ustr = _textString.mid( currentIndex, tokenCharCount );
//
			if( tokenResult == EndOfString )
			{
				// reached end of string, so this is the last line
				lineDone = true;
				currentIndex = _textString.getLength();
			}
			else if( tokenResult == FormatToken )
			{
				int	parameter_index = currentIndex + paragraphFormatStart.getLength();
				int	firstLineIndent = -1;
				int	secondLineIndent = -1;
				while ( parameter_index < currentIndex + tokenCharCount &&
						  secondLineIndent < 0 )
				{
					if ( _textString[parameter_index] >= (StringChar)'0' &&
						  _textString[parameter_index] <= (StringChar)'9' )
					{
						if ( firstLineIndent < 0 )
						{
							firstLineIndent = _textString[parameter_index] - (StringChar)'0';
						}
						else
						{
							secondLineIndent = _textString[parameter_index] - (StringChar)'0';
						}
					}

					parameter_index += 1;
				}

				if ( firstLineIndent >= 0 &&
					  secondLineIndent >= 0 )
				{
					_formatData.firstLineIndent = firstLineIndent;
					_formatData.secondLineIndent = secondLineIndent;
				}

				currentIndex += tokenCharCount;
				lineStartIndex = currentIndex;
			}
			else if ( _textString[currentIndex] == newlineChar )
			{
				// newline forced a line break
				startOfLine = true;
				currentIndex += tokenCharCount;

				if ( lineByteCount )
				{
					lineDone = true;
				}
				else
				{
					lineStartIndex = currentIndex;
					lineStartY += 3*singleLineHeight/4;
				}
			}
			else if ( _textString[currentIndex] == tabChar )
			{
				startOfLine = false;
				currentIndex += tokenCharCount;

				if ( lineByteCount )
				{
					lineDone = true;
					lineEndedByTab = true;
				}
				else
				{
					lineStartX += _tabSpaceCount * spacePixelSize;
					lineStartIndex = currentIndex;
				}
			}
			else
			{
				startOfLine = false;

				CGUIRegion tokenSize;

				getTokenSize( tokenSize, currentIndex, tokenCharCount );

				if ( convertLinePosition(currentSegmentPixelWidth+tokenSize.width, tokenSize.height, lineStartX, lineSize) )
				{
					// new token fits on line
					lineByteCount += tokenCharCount;
					currentIndex	+= tokenCharCount;
					currentSegmentPixelWidth += tokenSize.width;
				}
				else  // new token doesn't fit on line
				{
					lineDone = true;
					if ( lineByteCount <= 0 )	// token only text for line so must put on line
					{
						// must put token on line
						lineByteCount += tokenCharCount;
						currentIndex   += tokenCharCount;
						currentSegmentPixelWidth += tokenSize.width;
					}
					else
					{
						// update line region information without tokenSize.width
						convertLinePosition(currentSegmentPixelWidth, tokenSize.height, lineStartX, lineSize);
					}
				}
			}
		}	// while line not done

		// Create line_data struct
		//
		if ( lineByteCount > 0 )
		{
			LineData 	currentLineData;

			currentLineData.x = lineSize.x;
			currentLineData.y = lineStartY;

			currentLineData.index = lineStartIndex;
			currentLineData.textLength = lineByteCount;
			currentLineData.fontId = currentFontId;

			// eliminate first char if it is a blank (space character)
			if ( _textString[lineStartIndex] == spaceChar )
			{
				currentLineData.index++;
				currentLineData.textLength--;
			}

			//	Check for conbination of fonts for line data
			//
			semTake(_fontRangeSem, WAIT_FOREVER);
			size_t listSize = _fontRange.size();
			semGive( _fontRangeSem );

			if( listSize > 0 )
			{
				// check to see if font ids exist
				if( !_fontRangeIdsAdded )
				{
					addFontRangeFonts();
				}

				LineData 	fontRangeLineData;
				CGUIRegion fontRangeRegion( 0, 0, 0, 0 );

				fontRangeLineData.x = currentLineData.x;
				fontRangeLineData.y = currentLineData.y;
				fontRangeLineData.index = currentLineData.index;
				fontRangeLineData.textLength = 0; //currentLineData.textLength;
				fontRangeLineData.fontId = currentFontId;

				for( int i = currentLineData.index; i <= (currentLineData.index + currentLineData.textLength); i++ )
				{
					int currentChar = (int)_textString[i];
					CGUIFontId newFontId = getFontId( currentChar );

					if( newFontId == NULL )
						newFontId = _stylingRecord.fontId;

					if(( newFontId != currentFontId ) && ( i != (currentLineData.index + currentLineData.textLength )))
					{
						if( i != currentLineData.index )
						{
                     fontRangeLineData.textLength = i - fontRangeLineData.index;

							_lineData.push_back( fontRangeLineData );

                     //	update  fontRangeLineData to new values
                     getSize( fontRangeRegion, fontRangeLineData.index, fontRangeLineData.textLength, currentFontId );
                     fontRangeLineData.x += fontRangeRegion.width;
                     fontRangeLineData.index = i;
							fontRangeLineData.textLength = 0;	//	This will be automaticaly incremented when it drops out of the if-statement
						}
                  currentFontId = newFontId;
						fontRangeLineData.fontId = newFontId;
					}
					if( i != (currentLineData.index + currentLineData.textLength ))
                   fontRangeLineData.textLength++;	//	increment text length

				} //	for current line data

				_lineData.push_back( fontRangeLineData );
			}
			else
			{
				// associate fontId with line data
				currentLineData.fontId = _stylingRecord.fontId;
				// do what we always did
            _lineData.push_back(currentLineData);
			}

			xMax = (lineSize.x + lineSize.width - 1 > xMax) ? lineSize.x + lineSize.width - 1 : xMax;
			if ( currentLine <= 0 )
			{
				xMin = lineSize.x;
			}
			else
			{
				xMin = (lineSize.x < xMin) ? lineSize.x : xMin;
			}

			yMax = _lineData.back().y + lineSize.height + 1;
			currentLine++;
		}

		if ( !lineEndedByTab )
		{
			lineStartX = _formatData.secondLineIndent * _tabSpaceCount * spacePixelSize;
			lineStartY += singleLineHeight;
		}
		else
		{
			int tab_column = ( lineSize.x + lineSize.width - 1 - _requestedRegion.x ) / ( _tabSpaceCount * spacePixelSize ) + 1;
			lineStartX = tab_column * _tabSpaceCount * spacePixelSize;
		}
	}

	// Setup final region required by the text
	//
	region.x = xMin;
	region.y = 0;
	region.width = xMax - xMin + 1;
	region.height = yMax + 1;

	// Adjust line horizontal positions to left side of region
	//
	list<LineData>::iterator line;
	for ( line=_lineData.begin(); line != _lineData.end(); ++line )
	{
		(*line).x -= region.x;
	}
}


bool CGUIText::convertLinePosition(int width, int height, int indentPixels, CGUIRegion & region)
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
      region.x = _requestedRegion.x + indentPixels;
      available_pixel_width = ( _requestedRegion.width == 0 ) ? _display.width()-region.x : _requestedRegion.width-indentPixels;
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

		CGUIRegion    textRegion;
		convertTextToMultiline(textRegion);

		// find vertical region to place text
		if (_stylingRecord.attributes & VJUSTIFY_CENTER)
		{
			// center vertically within specified region
			if (textRegion.height <= _requestedRegion.height)
			{
				textRegion.vertShift(_requestedRegion.y + (_requestedRegion.height-textRegion.height)/2);
			}
			else
			{
				textRegion.vertShift(_requestedRegion.y);
			}
		}
		else if (_stylingRecord.attributes & VJUSTIFY_BOTTOM)
		{
			// justify at bottom of requested region
			if (textRegion.height <= _requestedRegion.height)
			{
				textRegion.vertShift(_requestedRegion.y + _requestedRegion.height - textRegion.height);
			}
			else
			{
				textRegion.vertShift(_requestedRegion.y);
			}
		}
		else
		{
			// justify at top of requested region
			textRegion.vertShift(_requestedRegion.y);
		}

		setRegion(textRegion);
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
//   uglFontSet(gc, _stylingRecord.fontId);

   list<LineData>::const_iterator   lineIter = _lineData.begin();
   while( lineIter != _lineData.end() )
   {
		LineData lineData;
		lineData.x = (*lineIter).x;
		lineData.y = (*lineIter).y;
		lineData.textLength = (*lineIter).textLength;
		lineData.fontId = (*lineIter).fontId;

		uglFontSet( gc, (*lineIter).fontId );
      uglTextDrawW( gc, (*lineIter).x + _region.x, (*lineIter).y + _region.y, (*lineIter).textLength, &copyTextString[(*lineIter).index] );

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


// Functions for FontRange
bool FontRange::operator == ( const FontRange fontRange ) const
{
	bool result = true;

	do
	{
      if( startIndex != fontRange.startIndex )
		{
			result = false;
			break;
		}

		if( endIndex != fontRange.endIndex )
		{
			result = false;
			break;
		}

	} while( false );   /* only do-while loop once */

	return result;
}

bool FontRange::operator < ( const FontRange fontRange  ) const
{
	bool result = false;

	do
	{
      if( startIndex < fontRange.startIndex )
		{
			result = true;
			break;
		}

		if( startIndex == fontRange.startIndex &&
			 endIndex < fontRange.endIndex )
		{
			result = true;
			break;
		}

	}while( false );	  /* only do-while loop once */

	return result;
}

bool FontRange::operator > ( const FontRange fontRange  ) const
{
	bool result = false;

	do
	{
      if( startIndex > fontRange.startIndex )
		{
			result = true;
			break;
		}

		if( startIndex == fontRange.startIndex &&
			 endIndex > fontRange.endIndex )
		{
			result = true;
			break;
		}

	}while( false );	  /* only do-while loop once */

	return result;
}

