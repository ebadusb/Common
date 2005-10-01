/*
 *	Copyright (c) 2004 by Gambro BCT, Inc.  All rights reserved.
 *
 *  $Header: //bctquad3/home/BCT_Development/vxWorks/Common/cgui/rcs/cgui_button.cpp 1.21 2006/05/15 21:48:45Z rm10919 Exp MS10234 $ 
 *  This file defines the base class for all button styles in the common GUI.
 *  An object of this class types can be used to generate a standard button.
 *  
 *  $Log: cgui_button.cpp $
 *  Revision 1.16  2005/08/11 16:24:22Z  cf10242
 *  TAOS IT 764 - fix callbacks
 *  Revision 1.15  2005/04/26 23:16:46Z  rm10919
 *  Made changes to cgui_text and cgui_text_item, plus added 
 *  classes for variable substitution in text strings.
 *  Revision 1.14  2005/04/08 19:51:42Z  cf10242
 *  Common IT 27 - change methods to set/change a bitmap after construction.
 *  Revision 1.13  2005/04/04 18:02:19Z  rm10919
 *  Add ability to char * as button text.
 *  Revision 1.12  2005/03/15 00:21:35Z  rm10919
 *  Change CGUIText to not add object to window object list of parent in constructor.
 *  Revision 1.11  2005/02/21 17:17:11Z  cf10242
 *  IT 133 - delete all allocated memory to avoid unrecovered memory
 *  Revision 1.10  2005/01/28 23:52:17Z  rm10919
 *  CGUITextItem class changed and put into own file.
 *  Revision 1.9  2005/01/18 18:38:52Z  rm10919
 *  Fix icon placement on button.
 *  Revision 1.8  2005/01/03 20:41:24Z  cf10242
 *  add an enablePressed method a button can shown as enabled and pressed
 *  Revision 1.7  2004/11/19 18:14:45Z  cf10242
 *  Integration checkin
 *  Revision 1.6  2004/11/18 22:33:37Z  rm10919
 *  Added ability to modify button text.
 *  Revision 1.5  2004/11/04 20:19:08Z  rm10919
 *  Common updates and changes.
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
		_enabledBitmap = NULL;
   }

   if (buttonData.disabledBitmapId)
   {
      _disabledBitmap = new CGUIBitmap (display, CGUIRegion(0,0,0,0), *buttonData.disabledBitmapId);
      if (!_enabled)
      {
         addObjectToFront(_disabledBitmap);
      }else
      {
         addObjectToBack(_disabledBitmap);
      }
   }
	else
		_disabledBitmap = NULL;
   
   if (buttonData.pressedBitmapId)
   {
      _pressedBitmap = new CGUIBitmap (display, CGUIRegion(0,0,0,0), *buttonData.pressedBitmapId);
      if (_pressed) 
      { 
         addObjectToFront(_pressedBitmap);
      }else
      {
         addObjectToBack(_pressedBitmap);
      }
   }
	else
		_pressedBitmap = NULL;
   
   if (buttonData.enabledTextItem)
   {
      if (!buttonData.enabledStylingRecord)
      {
         buttonData.enabledStylingRecord = new StylingRecord(buttonData.enabledTextItem->getStylingRecord());
      }
      if ((buttonData.enabledStylingRecord->region.width == 0) && (buttonData.enabledStylingRecord->region.height == 0))
      {
         buttonData.enabledStylingRecord->region.width = _enabledBitmap->getRegion().width;
         buttonData.enabledStylingRecord->region.height = _enabledBitmap->getRegion().height;
      }
      _enabledText = new CGUIText(display, buttonData.enabledTextItem, buttonData.enabledStylingRecord);
      _enabledText->setCaptureBackgroundColor();
      addObjectToFront(_enabledText);
   }
   else
   {
      _enabledText = NULL;
   }

   if (buttonData.disabledTextItem)
   {
      if (!buttonData.disabledStylingRecord)
      {
         buttonData.disabledStylingRecord = new StylingRecord(buttonData.disabledTextItem->getStylingRecord());
      }
      
      if ((buttonData.disabledStylingRecord->region.width == 0) && (buttonData.disabledStylingRecord->region.height == 0))
      {
         buttonData.disabledStylingRecord->region.width = _enabledBitmap->getRegion().width;
         buttonData.disabledStylingRecord->region.height = _enabledBitmap->getRegion().height;
      }
      _disabledText = new CGUIText(display, buttonData.disabledTextItem, buttonData.disabledStylingRecord);
      _disabledText->setCaptureBackgroundColor();
      _disabledText->setVisible(false);
      addObjectToFront(_disabledText);
   }
   else
   {
      _disabledText = NULL;
   }

   if (buttonData.pressedTextItem)
   {
      if (!buttonData.pressedStylingRecord)
      {
         buttonData.pressedStylingRecord = new StylingRecord(buttonData.pressedTextItem->getStylingRecord());
      }
      
      if ((buttonData.pressedStylingRecord->region.width == 0) && (buttonData.pressedStylingRecord->region.height == 0))
      {
         buttonData.pressedStylingRecord->region.width = _enabledBitmap->getRegion().width;
         buttonData.pressedStylingRecord->region.height = _enabledBitmap->getRegion().height;
      }
      _pressedText = new CGUIText(display, buttonData.pressedTextItem, buttonData.pressedStylingRecord);
      _pressedText->setCaptureBackgroundColor();
      _pressedText->setVisible(false);
      addObjectToFront(_pressedText);
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
   if (_enabledText) delete _enabledText;
   if (_disabledText) delete _disabledText;
   if (_pressedText) delete _pressedText;

   if (_enabledBitmap) delete _enabledBitmap;
   if (_disabledBitmap) delete _disabledBitmap;
   if (_pressedBitmap) delete _pressedBitmap;

   if (_iconPointer) delete _iconPointer;

   winCbRemove(_id, &CGUIWindow::uglPointerCallback);
   winCbRemove(_id, &CGUIWindow::uglPointerCallback);

}

// ENABLE
//  Set the state of the button to enabled.  
//  If currently invisible, the button is made visible.
void CGUIButton::enable(void)
{
      _enabled = true;
      if (_disabledBitmap) moveObjectToBack(_disabledBitmap);
		if (_pressedBitmap) moveObjectToBack(_pressedBitmap);
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

// ENABLEPRESSED
//  Set the state of the button to enabled and already pressed.  
void CGUIButton::enablePressed(void)
{
      _enabled = true;
      if (_disabledBitmap) moveObjectToBack(_disabledBitmap);
		if (_enabledBitmap) moveObjectToBack(_enabledBitmap);
      if (_pressedBitmap) moveObjectToFront(_pressedBitmap);

	   if (_iconPointer) moveObjectToFront(_iconPointer);


      if (_disabledText) _disabledText->setVisible(false);
      if (_enabledText)  _enabledText->setVisible(false);

      if (_pressedText)
      {
         _pressedText->setVisible(true);
         _pressedText->setCaptureBackgroundColor();
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
       if(_enabledBitmap)
       {
          deleteObject(_enabledBitmap);
          delete _enabledBitmap;
       }

      _enabledBitmap = new CGUIBitmap (_display, CGUIRegion(0,0,0,0), *enabledBitmapId);
      if(_enabled && !_pressed) 
        addObjectToFront(_enabledBitmap);
      else
        addObjectToBack(_enabledBitmap);

      if(_enabledText)
         _enabledText->setCaptureBackgroundColor();
    }
    else  // a null parameter means remove an existing bitmap
    {
      if(_enabledBitmap)
      {
         deleteObject(_enabledBitmap);
         delete _enabledBitmap;
         _enabledBitmap = NULL;
      }
    }
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
       if(_disabledBitmap)
       {
          deleteObject(_disabledBitmap);
          delete _disabledBitmap;
       }

      _disabledBitmap = new CGUIBitmap (_display, CGUIRegion(0,0,0,0), *disabledBitmapId);
      if(!_enabled && !_pressed) 
        addObjectToFront(_disabledBitmap);
      else
        addObjectToBack(_disabledBitmap);

      if(_disabledText)
         _disabledText->setCaptureBackgroundColor();
    }
    else  // a null parameter means remove an existing bitmap
    {
      if(_disabledBitmap)
      {
         deleteObject(_disabledBitmap);
         delete _disabledBitmap;
         _disabledBitmap = NULL;
      }
    }
}

// SET PRESSED BITMAP
// Loads a new pressed bitmap object for the button.
void CGUIButton::setPressedBitmap (CGUIBitmapInfo *pressedBitmapId) // ptr to bitmap object to display when button is pressed
{
   if (pressedBitmapId)
   {
      if(_pressedBitmap)
      {
         deleteObject(_pressedBitmap);
         delete _pressedBitmap;
      }

     _pressedBitmap = new CGUIBitmap (_display, CGUIRegion(0,0,0,0), *pressedBitmapId);
     if(_pressed) 
       addObjectToFront(_pressedBitmap);
     else
       addObjectToBack(_pressedBitmap);
     
     if(_pressedText)
        _pressedText->setCaptureBackgroundColor();
   }
   else  // a null parameter means remove an existing bitmap
   {
     if(_pressedBitmap)
     {
        deleteObject(_pressedBitmap);
        delete _pressedBitmap;
        _pressedBitmap = NULL;
     }
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

void CGUIButton::setText (const StringChar * string = NULL) // ptr to a text object associated with the button
{
   if (string)
   {
      setEnabledText(string);
      setDisabledText(string);
      setPressedText(string);
   }
}

void CGUIButton::setText()
{
   setEnabledText();
   setDisabledText();
   setPressedText();
}

void CGUIButton::setEnabledText (CGUITextItem * textItem)
{
   if (textItem)
   {
      if (_enabledText)
      {
         _enabledText->setText(textItem);
      }
      else
      {
         _enabledText = new CGUIText(_display, textItem);
         addObjectToFront(_enabledText);
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
         _enabledText = new CGUIText(_display);
         _enabledText->setText(string);
         addObjectToFront(_enabledText);
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

void CGUIButton::setEnabledText (const StringChar * string = NULL)
{
   if (string)
   {
      if (_enabledText)
      {
         _enabledText->setText(string);
      }
      else
      {
         _enabledText = new CGUIText(_display);
         _enabledText->setText(string);
         addObjectToFront(_enabledText);
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

void CGUIButton::setEnabledText()
{
   _enabledText->setText();
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
         _disabledText = new CGUIText(_display, textItem);
         addObjectToFront(_disabledText);
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
         _disabledText = new CGUIText(_display);
         _disabledText->setText(string);
         addObjectToFront(_disabledText);
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

void CGUIButton::setDisabledText (const StringChar * string = NULL)
{
   if (string)
   {
      if (_disabledText)
      {
         _disabledText->setText(string);
      }
      else
      {
         _disabledText = new CGUIText(_display);
         _disabledText->setText(string);
         addObjectToFront(_disabledText);
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

void CGUIButton::setDisabledText()
{
   _disabledText->setText();
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
         _pressedText = new CGUIText(_display, textItem);
         addObjectToFront(_pressedText);
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
         _pressedText = new CGUIText(_display);
         _pressedText->setText(string);
         addObjectToFront(_pressedText);
      }      
   }else
   {
      // NOT GOOD!!! Nothing to do.
   }
}

void CGUIButton::setPressedText (const StringChar * string = NULL)
{
   if (string)
   {
      if (_pressedText)
      {
         _pressedText->setText(string);
      }
      else
      {
         _pressedText = new CGUIText(_display);
         _pressedText->setText(string);
         addObjectToFront(_pressedText);
      }      
   }else
   {
      // NOT GOOD!!! Nothing to do.
   }
}

void CGUIButton::setPressedText()
{
   _pressedText->setText();
}

// SET TEXTSTYLE
// Set/change the style of the text associated with 
// this button in ALL states.  This is a pass-thru to the 
// text object previously set with this button.  
void CGUIButton::setStylingRecord (StylingRecord * textStylingRecord) // style record with appropriate features set
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
void CGUIButton::setEnabledStylingRecord (StylingRecord * enabledTextStylingRecord) // style record with appropriate features set
{                     
   if (_enabledText)  _enabledText->setStylingRecord(enabledTextStylingRecord);
}

void CGUIButton::setDisabledStylingRecord (StylingRecord * disabledTextStylingRecord)
{
   if (_disabledText) _disabledText->setStylingRecord(disabledTextStylingRecord);
}

void CGUIButton::setPressedStylingRecord (StylingRecord * pressedTextStylingRecord)
{
   if (_pressedText)  _pressedText->setStylingRecord(pressedTextStylingRecord);
}
  
void CGUIButton::setEnabledTextColor(CGUIColor color)
{
   _enabledText->setColor(color);
}

void CGUIButton::setDisabledTextColor(CGUIColor color)
{
   _disabledText->setColor(color);
}

void CGUIButton::setPressedTextColor(CGUIColor color)
{
   _pressedText->setColor(color);
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

      bitmapSize.height = iconId->getHeight();
      buttonSize = _enabledBitmap->getRegion();

      if ((x == -1 ) && (y == -1))
      {
         // Default placement of icon.
         _iconPointer = new CGUIBitmap(_display, CGUIRegion(3, buttonSize.height-bitmapSize.height-3, 0, 0), *iconId);
      }
      else
      {
         _iconPointer = new CGUIBitmap(_display, CGUIRegion(x, y, 0, 0), *iconId);
      }
      addObjectToFront(_iconPointer);
      _iconPointer->setVisible(visible);
   }
}

// ENABLE ICON
// Set a previously set icon as visible on the button
void CGUIButton::enableIcon ()
{
	if(_iconPointer)
	{
		_iconPointer->setVisible(true);
		moveObjectToFront(_iconPointer);
	}
}

// DISABLE ICON
// Set a previously set icon as invisible
void CGUIButton::disableIcon ()
{
	if(_iconPointer)
	{
		_iconPointer->setVisible(false);
	}
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
      // deleteObject(_enabledBitmap);

      if (_iconPointer) moveObjectToBack(_iconPointer);

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
      // deleteObject(_pressedBitmap);

      if (_iconPointer) moveObjectToFront(_iconPointer);

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
      if(_enabledBitmap) moveObjectToFront(_enabledBitmap);
      // deleteObject(_pressedBitmap);

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
	/***  I don't think this is ever called
   if (!_enabled)
   {
      moveObjectToFront(_disabledBitmap);
      if (_enabledBitmap)moveObjectToBack(_enabledBitmap);
      if (_pressedBitmap)moveObjectToBack(_pressedBitmap);

      if (_iconPointer) moveObjectToFront(_iconPointer);

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
	**/
}

// DO ON INVISIBLE
// actions performed when button is made invisible.  Actions are:
//	if button is visible, reset to invisible internally and with parent.
void CGUIButton::doOnInvisible()
{
}

