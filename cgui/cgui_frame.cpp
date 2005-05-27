/*
*	Copyright (c) 2004 by Gambro BCT, Inc.  All rights reserved.
*
*  Implement the class to create an empty non-filled frame.
*
* $Header $
* $Log $
*
*/
#include "cgui_frame.h"


CGUIFrame::CGUIFrame(CGUIDisplay & display, const CGUIRegion region,  CGUIColor shadedColor,
					 CGUIColor unshadedColor, unsigned short shadedLineWid, unsigned short unshadedLineWid) :
						CGUIWindowObject(display, region),
						_display(display),
						_region(region),
						_shadedColor(shadedColor),
						_unShadedColor(unshadedColor),
						_shadedLineWidth(shadedLineWid),
						_unShadedLineWidth(unshadedLineWid)
{
	_frameType = CGUI_SHADED_FRAME;
}

CGUIFrame::CGUIFrame(CGUIDisplay & display, const CGUIRegion region, CGUIColor color, unsigned short lineWid) :
						CGUIWindowObject(display, region),
						_display(display),
						_region(region),
						_color(color),
						_lineWidth(lineWid)
						
{
	_frameType = CGUI_SIMPLE_FRAME;
}

CGUIFrame::~CGUIFrame()
{
}

// setColor - change the colors used in a shaded frame boundary.  If used on a simple frame, no change is made.
void CGUIFrame::setColor(CGUIColor shadeColor, CGUIColor unshadedColor)
{
	if(_frameType == CGUI_SHADED_FRAME)
	{	
		_shadedColor = shadeColor;
		_unShadedColor = unshadedColor;

		if (_owner)
		{
			_owner->invalidateObjectRegion(this);
		}
	}
}

// setColor - change the color of a simple frame boundary.  If used on a shaded frame, no change is made.
void CGUIFrame::setColor(CGUIColor color)
{
	if(_frameType == CGUI_SIMPLE_FRAME)
	{
		_color = color;
		if (_owner)
		{
			_owner->invalidateObjectRegion(this);
		}
	}
}


// setLineSize - set the width in pixels of the lines that make up a shaded frame.  If used on a simple frame, no
// change is made.
void CGUIFrame::setLineSize (unsigned short shadedLineWid, unsigned short unShadedLineWid)
{
	if(_frameType == CGUI_SHADED_FRAME)
	{	
		_shadedLineWidth = shadedLineWid;
		_unShadedLineWidth = unShadedLineWid;

		if (_owner)
		{
			_owner->invalidateObjectRegion(this);
		}
	}
}

// setLineSize - set the width in pixels of a simple frame.  If used on a shaded frame, no change is made.
void CGUIFrame::setLineSize (unsigned short line)
{
	if(_frameType == CGUI_SIMPLE_FRAME)
	{
		_lineWidth = line;
		if (_owner)
		{
			_owner->invalidateObjectRegion(this);
		}
	}
}

// getColor - return the colors used in a shaded frame boundary.  If used on a simple frame, the same color is
// returned for each paramater.
void CGUIFrame::getColor(CGUIColor & shadeColor, CGUIColor & unshadedColor) const
{
	if(_frameType == CGUI_SHADED_FRAME)
	{
		shadeColor = _shadedColor;
		unshadedColor = _unShadedColor;
	}
	else
	{
		shadeColor = _color;
		unshadedColor = _color;
	}
}

// getColor - return the color of a simple frame boundary.  If used on a shaded frame, the shaded color is returned.
CGUIColor CGUIFrame::getColor(void) const
{
		CGUIColor retColor = _shadedColor;
		if(_frameType == CGUI_SIMPLE_FRAME)
			retColor = _color;
		return retColor;
}

// getLineSize - return the width in pixels of the lines that make up a shaded frame boundary  If used on a simple frame, the
// same value is returned in each argument.
void CGUIFrame::getLineSize (unsigned short & shadedLineWid, unsigned short & unShadedLineWid) const
{
	if(_frameType == CGUI_SHADED_FRAME)
	{
		shadedLineWid = _shadedLineWidth;
		unShadedLineWid = _unShadedLineWidth;
	}
	else
	{
		shadedLineWid = _lineWidth;
		unShadedLineWid = _lineWidth;
	}
}

unsigned short CGUIFrame::getLineSize() const
{
	unsigned short retWidth = 0;
	if(_frameType == CGUI_SIMPLE_FRAME)
		retWidth = _lineWidth;
	return retWidth;
};

void CGUIFrame::draw(UGL_GC_ID gc)
{
	if(gc != UGL_NULL_ID)
	{
		UGL_POS x1,y1,x2,y2;
		// set the linestyle to solid
		uglLineStyleSet(gc, UGL_LINE_STYLE_SOLID);
		uglBackgroundColorSet(gc, UGL_COLOR_TRANSPARENT);

		if(_frameType == CGUI_SHADED_FRAME)
		{
			// setup initial width and color for shaded lines	
			uglLineWidthSet(gc, (int)_shadedLineWidth);
			uglForegroundColorSet(gc, (UGL_COLOR)_shadedColor);
		}
		else
		{
			//simple frame so only one color and width
			uglLineWidthSet(gc, (int)_lineWidth);
			uglForegroundColorSet(gc, (UGL_COLOR)_color);
		}

		// the frame is drawn with line segments instead of polygon to allow for changes in line width and color
		// for a shaded frame.

		// draw the top of the frame (vertical segment from starting location)
		x1=_region.x;
		y1=_region.y;
		x2=x1;
		y2=_region.y+_region.height;
		uglLine(gc, x1,y1,x2,y2);

		// draw the left side the frame (horizontal from starting location)
		x2 = _region.x+_region.width;
		y2 = y1;
		uglLine(gc, x1,y1,x2,y2);

		if(_frameType == CGUI_SHADED_FRAME)
		{
			// set the line width and color for unshaded lines
			uglLineWidthSet(gc, (int)_unShadedLineWidth);
			uglForegroundColorSet(gc, (UGL_COLOR)_unShadedColor);
		}

		// draw bottom of frame
		y1=_region.y+_region.height;
		x2=_region.x+_region.width;
		y2=y1;
		uglLine(gc, x1,y1,x2,y2);

		// draw right side of frame
		x1=_region.x+_region.width;
		y1=_region.y+_region.height;
		y2=_region.y;
		uglLine(gc, x1,y1,x2,y2);
	}
}


	
