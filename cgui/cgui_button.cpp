/*
 *	Copyright (c) 2004 by Gambro BCT, Inc.  All rights reserved.
 *
 *  $Header: //bctquad3/home/BCT_Development/vxWorks/Common/cgui/rcs/cgui_button.cpp 1.21 2006/05/15 21:48:45Z rm10919 Exp MS10234 $ 
 *  This file defines the base class for all button styles in the common GUI.
 *  An object of this class types can be used to generate a standard button.
 *  
 *  $Log: cgui_button.cpp $
 *  Revision 1.4  2004/11/02 20:48:19Z  rm10919
 *  change setText() fucntions & add checks for bitmaps in enable() & disable().
 *  Revision 1.3  2004/11/01 17:27:21Z  cf10242
 *  Change TextItem to CGUITextItem
 *  Revision 1.2  2004/10/29 15:11:13Z  rm10919
 *  Revision 1.1  2004/10/22 20:16:19Z  rm10919
 *  Initial revision
 *
 */

#include <vxWorks.h>
#include "cgui_window_object.h"
#include "cgui_button.h"
#include "cgui_bitmap_info.h"


// CONSTRUCTOR
CGUIButton::CGUIButton  (CGUIDisplay        & display,                // reference to a cguidisplay object for display context
                         CGUIWindow         * parent,                 // pointer to a parent window
                         ButtonData         & buttonData,             // reference to button data for bitmaps, text and behavior
                         Message<long>      * pressEventObject = NULL,// ptr to int message object to output when button is pressed and released
                                                                      // can be NULL to indicate no message is output
                         Message<long>      * audioMessage = NULL,    // ptr to audio message to send when button is pressed
                         DataLog_Level      * buttonLevel = NULL,     // datalog level object used to log button press events
                         bool                 enabled = true,         // button will be constructed as enabled unless specified here
                         bool                 visible = true,         // button will be constructed as visbile unless otherwise specified here
                         bool                 pressed = false
                        ):CGUIWindow(display), _behaviorType(buttonData.type), _iconPointer(NULL), _pressed(pressed)
{
   if (!buttonLevel)
   {
      buttonLevel = &log_level_critical;
   }
   // Button size is based on enabled bitmap (all bitmaps should be the same size anyway)
   CGUIRegion buttonRegion = CGUIRegion(buttonData.left, buttonData.top, 0, 0); //buttonData.enabledBitmapId->getWidth(), buttonData.enabledBitmapId->getHeight());

   _enabled = enabled;

   if (buttonData.enabledBitmapId)
   {
      _enabledBitmap = new CGUIBitmap (display, CGUIRegion(0,0,0,0), *buttonData.enabledBitmapId);
      buttonRegion = CGUIRegion(buttonData.left, buttonData.top, _enabledBitmap->getRegion().width, _enabledBitmap->getRegion().height);
      setRegion(buttonRegion);
      addObjectToFront(_enabledBitmap);
   }
   else
   {
      //
      // Region not set for button so a button won't show on display!!!!!
      //
   }

   if (buttonData.disabledBitmapId)
   {
      _disabledBitmap = new CGUIBitmap (display, CGUIRegion(0,0,0,0), *buttonData.disabledBitmapId);
      addObjectToFront(_disabledBitmap);
   }
   
   if (buttonData.pressedBitmapId)
   {
      _pressedBitmap = new CGUIBitmap (display, CGUIRegion(0,0,0,0), *buttonData.pressedBitmapId);
      if (_pressed) 
      { 
         addObjectToFront(_pressedBitmap);
      }//else
     // {
     //    addObjectToBack(_pressedBitmap);
     // }
   }


   if (buttonData.enabledTextItem)
   {
      if ((buttonData.enabledTextStyle->region.width == 0) && (buttonData.enabledTextStyle->region.height == 0))
      {
         buttonData.enabledTextStyle->region.width = _enabledBitmap->getRegion().width;
         buttonData.enabledTextStyle->region.height = _enabledBitmap->getRegion().height;
      }
      _enabledText = new CGUIText(display, this, buttonData.enabledTextItem, buttonData.enabledTextStyle);
      _enabledText->setCaptureBackgroundColor();
   }
   else
   {
      _enabledText = NULL;
   }

   if (buttonData.disabledTextItem)
   {
      if ((buttonData.disabledTextStyle->region.width == 0) && (buttonData.disabledTextStyle->region.height == 0))
      {
         buttonData.disabledTextStyle->region.width = _enabledBitmap->getRegion().width;
         buttonData.disabledTextStyle->region.height = _enabledBitmap->getRegion().height;
      }
      _disabledText = new CGUIText(display, this, buttonData.disabledTextItem, buttonData.disabledTextStyle);
      _disabledText->setCaptureBackgroundColor();
      _disabledText->setVisible(false);
   }
   else
   {
      _disabledText = NULL;
   }

   if (buttonData.pressedTextItem)
   {
      if ((buttonData.pressedTextStyle->region.width == 0) && (buttonData.pressedTextStyle->region.height == 0))
      {
         buttonData.pressedTextStyle->region.width = _enabledBitmap->getRegion().width;
         buttonData.pressedTextStyle->region.height = _enabledBitmap->getRegion().height;
      }
      _pressedText = new CGUIText(display, this, buttonData.pressedTextItem, buttonData.pressedTextStyle);
      _pressedText->setCaptureBackgroundColor();
      _pressedText->setVisible(false);
   }
   else
   {
      _pressedText = NULL;
   }

   WIN_ATTRIB winAttrib = WIN_ATTRIB_NO_INPUT|WIN_ATTRIB_VISIBLE;
   assert(parent);
   attach(parent, winAttrib);

   winCbAdd(_id, MSG_PTR_BTN1_DOWN, 0, &CGUIWindow::uglPointerCallback, UGL_NULL);
   winCbAdd(_id, MSG_PTR_BTN1_UP, 0, &CGUIWindow::uglPointerCallback, UGL_NULL);

   if (pressEventObject)
   {
      _pressEventMessageEnabled = true;
      _buttonMessagePointer = pressEventObject;

   }
   else
   {
      _pressEventMessageEnabled = false;
      _releasedEventMessageEnabled = false;
      _buttonMessagePointer = NULL;
   }

}

// DESTRUCTOR
CGUIButton::~CGUIButton ()
{
   delete _enabledText;
   delete _disabledText;
   delete _pressedText;

   delete _enabledBitmap;
   delete _disabledBitmap;
   delete _pressedBitmap;

   delete _buttonMessagePointer;
   delete _audioMessagePointer;

   delete _iconPointer;

   delete _btnDataLogLevel;
}

// ENABLE
//  Set the state of the button to enabled.  
//  If currently invisible, the button is made visible.
void CGUIButton::enable(void)
{
   if (!_enabled)
   {
      _enabled = true;
      if (_disabledBitmap) moveObjectToBack(_disabledBitmap);
      if (_enabledBitmap) moveObjectToFront(_enabledBitmap);

      if (_iconPointer) moveObjectToFront(_iconPointer);

      if (_disabledText) _disabledText->setVisible(false);
      if (_pressedText)  _pressedText->setVisible(false);

      if (_enabledText)
      {
         _enabledText->setVisible(true);
         _enabledText->setCaptureBackgroundColor();
      }
   }
}

// DISABLE
//  Set the state of the button to disabled.
//  If currently invisible, the button is made visible.
void CGUIButton::disable()
{
   if (_enabled)
   {
      _enabled = false;
      if (_enabledBitmap) moveObjectToBack(_enabledBitmap);
      if (_disabledBitmap) moveObjectToFront(_disabledBitmap);

      if (_iconPointer) deleteObject(_iconPointer);

      if (_enabledText)  _enabledText->setVisible(false);
      if (_pressedText)  _pressedText->setVisible(false);

      if (_disabledText)
      {
         _disabledText->setVisible(true);
         _disabledText->setCaptureBackgroundColor();
      }
      _pressed = false;
   }
}

// INVISIBLE
//  Make the button invisible.  Uses CGUIWindow class to do this.
void CGUIButton::invisible()
{
   setDisabled(true);
}

// IS VISIBLE
// return flag indicating current state of visibility.  Uses CGUIWindow class to do this
bool CGUIButton::isVisible() const
{
   return true; //_disabled;
}

//SET MESSAGE
// set an int message to be output when the button is pressed.  The default is for a message to be sent
// on press and on release with the value of the message indicating which event occured.  This
// behavior can be modified to only send a message on press or release by using the enable/disable 
// methods provided below.  
// NULL removes a previously set message from being sent.
// There can be only one pressed message per button.  The message is automatically enabled after this call.
void CGUIButton::setMessage (Message<long>* newEventMessage)
{
   if (newEventMessage)
   {
      _buttonMessagePointer = newEventMessage;
      _pressEventMessageEnabled = true;
   }
   else
   {
      _buttonMessagePointer = NULL;
      _pressEventMessageEnabled = false;
   }
}

//ENABLE PRESSED MESSAGE
// Enables a message sent on button press.
void CGUIButton::enablePressedMessage()
{
   _pressEventMessageEnabled = true;
}

//DISABLE PRESSED MESSAGE
// Disables message sent on button press.
void CGUIButton::disablePressedMessage()
{
   _pressEventMessageEnabled = false;
}

//ENABLE RELEASED MESSAGE
// Enables message sent on button release.
void CGUIButton::enableReleasedMessage()
{
   _releasedEventMessageEnabled = true;
}

//DISABLE RELEASED MESSAGE
// Disables message sent on button release
void CGUIButton::disableReleasedMessage()
{
   _releasedEventMessageEnabled = false;
}

//SET PRESSED CALLBACK
// This method establishes the pointer to the routine that
// gets called when the button is pressed.
// There can be only one pressed callback per button.
// The callback is automatically enabled after this call.
void CGUIButton::setPressedCallback (const CallbackBase cb)
{
   _CBOnPressed = cb;
   _pressedCBEnabled = true;
}

//SET RELEASED CALLBACK
// This method establishes the pointer to the routine that
// gets called when the button is released.
// There can be only one released callback per button.
// The callback is automatically enabled after this call.
void CGUIButton::setReleasedCallback (const CallbackBase cb)
{        
   _CBOnReleased = cb;
   _releasedCBEnabled = true;
}

//ENABLE PRESSED CALLBACK
void CGUIButton::enablePressedCallback()
{
   _pressedCBEnabled = true;
}

//DISABLE PRESSED CALLBACK
// keeps the call back from last set_pressed_callback, but disables it until an
// enable_pressed_callback is called.
void CGUIButton::disablePressedCallback()
{
   _pressedCBEnabled = false;
}

//ENABLE RELEASED CALLBACK
void CGUIButton::enableReleasedCallback()
{
   _releasedCBEnabled = true;
}

//DISABLE RELEASED CALLBACK
// Keeps the call back from last set_pressed_callback,
// but disables it until an enable_pressed_callback is called.
void CGUIButton::disableReleasedCallback()
{
   _releasedCBEnabled = false;
}

// POINTER EVENT
// virtual function in CGUIWindow class that is implemented here for buttons to apply the
// correct callback function and send the correct message based on the action taken 
// by the operator, i.e. press, release
void CGUIButton::pointerEvent (const PointerEvent & event) // event structure received from UGL on button press/release
{    
   if (_enabled)
   {
      if (event.eventType == PointerEvent::ButtonPress)
      {
         doOnPress();

         if (_pressedCBEnabled)
         {
            _CBOnPressed();
         }

         if (_pressEventMessageEnabled)
         {
            _buttonMessagePointer->send(PointerEvent::ButtonPress);
         }
      }
      else if (event.eventType == PointerEvent::ButtonRelease)
      {
         if (_behaviorType == RaiseAfterRelease)
         {
            doOnEnable();

            if (_releasedCBEnabled)
            {
               _CBOnReleased();
               doOnRelease();
            }
         }
         if (_behaviorType == RaiseAfterCallback)
         {
            doOnEnable();

            if (_releasedCBEnabled)
            {
               _CBOnReleased();
            }
         }

         if (_releasedEventMessageEnabled)
         {
            _buttonMessagePointer->send(PointerEvent::ButtonRelease);
         }
      }
   }
}

//SET ENABLED BITMAP
// Loads a new enabled bitmap object for the button.
void CGUIButton::setEnabledBitmap (CGUIBitmapInfo *enabledBitmapId) // ptr to bitmap object to display when button is enabled
{     
   if (enabledBitmapId)
   {
//      _enabledBitmap = new CGUIBitmap (_display, CGUIRegion(0,0,0,0), *enabledBitmapId);
      //_enabledBitmap->setBitmap(enabledBitmapId);
      _enabledBitmap->setBitmap();
      if (_enabledText)
      {
         _enabledText->setCaptureBackgroundColor();
      }
   }
}

//SET DISABLED BITMAP
// Loads a new disabled bitmap object for the button.
void CGUIButton::setDisabledBitmap (CGUIBitmapInfo *disabledBitmapId) // ptr to bitmap object to display when button is disabled
{
   if (disabledBitmapId)
   {
      _disabledBitmap = new CGUIBitmap (_display, CGUIRegion(0,0,0,0), *disabledBitmapId);
   }
}

// SET PRESSED BITMAP
// Loads a new pressed bitmap object for the button.
void CGUIButton::setPressedBitmap (CGUIBitmapInfo *pressedBitmapId) // ptr to bitmap object to display when button is pressed
{
   if (pressedBitmapId)
   {
      _pressedBitmap = new CGUIBitmap (_display, CGUIRegion(0,0,0,0), *pressedBitmapId);
   }
}

// ENABLE AUDIO
// Enable the audio feedback when the button is pressed.
void CGUIButton::enableAudio()
{
}

// DISABLE AUDIO
// Disable the audio feedback when the button is pressed.
void CGUIButton::disableAudio()
{
}

// SET AUDIO
// set the audio associated with a button press.
void CGUIButton::setAudio (Message<long> *audioObject)
{
   if (audioObject)
   {
      _audioMessagePointer = audioObject;
   }
   else
   {
      _audioMessagePointer = NULL;
   }
}

// SET TEXT
// sets all the text associated with the button.  
//
void CGUIButton::setText (CGUITextItem * textItem = NULL) // ptr to a text object associated with the button
{
   if (textItem)
   {
      setEnabledText(textItem);
      setDisabledText(textItem);
      setPressedText(textItem);
   }
}

void CGUIButton::setText (const char * string = NULL) // ptr to a text object associated with the button
{
   if (string)
   {
      setEnabledText(string);
      setDisabledText(string);
      setPressedText(string);
   }
}

void CGUIButton::setEnabledText (CGUITextItem * textItem = NULL)
{
   if (textItem)
   {
      if (_enabledText)
      {
         _enabledText->setText(textItem);
      }
      else
      {
         _enabledText = new CGUIText(_display, this, textItem);
      }      
   }
   if (!textItem)
   {
      // NOT GOOD!!! Nothing to do.
   }
   else
   {
      if (_enabled)
      {
         _enabledText->setVisible(true);
      }
      else
      {
         _enabledText->setVisible(false);
      }
   }
}

void CGUIButton::setEnabledText (const char * string = NULL)
{
   if (string)
   {
      if (_enabledText)
      {
         _enabledText->setText(string);
      }
      else
      {
         _enabledText = new CGUIText(_display, this);
         _enabledText->setText(string);
      }      
   }
   if (!string)
   {
      // NOT GOOD!!! Nothing to do.
   }
   else
   {
      if (_enabled)
      {
         _enabledText->setVisible(true);
      }
      else
      {
         _enabledText->setVisible(false);
      }
   }
}

void CGUIButton::setDisabledText (CGUITextItem * textItem = NULL)
{
   if (textItem)
   {
      if (_disabledText)
      {
         _disabledText->setText(textItem);
      }
      else
      {
         _disabledText = new CGUIText(_display, this, textItem);
      }      
   }
   if (!textItem)
   {
      // NOT GOOD!!! Nothing to do.
   }
   else
   {
      if (_enabled)
      {
         _disabledText->setVisible(false);
      }
      else
      {
         _disabledText->setVisible(true);
      }
   }
}

void CGUIButton::setDisabledText (const char * string = NULL)
{
   if (string)
   {
      if (_disabledText)
      {
         _disabledText->setText(string);
      }
      else
      {
         _disabledText = new CGUIText(_display, this);
         _disabledText->setText(string);
      }      
   }
   if (!string)
   {
      // NOT GOOD!!! Nothing to do.
   }
   else
   {
      if (_enabled)
      {
         _disabledText->setVisible(false);
      }
      else
      {
         _disabledText->setVisible(true);
      }
   }
}

void CGUIButton::setPressedText (CGUITextItem * textItem = NULL)
{
   if (textItem)
   {
      if (_pressedText)
      {
         _pressedText->setText(textItem);
      }
      else
      {
         _pressedText = new CGUIText(_display, this, textItem);
      }      
   }else
   {
      // NOT GOOD!!! Nothing to do.
   }
}

void CGUIButton::setPressedText (const char * string = NULL)
{
   if (string)
   {
      if (_pressedText)
      {
         _pressedText->setText(string);
      }
      else
      {
         _pressedText = new CGUIText(_display, this);
         _pressedText->setText(string);
      }      
   }else
   {
      // NOT GOOD!!! Nothing to do.
   }
}

// SET TEXTSTYLE
// Set/change the style of the text associated with 
// this button in ALL states.  This is a pass-thru to the 
// text object previously set with this button.  
void CGUIButton::setTextStyle (CGUIText::StylingRecord * textStylingRecord) // style record with appropriate features set
{
   if (_enabledText)  _enabledText->setStylingRecord(textStylingRecord);
   if (_disabledText) _disabledText->setStylingRecord(textStylingRecord);
   if (_pressedText)  _pressedText->setStylingRecord(textStylingRecord);
}

// SET ENABLED TEXTSTYLE 
// SET DISABLED TEXTSTYLE
// SET PRESSED TEXTSTYLE
// set/change the style of text associated with this button for each of the four states that the button
// can be in.  Note these call does nothing if there is no text associated with the button.
// They will work if the text is disabled.
void CGUIButton::setEnabledTextStyle (CGUIText::StylingRecord * enabledTextStylingRecord) // style record with appropriate features set
{                     
   if (_enabledText)  _enabledText->setStylingRecord(enabledTextStylingRecord);
}

void CGUIButton::setDisabledTextStyle (CGUIText::StylingRecord * disabledTextStylingRecord)
{
   if (_disabledText) _disabledText->setStylingRecord(disabledTextStylingRecord);
}

void CGUIButton::setPressedTextStyle (CGUIText::StylingRecord * pressedTextStylingRecord)
{
   if (_pressedText)  _pressedText->setStylingRecord(pressedTextStylingRecord);
}

// SET ICON
// Set an icon to be associated with the button. 
void CGUIButton::setIcon (CGUIBitmapInfo * iconId,  // ptr to bitmap object for icon
                          const short x=-1,         // upper left X coordinate to start icon relative to button location (0 means coincident with x coordinate of upper left position of button)
                          const short y=-1,         // upper left Y coordinate to start icon relative to button location
                          bool visible=true         // visibility of icon upon creation, uses CGUIBitmap class to set visibility
                         )
{   
   if (_iconPointer)
   {
      deleteObject(_iconPointer);
      delete _iconPointer;
      _iconPointer = NULL;
   }

   if (iconId)
   {
      CGUIRegion bitmapSize;
      CGUIRegion buttonSize;

      _iconPointer = new CGUIBitmap(_display, CGUIRegion(0, 0, 0, 0), *iconId);
      bitmapSize = _iconPointer->getRegion();
      buttonSize = _enabledBitmap->getRegion();

      if ((x == -1 ) && (y == -1))
      {
         // Default placement of icon.
         _iconPointer->setRegion(CGUIRegion(3, buttonSize.height-bitmapSize.height-3, 0, 0));
      }
      else
      {
         _iconPointer->setRegion(CGUIRegion(x, y, 0, 0));
      }
      addObjectToFront(_iconPointer);
      _iconPointer->setVisible(visible);
   }
}

// ENABLE ICON
// Set a previously set icon as visible on the button
void CGUIButton::enableIcon ()
{
   _iconPointer->setVisible(true);
   moveObjectToFront(_iconPointer);
}

// DISABLE ICON
// Set a previously set icon as invisible
void CGUIButton::disableIcon ()
{
   _iconPointer->setVisible(false);
}

// The following methods are called when state of the button is changed.  These can be overriden
// and supllemented by derived types if different behavior is needed.
// DO ON PRESS
// actions performed by the button when it is pressed.  Actions are:
//  log button press event to data log if provided
//	verify button is enabled (disabled means the following actions are skipped)
//	generate audio feedback associated with button press, if any
//	perform callback associated with button press, if any
//  send event message associated with button press (=ButtonPress), if any
//	display pressed bitmap if provided (otherwise leave the current bitmap up)
//  display text in "pressed" textstyle if provided
void CGUIButton::doOnPress()
{
   if (_enabled)
   {
      if (_pressedBitmap) moveObjectToFront(_pressedBitmap);
      deleteObject(_enabledBitmap);

      if (_iconPointer) deleteObject(_iconPointer);

      if (_pressedText)
      {
         _pressedText->setCaptureBackgroundColor();
         _pressedText->setVisible(true);
      }

      if (_enabledText)  _enabledText->setVisible(false);
      if (_disabledText) _disabledText->setVisible(false);

      invalidateObjectRegion(_enabledBitmap);

      _pressed = true;

      if (_audioMessagePointer) _audioMessagePointer->send();
   }
}

// DO ON RELEASE
// actions performed when the button is released.  Actions are:
//	perform callback associated with button release, if any
//	send event message associated with button release, if any
//	display enabled bitmap
//	display text in "released" textstyle if provided.
void CGUIButton::doOnRelease()
{
   if (_enabled)
   {
      if (_enabledBitmap) moveObjectToFront(_enabledBitmap);
      deleteObject(_pressedBitmap);

      if (_iconPointer) deleteObject(_iconPointer);

      if (_enabledText)
      {
         _enabledText->setCaptureBackgroundColor();
         _enabledText->setVisible(true);

         if (_disabledText) _disabledText->setVisible(false);
         if (_pressedText) _pressedText->setVisible(false);

      }
      invalidateObjectRegion(_enabledBitmap);

      _pressed = false;

      if (_audioMessagePointer) _audioMessagePointer->send();
   }

}

// DO ON ENABLE
// actions performed when the button is enabled.
// Actions are:
//	if the button is invisible, reset to visible internally and with parent
//  display enabled bitmap
//  display text in "enabled" text style 
void CGUIButton::doOnEnable()
{
   if (_enabled)
   {
      addObjectToFront(_enabledBitmap);
      deleteObject(_pressedBitmap);

      if (_iconPointer) moveObjectToFront(_iconPointer);

      if (_enabledText)
      {
         _enabledText->setCaptureBackgroundColor();
         _enabledText->setVisible(true);

         if (_disabledText) _disabledText->setVisible(false);
         if (_pressedText)  _pressedText->setVisible(false);
      }

      invalidateObjectRegion(_enabledBitmap);
      _pressed = false;
   }
}

// DO ON DISABLE
// action performed when button is disabled.
// Actions are:
//	if the button is invisible, reset to visible internally and with parent
//  display disabled bitmap
//  display text in "disabled" text style
void CGUIButton::doOnDisable()
{
   if (!_enabled)
   {
      addObjectToFront(_disabledBitmap);
      if (_enabledBitmap)deleteObject(_enabledBitmap);
      if (_pressedBitmap)deleteObject(_pressedBitmap);

      if (_iconPointer) addObjectToFront(_iconPointer);

      //
      // Display disabled text else leave text as is.
      //
      if (_disabledText)
      {
         _disabledText->setCaptureBackgroundColor();
         _disabledText->setVisible(true);

         _enabledText->setVisible(false);
         _pressedText->setVisible(false);
      }

      invalidateObjectRegion(_enabledBitmap);
      _pressed = false;
   }
}

// DO ON INVISIBLE
// actions performed when button is made invisible.  Actions are:
//	if button is visible, reset to invisible internally and with parent.
void CGUIButton::doOnInvisible()
{
}

