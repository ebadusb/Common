/*
 *	Copyright (c) 2005 by Gambro BCT, Inc.  All rights reserved.
 *
 *  $Header: //BCTquad3/home/BCT_Development/vxWorks/Common/cgui/rcs/cgui_textbox.cpp 1.2 2005/09/30 22:42:12Z rm10919 Exp pn02526 $ 
 *  This file defines the base class for all text boxes in the common GUI.
 *  An object of this class types can be used to generate a text box.
 *  
 *  $Log: cgui_textbox.cpp $
 *  Revision 1.1  2005/07/19 15:01:12Z  pn02526
 *  Initial revision
 *
 */

#include <vxWorks.h>
#include "cgui_window_object.h"
#include "cgui_textbox.h"
#include "cgui_bitmap_info.h"     


// CONSTRUCTOR
CGUITextBox::CGUITextBox  (CGUIDisplay        & display,              // reference to a cguidisplay object for display context
                         CGUIWindow         * parent,                 // pointer to a parent window
                         TextBoxData        & boxData,                // reference to box data for text and bitmaps, rectangles and/or frames
                         bool                 visibility              // box will be constructed as visbile unless otherwise specified here
                        ):CGUIWindow(display)
                         ,_boxData(boxData)
                         ,_bitmap(NULL)
                         ,_rectangle(NULL)
                         ,_frame(NULL)
                         ,_text(NULL)
{
    // Initialize.
    setRegion(CGUIRegion(boxData.left, boxData.top, 0, 0));

    // Initialize graphic entities (setting region of this object in the process).
    if (_boxData.bitmapId)
    {
        // Set up the bitmap object to use in the background.
        // Force the bitmap object to evaluate its own region.
        _bitmap = new CGUIBitmap (_display, CGUIRegion(0,0,0,0), *(_boxData.bitmapId));
        setBitmap();
    }
    if( _boxData.rectangleRegion.width > 0 && _boxData.rectangleRegion.height > 0 )
    {
        // Construct the rectangle object to use in the background.
        _rectangle = new CGUIRectangle( _display, CGUIRegion(0,0,_boxData.rectangleRegion.width,_boxData.rectangleRegion.height), _boxData.color );
        setRectangle();
    }
    if( _boxData.frameRegion.width > 0 && _boxData.frameRegion.height > 0 )
    {
        // Construct the frame object to use in the background.
        if( boxData.frameType == CGUIFrame::CGUI_SIMPLE_FRAME )
        {
            _frame = new CGUIFrame(_display, CGUIRegion(0,0,_boxData.frameRegion.width,_boxData.frameRegion.height), _boxData.shadedColor, _boxData.shadedLineWidth);
        }
        else if( boxData.frameType == CGUIFrame::CGUI_SHADED_FRAME )
        {
            _frame = new CGUIFrame(_display, CGUIRegion(0,0,_boxData.frameRegion.width,_boxData.frameRegion.height),  
                         _boxData.shadedColor, _boxData.unshadedColor, 
                         _boxData.shadedLineWidth, _boxData.unshadedLineWidth);
        }
        setFrame();
    }
    // Now, initialize text (to fit this object, if region is set by any of the above).
    if (_boxData.textItem)
    {
        _stylingRecord = _boxData.textItem->getStylingRecord();  // Use the text item's styling record.
        // Correct the region for placement in the text box.
        getRegion(_stylingRecord.region);
        _stylingRecord.region.x = _boxData.textHMargin;
        _stylingRecord.region.y = _boxData.textVMargin;
        _stylingRecord.region.width -= 2*_boxData.textHMargin;
        _stylingRecord.region.height -= 2*_boxData.textVMargin;
        _text = new CGUIText(_display, _boxData.textItem, &_stylingRecord );
//        setText();
    }
    
    addObjects();

    // Put this window onto the parent window.
    WIN_ATTRIB winAttrib = WIN_ATTRIB_NO_INPUT|WIN_ATTRIB_VISIBLE;
    assert(parent);
    attach(parent, winAttrib);
}

// DESTRUCTOR
CGUITextBox::~CGUITextBox ()
{
   if (_text) delete _text;
   if (_bitmap) delete _bitmap;
   if (_rectangle) delete _rectangle;
   if (_frame) delete _frame;
}

// ATTACH TEXT BOX
// put everything onto this window.
void CGUITextBox::addObjects( )
{
        // The order in which things are attached to the window is determined by the heirarchy.
        for ( int j=0; j<3; j++ )
        {
            switch( _boxData.hierarchy[j] )
            {
                case BITMAP:
                {
                    if( _bitmap ) addObjectToFront( _bitmap );
                    break;
                }
                case RECTANGLE:
                {
                    if ( _rectangle ) addObjectToFront( _rectangle );
                    break;
                }
                case FRAME:
                {
                    if ( _frame ) addObjectToFront( _frame );
                    break;
                }
                default: break;
            }
        }
        if( _text )
        {
             addObjectToFront( _text );
        }
}

// DETACH TEXT BOX
// take everything off of the window.
void CGUITextBox::deleteObjects( void )
{
      if( _bitmap ) deleteObject( _bitmap );
      if ( _rectangle ) deleteObject( _rectangle );
      if ( _frame ) deleteObject( _frame );
      if( _text ) deleteObject( _text );
}

//SET BITMAP
// Loads a new bitmap object for the textbox.
void CGUITextBox::setBitmapObject (CGUIBitmapInfo * bitmapId) // ptr to bitmap object to display behind text
{
   deleteObjects();
   if( _bitmap )
   {
      delete _bitmap;  // delete the old object from memory 
   }
   _boxData.bitmapId = bitmapId;
   if (_boxData.bitmapId)
   {
      _bitmap = new CGUIBitmap (_display, CGUIRegion(0,0,0,0), *_boxData.bitmapId);
      setBitmap();
   }
   else
   {
       _bitmap = NULL;
   }
   addObjects();
}

void CGUITextBox::setBitmapMargins(unsigned short vMargin, unsigned short hMargin) // sets margin, only.
{
    deleteObjects();
    _boxData.bitmapVMargin = vMargin;
    _boxData.bitmapHMargin = hMargin;
    setBitmap();
    addObjects();
}

void CGUITextBox::setBitmap ()
{
    if( _bitmap )
    {
        CGUIRegion thisRegion;
        getRegion(thisRegion);
        // Get bitmap size.
        CGUIRegion bitmapRegion = _bitmap->getRegion();
        unsigned short width = bitmapRegion.width + 2*_boxData.bitmapHMargin;
        unsigned short height = bitmapRegion.height + 2*_boxData.bitmapVMargin;
        // Unconditionally set size of text box to max(bitmap,textbox).
        if( width < thisRegion.width ) width = thisRegion.width;
        if( height < thisRegion.height ) height = thisRegion.height;
        setRegion(CGUIRegion(thisRegion.x, thisRegion.y, width, height));
        // Place bitmap within the text box.
        _bitmap->setRegion( CGUIRegion(_boxData.bitmapHMargin, _boxData.bitmapVMargin, bitmapRegion.width, bitmapRegion.height ) );
    }
}

//SET RECTANGLE
// Loads a new rectangle object for the textbox.
void CGUITextBox::setRectangleRegion (const CGUIRegion & region) // ptr to rectangle object to display behind text.
{     
    _boxData.rectangleRegion = region;
    setRectangle();
    invalidateObjectRegion(_rectangle);
}

void CGUITextBox::setRectangleMargins(unsigned short vMargin, unsigned short hMargin) // margins
{
    _boxData.rectangleVMargin = vMargin;
    _boxData.rectangleHMargin = hMargin;
    setRectangle();
    invalidateObjectRegion(_rectangle);
}

void CGUITextBox::setRectangleColor(CGUIColor color)                // color of the background rectangle
{
    _boxData.color = color;
    setRectangle();
    invalidateObjectRegion(_rectangle);
}

void CGUITextBox::setRectangle ()
{     
      if( _rectangle )
      {
          CGUIRegion thisRegion;
          getRegion(thisRegion);
          // Get rectangle size.
          CGUIRegion rectangleRegion = _boxData.rectangleRegion;
          unsigned short width =  rectangleRegion.width + 2*_boxData.rectangleHMargin;
          unsigned short height =  rectangleRegion.height + 2*_boxData.rectangleVMargin;
          // Unconditionally set size of text box to max(rectangle,textbox).
          if( width < thisRegion.width ) width = thisRegion.width;
          if( height < thisRegion.height ) height = thisRegion.height;
          setRegion(CGUIRegion(thisRegion.x, thisRegion.y, width, height));
          // Place rectangle within text box.
          _rectangle->setRegion( CGUIRegion(_boxData.rectangleHMargin, _boxData.rectangleVMargin, rectangleRegion.width, rectangleRegion.height ) );
      }
}

//SET SIMPLE FRAME
// Sets up a new simple frame object for the textbox.
void CGUITextBox::setSimpleFrameRegion (const CGUIRegion & region)
{     
    _boxData.frameType = CGUIFrame::CGUI_SIMPLE_FRAME; // type of background frame: CGUI_SIMPLE_FRAME
    _boxData.frameRegion = region;
    setFrame();
    invalidateObjectRegion(_frame);
}

void CGUITextBox::setSimpleFrameColor(CGUIColor color)           // color
{
    _boxData.frameType = CGUIFrame::CGUI_SIMPLE_FRAME; // type of background frame: CGUI_SIMPLE_FRAME
    _boxData.shadedColor = color;
    setFrame();
    invalidateObjectRegion(_frame);
}

void CGUITextBox::setSimpleFrameLineWidth(unsigned short lineWidth)  // line width
{
    _boxData.frameType = CGUIFrame::CGUI_SIMPLE_FRAME; // type of background frame: CGUI_SIMPLE_FRAME
    _boxData.shadedLineWidth = lineWidth;
    setFrame();
    invalidateObjectRegion(_frame);
}
 
//SET SHADED FRAME
// Sets up a new shaded frame object for the textbox.
void CGUITextBox::setShadedFrameRegion (const CGUIRegion & region)
{     
    _boxData.frameType = CGUIFrame::CGUI_SHADED_FRAME;           // type of background frame: CGUI_SHADED_FRAME
    _boxData.frameRegion = region;
    setFrame();
    invalidateObjectRegion(_frame);
}

void CGUITextBox::setShadedFrameColors(CGUIColor shadedColor, CGUIColor unshadedColor) // colors     
{
    _boxData.frameType = CGUIFrame::CGUI_SHADED_FRAME;           // type of background frame: CGUI_SHADED_FRAME
    _boxData.shadedColor = shadedColor;
    _boxData.unshadedColor = unshadedColor;
    setFrame();
    invalidateObjectRegion(_frame);
}

void CGUITextBox::setShadedFrameLineWidths(unsigned short shadedLineWidth, unsigned short unshadedLineWidth) // line widths
{
    _boxData.frameType = CGUIFrame::CGUI_SHADED_FRAME;           // type of background frame: CGUI_SHADED_FRAME
	_boxData.shadedLineWidth = shadedLineWidth;
    _boxData.unshadedLineWidth = unshadedLineWidth;
    setFrame();
    invalidateObjectRegion(_frame);
}

//SET FRAME MARGINS
// Used to operate on both types of frame.
void CGUITextBox::setFrameMargins(unsigned short vMargin, unsigned short hMargin) // margins
{
    _boxData.frameVMargin = vMargin;
    _boxData.frameHMargin = hMargin;
    setFrame();
    invalidateObjectRegion(_frame);
}

// If we have a frame around the text box, redraw it.
void CGUITextBox::setFrame ()
{     
      if( _frame )
      {
          CGUIRegion thisRegion;
          getRegion(thisRegion);
          // Get frame size.
          CGUIRegion frameRegion = _boxData.frameRegion;
          unsigned short width =  frameRegion.width + 2*_boxData.frameHMargin;
          unsigned short height =  frameRegion.height + 2*_boxData.frameVMargin;
          // Unconditionally set size of text box to max(rectangle,textbox).
          if( width < thisRegion.width ) width = thisRegion.width;
          if( height < thisRegion.height ) height = thisRegion.height;
          setRegion(CGUIRegion(thisRegion.x, thisRegion.y, width, height));
          // Place frame within text box.
          _frame->setRegion( CGUIRegion(_boxData.frameHMargin, _boxData.frameVMargin, frameRegion.width, frameRegion.height ) );
      }
}


//SET HIERARCHY
void CGUITextBox::setHierarchy( Hierarchy bottom, Hierarchy middle, Hierarchy top )
{
    deleteObjects();
    _boxData.hierarchy[0] = bottom;
    _boxData.hierarchy[1] = middle;
    _boxData.hierarchy[2] = top;
    addObjects();
}


// SET TEXT
// Fit text into the region given by the current background, if any.
void CGUITextBox::setText(void)
{
    CGUIRegion thisRegion;
    getRegion(thisRegion);
    CGUIRegion textRegion = _text ? CGUIRegion(0,0,_text->width(),_text->height()) : CGUIRegion(0,0,0,0);
    // Set text coordinates.
    textRegion.x = _boxData.textHMargin;
    textRegion.y = _boxData.textVMargin;
    if ( thisRegion.width > 0 && thisRegion.height > 0 )
    {
       // if( textRegion.width+2*_boxData.textHMargin > thisRegion.width || textRegion.height+2*_boxData.textVMargin > thisRegion.height )
       // {
            // Fit the text into the box and let it wrap if it will.
//            textRegion.width = thisRegion.width - 2*_boxData.textHMargin;
//            textRegion.height = thisRegion.height - 2*_boxData.textVMargin;
       // }
    }
    else
    {
        // No background to fit to, just set the box dimensions by the text size.
        thisRegion.width = textRegion.width+2*_boxData.textHMargin;
        thisRegion.height = textRegion.height+2*_boxData.textVMargin;
    }
    if( _text )
    {
//        _text->setRegion( textRegion );
        _text->setCaptureBackgroundColor();
    }
}

void CGUITextBox::setText (CGUITextItem * textItem, StylingRecord * stylingRecord)
{
   _boxData.textItem = textItem;
   if (_boxData.textItem)
   {
       if( _text )
       {
           _text->setText(textItem);
           if( stylingRecord ) _text->setStylingRecord( stylingRecord );
       }
       else
       {
          _text = new CGUIText(_display, textItem, stylingRecord );
//          setText();
          addObjectToFront( _text );
       }
       _text->setVisible(true);
   }
}

void CGUITextBox::setText (const char * string)
{
    if (string)
    {
       if (_text)
       {
          _text->setText(string);
       }
       else
       {
          _text = new CGUIText(_display);
          _text->setText(string);
          addObjectToFront( _text );
       }      
       _text->setVisible(true);
    }
}

// SET TEXTSTYLE
// Set/change the style of the text associated with 
// this textbox.  This is a pass-thru to the 
// text object previously set with this textbox.  
void CGUITextBox::setStylingRecord (StylingRecord * textStylingRecord) // style record with appropriate features set
{
   if (_text)
   {
//       _text->setVisible(false);
//       deleteObjects();
//       CGUIRegion thisRegion;
//       getRegion(thisRegion);
//       StylingRecord sr;
//       sr.color = textStylingRecord->color;
//       sr.attributes = textStylingRecord->attributes;
//       if( thisRegion.width == 0 || thisRegion.height == 0)
//           sr.region =  textStylingRecord->region;
//       else
//           sr.region = thisRegion; // Region is controlled by the box size, not by the styling record.
//       sr.fontId = textStylingRecord->fontId;
//       sr.fontSize = textStylingRecord->fontSize;
//       _text->setStylingRecord( &sr );
       _text->setStylingRecord( textStylingRecord );
//       setText();
//       addObjects();
//       _text->setVisible(true);
   }
}

// SET TEXTCOLOR
void CGUITextBox::setTextColor(CGUIColor color)
{
   if (_text)
   {
       _text->setColor(color);
   }
}
