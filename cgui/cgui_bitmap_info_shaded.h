/*
* $Header: J:/BCT_Development/vxWorks/Common/cgui/rcs/cgui_bitmap_info_shaded.h 1.1 2008/11/06 22:19:41Z rm10919 Exp rm10919 $
*	This file defines the CGUIShadedBitmapInfo class which defines and creates 
*		a CGUIBitmapInfo that is the shaded bitmap data.  This class could also be
*		used when needing a bitmap for the button class.
*
*		Use the createBitmapShadedData() to create/use(find) this class,
*		this is where the work is done to create the shaded bitmap info.
*
*		There is only one instance of this class.
* 			
*
* $Log: cgui_bitmap_info_shaded.h $
* Revision 1.1  2008/11/06 22:19:41Z  rm10919
* Initial revision
*
*
*/

#ifndef _CGUI_BITMAP_INFO_SHADED_INCLUDE
#define _CGUI_BITMAP_INFO_SHADED_INCLUDE

#include "cgui_bitmap_info.h"
#include "cgui_graphics.h"

	
enum ShadeType
{ 
   NoShade,
   Solid, 		// start and end color match
   TopBottom,	// start color top, end color bottom
	BottomTop, 	// start color bottom, end color top
   LeftRight,	// start color left, end color right
	RightLeft/*,	// start color right, end color left
	CenterIn,	// start color outer edge, end color in center
	CenterOut */	// start color center, end color outer edge
};

struct RGB
{/*Lowest intensity at 0. Highest intensity at 255*/
	short red;	//	red
	short green;	//	green
	short blue;	//	blue
 
	bool operator == ( const RGB rgb ) const;
	bool operator < ( const RGB rgb ) const;
};

struct HSL
{
   short hue;	//	hue
	short sat;	//	saturation
	short lum;	//	lightness (luminance)

	HSL( void ) { memset( this, 0, sizeof( *this ) );}
};

struct ColorScheme
{
	RGB	colorStart;			//The starting lightness level of the chosen color. An RGB triplet.
	RGB	colorEnd;			//The ending lightness level of the chosen color. An RGB triplet.

	// needed for mapping function
	bool ColorScheme::operator == ( const ColorScheme colorScheme ) const;
	bool ColorScheme::operator < ( const ColorScheme colorScheme ) const;
};

enum BitmapShape
{ 
	NoShape,			// has not border around edge
	Square,			// has square edge, no background color or mask needed
	Rounded			// has rounded edges, Need mask for transparentcy????  Background color??? FUTURE!!!!!
};



class CGUIBitmapInfoShaded
{
public:
	CGUIBitmapInfoShaded();
	virtual ~CGUIBitmapInfoShaded();

	struct BitmapMetrics
	{
		ColorScheme colorScheme;		//	The start and end color for shading.  These will be defined at project level.
		ShadeType	shadeType;			// The shade of the chosen color to be placed on the button bitmap.
		short borderWidth;				// The width of the border, if there is no border the value is zero.
		short bitmapWidth;
		short bitmapHeight;

		BitmapMetrics( void ) { memset( this, 0, sizeof( *this )); };

		// needed for mapping function
		bool BitmapMetrics::operator == ( const BitmapMetrics bitmapMetrics ) const;
		bool BitmapMetrics::operator < ( const BitmapMetrics bitmapMetrics ) const;
	};


	// Creates and adds shaded bitmap info data to map, returns pinter to CGUIBimapInfo data (in map).
	//		If shaded bitmap exsists then it returns the CGUIBitmapInfo * without re-ceating it.
	// 
	static CGUIBitmapInfo * createShadedBitmapData( RGB startColor, RGB endColor, ShadeType shadeType,  short borderWidth, short bitmapWidth, short bitmapHeight, BitmapShape bitmapShape = NoShape );
	static CGUIBitmapInfo * createShadedBitmapData( const ColorScheme colorScheme, ShadeType shadeType, short borderWidth, short bitmapWidth, short bitmapHeight, BitmapShape bitmapShape = NoShape );
	static CGUIBitmapInfo * createShadedBitmapData( BitmapMetrics &bitmapMetrics, BitmapShape bitmapShape = NoShape );

protected:

private:
   static SEM_ID	_lock;

	static map< BitmapMetrics, CGUIBitmapInfo * > _shadedBitmapInfoCache;

	// creates the bitmap color data
	//
	static bool createShade( BitmapMetrics &bitmapMetrics );

	// if there is a border, the border color is added to bitmap color data
	//
	static void createBorder( UGL_DEVICE_ID devId, BitmapMetrics &bitmapMetrics, unsigned short *colorImage );
	
   //	rgbToHSL
   //		A conversion from the red, green, and blue triplet of the RGB color space to the more
   //		perceptual HSL color space. Hue, Saturation and Lightness. A struct RGB is passed in 
   //		and a struct HSL is returned.
	// 
   static HSL rgbToHSL( RGB rgb );

	//	hslToRGB
	// 	A conversion from a value of  hue, saturation, and lightness to the RGB color space
	//		The perceptual HSL color space values must be converted to RGB values for display color
	//		specification. A struct RGB is returned
	// 
   static RGB hslToRGB( HSL hsl );

	// Both used by hslToRGB function in converting to RGB
	//
	static short hslColorRangeConverter( float color, float p, float q );
	static float checkColorRange( float color );

private:
	// Declare copy constructor and assigned operator private to avoid use
	//
	CGUIBitmapInfoShaded( const CGUIBitmapInfoShaded& copy );
	CGUIBitmapInfoShaded& operator = ( const CGUIBitmapInfoShaded& obj );
};


#endif /* ifndef _CGUI_BITMAP_INFO_SHADED_INCLUDE */

