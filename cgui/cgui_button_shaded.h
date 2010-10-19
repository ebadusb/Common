/*
 * $Header$
 *
 *		Base class definition for creating buttons using CGUIBitmapInfoShaded to 
 *			create the button bitmaps for each button state ( Released, Disabled, Pressed),
 *			instead of owner of the button creating the button data.
 *
 * $Log: cgui_button_shaded.h $
 * Revision 1.1  2008/11/06 22:19:40Z  rm10919
 * Initial revision
 *
 */

#ifndef _CGUI_BUTTON_SHADED_INCLUDE
#define _CGUI_BUTTON_SHADED_INCLUDE

#include "cgui_button.h"
#include "cgui_bitmap_info_shaded.h"

class CGUIButtonShaded : public CGUIButton
{
public:

	struct ButtonDataShaded
	{
		ColorScheme 	colorScheme;
		ShadeType		shadeType;
      BitmapShape		bitmapShape;
		int				borderWidth;
		CGUITextItem	*	textItem;
		StylingRecord	*	stylingRecord;
		ButtonIcon		*	buttonIcon;		

		ButtonDataShaded(void){ memset( this, 0, sizeof( *this )); bitmapShape = NoShape; };
	};
	
	// Constructors for simple buttons.
   CGUIButtonShaded( CGUIDisplay & display, CGUIWindow * parent, CGUIRegion region, const ColorScheme enabledColorScheme, const ColorScheme disabledColorScheme, const ShadeType shadeType, const char * buttonId, CGUIButton::ButtonBehavior buttonBehavoir = CGUIButton::RaiseAfterCallback, CGUITextItem * textItem = NULL );
   CGUIButtonShaded( CGUIDisplay & display, CGUIWindow * parent, CGUIRegion region, ButtonDataShaded & enabledButtonDataShaded, ButtonDataShaded & disabledButtonDataShaded, const char * buttonId, CGUIButton::ButtonBehavior buttonBehavoir = CGUIButton::RaiseAfterCallback );

	// Constructor for all other buttons.
   CGUIButtonShaded( CGUIDisplay & display, CGUIWindow * parent, CGUIRegion region, ButtonDataShaded & enabledButtonDataShaded, ButtonDataShaded & disabledButtonDataShaded, ButtonDataShaded & pressedButtonDataShaded, const char * buttonId, CGUIButton::ButtonBehavior buttonBehavoir );

	virtual ~CGUIButtonShaded();

//	const CGUIWindow * const getButton(){ return (CGUIWindow *)_button; };
	
protected:

	ShadeType getPressedShadeType( const ShadeType enabledShadeType );
	
private:
	CGUIButtonShaded();
	CGUIButtonShaded( CGUIButton & copy );
	CGUIButtonShaded operator = ( CGUIButtonShaded & object );   
};

#endif /* CGUI_BUTTON_SHADED_INCLUDE */
