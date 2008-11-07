/*
 *	Copyright (c) 2004 by Gambro BCT, Inc.  All rights reserved.
 *
 *  Define the class to create an empty non-filled frame.
 * 	- Add method to fill in background with a CGUIColor.
 *
 * $Header: J:/BCT_Development/vxWorks/Common/cgui/rcs/cgui_frame.h 1.6 2007/12/04 01:06:00Z rm10919 Exp rm10919 $
 * $Log: cgui_frame.h $
 * Revision 1.6  2007/12/04 01:06:00Z  rm10919
 * Fix potential memory leak.
 * Revision 1.5  2007/11/15 21:01:52Z  rm10919
 * Add new method to create background for frame.
 *
 */
#ifndef _CGUI_FRAME_INCLUDE
#define _CGUI_FRAME_INCLUDE

#include <ugl/ugl.h>
#include <ugl/uglwin.h>
#include <ugl/uglinput.h>

#include "cgui_graphics.h"
#include "cgui_rectangle.h"
#include "cgui_window.h"
#include "cgui_window_object.h"


class CGUIFrame : public CGUIWindowObject
{
public:
	typedef enum CGUIFrameType { CGUI_SIMPLE_FRAME, CGUI_SHADED_FRAME };

	// CONSTRUCTOR - construct a shaded frame.  Must provide a display reference, region,
	//  and frame type. Also, colors and line widths for shaded and unshaded segemnts of frame
   CGUIFrame( CGUIDisplay & display, const CGUIRegion region,  
             CGUIColor shadedColor, CGUIColor unshadedColor, 
             unsigned short shadedLineWidth, unsigned short unshadedLineWidth );

	// CONSTRUCTOR - construct a simple frame.  Color and line width must be provided along
	//  with display reference and region for the frame.
	CGUIFrame( CGUIDisplay & display, const CGUIRegion region,  CGUIColor color, unsigned short lineWidthth );

	//DESTRUCTIOR
	virtual ~CGUIFrame(void);

	// SetColor - change the colors used in a shaded frame boundary.
	//   If used on a simple frame, no change is made.
	void setColor( CGUIColor shadeColor, CGUIColor unshadedColor );

	// SetColor - change the color of a simple frame boundary.
	//   If used on a shaded frame, no change is made.
	void setColor( CGUIColor color );

	// SetLineSize - set the width in pixels of the lines that make up a shaded frame.
	//   If used on a simple frame, no change is made.
	void setLineSize( unsigned short shadedLineWidth, unsigned short unShadedLineWidth );

	// SetLineSize - set the width in pixels of a simple frame.
	//   If used on a shaded frame, no change is made.
	void setLineSize( unsigned short line );

	// fill the frame with a background color
	void setBackgroundColor( CGUIColor backgroundColor );

	// getFrameType - return the type of frame this is
	CGUIFrameType getFrameType(void) const { return _frameType; };

	// getColor - return the colors used in a shaded frame boundary.
	//   If used on a simple frame, the same color is
	// 	returned for each paramater.
	void getColor( CGUIColor & shadeColor, CGUIColor & unshadedColor ) const;

	// getColor - return the color of a simple frame boundary.
	//   If used on a shaded frame, the shaded color is returned.
	CGUIColor getColor(void) const;

	CGUIColor getBackgroundColor(void) const { return _backgroundColor; };

	// getLineSize - return the width in pixels of the lines that make up a shaded frame boundary
	//   If used on a simple frame, the same value is returned in each argument.
	void getLineSize( unsigned short & shadedLineWidth, unsigned short & unShadedLineWidth ) const;

	// getLineSize - return the width in pixels of a simple frame.
	//   If used on a shaded frame, a 0 is returned.
	unsigned short getLineSize (void) const;

protected:
   virtual void draw( UGL_GC_ID gc );

private:
   virtual ClippingType clipSiblings(void) const { return Transparency; }

private:
	CGUIRegion _region;
	CGUIDisplay & _display;
	CGUIFrameType _frameType;

	CGUIColor _color;
	CGUIColor _shadedColor;
	CGUIColor _unShadedColor;
	CGUIColor _backgroundColor;
	
	unsigned short _lineWidth;
	unsigned short _shadedLineWidth;
	unsigned short _unShadedLineWidth;

   CGUIRectangle * _backgroundRectangle;
private:
	CGUIFrame();
	CGUIFrame ( const CGUIFrame &obj );
	CGUIFrame operator=( CGUIFrame &obj );
};

#endif /* #ifndef _CGUI_FRAME_INCLUDE */

