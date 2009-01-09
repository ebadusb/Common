/*
 *	Copyright (c) 2004 by Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: Z:/BCT_Development/vxWorks/Common/cgui/rcs/cgui_button_shaded.cpp 1.3 2009/01/08 17:04:54Z rm10919 Exp agauusb $
 *
 *
 * $Log: cgui_button_shaded.cpp $
 * Revision 1.2  2009/01/08 00:55:21Z  rm10919
 * Updates and bug fixes for shaded buttons.
 * Revision 1.1  2008/11/06 22:19:42Z  rm10919
 * Initial revision
 *
 */
 
#include <vxWorks.h>

#include "cgui_bitmap_info_shaded.h"
#include "cgui_button_shaded.h"

//	Simple Buttons
CGUIButtonShaded::CGUIButtonShaded( CGUIDisplay & display, CGUIWindow * parent, CGUIRegion region, const ColorScheme enabledColorScheme, const ColorScheme disabledColorScheme, const ShadeType shadeType, const char * buttonId, CGUIButton::ButtonBehavior buttonBehavoir, CGUITextItem * textItem ):
												CGUIButton( display )
{
	CGUIButton::ButtonData buttonData;
	
	buttonData.left = region.x;
	buttonData.top = region.y;
	buttonData.vMargin = 3;
	buttonData.hMargin = 3;

	buttonData.enabledBitmapId = CGUIBitmapInfoShaded::createShadedBitmapData( enabledColorScheme, shadeType, 1, region.width, region.height );
	buttonData.enabledTextItem = textItem;
	buttonData.enabledStylingRecord = NULL;
	buttonData.enabledButtonIcon = NULL;

	buttonData.disabledBitmapId =  CGUIBitmapInfoShaded::createShadedBitmapData( disabledColorScheme, shadeType, 1, region.width, region.height );
	buttonData.disabledTextItem = textItem;
	buttonData.disabledStylingRecord = NULL;
	buttonData.disabledButtonIcon = NULL;

	buttonData.pressedBitmapId = CGUIBitmapInfoShaded::createShadedBitmapData( enabledColorScheme, getPressedShadeType( shadeType ), 1, region.width, region.height );
	buttonData.pressedTextItem = NULL;
	buttonData.pressedStylingRecord = NULL;
	buttonData.pressedButtonIcon = NULL;

	buttonData.type = buttonBehavoir;
	
	if ( buttonId ) strcpy( buttonData.alternateButtonId, buttonId );

	setCGUIButton( display, parent, buttonData );
}

CGUIButtonShaded::CGUIButtonShaded( CGUIDisplay & display, CGUIWindow * parent, CGUIRegion region, ButtonDataShaded & enabledButtonDataShaded, ButtonDataShaded & disabledButtonDataShaded, const char * buttonId, CGUIButton::ButtonBehavior buttonBehavoir ):
												CGUIButton( display )
{		
	CGUIButton::ButtonData buttonData;
	
	buttonData.left = region.x;
	buttonData.top = region.y;
	buttonData.vMargin = 3;
	buttonData.hMargin = 3;

	buttonData.enabledBitmapId = CGUIBitmapInfoShaded::createShadedBitmapData( enabledButtonDataShaded.colorScheme, enabledButtonDataShaded.shadeType, enabledButtonDataShaded.borderWidth, region.width, region.height );
	buttonData.enabledTextItem = enabledButtonDataShaded.textItem;
	buttonData.enabledStylingRecord = enabledButtonDataShaded.stylingRecord;
	buttonData.enabledButtonIcon = enabledButtonDataShaded.buttonIcon;

	buttonData.disabledBitmapId =  CGUIBitmapInfoShaded::createShadedBitmapData( disabledButtonDataShaded.colorScheme, disabledButtonDataShaded.shadeType, disabledButtonDataShaded.borderWidth, region.width, region.height );
	buttonData.disabledTextItem = disabledButtonDataShaded.textItem;
	buttonData.disabledStylingRecord = disabledButtonDataShaded.stylingRecord;
	buttonData.disabledButtonIcon = disabledButtonDataShaded.buttonIcon;

	buttonData.pressedBitmapId = CGUIBitmapInfoShaded::createShadedBitmapData( enabledButtonDataShaded.colorScheme, getPressedShadeType( enabledButtonDataShaded.shadeType ), enabledButtonDataShaded.borderWidth, region.width, region.height );
	buttonData.pressedTextItem = NULL;
	buttonData.pressedStylingRecord = NULL;
	buttonData.pressedButtonIcon = NULL;

	buttonData.type = buttonBehavoir;
	
	if ( buttonId ) strcpy( buttonData.alternateButtonId, buttonId );

	setCGUIButton( display, parent, buttonData );
}

// Special Buttons
CGUIButtonShaded::CGUIButtonShaded( CGUIDisplay & display, CGUIWindow * parent, CGUIRegion region, ButtonDataShaded & enabledButtonDataShaded, ButtonDataShaded & disabledButtonDataShaded, ButtonDataShaded & pressedButtonDataShaded, const char * buttonId, CGUIButton::ButtonBehavior buttonBehavoir ):
												CGUIButton( display )
{
	CGUIButton::ButtonData buttonData;
	
	buttonData.left = region.x;
	buttonData.top = region.y;
	buttonData.vMargin = 3;
	buttonData.hMargin = 3;

	buttonData.enabledBitmapId = CGUIBitmapInfoShaded::createShadedBitmapData( enabledButtonDataShaded.colorScheme, enabledButtonDataShaded.shadeType, enabledButtonDataShaded.borderWidth, region.width, region.height );
	buttonData.enabledTextItem = enabledButtonDataShaded.textItem;
	buttonData.enabledStylingRecord = pressedButtonDataShaded.stylingRecord;
	buttonData.enabledButtonIcon = pressedButtonDataShaded.buttonIcon;

	buttonData.disabledBitmapId =  CGUIBitmapInfoShaded::createShadedBitmapData( disabledButtonDataShaded.colorScheme, disabledButtonDataShaded.shadeType, disabledButtonDataShaded.borderWidth, region.width, region.height );
	buttonData.disabledTextItem = pressedButtonDataShaded.textItem;
	buttonData.disabledStylingRecord = pressedButtonDataShaded.stylingRecord;
	buttonData.disabledButtonIcon = pressedButtonDataShaded.buttonIcon;

	buttonData.pressedBitmapId = CGUIBitmapInfoShaded::createShadedBitmapData( pressedButtonDataShaded.colorScheme, pressedButtonDataShaded.shadeType, pressedButtonDataShaded.borderWidth, region.width, region.height );
	buttonData.pressedTextItem = pressedButtonDataShaded.textItem;
	buttonData.pressedStylingRecord = pressedButtonDataShaded.stylingRecord;
	buttonData.pressedButtonIcon = pressedButtonDataShaded.buttonIcon;

	buttonData.type = buttonBehavoir;

	if ( buttonId ) strcpy( buttonData.alternateButtonId, buttonId );

	setCGUIButton( display, parent, buttonData );
}

CGUIButtonShaded::~CGUIButtonShaded()
{

}


ShadeType CGUIButtonShaded::getPressedShadeType( const ShadeType enabledShadeType )
{
	ShadeType pressedShadeType = NoShade;

	switch( enabledShadeType )
	{
		case NoShade:
			pressedShadeType = NoShade;
			break;

		case Solid:
			pressedShadeType = Solid;
			break;

		case TopBottom:
			pressedShadeType = BottomTop;
			break;

		case BottomTop:
			pressedShadeType = TopBottom;
			break;

		case LeftRight:
			pressedShadeType = RightLeft;
			break;

		case RightLeft:
			pressedShadeType = LeftRight;
			break;
	}

	return pressedShadeType;
}

