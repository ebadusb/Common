/*
 *	Copyright (c) 2004 by Gambro BCT, Inc.  All rights reserved.
 *
 *  $Header: //bctquad3/home/BCT_Development/vxWorks/Common/cgui/rcs/cgui_button.h 1.18 2006/05/15 21:48:45Z rm10919 Exp MS10234 $ 
 *  This file defines the base class for all button styles in the common GUI.  An object of this class types
 *  can be used to generate a standard button.
 *  
 *  $Log: cgui_button.h $
 *
 */
#ifndef _CGUI_BUTTON_INCLUDE
#define _CGUI_BUTTON_INCLUDE

#include "message.h"
#include "taos_datalog_levels.h"
#include "cgui_window.h"
#include "cgui_text.h"
#include "cgui_itmap.h"

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

protected:
   Message<long>     *_buttonMsgPtr;                  // used to communicate a message to other tasks when a button is pressed and released
   bool               _enabled;                       // current enabled/disabled state
   CGUIText          *_textPtr;                       // ptr to current text object
   CGUIBitmap        *_enabledBitmapPtr;              // ptr to enabled bitmap object
   StylingRecord      _enabledTextStyle;              // text styling record when button is enabled
   CGUIBitmap        *_disabledBitmapPtr;             // ptr to disabled bitmap object
   StylingRecord      _disabledTextStyle;             // text styling record when button is disabled
   CGUIBitmap        *_pressedBitmapPtr;              // ptr to pressed bitmap object
   StylingRecord      _pressedTextStyle;              // text styling record when button is pressed
   CGUIBitmap        *_releasedBitmapPtr;             // ptr to released bitmap object
   StylingRecord      _releasedTextStyle;             // text styling record when button is released
   Message<long>     *_audioMsgPtr;                   // ptr to audio message to send when button is pressed
   CallbackBase       _CBOnPressed;                   // callback object to use when button is pressed
   CallbackBase       _CBOnReleased;                  // callback object to use when button is released
   bool               _pressEventMsgEnabled;          // flag is true if message is sent on press of button
   bool               _relEventMsgEnabled;            // flag is true if message is sent on release of button
   bool               _pressedCBEnabled;              // flag is true if callback is made on button press
   bool               _releasedCBEnabled;             // flag is true is callbacl is made on button release
   CGUIBitmap        *_iconPtr;                       // ptr to the icon bitmap object
   DataLogLevel       _btnDataLogLevel                // level at which to log button press events

   public:
   // CONSTRUCTOR
   CGUIButton  (
               CGUIDisplay &        display,             // reference to a cguidisplay object for display context
               CGUIWindow *         parent,              // pointer to a parent window
               unsigned short       locateX,             // x coordinate of upper left corner of button.  Required.
               unsigned short       locateY,             // y coordinate of upper left corner of button.  Required.
               Message<long>*       pressEventObj=NULL,  // ptr to int message object to output when button is pressed and released
                                                         // can be NULL to indicate no message is output
               CGUIText*            textIdObj=NULL,      // ptr to text object containing string data, NULL indicates
                                                         // no text is displayed on the button
               CGUIBitmap*          enabledObj=NULL,     // ptr to bitmap object to use when button is enabled
               CGUIBitmap*          disabledObj=NULL,    // ptr to bitmap object to use when button is disabled
               CGUIBitmap*          pressedObj=NULL,     // ptr to bitmap object to use when button is pressed
               Message<long>*       audioMsg=NULL,       // ptr to audio message to send when button is pressed
               DataLogLevel         buttonLevel=NULL,    // datalog level object used to log button press events
               bool                 enabled=false,       // button will be constructed as disabled unless specified here
               bool                 visible=true         // button will be constructed as visbile unless otherwise specified here
               );

   // DESTRUCTOR
   virtual ~CGUIButton ();

   // ENABLE
   // set the state of the button to enabled.  If currently invisible, the button is made visible
   void enable();

   // DISABLE
   // set the state of the button to disabled.  If currently invisible, the button is made visible
   void disable();

   // IS ENABLED
   // returns true if the button is enabled
   bool isEnabled() const {return _enabled;};

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
   void setMessage   (
                     Message<long>* newEventMsg
                     );

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
   void setPressedCallback (
                           const CallbackBase cb
                           );

   //SET RELEASED CALLBACK
   // This method establishes the pointer to the routine that gets called when the button is released.
   // There can be only one released callback per button.
   // The callback is automatically enabled after this call.
   void setReleasedCallback   (
                              const CallbackBase cb
                              );

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
   virtual void pointerEvent  (
                              const PointerEvent & event    // event structure received from UGL on button press/release
                              );

   // BITMAP METHODS //

   //SET ENABLED BITMAP
   // loads a new enabled bitmap object for the button.
   void setEnabledBitmap   (
                           CGUIBitmap *enableObj         // ptr to bitmap object to display when button is enabled
                           );

   //SET DISABLED BITMAP
   // loads a new disabled bitmap object for the button.
   void setDisabledBitmap  (
                           CGUIBitmap *disableObj        // ptr to bitmap object to display when button is disabled
                           );

   // SET PRESSED BITMAP
   // loads a new pressed bitmap object for the button
   void setPressedBitmap   (
                           CGUIBitmap *pressedObj        // ptr to bitmap object to display when button is pressed
                           );

   // AUDIO METHODS //

   // ENABLE AUDIO
   // enable the audio feedback when the button is pressed
   void enableAudio();

   // DISABLE AUDIO
   // disable the audio feedback when the button is pressed
   void disableAudio();

   // SET AUDIO
   // set the audio associated with a button press.
   void setAudio  (
                  Message<long> *audioObj
                  );

   // TEXT METHODS //

   // SET TEXT
   // set the text associated with the button.  
   void setText   (
                  CGUIText *textid     // ptr to a text object associated with the button
                  );

   // SET TEXTSTYLE
   // set/change the style of the text associated with this button in ALL states.  This is a pass-thru to the 
   // text object previously set with this button.  
   void setTextstyle (
                     StylingRecord& textStyleRec      // style record with appropriate features set
                     );

   // SET ENABLED TEXTSTYLE 
   // SET DISABLED TEXTSTYLE
   // SET PRESSED TEXTSTYLE
   // set/change the style of text associated with this button for each of the four states that the button
   // can be in.  Note these call does nothing if there is no text associated with the button.
   // They will work if the text is disabled.
   void setEnabledTextstyle      (
                                 StylingRecord& enTextStyleRec       // style record with appropriate features set
                                 );
   void setDisabledTextstyle     (
                                 StylingRecord& disTextStyleRec      //	style record with appropriate features set
                                 );
   void setPressedTextstyle      (
                                 StylingRecord& prsTextStyleRec      // style record with appropriate features set
                                 );
   void setReleasedTextstyle     (
                                 StylingRecord& prsTextStyleRec      // style record with appropriate features set
                                 );       
   // ICON METHODS //

   // SET ICON
   // set an icon to be associated with the button. 
   void setIcon (CGUIBitmap *iconObj,        // ptr to bitmap object for icon
                 const short x=-1,             // upper left X coordinate to start icon relative to button location (0 means coincident with x coordinate of upper left position of button)
                 const short y=-1,             // upper left Y coordinate to start icon relative to button location
                 bool visible=true             // visibility of icon upon creation, uses CGUIBitmap class to set visibility
                );

   // ENABLE ICON
   // set a previously set icon as visible on the button
   void enableIcon   ();

   // DISABLE ICON
   // set a previously set icon as invisible
   void disableIcon  ();

protected:
   // The following methods are called when state of the button is changed.  These can be overriden
   // and supllemented by derived types if different behavior is needed.
   // DO ON PRESS
   // actions performed by the button when it is pressed.  Actions are:
   //    log button press event to data log if provided
   //		verify button is enabled (disabled means the following actions are skipped)
   //		generate audio feedback associated with button press, if any
   //		perform callback associated with button press, if any
   //    send event message associated with button press (=ButtonPress), if any
   //		display pressed bitmap if provided (otherwise leave the current bitmap up)
   //    display text in "pressed" textstyle if provided
   virtual void doOnPress();

   // DO ON RELEASE
   // actions performed when the button is released.  Actions are:
   //		perform callback associated with button release, if any
   //		send event message associated with button release, if any
   //		display enabled bitmap
   //		display text in "released" textstyle if provided
   virtual void doOnRelease();

   // DO ON ENABLE
   // actions performed when the button is enabled.
   // Actions are:
   //		if the button is invisible, reset to visible internally and with parent
   //    display enabled bitmap
   //    display text in "enabled" text style 
   virtual void doOnEnable();

   // DO ON DISABLE
   // action performed when button is disbaled.
   // Actions are:
   //		if the button is invisible, reset to visible internally and with parent
   //    display disabled bitmap
   //    display text in "disabled" text style
   virtual void doOnDisable();

   // DO ON INVISIBLE
   // actions performed when button is made invisible.  Actions are:
   //		if button is visible, reset to invisible internally and with parent
   virtual void doOnInvisible();
};

#endif /* #ifndef _CGUI_BITMAP_INCLUDE */
