/*
 *	Copyright (c) 2004 by Gambro BCT, Inc.  All rights reserved.
 *
 *  Define the class to create an empty non-filled frame.
 *
 * $Header $
 * $Log $
 *
 */

#ifndef _CGUI_FRAME_INCLUDE
#define _CGUI_FRAME_INCLUDE

#include <vxworks.h>
#include <ugl/ugl.h>
#include <ugl/uglwin.h>
#include <ugl/uglinput.h>

#include "cgui_graphics.h"
#include "cgui_window.h"
#include "cgui_window_object.h"


class CGUIFrame : public CGUIWindowObject
{
public:
	typedef enum CGUIFrameType  {
		CGUI_SIMPLE_FRAME,
		CGUI_SHADED_FRAME
	};

	// CONSTRUCTOR - construct a shaded frame.  Must provide a display reference, region, and frame type. Also,
	// colors and line widths for shaded and unshaded segemnts of frame
   CGUIFrame(CGUIDisplay & display, const CGUIRegion region,  CGUIColor shadedColor,
		       CGUIColor unshadedColor, unsigned short shadedLineWid, unsigned short unshadedLineWid);

	// CONSTRUCTOR - construct a simple frame.  color and line width must be provided along with display reference
	// and region for the frame.
	CGUIFrame(CGUIDisplay & display, const CGUIRegion region,  CGUIColor color, unsigned short lineWid);

	//DESTRUCTIOR
	virtual ~CGUIFrame(void);

	// setColor - change the colors used in a shaded frame boundary.  If used on a simple frame, no change is made.
	void setColor(CGUIColor shadeColor, CGUIColor unshadedColor);

	// setColor - change the color of a simple frame boundary.  If used on a shaded frame, no change is made.
	void setColor(CGUIColor color);

	// setLineSize - set the width in pixels of the lines that make up a shaded frame.  If used on a simple frame, no
	// change is made.
	void setLineSize (unsigned short shadedLineWid, unsigned short unShadedLineWid);

	// setLineSize - set the width in pixels of a simple frame.  If used on a shaded frame, no change is made.
	void setLineSize (unsigned short line);

	// getFrameType - return the type of frame this is
	CGUIFrameType getFrameType(void) const { return _frameType; };

	// getColor - return the colors used in a shaded frame boundary.  If used on a simple frame, the same color is
	// returned for each paramater.
	void getColor(CGUIColor & shadeColor, CGUIColor & unshadedColor) const;

	// getColor - return the color of a simple frame boundary.  If used on a shaded frame, the shaded color is returned.
	CGUIColor getColor(void) const;

	// getLineSize - return the width in pixels of the lines that make up a shaded frame boundary  If used on a simple frame, the
	// same value is returned in each argument.
	void getLineSize (unsigned short & shadedLineWidth, unsigned short & unShadedLineWidth) const;

	// getLineSize - return the width in pixels of a simple frame.  If used on a shaded frame, a 0 is returned.
	unsigned short getLineSize (void) const;

protected:
   virtual void draw(UGL_GC_ID gc);

private:
   virtual bool clipSiblings(void) const { return false;}

private:
	CGUIRegion _region;
	CGUIDisplay & _display;
	CGUIFrameType _frameType;
	CGUIColor _color;
	CGUIColor _shadedColor;
	CGUIColor _unShadedColor;
	unsigned short _lineWidth;
	unsigned short _shadedLineWidth;
	unsigned short _unShadedLineWidth;
};

#endif /* #ifndef _CGUI_FRAME_INCLUDE */
