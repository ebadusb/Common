/*
* $Header: K:/BCT_Development/vxWorks/Common/cgui/rcs/cgui_bitmap_info_shaded.cpp 1.4 2009/02/03 00:38:08Z rm10919 Exp wms10235 $
*
*	This file defines the CGUIBitmapInfoShaded class which defines and creates 
*		a CGUIBitmapInfo object that is the shaded bitmap data.  This class could also be
*		used when needing a bitmap for the button class.
*		Use the createBitmapShadedData() to create/use(find) an object in the class,
*		this is where the work is done to create the shaded bitmap info.
*		The createBitmapShadedData() first checks to see if the bitmap info
*		already exsists before creating the shaded bitmap data.
*
* $Log: cgui_bitmap_info_shaded.cpp $
* Revision 1.2  2009/01/08 00:55:18Z  rm10919
* Updates and bug fixes for shaded buttons.
* Revision 1.1  2008/11/06 22:19:41Z  rm10919
* Initial revision
*
*
*/

#include <vxWorks.h>

#include "cgui_bitmap_info_shaded.h"

//	Declare static variable
//		This is the only instance of this class.
map< CGUIBitmapInfoShaded::BitmapMetrics, CGUIBitmapInfo * > CGUIBitmapInfoShaded::_shadedBitmapInfoCache;

SEM_ID CGUIBitmapInfoShaded::_lock = semMCreate(SEM_Q_PRIORITY | SEM_INVERSION_SAFE);

CGUIBitmapInfoShaded::CGUIBitmapInfoShaded()
{
	
}

CGUIBitmapInfoShaded::~CGUIBitmapInfoShaded()
{
	
}

CGUIBitmapInfo * CGUIBitmapInfoShaded::createShadedBitmapData( RGB startColor, RGB endColor, ShadeType shadeType, short borderWidth, short bitmapWidth, short bitmapHeight, BitmapShape bitmapShape /* = NoShape */ )
{
	// return value variable
	//
	CGUIBitmapInfo * result = NULL;

	//	Fill in strutures.
	//
	BitmapMetrics bitmapMetrics;

	bitmapMetrics.colorScheme.colorStart = startColor;
	bitmapMetrics.colorScheme.colorEnd = endColor;
	bitmapMetrics.shadeType = shadeType;

	bitmapMetrics.borderWidth = borderWidth;

	bitmapMetrics.bitmapWidth = bitmapWidth;
	bitmapMetrics.bitmapHeight = bitmapHeight;
	
   result = createShadedBitmapData( bitmapMetrics );
	
	return result;
}


CGUIBitmapInfo * CGUIBitmapInfoShaded::createShadedBitmapData( const ColorScheme colorScheme, ShadeType shadeType, short borderWidth, short bitmapWidth, short bitmapHeight, BitmapShape bitmapShape /* = NoShape */ )
{
	// return value variable
	//
	CGUIBitmapInfo * result = NULL;

	//	Fill in struture.
	//
	BitmapMetrics bitmapMetrics;

	bitmapMetrics.colorScheme = colorScheme;
	bitmapMetrics.shadeType = shadeType;
	bitmapMetrics.borderWidth = borderWidth;
	bitmapMetrics.bitmapWidth = bitmapWidth;
	bitmapMetrics.bitmapHeight = bitmapHeight;
	
   result = createShadedBitmapData( bitmapMetrics );

	return result;
}


CGUIBitmapInfo * CGUIBitmapInfoShaded::createShadedBitmapData( BitmapMetrics &bitmapMetrics, BitmapShape bitmapShape /* = NoShape */ )
{	
	// return value variable
	//
	CGUIBitmapInfo * result = NULL;
	
	// Check to see if this shaded bitmap already exsists.
	//
	map < BitmapMetrics, CGUIBitmapInfo * >::iterator iter;
	
	semTake( _lock, WAIT_FOREVER );

	iter = _shadedBitmapInfoCache.find( bitmapMetrics );
	
	if( iter != _shadedBitmapInfoCache.end() )
	{
		CGUIBitmapInfo * shadedBitmapInfo = iter->second;
		result = shadedBitmapInfo;	
	}
	else 	// create the shaded bitmap info
	{
		createShade( bitmapMetrics );

		// Now find the newly created data.
		//
		iter = _shadedBitmapInfoCache.find( bitmapMetrics );

		if( iter != _shadedBitmapInfoCache.end() )
		{
			CGUIBitmapInfo * shadedBitmapInfo = iter->second;
			result = shadedBitmapInfo;
		}		
	}
	semGive( _lock );

	return result;
}


bool CGUIBitmapInfoShaded::createShade( BitmapMetrics &bitmapMetrics )
{
	UGL_REG_DATA *pRegistryData;
	UGL_DEVICE_ID devId;

	pRegistryData = uglRegistryFind (UGL_DISPLAY_TYPE, 0, 0, 0);
	devId = (UGL_DEVICE_ID)pRegistryData->id;

	int skip, step, repeat, extra, position, minValue, lValue = 0; 
	int rowSize = 0;
	int dimension = 0;
	
	long numberOfColors = 0;
	
	UGL_COLOR deviceColor;
	
	// fill 
	unsigned short * bitmapData = NULL;
   
	if(( bitmapMetrics.shadeType == TopBottom ) 
		|| ( bitmapMetrics.shadeType == BottomTop )
		|| ( bitmapMetrics.shadeType == Solid ))
	{
		dimension = (int)bitmapMetrics.bitmapHeight;
		rowSize  = (int)bitmapMetrics.bitmapWidth;
	}
	
	if(( bitmapMetrics.shadeType == RightLeft )
			|| ( bitmapMetrics.shadeType == LeftRight ))
	{
		dimension = (int)bitmapMetrics.bitmapWidth;
		rowSize  = (int)bitmapMetrics.bitmapHeight;
	}
	
	//2 Get the two extreme levels of lightness, the L value
	RGB rgb;

	rgb.red = bitmapMetrics.colorScheme.colorStart.red;
	rgb.green = bitmapMetrics.colorScheme.colorStart.green;
	rgb.blue = bitmapMetrics.colorScheme.colorStart.blue;
	
	HSL startColor = rgbToHSL( rgb );

	rgb.red = bitmapMetrics.colorScheme.colorEnd.red;
	rgb.green = bitmapMetrics.colorScheme.colorEnd.green;
	rgb.blue = bitmapMetrics.colorScheme.colorEnd.blue;
	
	HSL endColor = rgbToHSL( rgb );

	HSL hsl;

	//3.Determine the number of colors between the startColor and endColor colors. L_start - L_finish.
	numberOfColors = startColor.lum - endColor.lum;
	
	if( numberOfColors < 0 ) 
		numberOfColors = abs( numberOfColors );

	if( startColor.lum == endColor.lum )
		numberOfColors = 1;
	
	//4.Ensure that lVvalue is assigned the lightest of the two colors
	if( endColor.lum > startColor.lum )
	{
		//Starting from the lightest shade of the color
		lValue = endColor.lum;
		minValue = startColor.lum;

      rgb.red	= bitmapMetrics.colorScheme.colorEnd.red;
		rgb.green = bitmapMetrics.colorScheme.colorEnd.green;
		rgb.blue = bitmapMetrics.colorScheme.colorEnd.blue;

		hsl = rgbToHSL( rgb );
	}
	else
	{
		//Starting from the lightest shade of the color
		lValue = startColor.lum;
		minValue = endColor.lum;

      rgb.red	= bitmapMetrics.colorScheme.colorStart.red;
		rgb.green = bitmapMetrics.colorScheme.colorStart.green;
		rgb.blue = bitmapMetrics.colorScheme.colorStart.blue;

      hsl = rgbToHSL( rgb );
	}

	/*5.Alocate enough memory for the color bitmap image */
	// already did this step.
	
	bitmapData = new unsigned short[ bitmapMetrics.bitmapWidth * bitmapMetrics.bitmapHeight ];
	
	/*6.Store colors, 'dimension' pixels in length, with decreasing values in lightness,L_value.*/
	if( numberOfColors > dimension )
	{
		/*CASE1: More colors than rows*/
		  /*Move through the dimension 'step'colors at a time.*/
		step	= numberOfColors / dimension;
		
		/*Number of places, in the dimension,where an extra color will be passed by.*/
		skip	= numberOfColors - step * dimension;

		/*The positions in the dimension where an extra color will be skipped.*/
		if( skip != 0 )
			position	= dimension / skip;
		else
			position = NONE;

		hsl.lum = lValue; 
		rgb = hslToRGB( hsl );

		/*Fill the array, 'dimension' pixels in length with the corresponding shades of color
		'row_size pixels the same color*/
		for( int i = 0; i < dimension; i++ )
		{
			/*Step1.Convert to ARGB8888 format*/ 
			UGL_ARGB portableColor	= UGL_MAKE_RGB( rgb.red, rgb.green, rgb.blue );

			for( int j = rowSize * i; j< rowSize * ( i + 1 ); j++ )
			{
				/*Step2.Convert ARGB8888 to RGB565 format*/
				uglColorAlloc(devId, &portableColor, UGL_NULL, &deviceColor, 1);
				bitmapData[j] = (unsigned short)deviceColor;
			}

			/*Step over 'step' levels of shading*/
			lValue -= step;

			/*Determine if a 'skip' position has been reached*/
			if( i == position )
			{
				lValue--; //decrease an extra value in L_value.
				position += position; //move to the next position where a skip will occur
			}

			/*Convert to the new RGB value*/
			hsl.lum = lValue; 
			rgb = hslToRGB( hsl );
		}
	}
   else if( numberOfColors == dimension )
	{
		/*CASE2.Number of colors equal number of rows. One color for each row.*/
		/*Fill the array, 'dimension' pixels in length with the corresponding shades of color
		'row_size pixels the same color*/

		hsl.lum = lValue; 
		rgb = hslToRGB( hsl );

		for( int i = 0; i < dimension; i++ )
		{
			//Step1.Convert to ARGB8888 format 
			UGL_ARGB portableColor	= UGL_MAKE_RGB( rgb.red, rgb.green, rgb.blue );

			for(int j = rowSize * i; j< rowSize * ( i + 1 ); j++)
			{
				//Step2.Convert ARGB8888 to RGB565 format
				uglColorAlloc( devId, &portableColor, UGL_NULL, &deviceColor, 1 );
				bitmapData[j]= (unsigned short)deviceColor;
			}

			//New shade
			lValue--;
			//Convert to the new RGB value
			hsl.lum = lValue; 
			rgb   = hslToRGB( hsl );
		}
	}
	 else
	{
		/*CASE3.More rows than colors*/
		/*Move through the dimension repeating each color 'repeat' times.*/
		repeat = dimension / numberOfColors;
		
		/*Number of places, in the dimension,where a color will be repeated for an extra row.*/
		extra = dimension - numberOfColors * repeat;
		
		/*The positions in the dimension where a color will be repeated for an extra row.*/
		if( extra != 0 )
			position = dimension / extra;
		else 
			position = NONE;

		hsl.lum = lValue; 
		rgb = hslToRGB( hsl );
		
		/*Fill the array, 'dimension' pixels in length with the corresponding shades of color
		'row_size pixels the same color*/
		int i = 0;
		int posCount = 0;
		
		UGL_ARGB portableColor;
		
		while( i < dimension )
		{
			for( int k = 0; k < repeat; k++ )
			{
				//Repeat the row color "repeat" times
				portableColor	= UGL_MAKE_RGB( rgb.red, rgb.green, rgb.blue );
				
				for( int j = rowSize * i; j< rowSize * ( i + 1 ); j++ )
				{
					uglColorAlloc(devId, &portableColor, UGL_NULL, &deviceColor, 1 );
					bitmapData[j]= (unsigned short)deviceColor;
				}
				i++; 
				posCount++;
				
				if( posCount == position )
				{
					//Add in an extra row of same color every "position" rows down the dimension
					for(int j = rowSize * i; j < rowSize * ( i + 1 ); j++ )
					{
						uglColorAlloc( devId, &portableColor, UGL_NULL, &deviceColor, 1 );
						bitmapData[j]= (unsigned short)deviceColor;
					}
					i++;
					posCount=0;

					if( i >= dimension )
						break;
				}
			}
			
			/*move to next level of shading as long as there is a color left to use*/
			if( lValue >= minValue )
			{
				/*New shade*/
				lValue--;
            hsl.lum = lValue; 
				
				/*Convert to the new RGB value*/
				rgb = hslToRGB( hsl );
			}
		}//while
	}//else

	unsigned short* colorImage = new unsigned short[ bitmapMetrics.bitmapHeight * bitmapMetrics.bitmapWidth ];
	unsigned short* colors = new unsigned short[ dimension ];

	switch( bitmapMetrics.shadeType )
	{
		case TopBottom:
			for( int l = 0; l < dimension; l++ )
			{
				colors[l] = bitmapData[ l * rowSize ];
			}
			
			for( int i = 0; i < dimension; i++ )
			{
				for( int j = rowSize * i; j< rowSize * ( i + 1 ); j++ )
				{
					colorImage[j] = colors[ ( dimension - 1 ) - i ];
				}
			}
		break;

		case BottomTop:
         for( int l = 0; l < dimension; l++ )
			{
				colors[l] = bitmapData[ l * rowSize ];

			}
			for( int i = 0; i < dimension; i++ )
			{
				for( int j = rowSize * i; j< rowSize * ( i + 1 ); j++)
				{
					colorImage[j] = colors[i];
				}
			}
		break;

		case LeftRight:
			for( int l = 0; l < dimension; l++ )
			{
				colors[l] = bitmapData[ l * rowSize ];
			}

			for( int k = 0; k< rowSize; k++ )
			{
				for( int j = 0; j< dimension; j++ )
				{
					colorImage[ j + ( k * dimension ) ] = colors[j];
				}
			}
		break;

		case RightLeft:
			for( int l = 0; l < dimension; l++ )
			{
				colors[l] = bitmapData[ l * rowSize ];
			}

			for( int k = 0; k< rowSize; k++ )
			{
				for( int j = 0; j< dimension; j++ )
				{
					colorImage[ j + ( k * dimension ) ] = colors[ ( dimension - 1 ) - j ];
				}
			}
		break;

		case NoShade:
		case Solid:
			for( int l = 0; l < dimension; l++ )
			{
				colors[l] = bitmapData[ l * rowSize ];
			}
			
			for( int i = 0; i < dimension; i++ )
			{
				for( int j = rowSize * i ; j< rowSize * ( i + 1 ); j++ )
				{
					colorImage[j] = colors[ dimension / 2 ];
				}
         }
		break;
	}
	
	if( bitmapMetrics.borderWidth > 0 )
	{
      createBorder( devId, bitmapMetrics, colorImage );
	}

	CGUIBitmapInfo * shadedBitmapInfo = new CGUIBitmapInfo ( colorImage, sizeof( *colorImage ), bitmapMetrics.bitmapWidth, bitmapMetrics.bitmapHeight );
	
	_shadedBitmapInfoCache[ bitmapMetrics ] = shadedBitmapInfo;

	return true;
}


void CGUIBitmapInfoShaded::createBorder( UGL_DEVICE_ID devId, BitmapMetrics &bitmapMetrics, unsigned short *colorImage )
{
	int dimension = bitmapMetrics.bitmapHeight;
	int rowSize = bitmapMetrics.bitmapWidth;

	UGL_COLOR highBorderColor;
	UGL_COLOR lowBorderColor;
	
	UGL_ARGB whiteARGB	= UGL_MAKE_RGB( 250, 250, 250);
	UGL_ARGB blackARGB	= UGL_MAKE_RGB( 0, 0, 0);

   if( bitmapMetrics.shadeType == TopBottom ||
		 bitmapMetrics.shadeType == LeftRight ||
		 bitmapMetrics.shadeType == Solid )
	{
		uglColorAlloc( devId, &whiteARGB, UGL_NULL, &lowBorderColor, 1);
		uglColorAlloc(devId, &blackARGB, UGL_NULL, &highBorderColor, 1);
   }
	else
	{
		// Case for BottomTop and RightLeft
		uglColorAlloc( devId, &whiteARGB, UGL_NULL, &highBorderColor, 1);
		uglColorAlloc(devId, &blackARGB, UGL_NULL, &lowBorderColor, 1);
	}
	//
	//	Border for the top and bottom of button.
	// 
	for( int i = 0; i < rowSize; i++ )
	{
		//Bottom Border
		for( int k = 0; k < bitmapMetrics.borderWidth; k++ )
		{
			colorImage[( dimension - ( bitmapMetrics.borderWidth - k )) * rowSize + i ] = (unsigned short)lowBorderColor;
		}
		//
		//Top Border
		for( int k = 0; k < bitmapMetrics.borderWidth; k++ )
		{
			colorImage[ i + rowSize * ( bitmapMetrics.borderWidth - k - 1 )]  = (unsigned short)highBorderColor;
		}	//	inside for loop 
	}	//	outside for loop
	//
	// Border for the right and left sides of button.
	//
	for( int i = 0; i < dimension; i++ )
	{
		//Right White Line on Far Right
		for( int k = 0; k < bitmapMetrics.borderWidth; k++ )
		{
			colorImage[( i + 1 ) * rowSize - ( bitmapMetrics.borderWidth - k )] = (unsigned short)lowBorderColor;
		}
		//
		//Left Black Line on Far Left
		for( int k = 0; k < bitmapMetrics.borderWidth; k++)
		{
			colorImage[ i * rowSize + ( bitmapMetrics.borderWidth - k - 1 )] = (unsigned short)highBorderColor;
		}	// inside for loop
	}	// outside for loop
}


HSL CGUIBitmapInfoShaded::rgbToHSL( RGB rgb )
{
	// return value
	HSL returnHSL;
	
   float red, green, blue, hue, sat, lum, maxValue, minValue = 0.0;

	enum MaxColor { RedMax, GreenMax, BlueMax, MaxEqualMin };

	MaxColor maxColor = RedMax;
	
	//	normalize RGB values
	//
	red	= (float)( rgb.red / 255.0 );
	green	= (float)( rgb.green / 255.0 );
	blue	= (float)( rgb.blue / 255.0 );

	//	Determine maxColor
	//
	if(( rgb.red >= rgb.green ) && ( rgb.red >= rgb.blue ))
	{
		maxColor	= RedMax;
		maxValue	= red;
	}
	else if(( rgb.green >= rgb.red ) && ( rgb.green >= rgb.blue ))
	{
		maxColor	= GreenMax;
		maxValue	= green;
	}
	else if(( rgb.blue >= rgb.red ) && ( rgb.blue >= rgb.green ))
	{
		maxColor = BlueMax;
		maxValue	= blue;}

	//Determine min
	//
	if(( rgb.red <= rgb.green ) && ( rgb.red <= rgb.blue ))
	{
		minValue	= red;
	}
	else if(( rgb.green <= rgb.red ) && ( rgb.green <= rgb.blue ))
	{
		minValue	= green;
	}
	else if(( rgb.blue <= rgb.red ) && ( rgb.blue <= rgb.green ))
	{
		minValue	= blue;
	}

	if( maxValue == minValue ) { maxColor = MaxEqualMin; }

	//Calculate Hue (hsl.hue)
	//
	switch( maxColor )
	{
		case RedMax:
			if( rgb.green >= rgb.blue )
				hue = ( 60 * (( green - blue ) / ( maxValue - minValue )));
			else
				hue = ( 60 * (( green - blue ) / ( maxValue - minValue ))) + 360;

         returnHSL.hue = (long)(( hue / 360 ) * 255 );
		break;

		case GreenMax:
         hue = ( 60 * (( blue - red ) / ( maxValue - minValue ))) + 120;

			returnHSL.hue = (long)(( hue / 360 ) * 255 );
		break;

		case BlueMax:
         hue = ( 60 * (( red - green ) / ( maxValue - minValue ))) + 240;

         returnHSL.hue = (long)(( hue / 360 ) * 255 );
		break;

		case MaxEqualMin:
			if( maxValue == 1.0 )
				returnHSL.hue = 170;
         else
				returnHSL.hue = 255;
		break;
   } /* end switch( MaxColor ) */

	//Calculate Luminance (returnHSL.lum)
	//
	lum = (float)( 0.5  * ( maxValue + minValue ));
	returnHSL.lum	= (long) ( 0.5 * ( maxValue + minValue ) * 255 );

	//Calculate Saturation (returnHSL.sat)
	//
	if(( lum == 0.0 ) || ( maxValue == minValue ))
	{
		returnHSL.sat = 0;
	}
   else if(( lum > 0.0 ) && ( lum <= 0.5 ))
	{
		sat = ( maxValue - minValue) / ( maxValue + minValue );
      returnHSL.sat = (long)( sat * 255 );
	}
   else if( lum > 0.5 )
	{
      sat = ( maxValue - minValue) / ( 2.0 - ( maxValue + minValue ));
		returnHSL.sat = (long)( sat * 255 );
	}

	return returnHSL;
}

RGB CGUIBitmapInfoShaded::hslToRGB( HSL hsl )
{
	// return value
	RGB returnRGB;
	
   float hue, sat, lum, q, p, tempRed, tempGreen, tempBlue = 0.0;

	if( hsl.sat == 0 )
	{
		returnRGB.red = hsl.lum;
		returnRGB.green = hsl.lum;
		returnRGB.blue = hsl.lum;
		return returnRGB;
	}
	
	//	normalize HSL values (values must be in the range of [0,1) ).
	hue = (float)hsl.hue / 255.0;
	sat = (float)hsl.sat / 255.0;
	lum = (float)hsl.lum / 255.0;
	
	//Calculate q
	if( lum < 0.5 )
		q = lum * ( 1.0 + sat );
	else
		q = lum + sat - ( lum * sat );

	//Calculate p
	p = 2.0 * lum - q;

	//Calculate tempRed
	tempRed = hue + ( 1.0 / 3.0 );
   tempRed = checkColorRange( tempRed );

	//Calculate tempGreen
	tempGreen = hue;
   tempGreen = checkColorRange( tempGreen );

	//Calculate tempBlue
	tempBlue = hue - (1.0 / 3.0 );
   tempBlue = checkColorRange( tempBlue );

	//Convert numbers to RGB
	returnRGB.red = hslColorRangeConverter( tempRed, p, q );
	returnRGB.green = hslColorRangeConverter( tempGreen, p, q );
   returnRGB.blue = hslColorRangeConverter( tempBlue, p, q );

	return returnRGB;
}

float CGUIBitmapInfoShaded::checkColorRange( float color )
{
   if( color < 0.0 )
		color += 1.0;
	else if( color > 1.0 )
		color -= 1.0;

	return color;
}

short CGUIBitmapInfoShaded::hslColorRangeConverter( float color, float p, float q )
{
	float returnColor = 0;
	
	if( color < (1.0 / 6.0 ))
	{
		returnColor = p + (( q - p ) * 6.0 * color );
	}
	else if(( color >= (1.0 / 6.0 )) && 
			  ( color < 0.5 ))
	{
		returnColor = q;
	}
	else if(( color >= 0.5 ) && 
			  ( color < ( 2.0 / 3.0 )))
	{
		returnColor = p + (( q - p ) * 6.0 * ((2.0 / 3.0 ) - color ));
	}
	else
	{
		returnColor = p;
	}

	return (short)(returnColor * 255.0);
}


bool RGB::operator == ( const RGB rgb ) const
{
	bool result = false;

	unsigned short color1, color2;

	color1 = (( blue  << 16 ) +
				 ( green  << 8 ) +
				 ( red  ));

	color2 = (( rgb.blue << 16 ) +
				 ( rgb.green << 8 ) +
				 ( rgb.red  ));

	if( color1 == color2 ) 
      result = true;
	else
		result = false;

	return result;
}

bool RGB::operator < ( const RGB rgb ) const
{
	bool result = false;

	unsigned short color1, color2;

	color1 = (( blue << 16 ) +
				 ( green << 8 ) +
				 ( red  ));

	color2 = (( rgb.blue << 16 ) +
				 ( rgb.green << 8 ) +
				 ( rgb.red ));

	if( color1 < color2 ) 
	{
		result = true;
	}
	else
	{
		result = false;
	}

	return result;
}


bool ColorScheme::operator == ( const ColorScheme colorScheme ) const
{
   bool result = true;
	
	do
	{
		if( colorStart != colorScheme.colorStart ) 
		{
			result = false;
			break;
		}

		if( colorEnd != colorScheme.colorEnd )
		{
			result = false;
			break;
		}
		
	} while ( false );	  /* only do-while loop once */

	return result;
}

bool ColorScheme::operator < ( const ColorScheme colorScheme ) const
{
	bool result = false;

	do
	{
		if( colorStart < colorScheme.colorStart ) 
		{
			result = true;
			break;
		}

		if( colorStart == colorScheme.colorStart && 
			 colorEnd < colorScheme.colorEnd )
		{
			result = true;
			break;
		}

	} while ( false );	  /* only do-while loop once */

	return result;
}


bool CGUIBitmapInfoShaded::BitmapMetrics::operator == ( const CGUIBitmapInfoShaded::BitmapMetrics bitmapMetrics ) const
{
	bool result = true;

	do
	{
      if( colorScheme != bitmapMetrics.colorScheme )
		{
			result = false;
			break;
		}

		if( borderWidth != bitmapMetrics.borderWidth )
		{
			result = false;
			break;	
		}

		if( bitmapWidth != bitmapMetrics.bitmapWidth )
		{
			result = false;
			break;
		}		

		if( bitmapHeight != bitmapMetrics.bitmapHeight )
		{
			result = false;
			break;
		}		

	} while( false );   /* only do-while loop once */

	return result;
}

bool CGUIBitmapInfoShaded::BitmapMetrics::operator < ( const CGUIBitmapInfoShaded::BitmapMetrics bitmapMetrics  ) const
{
	bool result = false;

	do
	{
      if( bitmapWidth < bitmapMetrics.bitmapWidth )
		{
			result = true;
			break;
		}		

		if( bitmapWidth == bitmapMetrics.bitmapWidth &&
			 bitmapHeight < bitmapMetrics.bitmapHeight )
		{
			result = true;
			break;
		}

		if( bitmapWidth == bitmapMetrics.bitmapWidth &&
			 bitmapHeight == bitmapMetrics.bitmapHeight &&
			 colorScheme.colorStart < bitmapMetrics.colorScheme.colorStart )
		{
			result = true;
			break;
		}

		if( bitmapWidth == bitmapMetrics.bitmapWidth &&
			 bitmapHeight == bitmapMetrics.bitmapHeight &&
			 colorScheme.colorStart == bitmapMetrics.colorScheme.colorStart &&
          colorScheme.colorEnd < bitmapMetrics.colorScheme.colorEnd )
		{
			result = true;
			break;
		}

		if( bitmapWidth == bitmapMetrics.bitmapWidth &&
			 bitmapHeight == bitmapMetrics.bitmapHeight &&
			 colorScheme.colorStart == bitmapMetrics.colorScheme.colorStart &&
			 colorScheme.colorEnd == bitmapMetrics.colorScheme.colorEnd &&
			 shadeType < bitmapMetrics.shadeType )
		{
			result = true;
			break;
		}

		if( bitmapWidth == bitmapMetrics.bitmapWidth &&
			 bitmapHeight == bitmapMetrics.bitmapHeight &&
			 colorScheme == bitmapMetrics.colorScheme &&
			 shadeType == bitmapMetrics.shadeType &&
			 borderWidth < bitmapMetrics.borderWidth )
		{
			result = true;
			break;
		}

	}while( false );	  /* only do-while loop once */

	return result;
}

