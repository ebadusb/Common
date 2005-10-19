/*
 *	Copyright (c) 2004 by Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: K:/BCT_Development/vxWorks/Common/cgui/rcs/cgui_text.cpp 1.27 2006/07/12 23:36:07Z rm10919 Exp jl11312 $
 * $Log: cgui_text.cpp $
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
#include "cgui_variable_db_container.h"
#include "datalog_levels.h"

static StringChar newline_char = '\n';
static StringChar space_char = ' ';
static StringChar null_char = '\0';

const int textBlockSize = 16;

UGL_ORD option;

int currentLanguage = 0;

CGUIVariableDatabaseContainer CGUIText::_variableDictionary;

CGUIText::CGUIText(CGUIDisplay & display)
: CGUIWindowObject(display)
{   

}

CGUIText::CGUIText(CGUIDisplay & display, CGUITextItem * textItem, StylingRecord * stylingRecord = NULL)
                  :CGUIWindowObject(display), _textString(NULL), _pixelRegion(0, 0, 0, 0)
{
   initializeData(textItem, stylingRecord);
}

CGUIText::CGUIText(CGUIDisplay & display, CGUITextItem * textItem, CGUIColor backgroundColor, StylingRecord * stylingRecord = NULL)
                  :CGUIWindowObject(display), _textString(NULL), _pixelRegion(0, 0, 0, 0)
{
   initializeData(textItem, stylingRecord);
   setBackgroundColor(backgroundColor);
}

CGUIText::~CGUIText()
{
   if (_textString)
   {
      delete _textString;
      _textString = NULL;
   }
}

void CGUIText::initializeData(CGUITextItem * textItem, StylingRecord * stylingRecord)
{
   _textItem =  textItem;

   if (_textItem)
   {
      if (_textItem->isInitialized())
      {
         const StringChar * string = _textItem->getText(_textItem->getLanguageId());

         //
         //  If styling record from constructor is null, 
         //  set the the cgui_text._stylingRecord to 
         //  the cgui_text_item._stylingRecord by default.
         //
         if (!stylingRecord)
         {
            _stylingRecord = _textItem->getStylingRecord();

         }
         else
            _stylingRecord = * stylingRecord;

         // Set the requested region.
         _requestedRegion = _stylingRecord.region;

         if (string)
         {
            setText(string);
         }
         else
         {
            _textString = new StringChar[textBlockSize+1];
            *_textString =  null_char;
            _stringLength = 0;
            _stringSize = textBlockSize;
            //DataLog(log_level_cgui_info) << "Creating new text block " << __LINE__ << "  New length = " << _stringSize << endmsg; 
         }
      }
   }
   _languageSetByApp = false;
}

void CGUIText::setAttributes(unsigned int attributes)
{
   _stylingRecord.attributes = attributes;
   computeTextRegion();
   _owner->invalidateObjectRegion(this);
}

void CGUIText::setBackgroundColor(CGUIColor color)
{
   _backgroundColorSet = true;
   _backgroundColor = color;
}

void CGUIText::setColor(CGUIColor color)
{
   _stylingRecord.color = color;
   _owner->invalidateObjectRegion(this);
}

void CGUIText::setColor(int red, int green, int blue)
{
   _stylingRecord.color = MakeCGUIColor(red, green, blue);
   _owner->invalidateObjectRegion(this);
}

void CGUIText::setFontId(CGUIFontId fontId)
{
   _stylingRecord.fontId = fontId;
   computeTextRegion();
   _owner->invalidateObjectRegion(this);
}

void CGUIText::setFontSize(int fontSize)
{
   _stylingRecord.fontSize = fontSize;
   computeTextRegion();
   _owner->invalidateObjectRegion(this);
}

void CGUIText::setLanguage(LanguageId configLanguage)
{  
   //
   // This animal will look different as the
   // language module is developed.
   //
   _configLanguage = configLanguage;
   _textItem->setLanguageId(configLanguage);
   _languageSetByApp = true;
}

void CGUIText::setStylingRecord (StylingRecord * stylingRecord)
{
   _stylingRecord = * stylingRecord;
   _requestedRegion = stylingRecord->region;
   computeTextRegion();
   if (_owner)
      _owner->invalidateObjectRegion(this);
}

void CGUIText::setText(CGUITextItem * textItem)
{
   if (textItem)
   {
      _textItem =  textItem;

      if (_textItem->isInitialized())
      {
         const StringChar * string = _textItem->getText(_textItem->getLanguageId());

         if (string)
         {
            setText(string);
         }
         else if (!_textString)
         {
            //DataLog(log_level_cgui_info) << "Creating new text block " << __LINE__ << "  New length = " << _stringSize << endmsg; 
            _textString = new StringChar[textBlockSize+1];
            *_textString =  null_char;
            _stringLength = 0;
            _stringSize = textBlockSize;
         }
      }
   }
}

void CGUIText::setText()
{
   setText(_textItem->getText(_textItem->getLanguageId()));
}

void CGUIText::setText(const StringChar * string)
{
   int newLength = 0;
   if (string)
   {
      while (string[newLength])
         newLength += 1;

      if (_textString)
      {
         // find length of new string and compare to old
         if (newLength > _stringSize)
         {
            delete _textString;                                  
            _textString = new StringChar[newLength+textBlockSize+1];
            //DataLog(log_level_cgui_info) << "Deleting/Creating new text block " << __LINE__ << " Old length = " << _stringSize << "  New length = " << newLength+textBlockSize << endmsg; 
            _stringLength = newLength;
            _stringSize = newLength+textBlockSize;
         }
      }
      else
      {
         _stringSize = newLength;
         if (newLength < textBlockSize)
            _stringSize = textBlockSize;

         //DataLog(log_level_cgui_info) << "Creating new text block " << __LINE__ << "  New length = " << _stringSize << endmsg; 
         _textString = new StringChar[_stringSize+1];
      }

      _stringLength = newLength;
      memcpy(_textString, string, _stringLength * sizeof(StringChar));

      _textString[_stringLength] = null_char;  // add the NULL StringChar
   }
   else
   {
      setRegion(CGUIRegion(0, 0, 0, 0));
   }
   computeTextRegion();

}

void CGUIText::setText(const char * string)                 
{
   if (string)
   {
      int newLength = strlen(string)+1;
      //DataLog(log_level_cgui_info) << "Size of char string = " << strlen(string) << endmsg;

      if (_textString)
      {
         // find length of new string and compare to old
         if (newLength > _stringSize)
         {
            delete _textString;                                  
            _textString = new StringChar[newLength+textBlockSize+1];
            //DataLog(log_level_cgui_info) << "Deleting/Creating new text block " << __LINE__ << " Old length = " << _stringSize << "  New length = " << newLength+textBlockSize << endmsg; 
            _stringLength = newLength;
            _stringSize = newLength+textBlockSize;
         }
      }
      else
      {
         _stringSize = newLength;
         if (newLength < textBlockSize)
            _stringSize = textBlockSize;

         //DataLog(log_level_cgui_info) << "Creating new text block " << __LINE__ << "  New length = " << _stringSize << endmsg;
         _textString = new StringChar[_stringSize+1];
      }
      _stringLength = newLength;

      _textString = convertToStringChar(string);

//      for (int i=0; i<_stringLength; i++)
//         _textString[i] = string[i];

//      _textString[_stringLength] = '\0';
   }
   else
   {
      setRegion(CGUIRegion(0, 0, 0, 0));
   }

   computeTextRegion();

} // END set_text

void CGUIText::getText(StringChar * string)
{
   // this makes a copy of the text.  consider changing to just send point to texString
   memcpy(string, _textString, _stringLength * sizeof(StringChar));
   string[_stringLength] = null_char;
}

void CGUIText::getSize(CGUIRegion & region, int startIndex, int length)
{
   if (_stylingRecord.fontId == UGL_NULL_ID ||
       !_stringLength ||
       startIndex >= _stringLength)
   {
      region = CGUIRegion(0, 0, 0, 0);
   }
   else
   {
      UGL_SIZE uglStartIndex = startIndex;
      UGL_SIZE uglLength = length;

      if (uglStartIndex < 0)
      {
         uglStartIndex = 0;
      }

      if (uglLength < 0 || uglStartIndex+uglLength > _stringLength)
      {
         uglLength = _stringLength - startIndex;
      }

      UGL_SIZE width = 0;
      UGL_SIZE height = 0;

      if (_stylingRecord.attributes & BOLD)
      {
//         option = AGFA_ITYPE_BOLD_OFF;
         option = 0;

         uglFontInfo(_stylingRecord.fontId, UGL_FONT_WEIGHT_SET, &option);
      }
      uglTextSizeGetW(_stylingRecord.fontId, &width, &height, uglLength, &_textString[uglStartIndex]);
      region = CGUIRegion(0, 0, width, height);
   }
} // END get_size

const CGUIRegion & CGUIText::getPixelSize()
{   
   getSize(_pixelRegion, 0, _stringLength);
   return _pixelRegion;
}

int CGUIText::getToken(int start_index)
{
   // this flag is false before encountering non-blank token characters
   // and true after encountering non-blank token characters. This allows
   // leading blanks to be included in the token, and the first trailing
   // blank to delimit the token.
   bool look_for_trailing_blank = false;
   bool token_ended = false;

   int current_index = start_index;
   int token_char_count = 0;

   // keep moving characters from the text string to the token string until
   // a TRAILING blank or EOS is found.
   while (_textString[current_index] != 0 && !token_ended)
   {

      // if this character is an explicit newline and the token buffer is empty, return
      // the newline, otherwise if this character is an explicit newline and the token
      // buffer contains something, return the contents of the token buffer. In this
      // manner, newlines will be returned alone to the multiline manager.
      if (_textString[current_index] == newline_char)
      {
         if (current_index == start_index) // nothing has been added to the token buffer, just return the Newline 
         {
            // add the newline to the token
            token_char_count = 1; 

         }
         token_ended = true;
      }

      // if this is the delimiting trailing blank, leave the loop and return the token
      else if ((_textString[current_index] == space_char) 
               &&
               (look_for_trailing_blank))
      {
         token_ended = true;
      }
      else
      {
         if (_textString[current_index] != space_char)
         {
            look_for_trailing_blank = true;
         }
         // add this character to the token
         token_char_count++;
         current_index++;
      }
   }
   return token_char_count;
} // END get_token

void CGUIText::convertTextToMultiline(CGUIRegion & region)
{
   int   current_index = 0;
   int   current_line = 0;
   int  x_min = 0;
   int  x_max = 0;
   int  y_max = 0;

   _lineData.clear();

   while (current_index < _stringLength)
   {
      int    current_segment_pixel_width = 0;
      int    line_start_index = current_index;
      int    line_byte_count = 0;
      int    blank_line_count = 0;
      bool   line_done = false;
      bool   foce_line = false;

      CGUIRegion line_size;

      while (!line_done)
      {
         int   token_byte_count;

         // get the next blank delimited token
         token_byte_count = getToken(current_index);
         if (token_byte_count == 0)
         {
            // reached end of string, so this is the last line
            line_done = true;
            current_index = _stringLength;
         }
         else
         {
            if (_textString[current_index] == newline_char) //'\n')
            {
               // newline forced a line break
               current_index += token_byte_count;
               if (line_byte_count)
               {
                  line_done = true;
               }
               else
               {
                  line_start_index = current_index;
                  blank_line_count += 1;
               }
            }
            else
            {
               // not a newline, see if token will fit on current line
               CGUIRegion token_size;
               getSize(token_size, current_index, token_byte_count);

               if (convertLinePosition(current_segment_pixel_width+token_size.width, token_size.height, line_size))
               {
                  // new token fits on line
                  line_byte_count += token_byte_count;
                  current_index   += token_byte_count;
                  current_segment_pixel_width += token_size.width;
               }
               else
               {
                  line_done = true;
                  if (line_byte_count == 0)
                  {
                     // must put token on line
                     line_byte_count += token_byte_count;
                     current_index   += token_byte_count;
                     current_segment_pixel_width += token_size.width;
                  }
                  else
                  {
                     convertLinePosition(current_segment_pixel_width, token_size.height, line_size);
                  }
               }
            }
         }
      }

      //
      // Create line_data struct
      //
      if (line_byte_count > 0)
      {
         LineData    current_line_data;

         current_line_data.x = line_size.x;
         if (current_line == 0)
         {
            current_line_data.y = blank_line_count*line_size.height;
         }
         else
         {
            current_line_data.y = _lineData.back().y + (blank_line_count+1)*line_size.height;
         }

         current_line_data.index = line_start_index;
         current_line_data.textLength = line_byte_count;
         if (_textString[line_start_index] == space_char)
         {
            current_line_data.index++;
            current_line_data.textLength--;
         }

         _lineData.push_back(current_line_data);

         x_max = (line_size.x + line_size.width - 1 > x_max) ? line_size.x + line_size.width - 1 : x_max;
         if (current_line == 0)
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
   }

   //
   // Setup final region required by the text
   //
   region.x = x_min;
   region.y = 0;
   region.width = x_max - x_min + 1;
   region.height = y_max + 1;

   //
   // Adjust line horizontal positions to left side of region
   //
   list<LineData>::iterator line;
   for (line=_lineData.begin(); line != _lineData.end(); ++line) (*line).x -= region.x;
} // END convert_text_to_multiline


bool CGUIText::convertLinePosition(int width, int height, CGUIRegion & region)
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
      // LEFT alignment: text will start at _requestedRegion.x
      region.x = _requestedRegion.x;
      available_pixel_width = ( _requestedRegion.width == 0 ) ? _display.width()-region.x : _requestedRegion.width;
   }

   region.y = 0;
   region.width = width;
   region.height = height;

   return( width <= available_pixel_width );
} // END convert_line_position


void CGUIText::computeTextRegion(void)
{
   CGUIRegion    text_region;
   if (_stringLength > 0) handleVariableSubstitution();
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
} // END compute_text_region

void CGUIText::setRegion(const CGUIRegion & newRegion)
{
   if (_owner)
      _owner->setObjectRegion(this, newRegion);
   else
      _region = newRegion;
}

void CGUIText::draw(UGL_GC_ID gc)
{
   if (!_stringLength ||
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

   uglBackgroundColorSet(gc, backgroundColor);
   uglForegroundColorSet(gc, _stylingRecord.color);
   uglFontSet(gc, _stylingRecord.fontId);

   list<LineData>::const_iterator   lineIter = _lineData.begin();
   while (lineIter != _lineData.end())
   {
      uglTextDrawW(gc, (*lineIter).x + _region.x, (*lineIter).y + _region.y, (*lineIter).textLength, &_textString[(*lineIter).index]);
      ++lineIter;
   }
} // END draw

void CGUIText::handleVariableSubstitution(void)
{
   size_t newStringSize = _stringLength+1;
   StringChar * newTextString = (StringChar *)malloc(newStringSize * sizeof(StringChar));
   size_t newStringLength = 0;
   size_t idx = 0;

   while (_textString[idx] != null_char &&
          idx < _stringLength)
   {
      // Check for start of variable substitution string
      //
      if (_textString[idx] == (wchar_t)'#' &&
          _textString[idx+1] == (wchar_t)'!' &&
          _textString[idx+2] == (wchar_t)'{')
      {
         // Find ending '}' character if any
         //
         size_t subStartIdx = idx+3;
         size_t subEndIdx = subStartIdx;

         while (_textString[subEndIdx] != null_char &&
                _textString[subEndIdx] != (wchar_t)'}' &&
                subEndIdx < _stringLength)
         {
            subEndIdx += 1;
         }

         if (_textString[subEndIdx] == '}' &&
             subEndIdx-subStartIdx > 0)
         {
            // Have a valid variable substitution string - lookup the value
            //

            char  * variableName = new char[subEndIdx-subStartIdx+1];

            for (int i=0; i<subEndIdx-subStartIdx; i++)
            {
               variableName[i] = (char)(_textString[subStartIdx+i] & 0x00ff);
            
            }

            variableName[subEndIdx-subStartIdx] = '\0';
            StringChar * variableText = _variableDictionary.variableLookUp(variableName);
            delete[] variableName;

            int variableTextLength = 0;
            if (variableText)
            {
               while (variableText[variableTextLength])
                  variableTextLength += 1;
               
               // Value is present, copy to the string and setup to continue with
               // next character after substitution string
               //
               newStringSize += (variableTextLength + textBlockSize);
               newTextString = (StringChar *)realloc(newTextString, (newStringSize * sizeof(StringChar) + textBlockSize));

               for (int i=0; i<variableTextLength; i++)
               {
                  newTextString[newStringLength++] = (StringChar)variableText[i];
               }

               idx = subEndIdx + 1;
               continue;
            }
         }
      }

      newTextString[newStringLength++] = _textString[idx++];
   }
   if (_textString) delete[] _textString;
   _textString = new StringChar[newStringLength+1];
   memcpy(_textString, newTextString, newStringLength * sizeof(StringChar));
   _textString[newStringLength] = null_char;
   _stringLength = newStringLength;
   _stringSize = newStringLength + textBlockSize;

   free(newTextString);
   newTextString = NULL;
}

