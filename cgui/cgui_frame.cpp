/*
 * $Header: J:/BCT_Development/vxWorks/Common/cgui/rcs/cgui_frame.cpp 1.6 2007/11/15 21:01:51Z rm10919 Exp rm10919 $
 *
 * Implement the class to create an empty non-filled frame.
 *
 * $Log: cgui_frame.cpp $
 * Revision 1.6  2007/11/15 21:01:51Z  rm10919
 * Add new method to create background for frame.
 * Revision 1.5  2007/04/14 18:05:18Z  jl11312
 * - handle deletion of objects referenced by an active screen (common IT 81)
 *
 */

#include <vxworks.h>

#include "cgui_frame.h"

CGUIFrame::CGUIFrame( CGUIDisplay & display, const CGUIRegion region,  CGUIColor shadedColor,
                      CGUIColor unshadedColor, unsigned short shadedLineWidth, unsigned short unshadedLineWidth ):
                      CGUIWindowObject( display, region ),
							 _display( display ),
							 _region( region ),
							 _shadedColor( shadedColor ),
							 _unShadedColor( unshadedColor ),
							 _shadedLineWidth( shadedLineWidth ),
							 _unShadedLineWidth( unshadedLineWidth ),
							 _backgroundRectangle( NULL )
{
	_frameType = CGUI_SHADED_FRAME;
}

CGUIFrame::CGUIFrame( CGUIDisplay & display, const CGUIRegion region, CGUIColor color, unsigned short lineWidth ):
                      CGUIWindowObject( display, region ),
                      _display( display ),
                      _region( region ),
                      _color( color ),
                      _lineWidth( lineWidth ),
							 _backgroundRectangle( NULL )
{
	_frameType = CGUI_SIMPLE_FRAME;
}

CGUIFrame::~CGUIFrame()
{
	if( _backgroundRectangle )
	{
		if( _owner ) _owner->deleteObject( _backgroundRectangle );

      delete _backgroundRectangle;
	}
}

// setColor - change the colors used in a shaded frame boundary.
//   If used on a simple frame, no change is made.
void CGUIFrame::setColor( CGUIColor shadeColor, CGUIColor unshadedColor )
{
	if( _frameType == CGUI_SHADED_FRAME )
	{	
		_shadedColor = shadeColor;
		_unShadedColor = unshadedColor;

		if( _owner )
		{
			_owner->invalidateObjectRegion( this );
		}
	}
}

// setColor - change the color of a simple frame boundary.  
//   If used on a shaded frame, no change is made.
void CGUIFrame::setColor( CGUIColor color )
{
	if( _frameType == CGUI_SIMPLE_FRAME )
	{
		_color = color;
		if( _owner )
		{
			_owner->invalidateObjectRegion( this );
		}
	}
}

// setLineSize - set the width in pixels of the lines that make up a shaded frame.
//   If used on a simple frame, no change is made.
void CGUIFrame::setLineSize( unsigned short shadedLineWidth, unsigned short unShadedLineWidth )
{
	if( _frameType == CGUI_SHADED_FRAME )
	{	
		_shadedLineWidth = shadedLineWidth;
		_unShadedLineWidth = unShadedLineWidth;

		if( _owner )
		{
			_owner->invalidateObjectRegion( this );
		}
	}
}

// setLineSize - set the width in pixels of a simple frame.
//   If used on a shaded frame, no change is made.
void CGUIFrame::setLineSize( unsigned short line )
{
	if( _frameType == CGUI_SIMPLE_FRAME )
	{
		_lineWidth = line;

		if( _owner )
		{
			_owner->invalidateObjectRegion( this );
		}
	}
}

void CGUIFrame::setBackgroundColor( CGUIColor backgroundColor )
{
	
	_backgroundColor = backgroundColor;

	if( _backgroundRectangle )
	{
		if( _owner ) _owner->deleteObject( _backgroundRectangle );

		delete _backgroundRectangle;
	}

	_backgroundRectangle = new CGUIRectangle( _display, _region, backgroundColor );

	if( _owner )
	{
		_owner->addObjectToFront( _backgroundRectangle );	
		_owner->invalidateObjectRegion( this );
	}
}

// getColor - return the colors used in a shaded frame boundary.
//   If used on a simple frame, the same color is returned for each paramater.
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

// getColor - return the color of a simple frame boundary.
//   If used on a shaded frame, the shaded color is returned.
CGUIColor CGUIFrame::getColor(void) const
{
		CGUIColor returnColor = _shadedColor;
		if(_frameType == CGUI_SIMPLE_FRAME)
			returnColor = _color;
		return returnColor;
}

// getLineSize - return the width in pixels of the lines
//  that make up a shaded frame boundary.
//   If used on a simple frame, the same value is returned in each argument.
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
	unsigned short returnWidth = 0;
	if(_frameType == CGUI_SIMPLE_FRAME)
		returnWidth = _lineWidth;
	return returnWidth;
};

void CGUIFrame::draw( UGL_GC_ID gc )
{
	if( gc != UGL_NULL_ID )
	{
		UGL_POS x1,y1,x2,y2;
#if CPU!=SIMNT
      int ltOffset;
      int rbOffset;
#endif
		// set the linestyle to solid
		uglLineStyleSet( gc, UGL_LINE_STYLE_SOLID );
		uglBackgroundColorSet( gc, UGL_COLOR_TRANSPARENT );

		if( _frameType == CGUI_SHADED_FRAME )
		{
			// setup initial width and color for shaded lines	
			uglLineWidthSet( gc, (int)_shadedLineWidth );
			uglForegroundColorSet( gc, (UGL_COLOR)_shadedColor );
#if CPU!=SIMNT
         // correct for offsets that will be applied by uglGenericLine
         ltOffset = ( _shadedLineWidth - 1 ) / 2;
         rbOffset = _shadedLineWidth / 2 + 1;
#endif
		}
		else
		{
			//simple frame so only one color and width
			uglLineWidthSet( gc, (int)_lineWidth );
			uglForegroundColorSet( gc, (UGL_COLOR)_color );
#if CPU!=SIMNT
         // correct for offsets that will be applied by uglGenericLine
         ltOffset = ( _lineWidth - 1 ) / 2;
         rbOffset = _lineWidth / 2 + 1;
#endif
		}

		// the frame is drawn with line segments instead of polygon to allow for changes in line width and color
		// for a shaded frame.

		// draw the left side of the frame (vertical segment from starting location)
#if CPU==SIMNT
		x1=_region.x;
		y1=_region.y;
		y2=_region.y+_region.height;
		x2=x1;
#else
		x1=_region.x + ltOffset;
		y1=_region.y + ltOffset;
		y2=_region.y+_region.height - rbOffset;
#endif
		x2=x1;
		uglLine( gc, x1,y1,x2,y2 );

		// draw the top of the frame (horizontal from starting location)
#if CPU==SIMNT
		x2 = _region.x+_region.width;
#else
		x2 = _region.x+_region.width - rbOffset;
#endif
		y2 = y1;
		uglLine( gc, x1,y1,x2,y2 );

		if( _frameType == CGUI_SHADED_FRAME )
		{
         // set the line width and color for unshaded lines
			uglLineWidthSet( gc, (int)_unShadedLineWidth );
			uglForegroundColorSet( gc, (UGL_COLOR)_unShadedColor );
         // correct for offsets that will be applied by uglGenericLine
#if CPU!=SIMNT
         ltOffset = ( _unShadedLineWidth - 1 ) / 2;
         rbOffset = _unShadedLineWidth / 2 + 1;
         x1=_region.x + ltOffset;
#endif
		}

		// draw bottom of frame
#if CPU==SIMNT
		y1=_region.y+_region.height;
		x2=_region.x+_region.width;
#else
		y1=_region.y+_region.height - rbOffset;
		x2=_region.x+_region.width - rbOffset;
#endif
		y2=y1;
		uglLine( gc, x1,y1,x2,y2 );

		// draw right side of frame
		x1=x2;
#if CPU==SIMNT
		y1=_region.y+_region.height;
		y2=_region.y;
#else
		y1=_region.y+_region.height - rbOffset;
		y2=_region.y + ltOffset;
#endif
		uglLine( gc, x1,y1,x2,y2 );
	}
}
