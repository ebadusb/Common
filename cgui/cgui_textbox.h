/*
 *	Copyright (c) 2005 by Gambro BCT, Inc.  All rights reserved.
 *
 *  $Header: //BCTquad3/home/BCT_Development/vxWorks/Common/cgui/rcs/cgui_textbox.h 1.1 2005/07/15 21:18:53Z pn02526 Exp pn02526 $ 
 *  This file defines the base class for all text box styles in the common GUI.  An object of this class types
 *  can be used to generate a standard text box.
 *  
 *  $Log: cgui_textbox.h $
 *
 */
#ifndef _CGUI_TEXTBOX_INCLUDE
#define _CGUI_TEXTBOX_INCLUDE

#include "cgui_window.h"
#include "cgui_text.h"
#include "cgui_bitmap.h"
#include "cgui_rectangle.h"
#include "cgui_frame.h"
#include "datalog_levels.h"

// CGUITextBox is the parent class for all text box styles.  Specialized text boxes
// can derive from this class and pick up many of the standard behaviors of
// a text box, but override some features for specialized behavior.
// The public methods of the class are grouped below into categories based on the things they affect.
// The groups are: bitmaps, filled rectangles, frames, text, and background hierarchy.
// There are some basic rules in this class that are important to its usage:
// 1) A text box can be bordered by a frame.
// 2) A text box can have a bitmap and/or filled rectangle for a background.
// 3) The heirachy of background objects is under programmer control.
// 4) There may be zero or one text items associated with the text box.
// 5) The text item is always in front of the background objects (bitmap, recangle, and/or frame).
// 6) The text item is not force fitted to the text box's region.
// 7) A text box's region is defined by the BoxData structure passed to its constructor:
//      Position coordinates are as given in the structure,
//      The extent is derived from the maximal intersection of the
//      background object(s) (bitmap, rectangle,frame).i
//      If no background objects then the extent of the text item is used.

class CGUITextBox : public CGUIWindow
{    
public:

  typedef enum Hierarchy { VACANT, BITMAP, RECTANGLE, FRAME };

   struct TextBoxData
   {
      unsigned short left;               // position of top/left corner of text box in screen pixels
      unsigned short top;
      
      // TEXT DATA
      unsigned short textVMargin;        // vertical and horizontal margins from the edges
      unsigned short textHMargin;        //  of the text box to the sides of the text item's region.
      CGUITextItem * textItem;           // label text (if any)

      // BITMAP DATA
      unsigned short bitmapVMargin;      // vertical and horizontal margins from the edges
      unsigned short bitmapHMargin;      // of the text box to the sides of the background bitmap's region.
      CGUIBitmapInfo * bitmapId;         // background bitmap id

      // RECTANGLE DATA
      unsigned short rectangleVMargin;   // vertical and horizontal margins from the edges
      unsigned short rectangleHMargin;   // of the text box to the sides of the background rectangle's region.
      CGUIRegion rectangleRegion;        // region of the background rectangle; for sizing only as x&y are ignored
      CGUIColor color;                   // fill color of the background rectangle

      // FRAME DATA
      unsigned short frameVMargin;       // vertical and horizontal margins from the edges
      unsigned short frameHMargin;       // of the text box to the sides of the background frame's region.
      CGUIFrame::CGUIFrameType frameType;// type of background frame: CGUI_SIMPLE_FRAME or CGUI_SHADED_FRAME
      CGUIRegion frameRegion;            // region of the background frame; for sizing only as x&y are ignored
      CGUIColor shadedColor;             // shaded (CGUI_SHADED_FRAME) or only (CGUI_SIMPLE_FRAME) color
      CGUIColor unshadedColor;           // unshaded color (CGUI_SHADED_FRAME) or ignored (CGUI_SIMPLE_FRAME)
      unsigned short shadedLineWidth;    // shaded (CGUI_SHADED_FRAME) or only (CGUI_SIMPLE_FRAME) line width
      unsigned short unshadedLineWidth;  // unshaded line width (CGUI_SHADED_FRAME) or ignored (CGUI_SIMPLE_FRAME)

      Hierarchy hierarchy[3];            // Of BITMAP, RECTANGLE, FRAME, which is 1st, 2nd, 3rd.
   };


protected:

    TextBoxData             _boxData;
   
   CGUIText                *_text;               // ptr to current text object component
   StylingRecord            _stylingRecord;

   CGUIBitmap              *_bitmap;             // ptr to current bitmap object component

   CGUIRectangle           *_rectangle;          // ptr to current rectangle object component

   CGUIFrame               *_frame;              // ptr to current frame object component

public:
   // CONSTRUCTOR
   CGUITextBox  (CGUIDisplay   & display,                // reference to a cguidisplay object for display context
                CGUIWindow    * parent,                  // pointer to the parent window
                TextBoxData    & textBoxData,            // reference to text box data for background(s), text
                bool            visible = true         // text box will be constructed as visbile unless otherwise specified here
                );

   // DESTRUCTOR
   virtual ~CGUITextBox ();

   // VISIBILITY
   // make the entire text box visible (TRUE) or invisible (FALSE).
   inline void setVisible(bool TorF) { setTextVisible(TorF); setBitmapVisible(TorF); setRectangleVisible(TorF); setFrameVisible(TorF); };

   // IS VISIBLE
   // return flag indicating current state of text box visibility.
   // A text box is considered visible if any of its components is visible.
   inline bool isVisible() const { return isTextVisible() || isBitmapVisible() || isRectangleVisible() || isFrameVisible(); };

   inline void setTextVisible(bool TorF) { if (_text) _text->setVisible(TorF); };
   inline void setBitmapVisible(bool TorF) { if (_bitmap) _bitmap->setVisible(TorF); };
   inline void setRectangleVisible(bool TorF) { if (_rectangle) _rectangle->setVisible(TorF); };
   inline void setFrameVisible(bool TorF) { if (_frame) _frame->setVisible(TorF); };

   inline bool isTextVisible() const { if (_text) return _text->isVisible(); else return false; };
   inline bool isBitmapVisible() const { if (_bitmap) return _bitmap->isVisible(); else return false; };
   inline bool isRectangleVisible() const { if (_rectangle) return _rectangle->isVisible(); else return false; };
   inline bool isFrameVisible() const { if (_frame) return _frame->isVisible(); else return false; };

   // BACKGROUND METHODS //

   // SET HIERARCHY
   // sets the hierarchy for displaying background objects.
   void setHierarchy( Hierarchy bottom, Hierarchy middle, Hierarchy top );

   //SET BITMAP
   // loads a new bitmap object for the text box.   If none exists, implicitily creates one.
   void setBitmapObject(CGUIBitmapInfo * bitmapInfo); // set bitmap object to display in the background of the text box
   void setBitmapMargins(unsigned short bitmapVMargin, unsigned short bitmapHMargin); // sets margin, only.
   inline void setBitmap(CGUIBitmapInfo * bitmapInfo,  // set bitmap object to display in the background of the text box
                  unsigned short bitmapVMargin,
                  unsigned short bitmapHMargin
                 ) {setBitmapMargins(bitmapVMargin, bitmapHMargin);
                    setBitmapObject(bitmapInfo); };


   //SET RECTANGLE
   // sets a background rectangle object for the text box. If none exists, implicitily creates one.
   void setRectangleRegion(const CGUIRegion & region);      // region of the background rectangle; for sizing only as x&y are ignored
   void setRectangleMargins(unsigned short vMargin, unsigned short hMargin);  // margins
   void setRectangleColor(CGUIColor color);                // color of the background rectangle
   inline void setRectangle(const CGUIRegion & region,       // region of the background rectangle; for sizing only as x&y are ignored
                     unsigned short vMargin,
                     unsigned short hMargin,
                     const CGUIColor color                  // color of the background rectangle
                    ) { setRectangleRegion(region);
                        setRectangleMargins(vMargin,hMargin);
                        setRectangleColor(color); };

   //SET FRAME
   void setFrameMargins(unsigned short vMargin, unsigned short hMargin);

   // sets a new Simple Frame for the text box.  If none exists, implicitily creates one.
   void setSimpleFrameRegion(const CGUIRegion & region); // region of the background frame; for sizing only as x&y are ignored
   void setSimpleFrameColor(CGUIColor color);           // color
   void setSimpleFrameLineWidth(unsigned short lineWidth);  // line width
   inline void setSimpleFrame(const CGUIRegion & region,  // region of the background frame; for sizing only as x&y are ignored
                       unsigned short frameVMargin,
                       unsigned short frameHMargin,
                       CGUIColor color,      // color
                       unsigned short lineWidth    // line width
                      ) { setSimpleFrameRegion(region);
                          setFrameMargins(frameVMargin, frameHMargin);
                          setSimpleFrameColor(color);
                          setSimpleFrameLineWidth(lineWidth); };

   // sets a new Shaded Frame for the text box.  If none exists, implicitily creates one.
   void setShadedFrameRegion(const CGUIRegion & region);        // region of the background frame; for sizing only as x&y are ignored
   void setShadedFrameColors(CGUIColor shadedColor, CGUIColor unshadedColor); // colors     
   void setShadedFrameLineWidths(unsigned short shadedLineWidth, unsigned short unshadedLineWidth); // line widths
   inline void setShadedFrame(const CGUIRegion & region,         // region of the background frame; for sizing only as x&y are ignored
                       unsigned short frameVMargin,
                       unsigned short frameHMargin,
                       CGUIColor shadedColor,             // shaded color          
                       CGUIColor unshadedColor,           // unshaded color     
                       unsigned short shadedLineWidth,    // shaded line width     
                       unsigned short unshadedLineWidth   // unshaded line width
                      ) { setShadedFrameRegion(region);
                          setFrameMargins(frameVMargin, frameHMargin);
                          setShadedFrameColors(shadedColor, unshadedColor);
                          setShadedFrameLineWidths(shadedLineWidth, unshadedLineWidth); };


   // TEXT METHODS //

   // SET TEXT
   // Set the text associated with the text box.  If none exists, implicitily creates one.
   void setText(CGUITextItem * textItem, StylingRecord * stylingRecord = NULL); // ptr to a text object to be associated with the text box
   void setText(const char * string); // ptr to a text string to be associated with the text box
  
   // SET TEXT STYLE
   // set/get the style of the text associated with this text box.  This is a pass-thru to the 
   // text object previously associated with this text box via the constructor or one of the setText methods.  
   void setStylingRecord(StylingRecord * textStylingRecord); // style record with appropriate features set
   StylingRecord * getStylingRecord(void){ return _text->getStylingRecord();}
  
   // SET TEXT COLOR
   // set/get the color the text associated with this text box.  This is a pass-thru to the 
   // text object previously associated with this text box via the constructor or one of the setText methods.  
   void setTextColor(CGUIColor color);
   CGUIColor getTextColor(void);
   
protected:


    void addObjects(void);
    void deleteObjects(void);

    void setTextBox(void);
    void setBitmap(void);
    void setRectangle(void);
    void setFrame(void);
    void setText(void);

private:
	CGUITextBox(void);
	CGUITextBox (CGUITextBox & copy);
	CGUITextBox operator=(CGUITextBox &obj);
};

#endif /* #ifndef _CGUI_TEXTBOX_INCLUDE */
