/*
 *	Copyright (c) 2004 by Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: L:/vxWorks/Common/cgui/rcs/cgui_text.h 1.7 2004/11/18 22:31:41Z rm10919 Exp cf10242 $
 * $Log: cgui_text.h $
 * Revision 1.3  2004/09/30 17:00:52Z  cf10242
 * Correct for initial make to work
 * Revision 1.2  2004/09/28 19:47:27Z  rm10919
 * Missed naming changes to CGUI.
 * Revision 1.1  2004/09/20 18:18:09Z  rm10919
 * Initial revision
 *
 */

//
// This file contains the class used to wrap the underlying text of a
// window.  The intent is to limit dependencies on the graphics
// system to this header file and to the implementation files associated
// with it. All interaction with the window text from the application
// should be through the publically accessible functions and data defined
// in this file.
//

#ifndef _CGUI_TEXT_INCLUDE
#define _CGUI_TEXT_INCLUDE

#include <vxworks.h>
#include <map>
#include <list>
#include <string>
#include <stdio.h>

#include <ugl/ugl.h>
#include <ugl/uglwin.h>
#include <ugl/uglinput.h>

#include "cgui_graphics.h"
#include "cgui_window_object.h"


class CGUIWindow;
class CGUIText : public CGUIWindowObject
{
public:
   //
   //  These are the attributes a text string can have.
   //  They specify how the text is justified or aligned
   //  and what style type it has in it's CGUIRegion.  
   //
   enum
   {
      VJUSTIFY_TOP = 0x0000, VJUSTIFY_CENTER = 0x0001, VJUSTIFY_BOTTOM = 0x0002
   };    // vertical justification options
   enum
   {
      HJUSTIFY_LEFT = 0x0000, HJUSTIFY_CENTER = 0x0010, HJUSTIFY_RIGHT = 0x00020
   };    // horizontal justification options

   enum
   {
      NTS = 0x0000, BOLD = 0x0100, ITALIC = 0x0200, BOLD_ITALIC = 0x0300
   };    // text style options, this may be controled by the font or the font driver.  
         // NTS - no text style, default attribute.

   //
   //  The LEFTTORIGHT and RIGHTTOLEFT attributes are for 
   //  specifing from which direction the text is read.
   //  This will only become an issue when a language is added
   //  that is read from right to left instead of left to right.
   //  The default IS LEFTTORIGHT.
   //
   enum
   {
      LEFT_TO_RIGHT = 0x0000, RIGHT_TO_LEFT = 0x1000
   };    // text direction options
   
   struct StylingRecord
   {
      CGUIColor     color;       // color of the text as applied to whole string
      unsigned int  attributes;  // attributes from the attribute enums above
      CGUIRegion    region;      // placement in window or _owner relative
      CGUIFontId    fontId;      // font used, this should use the _owner default value
      int           fontSize;    // size of font most of the time
   };

   //
   // Constructors
   //
   CGUIText(CGUIDisplay & display, CGUIWindow * parent);
   CGUIText(CGUIDisplay & display, CGUIWindow * parent, TextItem * textItem, StylingRecord * stylingRecord = NULL);
   CGUIText(CGUIDisplay & display, CGUIWindow * parent, TextItem * textItem, CGUIColor backgroundColor, StylingRecord * stylingRecord = NULL);

   //
   // Destructor
   //
   virtual ~CGUIText();

   //
   // SET_ATTRIBUTES
   // This methods set the attributes of the text.
   // First bit sets vertical justification. Default value is TOP.
   // Second bit sets the horizontal justification. Default value is LEFT.
   // Third bit sets the style (no text style (NTS), BOLD, ITALIC, BOLDITALIC.  Default value is NTS. Note: if BOLD and ITALIC are added together you still get BOLDITALIC.
   // Fourth bit is for text direction. Is it read LEFTTORIGHT or RIGHTTOLEFT? Default is LEFTTORIGHT.
   //
   void setAttributes(unsigned int attributes);
   unsigned int getAttributes(void) { return _stylingRecord.attributes;}

   //
   // SET_BACKGROUND_COLOR
   // This method sets the background color for the text.
   //
   void setBackgroundColor(CGUIColor color);
   
   //
   // SET_COLOR
   // These methods set the text color by sending on CGUIColor
   // or by sending the RGB numbers.
   //
   void setColor( CGUIColor color);
   void setColor(int red, int green, int blue);
   CGUIColor getColor(void) { return _stylingRecord.color;}

   //
   // SET_CAPTURE_BACKGROUND_COLOR
   // This method determines if the background is determined by
   // by this class' draw method or if the background color
   // is set by the CGUIColor passed in the constructor.  This method
   // is primarily for determining the text background color for the
   // simulator.
   //
   void setCaptureBackgroundColor(void) { _captureBackgroundColor = true; _backgroundColorSet = false;}

   //
   // SET_FONT_ID
   // This methods sets the font for the text.
   // The font used may dictate some styling
   // attributes, e.g., BOLD and ITALIC, of the text.
   // The font is also dependent on the language
   // to be displayed.
   //
   void setFontId(CGUIFontId fontId);
   CGUIFontId getFontId(void) { return _stylingRecord.fontId;}

   //
   // SET_FONT_SIZE
   // This method sets the font size for the text. 
   //
   void setFontSize(int fontSize);
   int getFontSize(void) { return _stylingRecord.fontSize;}

   //
   // SET_LANGUAGE
   // This methods sets the language the text will use.
   // These is an interdependency between the font and the 
   // language.
   //
   void setLanguage(LanguageId configLanguage);
   LanguageId getLanguage(void) { return _configLanguage;}

   //
   // SET_REGION
   // This method sets the region by x, y, Width, and height
   // for the text.
   //
//   void setRegion(CGUIRegion region);
   CGUIRegion getRegion(void) { return _stylingRecord.region;}


   //
   // SET_STYLING_RECORD
   // This method sets the styling record of the 
   // text.  Use this method if several members
   // of the styling record need to be set instead
   // calling each individual method.
   //
   void setStylingRecord(StylingRecord * stylingRecord);
   void getStylingRecord(StylingRecord copyofStylingRecord) { copyofStylingRecord = _stylingRecord;};

   //
   // SET_TEXT
   // This methods sets the text string.  When using this method
   // only use strings from the text database.
   //
   void setText(TextItem * textItem);
   void setText(const char * string);
   void setText(const StringChar * string);
   void getText(char &bufferPtr);

protected:

   //
   // These methods are used to create the text lines
   // as they will appear in the window.  Together they
   // work as the text wrapping method for the text.
   //
   void getSize(CGUIRegion & region, int startIndex = -1, int length = -1);
   int  getToken(int start_index);
   void convertTextToMultiline(CGUIRegion & region);
   bool convertLinePosition(int width, int height, CGUIRegion & region);
   void computeTextRegion(void);


   virtual void draw(UGL_GC_ID gc);

private:
   //
   // Text is never clipped by another window object and
   // always appears on top.
   //
   virtual bool clipSiblings(void) const { return false;}

   //
   // Initialize values for text string object.
   //
   void initializeData(CGUIWindow * parent, TextItem * textItem, StylingRecord * stylingRecord);
   
   //
   // Substitute value in for varible in text string object.
   //
   void handleVariableSubstitution(void);
protected:
   //
   // This structure holds the individual text
   // for each line.  It is used to wrap the 
   // text in the display area.
   //
   struct LineData
   {
      short x, y;                   // offset from top left corner of text region to top left corner of line
      unsigned short index;         // index into text string for start of line
      unsigned short textLength;    // number of text characters on line
                                    // the ofsset for RIGHTTOLEFT text string may have a negative offset.
   };

   TextItem       _textItem;        // This is the string id or the key to the string id
   StylingRecord  _stylingRecord;   // holds the styling record for an object

   list<LineData> _lineData;        // list of text lines for object
   unsigned short _stringLength;

   //
   // These are used to determine the background color for the text.
   //
   bool           _captureBackgroundColor;
   bool           _backgroundColorSet;
   CGUIColor      _backgroundColor;

   bool           _languageSetByApp; // flag for determining if _textString needs updating based on language
private:
   // This holds the actual text for the string
   // from the text string database.
   // It may have embedded styling infromation
   // for the string.
   StringChar *   _textString;

   CGUIRegion     _requestedRegion; // used to determine area for LineData
   LanguageId     _configLanguage;  // language of text string, need to know where to look for string. Not sure if this is need _textItem._languageId may be used.
};
#endif /* #ifndef _CGUI_TEXT_INCLUDE */
