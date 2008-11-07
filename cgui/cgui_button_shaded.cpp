/*
 *	Copyright (c) 2004 by Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: Z:/BCT_Development/vxWorks/Common/cgui/rcs/cgui_button_shaded.cpp 1.3 2009/01/08 17:04:54Z rm10919 Exp agauusb $
 *
 *
 * $Log: cgui_button_shaded.cpp $
 *
 */
 
#include <vxWorks.h>

#include "cgui_bitmap_info_shaded.h"
#include "cgui_button_shaded.h"

CGUIButtonShaded::CGUIButtonShaded( CGUIDisplay & display, CGUIWindow * parent, CGUIRegion region, const CGUIBitmapInfoShaded::ColorScheme enabledColorScheme, const CGUIBitmapInfoShaded::ColorScheme disabledColorScheme, const CallbackBase buttonReleasedCallback, const char * buttonId, CGUITextItem * textItem, CGUIButton::ButtonBehavior buttonBehavoir ):
												_display( display ), _parent( parent ),
												_region( region ),
												_buttonReleasedCallback( buttonReleasedCallback )
{
	_buttonData.left = _region.x;
	_buttonData.top = _region.y;
	_buttonData.vMargin = 3;
	_buttonData.hMargin = 3;

	_buttonData.enabledBitmapId = CGUIBitmapInfoShaded::createShadedBitmapData( enabledColorScheme, CGUIBitmapInfoShaded::BottomTop, 1, _region.width, _region.height );
	_buttonData.enabledTextItem = textItem;
	_buttonData.enabledStylingRecord = NULL;
	_buttonData.enabledButtonIcon = NULL;

	_buttonData.disabledBitmapId =  CGUIBitmapInfoShaded::createShadedBitmapData( disabledColorScheme, CGUIBitmapInfoShaded::BottomTop, 1, _region.width, _region.height );
	_buttonData.disabledTextItem = textItem;
	_buttonData.disabledStylingRecord = NULL;
	_buttonData.disabledButtonIcon = NULL;

	_buttonData.pressedBitmapId = CGUIBitmapInfoShaded::createShadedBitmapData( enabledColorScheme, CGUIBitmapInfoShaded::TopBottom, 1, _region.width, _region.height );
	_buttonData.pressedTextItem = NULL;
	_buttonData.pressedStylingRecord = NULL;
	_buttonData.pressedButtonIcon = NULL;

	_buttonData.type = buttonBehavoir;
	
	if ( buttonId ) strcpy( _buttonData.alternateButtonId, buttonId );

	_button = new CGUIButton( _display, _parent, _buttonData );
	_button->setReleasedCallback( Callback<CGUIButtonShaded>( this, &CGUIButtonShaded::buttonCallback ));
	_button->enable();
}

CGUIButtonShaded::CGUIButtonShaded( CGUIDisplay & display, CGUIWindow * parent, CGUIRegion region, const CGUIBitmapInfoShaded::ColorScheme enabledColorScheme, const CGUIBitmapInfoShaded::ColorScheme disabledColorScheme, const CallbackBase buttonReleasedCallback, const char * buttonId, CGUIButton::ButtonStateType buttonState, CGUIButton::ButtonBehavior buttonBehavoir, CGUITextItem * enabledTextItem , CGUITextItem * disabledTextItem , CGUITextItem * pressedTextItem ):
												_display( display ), _parent( parent ),
												_region( region ),
												_buttonReleasedCallback( buttonReleasedCallback )

{
	_buttonData.left = _region.x;
	_buttonData.top = _region.y;
	_buttonData.vMargin = 3;
	_buttonData.hMargin = 3;

	_buttonData.enabledBitmapId = CGUIBitmapInfoShaded::createShadedBitmapData( enabledColorScheme, CGUIBitmapInfoShaded::BottomTop, 1, _region.width, _region.height );
	_buttonData.enabledTextItem = enabledTextItem;
	_buttonData.enabledStylingRecord = NULL;
	_buttonData.enabledButtonIcon = NULL;

	_buttonData.disabledBitmapId =  CGUIBitmapInfoShaded::createShadedBitmapData( disabledColorScheme, CGUIBitmapInfoShaded::BottomTop, 1, _region.width, _region.height );
	_buttonData.disabledTextItem = disabledTextItem;
	_buttonData.disabledStylingRecord = NULL;
	_buttonData.disabledButtonIcon = NULL;

	_buttonData.pressedBitmapId = CGUIBitmapInfoShaded::createShadedBitmapData( enabledColorScheme, CGUIBitmapInfoShaded::TopBottom, 1, _region.width, _region.height );
	_buttonData.pressedTextItem = pressedTextItem;
	_buttonData.pressedStylingRecord = NULL;
	_buttonData.pressedButtonIcon = NULL;

	_buttonData.type = buttonBehavoir;

	if ( buttonId ) strcpy( _buttonData.alternateButtonId, buttonId );

	_button = new CGUIButton( _display, _parent, _buttonData );
	_button->setReleasedCallback( Callback<CGUIButtonShaded>( this, &CGUIButtonShaded::buttonCallback ));

   switch( buttonState )
	{
		case CGUIButton::Pressed:
			_button->enablePressed();
			break;

		case CGUIButton::NoButtonState:
		case CGUIButton::Released:
			_button->enableReleased();
			break;

		case CGUIButton::Disabled:
			_button->disable();
			break;
	}
}

CGUIButtonShaded::~CGUIButtonShaded()
{
	if( _button ) delete _button;
}

void CGUIButtonShaded::buttonCallback()
{
	_buttonReleasedCallback();
}


// Set button state functions.
//
void CGUIButtonShaded::disable()
{
	_button->disable();
}

void CGUIButtonShaded::enable()
{
	_button->enable();
}

void CGUIButtonShaded::enablePressed()
{
	_button->enablePressed();
}

void CGUIButtonShaded::enableReleased()
{
	_button->enableReleased();
}

// Set button text functions.
//
void CGUIButtonShaded::setText( CGUITextItem * textItem )
{
	_button->setText( textItem );
}

void CGUIButtonShaded::setText( const StringChar * string )
{
	_button->setText( string );
}

void CGUIButtonShaded::setText()
{
	_button->setText();
}

void CGUIButtonShaded::setEnabledText( CGUITextItem * textItem )
{
	_button->setEnabledText( textItem );
}

void CGUIButtonShaded::setEnabledText( const StringChar * string )
{
	_button->setEnabledText( string );
}

void CGUIButtonShaded::setStylingRecord( StylingRecord * newStylingRecord )
{
	_button->setStylingRecord( newStylingRecord );
}


void CGUIButtonShaded::setEnabledTextColor( CGUIColor color )
{
	_button->setEnabledTextColor( color );
}

void CGUIButtonShaded::setDisabledTextColor( CGUIColor color )
{
	_button->setDisabledTextColor( color );
}


void CGUIButtonShaded::setPressedTextColor( CGUIColor color )
{
	_button->setPressedTextColor( color );
}


void CGUIButtonShaded::setTextColor( CGUIColor color )
{
	_button->setTextColor( color );
}


void CGUIButtonShaded::addIcon( CGUIBitmapInfo * bitmapInfo, const short x, const short y, CGUIButton::ButtonStateType buttonStateType = CGUIButton::NoButtonState )
{
	_button->addIcon( bitmapInfo, x, y, buttonStateType );
}


//	Messages and Callbacks
// 
void CGUIButtonShaded::setMessage( Message<long>* newEventMessage )
{
	_button->setMessage( newEventMessage );
}

void CGUIButtonShaded::enablePressedMessage()
{
	_button->enablePressedMessage();
}

void CGUIButtonShaded::disablePressedMessage()
{
	_button->disablePressedMessage();
}

void CGUIButtonShaded::enableReleasedMessage()
{
	_button->enableReleasedMessage();
}

void CGUIButtonShaded::disableReleasedMessage()
{
	_button->disableReleasedMessage();
}


void CGUIButtonShaded::setPressedCallback( const CallbackBase cb )
{
	_button->setPressedCallback( cb );
}

void CGUIButtonShaded::setReleasedCallback( const CallbackBase cb )
{
	_button->setReleasedCallback( cb );
}


void CGUIButtonShaded::enablePressedCallback()
{
	_button->enablePressedCallback();
}

void CGUIButtonShaded::disablePressedCallback()
{
	_button->disablePressedCallback();
}


void CGUIButtonShaded::enableReleasedCallback()
{
	_button->enableReleasedCallback();
}

void CGUIButtonShaded::disableReleasedCallback()
{
	_button->disableReleasedCallback();
}

