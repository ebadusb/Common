/*
 * $Header: J:/BCT_Development/vxWorks/Common/cgui/rcs/cgui_button_shaded.h 1.1 2008/11/06 22:19:40Z rm10919 Exp rm10919 $
 *
 *		Base class definition for creating buttons using CGUIBitmapInfoShaded to 
 *			create the button bitmaps for each button state ( Released, Disabled, Pressed),
 *			instead of owner of the button creating the button data.
 *
 * $Log: cgui_button_shaded.h $
 *
 */

#ifndef _CGUI_BUTTON_SHADED_INCLUDE
#define _CGUI_BUTTON_SHADED_INCLUDE

#include "cgui/cgui_button.h"

class CGUIButtonShaded
{
public:
	// Constructor for button without text.
   CGUIButtonShaded( CGUIDisplay & display, CGUIWindow * parent, CGUIRegion region, const CGUIBitmapInfoShaded::ColorScheme enabledColorScheme, const CGUIBitmapInfoShaded::ColorScheme disabledColorScheme, const CallbackBase buttonReleasedCallback, const char * buttonId, CGUITextItem * textItem = NULL, CGUIButton::ButtonBehavior buttonBehavoir = CGUIButton::RaiseAfterCallback );

	// Constructor for button WITH text.
   CGUIButtonShaded( CGUIDisplay & display, CGUIWindow * parent, CGUIRegion region, const CGUIBitmapInfoShaded::ColorScheme enabledColorScheme, const CGUIBitmapInfoShaded::ColorScheme disabledColorScheme, const CallbackBase buttonReleasedCallback, const char * buttonId, CGUIButton::ButtonStateType buttonState, CGUIButton::ButtonBehavior buttonBehavoir, CGUITextItem * enabledTextItem , CGUITextItem * disabledTextItem , CGUITextItem * pressedTextItem );

	virtual ~CGUIButtonShaded();

	const CGUIWindow * const getButton(){ return (CGUIWindow *)_button; };
	
	//	Wrapper functions for the button manipulation from the CGUIButton class.
	//
	// 	Set button state.
	void disable();
	void enable();
	void enablePressed();
	void enableReleased();

	//	Set button text.
	// 
	void setText( CGUITextItem * textItem );
   void setText( const StringChar * string );
	void setText();
	void setEnabledText( CGUITextItem * textItem );
   void setEnabledText( const StringChar * string );

	void setStylingRecord( StylingRecord * newStylingRecord );

	//	Set button text color.
	// 
	void setEnabledTextColor( CGUIColor color );
	void setDisabledTextColor( CGUIColor color );
	void setPressedTextColor( CGUIColor color );
	void setTextColor( CGUIColor color );
	
	void addIcon( CGUIBitmapInfo * bitmapInfo, const short x, const short y, CGUIButton::ButtonStateType buttonStateType = CGUIButton::NoButtonState );

	// 	Set message and callback functions.
	//
	void setMessage (Message<long>* newEventMessage);
	void enablePressedMessage();
	void disablePressedMessage();
	void enableReleasedMessage();
   void disableReleasedMessage();

	void setPressedCallback (const CallbackBase cb);
	void setReleasedCallback (const CallbackBase cb);

	void enablePressedCallback();
   void disablePressedCallback();

	void enableReleasedCallback();
   void disableReleasedCallback();
 
	 
protected:
   const CallbackBase _buttonReleasedCallback;

	virtual void buttonCallback();

	CGUIWindow * _parent;

	CGUIRegion _region;

	CGUIButton::ButtonData _buttonData;
	CGUIButton * _button;
   
private:
   CGUIDisplay & _display;
};

#endif /* CGUI_BUTTON_SHADED_INCLUDE */
