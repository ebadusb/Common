/*
 *	Copyright (c) 2004 by Gambro BCT, Inc.  All rights reserved.
 *
 *  $Header: K:/BCT_Development/vxWorks/Common/cgui/rcs/cgui_button.h 1.28 2009/01/08 00:55:20Z rm10919 Exp wms10235 $ 
 *  This file defines the base class for all button styles in the common GUI.  An object of this class types
 *  can be used to generate a standard button.
 *  
 *  $Log: cgui_button.h $
 *  Revision 1.26  2008/11/06 22:24:15Z  rm10919
 *  Add transparent and shaded bitmaps and shaded buttons.
 *  Revision 1.25  2008/05/20 20:29:20Z  jl11312
 *  - corrected handling of alternate logging text
 *  Revision 1.24  2006/11/13 20:21:14Z  jd11007
 *  IT 65 - Memory leak fixes.
 *  Revision 1.23  2006/11/01 16:35:34Z  rm10919
 *  Add enableWhenPressed to have enable bitmap move to front regardless of button state.
 *  Revision 1.22  2006/10/11 21:33:15Z  rm10919
 *  Take account for vMargin and hMargin in determining text region.
 *  Revision 1.21  2006/10/07 19:27:41Z  cf10242
 *  IT 59: generic button press logging
 *  Revision 1.20  2006/09/18 23:38:28Z  cf10242
 *  IT 56: allow button to be attached to root window
 *  Revision 1.19  2006/06/16 16:10:07Z  MS10234
 *   - add a variable to keep the state of the button based on the event callback
 *  Revision 1.18  2006/05/15 21:48:45Z  rm10919
 *  Add setTextColor method to change text color for all states of the button.
 *  Revision 1.17  2005/09/30 22:40:42Z  rm10919
 *  Get the variable database working!
 *  Revision 1.16  2005/04/26 23:16:47Z  rm10919
 *  Made changes to cgui_text and cgui_text_item, plus added 
 *  classes for variable substitution in text strings.
 *  Revision 1.15  2005/04/04 18:02:57Z  rm10919
 *  Add ability to add char * strings to button text.
 *  Revision 1.14  2005/03/15 00:21:35Z  rm10919
 *  Change CGUIText to not add object to window object list of parent in constructor.
 *  Revision 1.13  2005/02/21 17:17:11Z  cf10242
 *  IT 133 - delete all allocated memory to avoid unrecovered memory
 *  Revision 1.12  2005/01/28 23:52:17Z  rm10919
 *  CGUITextItem class changed and put into own file.
 *  Revision 1.11  2005/01/03 20:41:25Z  cf10242
 *  add an enablePressed method a button can shown as enabled and pressed
 *  Revision 1.10  2004/12/17 15:10:18Z  rm10919
 *  Change disableIcon to public not protected.
 *  Revision 1.9  2004/11/19 18:14:47Z  cf10242
 *  Integration checkin
 *  Revision 1.8  2004/11/18 22:33:37Z  rm10919
 *  Added ability to modify button text.
 *  Revision 1.7  2004/11/04 20:19:08Z  rm10919
 *  Common updates and changes.
 *  Revision 1.6  2004/11/02 20:48:19Z  rm10919
 *  change setText() fucntions & add checks for bitmaps in enable() & disable().
 *  Revision 1.5  2004/11/01 17:27:22Z  cf10242
 *  Change TextItem to CGUITextItem
 *  Revision 1.4  2004/10/29 15:11:13Z  rm10919
 *  Revision 1.3  2004/10/22 20:14:34Z  rm10919
 *  CGUIButton updates and changes.
 *  Revision 1.2  2004/09/30 17:00:52Z  cf10242
 *  Correct for initial make to work
 *  Revision 1.1  2004/09/20 18:18:04Z  rm10919
 *  Initial revision
 *
 */
#ifndef _CGUI_BUTTON_INCLUDE
#define _CGUI_BUTTON_INCLUDE

#include "message.h"
#include "datalog_levels.h"
#include "cgui_window.h"
#include "cgui_text.h"
#include "cgui_bitmap.h"

#define MAX_BUTTON_LOG_SIZE  64

// CGUIButton is the parent class for all button styles.  Specialized buttons
// can derive from this class and pick up many of the standard behaviors of
// a button, but override some features for specialized behavior.
// The public methods of the class are grouped below into categories based on the things they affect.
// The groups are: messages/callbacks, bitmaps, audio, text, and icons.
// There are some basic rules in this class that are important to its usage:
// 1) all buttons are bitmaps,
// 2) a button has one of the following states: invisible, enabled, disabled, pressed, released,
// 3) there is one text string associated with the button (although the styling can be different by state),
// 4) there is one icon associated with the button,
// 5) there is one message associated with the button press,
// 6) there is one message associated with the button release,
// 7) there is one callback routine associated with a button press,
// 8) there is one callback routine associated with a button release


class CGUIButton : public CGUIWindow
{    
public:
   enum ButtonBehavior
   {
      RaiseAfterRelease,         // button switches back to up bitmap on touch release, then executes callback
      RaiseAfterCallback,        // callback is executed on touch release, then button switches back to up bitmap
      Manual                     // callback is executed on touch release, button stays down (setUp must be used to raise button again)
   };

	enum ButtonStateType
	{
		Released,
		Pressed,
		Disabled,
		NoButtonState
	};

struct ButtonIcon
{
   ButtonIcon( void ){ _iconBitmap = NULL; _buttonStateType = NoButtonState; _iconId = 0; };
	
   ButtonIcon( CGUIDisplay & display, CGUIBitmapInfo * iconBitmapInfo, ButtonStateType buttonStateType, short x, short y ):
               _buttonStateType( buttonStateType )
	{ 
		_iconId = setIconId();
		_iconBitmap = new CGUIBitmap( display, CGUIRegion( x, y, 0, 0 ), *iconBitmapInfo);
	};

	~ButtonIcon(void)
	{ 
		if( _iconBitmap ) delete _iconBitmap;
	}

   int getIconId( void ){ return _iconId; };

	void setButtonStateType( ButtonStateType buttonStateType ){ _buttonStateType = buttonStateType; };
	CGUIBitmap * getBitmap( void ){ return _iconBitmap; };
	ButtonStateType getButtonStateType( void ){ return _buttonStateType; };
	void setVisible( bool newVisible ){ _iconBitmap->setVisible( newVisible ); };
   
private:

	CGUIBitmap * _iconBitmap;
   ButtonStateType _buttonStateType;
	int _iconId;

	static int _iconCounter;

   int setIconId( void ){ if( _iconId < 1 ) _iconId = _iconCounter++; return _iconId; };
};
  
   struct ButtonData
   {
      unsigned short left;               // position of top/left corner of button in screen pixels
      unsigned short top;

      unsigned short vMargin;            // vertical and horizontal margins from edge of button to
      unsigned short hMargin;            // where any button labels are allowed to start

      CGUIBitmapInfo	* enabledBitmapId;		// enabled state bitmap id
      CGUITextItem	* enabledTextItem;		// label text (if any) used in enabled state
      StylingRecord	* enabledStylingRecord;	// label text styling information in enabled state
		ButtonIcon		* enabledButtonIcon;		//	background icon associated with the enabled state

      CGUIBitmapInfo	* disabledBitmapId;		// disableded state bitmap id
      CGUITextItem	* disabledTextItem;		// attributes for label text used in disabled state
      StylingRecord	* disabledStylingRecord;// label text styling information in disableded state
		ButtonIcon		* disabledButtonIcon;	//	background icon associated with the disabled state

      CGUIBitmapInfo	* pressedBitmapId;		// pressed state bitmap id
      CGUITextItem	* pressedTextItem;		// label text used in pressed state
      StylingRecord	* pressedStylingRecord;	// label text styling information in pressed state
		ButtonIcon		* pressedButtonIcon;		//	background icon associated with the pressed state

		char alternateButtonId[MAX_BUTTON_LOG_SIZE+1];	// enabled text item ID is used.  If none, then this field is used

      ButtonBehavior type;							// button behavior  

		ButtonData(void){ memset( this, 0, sizeof( *this )); type = RaiseAfterRelease; };
   };


protected:

	ButtonStateType			_buttonState;

   Message<long>           *_buttonMessagePointer; // used to communicate a message to other tasks when a button is pressed and released
   bool                     _enabled;              // current enabled/disabled state
   
   CGUIText                *_enabledText;          // ptr to current text object
   CGUIBitmap              *_enabledBitmap;        // ptr to enabled bitmap object
   
   CGUIText                *_disabledText;         // ptr to disabled text object
   CGUIBitmap              *_disabledBitmap;       // ptr to disabled bitmap object
   
   CGUIText                *_pressedText;          // ptr to pressed text object
   CGUIBitmap              *_pressedBitmap;        // ptr to pressed bitmap object

   CGUIRegion              _textRegion;            // area in which text can appear, uses v and h margin
   bool                    _haveTextRegion;        // flag to use textRegion member

   Message<long>           *_audioMessagePointer;  // ptr to audio message to send when button is pressed
   CallbackBase             _CBOnPressed;          // callback object to use when button is pressed
   CallbackBase             _CBOnReleased;         // callback object to use when button is released
   
   bool                     _pressEventMessageEnabled;   // flag is true if message is sent on press of button
   bool                     _releasedEventMessageEnabled;// flag is true if message is sent on release of button
   bool                     _pressedCBEnabled;     // flag is true if callback is made on button press
   bool                     _releasedCBEnabled;    // flag is true is callbacl is made on button release
   
   CGUIBitmap              *_iconPointer;          // ptr to the icon bitmap object

	list< ButtonIcon *> _iconList;						// list of transparent icons
   ButtonBehavior           _behaviorType;         // how does button behave when pressed
   
   DataLog_Level           *_btnDataLogLevel;      // level at which to log button press events

   char 				   _buttonPressLogText[MAX_BUTTON_LOG_SIZE+1];  // button press logging text
   bool					_alternateLogTextUsed;			// true if alternate log text was specified

	StylingRecord *			_enabledStylingRecordSaved;	// so that we can delete it upon destruction
	StylingRecord *			_disabledStylingRecordSaved;	// so that we can delete it upon destruction
	StylingRecord *			_pressedStylingRecordSaved;	// so that we can delete it upon destruction

public:
   // CONSTRUCTOR
   CGUIButton  (CGUIDisplay   & display,                 // reference to a cguidisplay object for display context
                CGUIWindow    * parent,                  // pointer to a parent window
                ButtonData    & buttonData,              // reference to button data for bitmaps, text and behavoir
                Message<long> * pressEventObject = NULL, // ptr to int message object to output when button is pressed and released
                                                         // can be NULL to indicate no message is output
                Message<long> * audioMsg = NULL,         // ptr to audio message to send when button is pressed
                DataLog_Level * buttonLevel = NULL,      // datalog level object used to log button press events
                bool            enabled = true,          // button will be constructed as disabled unless specified here
                bool            visible = true,          // button will be constructed as visbile unless otherwise specified here
                bool            pressed = false
                );

   // DESTRUCTOR
   virtual ~CGUIButton ();

   // ENABLE
   // set the state of the button to enabled.  If currently invisible, the button is made visible
   void enable(void);

   // ENABLEWHENPRESSED
   // set the state of the button to enabled with enable bitmap on top.  If currently invisible, the button is made visible.
   void enableWhenPressed(void);
	void enableReleased( void );

	// ENABLEPRESSED
	void enablePressed(void);

   // DISABLE
   // set the state of the button to disabled.  If currently invisible, the button is made visible
   void disable();

   // IS ENABLED
   // returns true if the button is enabled
   bool isEnabled() const {return _enabled;};

   virtual void attach(CGUIWindow * window);

   // INVISIBLE
   // make the button invisible.  Uses CGUIWindow class to do this.
   void invisible();

   // IS VISIBLE
   // return flag indicating current state of visibility.  Uses CGUIWindow class to do this
   bool isVisible() const;

   // MESSAGE, CALLBACK METHODS //

   //SET MESSAGE
   // set an int message to be output when the button is pressed.  The default is for a message to be sent
   // on press and on release with the value of the message indicating which event occured.  This
   // behavior can be modified to only send a message on press or release by using the enable/disable 
   // methods provided below.  
   // NULL removes a previously set message from being sent.
   // There can be only one pressed message per button.  The message is automatically enabled after this call.
   void setMessage (Message<long>* newEventMessage);

   //ENABLE PRESSED MESSAGE
   // enables a message sent on button press
   void enablePressedMessage();

   //DISABLE PRESSED MESSAGE
   // disables message sent on button press
   void disablePressedMessage();

   //ENABLE RELEASED MESSAGE
   // enables message sent on button release
   void enableReleasedMessage();

   //DISABLE RELEASED MESSAGE
   // disables message sent on button release
   void disableReleasedMessage();

   //SET PRESSED CALLBACK
   // This method establishes the pointer to the routine that gets called when the button is pressed.
   // There can be only one pressed callback per button.
   // The callback is automatically enabled after this call.
   void setPressedCallback (const CallbackBase cb);

   //SET RELEASED CALLBACK
   // This method establishes the pointer to the routine that gets called when the button is released.
   // There can be only one released callback per button.
   // The callback is automatically enabled after this call.
   void setReleasedCallback (const CallbackBase cb);

   //ENABLE PRESSED CALLBACK
   void enablePressedCallback();

   //DISABLE PRESSED CALLBACK
   // keeps the call back from last set_pressed_callback, but disables it until an
   // enable_pressed_callback is called.
   void disablePressedCallback();

   //ENABLE RELEASED CALLBACK
   void enableReleasedCallback();

   //DISABLE RELEASED CALLBACK
   // keeps the call back from last set_pressed_callback, but disables it until an
   // enable_pressed_callback is called.
   void disableReleasedCallback();

   // POINTER EVENT
   // virtual function in CGUIWindow class that is implemented here for buttons to apply the
   // correct callback function and send the correct message based on the action taken 
   // by the operator, i.e. press, release
   virtual void pointerEvent(const PointerEvent & event); // event structure received from UGL on button press/release

   // BITMAP METHODS //

   //SET ENABLED BITMAP
   // loads a new enabled bitmap object for the button.
   void setEnabledBitmap(CGUIBitmapInfo *enableBitmapInfo); // ptr to bitmap object to display when button is enabled

   //SET DISABLED BITMAP
   // loads a new disabled bitmap object for the button.
   void setDisabledBitmap(CGUIBitmapInfo *disableBitmapInfo); // ptr to bitmap object to display when button is disabled

   // SET PRESSED BITMAP
   // loads a new pressed bitmap object for the button
   void setPressedBitmap(CGUIBitmapInfo *pressedBitmapInfo); // ptr to bitmap object to display when button is pressed

   // AUDIO METHODS //

   // ENABLE AUDIO
   // enable the audio feedback when the button is pressed
   void enableAudio();

   // DISABLE AUDIO
   // disable the audio feedback when the button is pressed
   void disableAudio();

   // SET AUDIO
   // set the audio associated with a button press.
   void setAudio(Message<long> *audioObject);

   // TEXT METHODS //

   // SET TEXT
   // set the text associated with the button.  
   void setText(CGUITextItem * textItem); // ptr to a text object associated with the button
   void setText(const char * string); // ptr to a text object associated with the button
   void setText(const StringChar * string); // ptr to a text object associated with the button
   void setText();  // forces a redraw or update to button text.
  
   void setEnabledText(CGUITextItem * textItem);
   void setEnabledText(const char * string);
   void setEnabledText(const StringChar * string);
   void setEnabledText();
   void setDisabledText(CGUITextItem * textItem);
   void setDisabledText(const char * string);
   void setDisabledText(const StringChar * string);
   void setDisabledText();
   void setPressedText(CGUITextItem * textItem);
   void setPressedText(const char * string);
   void setPressedText(const StringChar * string);
   void setPressedText();

   // SET TEXTSTYLE
   // set/change the style of the text associated with this button in ALL states.  This is a pass-thru to the 
   // text object previously set with this button.  
   void setStylingRecord(StylingRecord * textStylingRecord); // style record with appropriate features set

   // SET ENABLED TEXTSTYLE 
   // SET DISABLED TEXTSTYLE
   // SET PRESSED TEXTSTYLE
   // set/change the style of text associated with this button for each of the four states that the button
   // can be in.  Note these call does nothing if there is no text associated with the button.
   // They will work if the text is disabled.
   void setEnabledStylingRecord(StylingRecord * enabledTextStylingRecord); // style record with appropriate features set
   void setDisabledStylingRecord(StylingRecord * disabledTextStylingRecord); //	style record with appropriate features set
   void setPressedStylingRecord(StylingRecord * pressedTextStylingRecord); // style record with appropriate features set

   StylingRecord * getEnabledStylingRecord(void){ return _enabledText->getStylingRecord();}
   StylingRecord * getDisabledStylingRecord(void){ return _disabledText->getStylingRecord();}
   StylingRecord * getPressedStylingRecord(void){ return _pressedText->getStylingRecord();}
  
   void setEnabledTextColor(CGUIColor color);
   void setDisabledTextColor(CGUIColor color);
   void setPressedTextColor(CGUIColor color);
   void setTextColor(CGUIColor color);
   
   // ICON METHODS //

   // SET ICON
   // set an icon to be associated with the button. 
   void setIcon(CGUIBitmapInfo *iconId,            // ptr to bitmap object for icon
                const short x=-1,             // upper left X coordinate to start icon relative to button location (0 means coincident with x coordinate of upper left position of button)
                const short y=-1,             // upper left Y coordinate to start icon relative to button location
                bool visible=true             // visibility of icon upon creation, uses CGUIBitmap class to set visibility
               );

   // ENABLE ICON
   // set a previously set icon as visible on the button
   void enableIcon();

   // DISABLE ICON
   // set a previously set icon as invisible
   void disableIcon();

	// icon management
	// add icon object to button icon list
	int addIcon( CGUIBitmapInfo * bitmapInfo, const short x, const short y, ButtonStateType buttonStateType = NoButtonState );

	// change the button state type of an icon in the list
	bool setIconButtonStateType( int iconId, ButtonStateType buttonStateType );
	
	// remove icon from icon list
	bool removeIcon( int iconId );

protected:
   // The following methods are called when state of the button is changed.  These can be overriden
   // and supllemented by derived types if different behavior is needed.
   // DO ON PRESS
   // actions performed by the button when it is pressed.  Actions are:
   // log button press event to data log if provided
   // verify button is enabled (disabled means the following actions are skipped)
   // generate audio feedback associated with button press, if any
   // perform callback associated with button press, if any
   // send event message associated with button press (=ButtonPress), if any
   // display pressed bitmap if provided (otherwise leave the current bitmap up)
   // display text in "pressed" textstyle if provided
   virtual void doOnPress();

   // DO ON RELEASE
   // actions performed when the button is released.  Actions are:
   // perform callback associated with button release, if any
   // send event message associated with button release, if any
   // display enabled bitmap
   // display text in "released" textstyle if provided
   virtual void doOnRelease();

   // DO ON ENABLE
   // actions performed when the button is enabled.
   // Actions are:
   //  	if the button is invisible, reset to visible internally and with parent
   //   display enabled bitmap
   //   display text in "enabled" text style 
   virtual void doOnEnable();

   // DO ON DISABLE
   // action performed when button is disabled.
   // Actions are:
   //  	if the button is invisible, reset to visible internally and with parent
   //   display disabled bitmap
   //   display text in "disabled" text style
   virtual void doOnDisable();
   // DO ON INVISIBLE
   // actions performed when button is made invisible.  Actions are:
   //  	if button is visible, reset to invisible internally and with parent
   virtual void doOnInvisible();


private:
   bool _pressed;

private:
	CGUIButton();
	CGUIButton (CGUIButton & copy);
	CGUIButton operator=(CGUIButton &obj);

};

#endif /* #ifndef _CGUI_BUTTON_INCLUDE */
