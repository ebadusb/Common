/*
 *        Copyright (c) 2004 by Gambro BCT, Inc.  All rights reserved.
 *
 *  $Header$
 *  This file defines the base class for all button styles in the common GUI.
 *  An object of this class types can be used to generate a standard button.
 */

#include <vxWorks.h>
#include "cgui_window_object.h"
#include "cgui_button.h"
#include "cgui_bitmap_info.h"
#include "datalog_reserved_stream.h"
#include "gui_button_press_message_res.h"

using namespace GuiButtonPressMessageRes;

// set static variable
int CGUIButton::ButtonIcon::_iconCounter = 0;
const bool CGUIButton::PRESSED = true;
const bool CGUIButton::RELEASED = !CGUIButton::PRESSED;
const bool CGUIButton::ENABLED = true;
const bool CGUIButton::DISABLED = !CGUIButton::ENABLED;

// CONSTRUCTOR
CGUIButton::CGUIButton  (CGUIDisplay        & display,                            // reference to a cguidisplay object for display context
                         CGUIWindow         * parent,                                     // pointer to a parent window
                         ButtonData         & buttonData,                         // reference to button data for bitmaps, text and behavior
                         Message<long>      * pressEventObject /*= NULL*/,        // ptr to int message object to output when button is pressed and released
                                                                                                                                                                                                  // can be NULL to indicate no message is output
                         Message<long>      * audioMessage /*= NULL*/,            // ptr to audio message to send when button is pressed
                         DataLog_Level      * buttonLevel /*= NULL*/,             // datalog level object used to log button press events
                         bool                 enabled /*= true*/,                 // button will be constructed as enabled unless specified here
                         bool                 visible /*= true*/,                 // button will be constructed as visbile unless otherwise specified here
                         bool                 pressed /*= false*/
                        ) :
CGUIWindow(display),
   _buttonMessagePointer( NULL ),
   _enabled( enabled ),
   _enabledText( NULL ),
   _enabledBitmap( NULL ),
   _disabledText( NULL ),
   _disabledBitmap( NULL ),
   _pressedText( NULL ),
   _pressedBitmap( NULL ),
   _pressedDisabledText( NULL ),
   _pressedDisabledBitmap( NULL ),
   _textRegion(),
   _haveTextRegion( false ),
   _audioMessagePointer( NULL ),
   _pressEventMessageEnabled( false ),
   _releasedEventMessageEnabled( false ),
   _pressedCBEnabled( false ),
   _releasedCBEnabled( false ),
   _btnDataLogLevel( NULL ),
   _iconPointer( NULL ),
   _behaviorType(buttonData.type),
   _pressed(pressed),
   _alternateLogTextUsed(false)
{
   initializeButton( display, parent, buttonData,
                     pressEventObject, audioMessage, buttonLevel, enabled, visible, pressed );

}

CGUIButton::CGUIButton(CGUIDisplay & display)
:CGUIWindow(display),
   _buttonMessagePointer( 0 ),
   _enabled( true ),
   _enabledText( NULL ),
   _enabledBitmap( NULL ),
   _disabledText( NULL ),
   _disabledBitmap( NULL ),
   _pressedText( NULL ),
   _pressedBitmap( NULL ),
   _pressedDisabledText( NULL ),
   _pressedDisabledBitmap( NULL ),
   _textRegion(),
   _haveTextRegion( false ),
   _audioMessagePointer( NULL ),
   _pressEventMessageEnabled( false ),
   _releasedEventMessageEnabled( false ),
   _pressedCBEnabled( false ),
   _releasedCBEnabled( false ),
   _behaviorType( RaiseAfterRelease ),
   _btnDataLogLevel( NULL ),
   _iconPointer( 0 ),
   _pressed( false ),
   _alternateLogTextUsed(false)
{

}

void CGUIButton::setCGUIButton( CGUIDisplay & display, CGUIWindow * parent, ButtonData & buttonData,
                                Message<long>      * pressEventObject /*= NULL*/,     // ptr to int message object to output when button is pressed and released
                                                                                                                                                                                                                          // can be NULL to indicate no message is output
                                Message<long>      * audioMessage /*= NULL*/,                 // ptr to audio message to send when button is pressed
                                DataLog_Level      * buttonLevel /*= NULL*/,                  // datalog level object used to log button press events
                                bool                 enabled /*= true*/,                              // button will be enabled unless specified here
                                bool                 visible /*= true*/,                              // button will be visbile unless otherwise specified here
                                bool                 pressed /*= false*/ )
{
   _behaviorType = buttonData.type;
   _iconPointer = NULL;
   _pressed = pressed;
   _enabled = enabled;
   _alternateLogTextUsed = false;

   initializeButton( display, parent, buttonData,
                     pressEventObject, audioMessage, buttonLevel, enabled, visible, pressed );
}

void CGUIButton::initializeButton( CGUIDisplay & display, CGUIWindow * parent, ButtonData & buttonData,
                                   Message<long>      * pressEventObject /*= NULL*/ ,
                                   Message<long>      * audioMessage /*= NULL*/ ,
                                   DataLog_Level      * buttonLevel /*= NULL */,
                                   bool                 enabled /*= true */,
                                   bool                 visible /*= true */,
                                   bool                 pressed /*= false*/ )
{
   _buttonPressLogText[MAX_BUTTON_LOG_SIZE] = 0;

   if ( !buttonLevel )
   {
      buttonLevel = &log_level_critical;
   }
  // Button size is based on enabled bitmap (all bitmaps should be the same size anyway)
   CGUIRegion buttonRegion = CGUIRegion(buttonData.left, buttonData.top, 0, 0); //buttonData.enabledBitmapId->getWidth(), buttonData.enabledBitmapId->getHeight());

   _enabled = enabled;
   _pressed = pressed;

   if ( buttonData.enabledBitmapId )
   {
      _enabledBitmap = new CGUIBitmap (display, CGUIRegion(0,0,0,0), *buttonData.enabledBitmapId);
      buttonRegion = CGUIRegion(buttonData.left, buttonData.top, _enabledBitmap->getRegion().width, _enabledBitmap->getRegion().height);
      setRegion(buttonRegion);
      addObjectToFront(_enabledBitmap);
   }

   if ( buttonData.disabledBitmapId )
   {
      _disabledBitmap = new CGUIBitmap (display, CGUIRegion(0,0,0,0), *buttonData.disabledBitmapId);
      if ( !buttonData.pressedDisabledBitmapId )
      {
         _pressedDisabledBitmap = new CGUIBitmap (display, CGUIRegion(0,0,0,0), *buttonData.disabledBitmapId);
         if ( DISABLED == _enabled && PRESSED == _pressed )
         {
            addObjectToFront( _pressedDisabledBitmap );
         }
         else
         {
            addObjectToBack( _pressedDisabledBitmap );
         }
      }

      if ( DISABLED == _enabled && RELEASED == _pressed )
      {
         addObjectToFront(_disabledBitmap);
      }
      else
      {
         addObjectToBack(_disabledBitmap);
      }
   }

   if ( buttonData.pressedBitmapId )
   {
      _pressedBitmap = new CGUIBitmap (display, CGUIRegion(0,0,0,0), *buttonData.pressedBitmapId);

      if ( PRESSED == _pressed && ENABLED == _enabled )
      {
         addObjectToFront(_pressedBitmap);
      }
      else
      {
         addObjectToBack(_pressedBitmap);
      }
   }

   if ( buttonData.pressedDisabledBitmapId )
   {
      _pressedDisabledBitmap = new CGUIBitmap( display, CGUIRegion(), *buttonData.pressedDisabledBitmapId );

      if ( PRESSED == _pressed && DISABLED == _enabled )
      {
         addObjectToFront( _pressedDisabledBitmap );
      }
      else
      {
         addObjectToBack( _pressedDisabledBitmap );
      }
   }

  // Background Icon Bitmaps
   if ( buttonData.enabledButtonIcon )
   {
      _iconList.push_back( buttonData.enabledButtonIcon );

      if ( ENABLED == _enabled && RELEASED == _pressed )
      {
         buttonData.enabledButtonIcon->setVisible( true );
         addObjectToFront( buttonData.enabledButtonIcon->getBitmap());
      }
      else
      {
         buttonData.enabledButtonIcon->setVisible( false );
         addObjectToBack( buttonData.enabledButtonIcon->getBitmap());
      }
   }

   if ( buttonData.disabledButtonIcon )
   {
      _iconList.push_back( buttonData.disabledButtonIcon );

      if ( !buttonData.pressedDisabledButtonIcon )
      {

         CGUIBitmapInfo* bitmapInfo = buttonData.disabledButtonIcon->getBitmapInfo();

         if ( bitmapInfo )
         {
            ButtonIcon* icon = new ButtonIcon( display,
               bitmapInfo,
               CGUIButton::PRESSED,
               CGUIButton::DISABLED,
               buttonData.disabledButtonIcon->width(),
               buttonData.disabledButtonIcon->height() );

            _iconList.push_back( icon );

            if ( DISABLED == _enabled && PRESSED == _pressed )
            {
               icon->setVisible( true );
               addObjectToFront( icon->getBitmap() );
            }
            else
            {
               icon->setVisible( false );
               addObjectToBack( icon->getBitmap() );
            }
         }
      }

      if ( DISABLED == _enabled && RELEASED == _pressed )
      {
         buttonData.disabledButtonIcon->setVisible( true );
         addObjectToFront( buttonData.disabledButtonIcon->getBitmap());
      }
      else
      {
         buttonData.disabledButtonIcon->setVisible( false );
         addObjectToBack( buttonData.disabledButtonIcon->getBitmap());
      }
   }

   if ( buttonData.pressedButtonIcon )
   {
      _iconList.push_back( buttonData.pressedButtonIcon );

      if ( PRESSED ==_pressed && ENABLED == _enabled )
      {
         buttonData.pressedButtonIcon->setVisible( true );
         addObjectToFront( buttonData.pressedButtonIcon->getBitmap());
      }
      else
      {
         buttonData.pressedButtonIcon->setVisible( false );
         addObjectToBack( buttonData.pressedButtonIcon->getBitmap());
      }
   }

   if ( buttonData.pressedDisabledButtonIcon )
   {
      _iconList.push_back( buttonData.pressedDisabledButtonIcon );

      if ( PRESSED == _pressed && DISABLED == _enabled )
      {
         buttonData.pressedDisabledButtonIcon->setVisible( true );
         addObjectToFront(buttonData.pressedDisabledButtonIcon->getBitmap() );
      }
      else
      {
         buttonData.pressedDisabledButtonIcon->getBitmap()->setVisible( false );
         addObjectToBack( buttonData.pressedDisabledButtonIcon->getBitmap() );
      }
   }

  //
  // Text stuff
  //
   if ( buttonData.hMargin > 0 && _enabledBitmap )
   {
      _textRegion.x = buttonData.hMargin;
      _textRegion.width = _enabledBitmap->getRegion().width - 2 * buttonData.hMargin;
      _haveTextRegion = true;
   }
   if ( buttonData.vMargin > 0 && _enabledBitmap )
   {
      _textRegion.y = buttonData.vMargin;
      _textRegion.height = _enabledBitmap->getRegion().height - 2 * buttonData.hMargin;
      _haveTextRegion = true;
   }

   if ( buttonData.vMargin < 1 || buttonData.hMargin < 1 ) _haveTextRegion = false;

   if ( buttonData.enabledTextItem )
   {
      if ( !buttonData.enabledStylingRecord )
      {
         buttonData.enabledStylingRecord = new StylingRecord(buttonData.enabledTextItem->getStylingRecord());
      }

      if ( _haveTextRegion )
      {
         buttonData.enabledStylingRecord->region = _textRegion;
      }
      else if ( (buttonData.enabledStylingRecord->region.width == 0) && (buttonData.enabledStylingRecord->region.height == 0) )
      {
         if ( _enabledBitmap )
         {
            buttonData.enabledStylingRecord->region.width = _enabledBitmap->getRegion().width;
            buttonData.enabledStylingRecord->region.height = _enabledBitmap->getRegion().height;
         }
      }

      _enabledText = new CGUIText(display, buttonData.enabledTextItem, buttonData.enabledStylingRecord);
      _enabledText->setCaptureBackgroundColor();
      addObjectToFront(_enabledText);
   }

   if ( buttonData.alternateButtonId[0] )
   {
      strncpy(_buttonPressLogText, buttonData.alternateButtonId, MAX_BUTTON_LOG_SIZE);
      _alternateLogTextUsed = true;
   }
   else if ( buttonData.enabledTextItem )
   {
      strncpy(_buttonPressLogText, buttonData.enabledTextItem->getId(), MAX_BUTTON_LOG_SIZE);
   }
   else
   {
      strncpy(_buttonPressLogText, "NO ID", MAX_BUTTON_LOG_SIZE);
   }

   if ( buttonData.disabledTextItem )
   {
      if ( !buttonData.disabledStylingRecord )
      {
         buttonData.disabledStylingRecord = new StylingRecord(buttonData.disabledTextItem->getStylingRecord());
      }

      if ( _haveTextRegion )
      {
         buttonData.disabledStylingRecord->region = _textRegion;
      }
      else if ( (buttonData.disabledStylingRecord->region.width == 0) && (buttonData.disabledStylingRecord->region.height == 0) )
      {
         if ( _enabledBitmap )
         {
            buttonData.disabledStylingRecord->region.width = _enabledBitmap->getRegion().width;
            buttonData.disabledStylingRecord->region.height = _enabledBitmap->getRegion().height;
         }
      }

      _disabledText = new CGUIText(display, buttonData.disabledTextItem, buttonData.disabledStylingRecord);
      _disabledText->setCaptureBackgroundColor();
      _disabledText->setVisible(false);
      addObjectToFront(_disabledText);
      _textList.push_back( StateTextStruct( _disabledText, RELEASED, DISABLED ) );

      // If no pressed disabled text item is supplied use the disabled
      if ( !buttonData.pressedDisabledTextItem )
      {
         _pressedDisabledText = new CGUIText(display, buttonData.disabledTextItem, buttonData.disabledStylingRecord);
         _pressedDisabledText->setCaptureBackgroundColor();
         addObjectToFront(_pressedDisabledText);
         _textList.push_back( StateTextStruct( _pressedDisabledText, PRESSED, DISABLED ) );
      }
   }

   if ( buttonData.pressedTextItem )
   {
      if ( !buttonData.pressedStylingRecord )
      {
         buttonData.pressedStylingRecord = new StylingRecord(buttonData.pressedTextItem->getStylingRecord());
      }

      if ( _haveTextRegion )
      {
         buttonData.pressedStylingRecord->region = _textRegion;
      }
      else if ( (buttonData.pressedStylingRecord->region.width == 0) && (buttonData.pressedStylingRecord->region.height == 0) )
      {
         if ( _enabledBitmap )
         {
            buttonData.pressedStylingRecord->region.width = _enabledBitmap->getRegion().width;
            buttonData.pressedStylingRecord->region.height = _enabledBitmap->getRegion().height;
         }
      }

      _pressedText = new CGUIText(display, buttonData.pressedTextItem, buttonData.pressedStylingRecord);
      _pressedText->setCaptureBackgroundColor();
      _pressedText->setVisible(false);
      addObjectToFront(_pressedText);
   }

   if ( buttonData.pressedDisabledTextItem )
   {
      if ( !buttonData.pressedDisabledStylingRecord )
      {
         buttonData.pressedDisabledStylingRecord = new StylingRecord(buttonData.pressedDisabledTextItem->getStylingRecord());
      }

      if ( _haveTextRegion )
      {
         buttonData.pressedDisabledStylingRecord->region = _textRegion;
      }
      else if ( (buttonData.pressedDisabledStylingRecord->region.width == 0) && (buttonData.pressedDisabledStylingRecord->region.height == 0) )
      {
         if ( _disabledBitmap )
         {
            buttonData.pressedDisabledStylingRecord->region.width = _disabledBitmap->getRegion().width;
            buttonData.pressedDisabledStylingRecord->region.height = _disabledBitmap->getRegion().height;
         }
      }

      _pressedDisabledText = new CGUIText(display, buttonData.pressedDisabledTextItem, buttonData.pressedDisabledStylingRecord);
      _pressedDisabledText->setCaptureBackgroundColor();
      _pressedDisabledText->setVisible(false);
      addObjectToFront(_pressedDisabledText);
   }

   attach(parent);

   if ( pressEventObject )
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

   // call helper to display the correct text for this state
   updateButtonText();

}

void CGUIButton::attach(CGUIWindow * parent)
{
   WIN_ATTRIB winAttrib = WIN_ATTRIB_NO_INPUT|WIN_ATTRIB_VISIBLE;
   CGUIWindow::attach(parent, winAttrib);

   winCbAdd(_id, MSG_PTR_BTN1_DOWN, 0, &CGUIWindow::uglPointerCallback, UGL_NULL);
   winCbAdd(_id, MSG_PTR_BTN1_UP, 0, &CGUIWindow::uglPointerCallback, UGL_NULL);
}


// DESTRUCTOR
CGUIButton::~CGUIButton ()
{
   if ( _enabledText ) delete _enabledText;
   if ( _disabledText ) delete _disabledText;
   if ( _pressedText ) delete _pressedText;
   if ( _pressedDisabledText ) delete _pressedDisabledText;

   if ( _enabledBitmap ) delete _enabledBitmap;
   if ( _disabledBitmap ) delete _disabledBitmap;
   if ( _pressedBitmap ) delete _pressedBitmap;
   if ( _pressedDisabledBitmap ) delete _pressedDisabledBitmap;

   if ( _iconPointer ) delete _iconPointer;

  // delete transparent bitmaps
   if ( _iconList.size() > 0 )
   {
      for ( vector< ButtonIcon *>::iterator iconIter = _iconList.begin(); iconIter != _iconList.end(); ++iconIter )
      {
         delete (*iconIter);
      }

      _iconList.clear();
   }

   winCbRemove(_id, &CGUIWindow::uglPointerCallback);
   winCbRemove(_id, &CGUIWindow::uglPointerCallback);
}

void CGUIButton::enable(const bool beEnabled)
{
   if ( beEnabled )
      enable();
   else
      disable();
}

/**
 *  Set the state of the button to enabled.
 *  If currently invisible, the button is made visible.
 * Almost identical to enableReleased().
 *
 */
void CGUIButton::enable(void)
{
   enableReleased();
}

/**
 * ENABLEReleased when top bitmap is pressed but  the button is in the
 *        enabled state to put the released bitmap back on top.
 *        Set the state of the button to enabled.
 *        If currently invisible, the button is made visible. *
 *
 */
void CGUIButton::enableWhenPressed(void)
{
  // Renamed the function to more accurately
  //              describe the function.  Needed to keep
  //      this function for code maintenance.
   enableReleased();
}

/**
 * Almost identical to enable(). One of these functions should really be
 * implemented in terms of the other.
 *
 */
void CGUIButton::enableReleased( void )
{
   if ( DISABLED == _enabled || PRESSED == _pressed )
   {
      _enabled = ENABLED;
      _pressed = RELEASED;

      updateButtonBitmap();

      updateButtonIcon();

      setDisabled(false);
      setWindowVisibility(true);

      // call helper to display the correct text for this state
      updateButtonText();
   }
}

///  Set the state of the button to enabled and already pressed.
void CGUIButton::enablePressed(void)
{
   if ( DISABLED == _enabled || RELEASED == _pressed )  // Button must be either disabled or up to execute this logic.
   {
      _enabled = ENABLED;
      _pressed = PRESSED;

      updateButtonBitmap();

      // call helper operation to display the correct icon for this state
      updateButtonIcon();

      setDisabled(false);
      setWindowVisibility(true);

      // call helper to display the correct text for this state
      updateButtonText();
   }
}

/**
 * Set the state of the button to DISABLED / PRESSED
 *
 */
void CGUIButton::disablePressed()
{
   if ( ENABLED == _enabled || RELEASED == _pressed )
   {
      _enabled = DISABLED;
      _pressed = PRESSED;

      updateButtonBitmap();
      updateButtonIcon();
      setDisabled(false);
      setWindowVisibility(true);

      // call helper to display the correct text for this state
      updateButtonText();
   }
}

/**
 * Set the state of the button to DISABLED / RELEASED
 *
 */
void CGUIButton::disableReleased()
{
   if ( ENABLED == _enabled || PRESSED == _pressed )
   {
      _enabled = DISABLED;
      _pressed = RELEASED;

      updateButtonBitmap();
      updateButtonIcon();
      setDisabled(false);
      setWindowVisibility(true);

      // call helper to display the correct text for this state
      updateButtonText();
   }
}

/**
 *  Set the state of the button to disabled.
 *  If currently invisible, the button is made visible.
 *
 */
void CGUIButton::disable()
{
   if ( ENABLED == _enabled )
   {
      _enabled = DISABLED;

      updateButtonBitmap();

      updateButtonIcon();

      setDisabled(false);
      setWindowVisibility(true);

      // call helper to display the correct text for this state
      updateButtonText();
   }
}

/**
 * Enable the current is it is currently pressed
 *
 */
void CGUIButton::enableIfReleased(void)
{
   if ( !isPressed() ) enable();
}

/**
 * Update the visibility of the text to correspond to the current state
 *
 */
void CGUIButton::updateButtonIcon()
{
   ButtonIcon* iconPtr = NULL;

   // use a reverse iterator because in some cases there are multiple overlayed
   // icons used and they need to be displayed in the opposite order they are added to the list
   for ( vector< ButtonIcon *>::reverse_iterator riter = _iconList.rbegin();
       riter != _iconList.rend(); ++riter )
   {
      iconPtr = *riter;
      if ( iconPtr )
      {
         if ( iconPtr->enabled() == _enabled && iconPtr->pressed() == _pressed )
         {
            iconPtr->setVisible( true );
            moveObjectToFront( iconPtr->getBitmap() );
         }
         else
         {
            iconPtr->setVisible( false );
            moveObjectToBack( iconPtr->getBitmap() );
         }
      }
   }
}

///  Make the button invisible.  Uses CGUIWindow class to do this.
void CGUIButton::invisible()
{
   setDisabled(true);
   setWindowVisibility(false);
}

/// return flag indicating current state of visibility.  Uses CGUIWindow class to do this
bool CGUIButton::isVisible() const
{
   return winVisibleGet(_id);
}

/**
 * set an int message to be output when the button is pressed.  The default is for a message to be sent
 * on press and on release with the value of the message indicating which event occured.  This
 * behavior can be modified to only send a message on press or release by using the enable/disable
 * methods provided below.
 * NULL removes a previously set message from being sent.
 * There can be only one pressed message per button.  The message is automatically enabled after this call.
 *
 * @param newEventMessage
 */
void CGUIButton::setMessage (Message<long>* newEventMessage)
{
   if ( newEventMessage )
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

/// Enables a message sent on button press.
void CGUIButton::enablePressedMessage()
{
   _pressEventMessageEnabled = true;
}

/// Disables message sent on button press.
void CGUIButton::disablePressedMessage()
{
   _pressEventMessageEnabled = false;
}

/// Enables message sent on button release.
void CGUIButton::enableReleasedMessage()
{
   _releasedEventMessageEnabled = true;
}

/// Disables message sent on button release
void CGUIButton::disableReleasedMessage()
{
   _releasedEventMessageEnabled = false;
}

/**
 * This method establishes the pointer to the routine that
 * gets called when the button is pressed.
 * There can be only one pressed callback per button.
 * The callback is automatically enabled after this call.
 *
 * @param cb
 */
void CGUIButton::setPressedCallback (const CallbackBase cb)
{
   _CBOnPressed = cb;
   _pressedCBEnabled = true;
}

/**
 * This method establishes the pointer to the routine that
 * gets called when the button is released.
 * There can be only one released callback per button.
 * The callback is automatically enabled after this call. *
 *
 * @param cb
 */
void CGUIButton::setReleasedCallback (const CallbackBase cb)
{
   _CBOnReleased = cb;
   _releasedCBEnabled = true;
}

/// ENABLE PRESSED CALLBACK
void CGUIButton::enablePressedCallback()
{
   _pressedCBEnabled = true;
}

/**
 * keeps the call back from last set_pressed_callback, but disables it until an
 * enable_pressed_callback is called.
 */
void CGUIButton::disablePressedCallback()
{
   _pressedCBEnabled = false;
}

//ENABLE RELEASED CALLBACK
void CGUIButton::enableReleasedCallback()
{
   _releasedCBEnabled = true;
}

/**
 * Keeps the call back from last set_pressed_callback,
 * but disables it until an enable_pressed_callback is called.
 */
void CGUIButton::disableReleasedCallback()
{
   _releasedCBEnabled = false;
}

/**
 * POINTER EVENT
 * virtual function in CGUIWindow class that is implemented here for buttons to apply the
 * correct callback function and send the correct message based on the action taken
 * by the operator, i.e. press, release
 *
 * @param event
 */
void CGUIButton::pointerEvent (const PointerEvent & event) // event structure received from UGL on button press/release
{
   if ( ENABLED == _enabled )
   {
      if ( event.eventType == PointerEvent::ButtonPress )
      {
         doOnPress();

         if ( _pressedCBEnabled )
         {
            _CBOnPressed();
         }

         if ( _pressEventMessageEnabled )
         {
            _buttonMessagePointer->send(PointerEvent::ButtonPress);
         }
      }
      else if ( event.eventType == PointerEvent::ButtonRelease )
      {
         if ( _behaviorType == RaiseAfterRelease )
         {
            doOnEnable();

            if ( _releasedCBEnabled )
            {
               _CBOnReleased();
               doOnRelease();
            }
         }
         if ( _behaviorType == RaiseAfterCallback )
         {
            doOnEnable();

            if ( _releasedCBEnabled )
            {
               _CBOnReleased();
            }
         }

         if ( _releasedEventMessageEnabled )
         {
            _buttonMessagePointer->send(PointerEvent::ButtonRelease);
         }
      }
   }
   else
      DataLog( log_level_cgui_info ) << "Button press on a DISABLED button - cguiButton::pointerEvent -- " << _buttonPressLogText << "   " << endmsg;
}

/**
 * Helper operation which will make the bitmap associated with the current state
 * visible by moving it the front of the z-order sorted list
 *
 */
void CGUIButton::updateButtonBitmap()
{
   if ( RELEASED == _pressed && ENABLED == _enabled )
   {
      // Move the ENABLED / RELEASD BITMAP to the front
      if ( _enabledBitmap ) moveObjectToFront( _enabledBitmap );
      if ( _iconPointer ) moveObjectToFront(_iconPointer);

      // Move the Other Bitmaps to the back
      if ( _disabledBitmap ) moveObjectToBack( _disabledBitmap );
      if ( _pressedBitmap ) moveObjectToBack( _pressedBitmap );
      if ( _pressedDisabledBitmap ) moveObjectToBack( _pressedDisabledBitmap );
   }
   else if ( PRESSED == _pressed && ENABLED == _enabled )
   {
      // Move the ENABLED / PRESSED BITMAP to the front
      if ( _pressedBitmap ) moveObjectToFront( _pressedBitmap );
      if ( _iconPointer ) moveObjectToBack(_iconPointer);

      // Move the Other Bitmaps to the back
      if ( _disabledBitmap ) moveObjectToBack( _disabledBitmap );
      if ( _enabledBitmap ) moveObjectToBack( _enabledBitmap );
      if ( _pressedDisabledBitmap ) moveObjectToBack( _pressedDisabledBitmap );
   }
   else if ( RELEASED == _pressed && DISABLED == _enabled )
   {
      // Move the RELASED / DISABLED BITMAP to the front
      if ( _disabledBitmap ) moveObjectToFront( _disabledBitmap );
      if ( _iconPointer ) moveObjectToFront(_iconPointer);
      // Move the Other Bitmaps to the back
      if ( _pressedBitmap ) moveObjectToBack( _pressedBitmap );
      if ( _enabledBitmap ) moveObjectToBack( _enabledBitmap );
      if ( _pressedDisabledBitmap ) moveObjectToBack( _pressedDisabledBitmap );
   }
   else if ( PRESSED == _pressed && DISABLED == _enabled )
   {
      // Move the PRESSED / DISABLED BITMAP to the front
      if ( _pressedDisabledBitmap ) moveObjectToFront( _pressedDisabledBitmap );
      if ( _iconPointer ) moveObjectToBack(_iconPointer);

      // Move the Other Bitmaps to the back
      if ( _pressedBitmap ) moveObjectToBack( _pressedBitmap );
      if ( _enabledBitmap ) moveObjectToBack( _enabledBitmap );
      if ( _disabledBitmap ) moveObjectToBack( _disabledBitmap );
   }
}

/**
 * Create a new bitmap using the given BitmapInfo and set it to correspond to the
 * specified state.
 *
 * @param bitmapInfo
 * @param pressed
 * @param enabled
 */
void CGUIButton::setBitmap( CGUIBitmapInfo *bitmapInfo, bool pressed, bool enabled )
{
   // Handle the ENABLED / RELEASED case
   if ( RELEASED == pressed && ENABLED == enabled )
   {
      if ( bitmapInfo )
      {
         if ( _enabledBitmap )
         {
            deleteObject(_enabledBitmap);
            delete _enabledBitmap;
         }
         _enabledBitmap = new CGUIBitmap (_display, CGUIRegion(0,0,0,0), *bitmapInfo);

         if ( RELEASED == _pressed && ENABLED == _enabled )
         {
            addObjectToFront( _enabledBitmap );
         }
         else
         {
            addObjectToBack(_enabledBitmap );
         }

         if ( _enabledText ) _enabledText->setCaptureBackgroundColor();
      }
      else  // a null parameter means remove an existing bitmap
      {
         if ( _enabledBitmap )
         {
            deleteObject(_enabledBitmap);
            delete _enabledBitmap;
            _enabledBitmap = NULL;
         }
      }
      if ( bitmapInfo )
      {
         _enabledBitmap->setBitmap();
         if ( _enabledText )
         {
            _enabledText->setCaptureBackgroundColor();
         }
      }
   }
   // Handle the ENABLED / PRESSED case
   if ( PRESSED == pressed && ENABLED == enabled )
   {
      if ( bitmapInfo )
      {
         if ( _pressedBitmap )
         {
            deleteObject(_pressedBitmap);
            delete _pressedBitmap;
         }
         _pressedBitmap = new CGUIBitmap (_display, CGUIRegion(0,0,0,0), *bitmapInfo);

         if ( PRESSED == _pressed && ENABLED == _enabled )
         {
            addObjectToFront( _pressedBitmap );
         }
         else
         {
            addObjectToBack(_pressedBitmap );
         }

         if ( _pressedText ) _pressedText->setCaptureBackgroundColor();
      }
      else  // a null parameter means remove an existing bitmap
      {
         if ( _pressedBitmap )
         {
            deleteObject(_pressedBitmap);
            delete _pressedBitmap;
            _pressedBitmap = NULL;
         }
      }
      if ( bitmapInfo )
      {
         _pressedBitmap->setBitmap();
         if ( _pressedText )
         {
            _pressedText->setCaptureBackgroundColor();
         }
      }
   }
   // Handle the RELEASED / DISABLED case
   if ( RELEASED == pressed && DISABLED == enabled )
   {
      if ( bitmapInfo )
      {
         if ( _disabledBitmap )
         {
            deleteObject(_disabledBitmap);
            delete _disabledBitmap;
         }
         _disabledBitmap = new CGUIBitmap (_display, CGUIRegion(0,0,0,0), *bitmapInfo);

         if ( RELEASED == _pressed && DISABLED == _enabled )
         {
            addObjectToFront( _disabledBitmap );
         }
         else
         {
            addObjectToBack(_disabledBitmap );
         }

         if ( _disabledText ) _disabledText->setCaptureBackgroundColor();
      }
      else  // a null parameter means remove an existing bitmap
      {
         if ( _disabledBitmap )
         {
            deleteObject(_disabledBitmap);
            delete _disabledBitmap;
            _disabledBitmap = NULL;
         }
      }
      if ( bitmapInfo )
      {
         _disabledBitmap->setBitmap();
         if ( _disabledText )
         {
            _disabledText->setCaptureBackgroundColor();
         }
      }
   }
   // Handle the PRESSED / DISABLED case
   if ( PRESSED == pressed && DISABLED == enabled )
   {
      if ( bitmapInfo )
      {
         if ( _pressedDisabledBitmap )
         {
            deleteObject(_pressedDisabledBitmap);
            delete _pressedDisabledBitmap;
         }
         _pressedDisabledBitmap = new CGUIBitmap (_display, CGUIRegion(0,0,0,0), *bitmapInfo);

         if ( PRESSED == _pressed && DISABLED == _enabled )
         {
            addObjectToFront( _pressedDisabledBitmap );
         }
         else
         {
            addObjectToBack(_pressedDisabledBitmap );
         }

         if ( _pressedDisabledText ) _pressedDisabledText->setCaptureBackgroundColor();
      }
      else  // a null parameter means remove an existing bitmap
      {
         if ( _pressedDisabledBitmap )
         {
            deleteObject(_pressedDisabledBitmap);
            delete _pressedDisabledBitmap;
            _pressedDisabledBitmap = NULL;
         }
      }
      if ( bitmapInfo )
      {
         _pressedDisabledBitmap->setBitmap();
         if ( _pressedDisabledText )
         {
            _pressedDisabledText->setCaptureBackgroundColor();
         }
      }
   }

   // call the helper to display the bitmap for the current state
   updateButtonBitmap();

}

// SET AUDIO
/// set the audio associated with a button press.
void CGUIButton::setAudio (Message<long> *audioObject)
{
   if ( audioObject )
   {
      _audioMessagePointer = audioObject;
   }
   else
   {
      _audioMessagePointer = NULL;
   }
}

/**
 * Get the text corrresponding to the specified state
 *
 * @param pressed
 * @param enabled
 *
 * @return CGUIText*
 */
CGUIText* CGUIButton::getText(const bool pressed, const bool enabled )
{
   if ( pressed && enabled )
   {
      return _pressedText;
   }
   else if ( pressed && !enabled )
   {
      return _pressedDisabledText;
   }
   else if ( !pressed && !enabled )
   {
      return _disabledText;
   }
   else
   {
      return _enabledText;
   }
}

/**
 * sets all the text associated with the button.
 *
 * @param textItem
 */
void CGUIButton::setText (CGUITextItem * textItem = NULL) // ptr to a text object associated with the button
{
   setText( textItem, RELEASED, ENABLED );
   setText( textItem, PRESSED, ENABLED );
   setText( textItem, RELEASED, DISABLED );
   setText( textItem, PRESSED, DISABLED );
}

/**
 * sets all the text associated with the button.
 *
 * @param string
 */
void CGUIButton::setText (const char * string = NULL) // ptr to a text object associated with the button
{
   setText( string, RELEASED, ENABLED );
   setText( string, PRESSED, ENABLED );
   setText( string, RELEASED, DISABLED );
   setText( string, PRESSED, DISABLED );
}

/**
 * Set the specified text to correspond to the state specified by the pressed and enabled parameters
 *
 * @param string
 * @param pressed
 * @param enabled
 */
void CGUIButton::setText(const char * string, const bool pressed, const bool enabled )
{
   if ( string )
   {
      // Handle the RELEASED / ENABLED case
      if ( RELEASED == pressed && ENABLED == enabled )
      {
         if ( _enabledText )
         {
            _enabledText->setText(string);
         }
         else
         {
            _enabledText = new CGUIText(_display);
            _enabledText->setText(string);
            addObjectToFront(_enabledText );
         }
      }
      // Handle the PRESSED / ENABLED case
      else if ( PRESSED == pressed && ENABLED == enabled )
      {
         if ( _pressedText )
         {
            _pressedText->setText( string );
         }
         else
         {
            _pressedText = new CGUIText( _display );
            _pressedText->setText( string );
            addObjectToFront( _pressedText );
         }
      }
      // Handle the RELEASED / DISABLED case
      else if ( RELEASED == pressed && DISABLED == enabled )
      {
         if ( _disabledText )
         {
            _disabledText->setText( string );
         }
         else
         {
            _disabledText = new CGUIText( _display );
            _disabledText->setText( string );
            addObjectToFront( _disabledText );
         }
      }
      // Handle the PRESED / DISABLED case
      else if ( PRESSED == pressed && DISABLED == enabled )
      {
         if ( _pressedDisabledText )
         {
            _pressedDisabledText->setText( string );
         }
         else
         {
            _pressedDisabledText = new CGUIText( _display );
            _pressedDisabledText->setText( string );
            addObjectToFront( _pressedDisabledText );
         }
      }
   }

   updateButtonText();
}

/**
 * Update the displayed text to make the current button state
 *
 */
void CGUIButton::updateButtonText()
{
   if ( RELEASED == _pressed && ENABLED == _enabled )
   {
      // make Released Enabled Text Visible
      if ( _enabledText )
      {
         _enabledText->setVisible( true );
         _enabledText->setCaptureBackgroundColor();
      }
      // Hide all other text
      if ( _disabledText ) _disabledText->setVisible( false );
      if ( _pressedText )_pressedText->setVisible( false );
      if ( _pressedDisabledText )_pressedDisabledText->setVisible( false );
   }
   else if ( PRESSED == _pressed && ENABLED == _enabled )
   {
      // make Pressed Enabled Text Visible
      if ( _pressedText )
      {
         _pressedText->setVisible( true );
         _pressedText->setCaptureBackgroundColor();
      }
      // Hide all other text
      if ( _disabledText ) _disabledText->setVisible( false );
      if ( _enabledText ) _enabledText->setVisible( false );
      if ( _pressedDisabledText ) _pressedDisabledText->setVisible( false );
   }
   else if ( RELEASED == _pressed && DISABLED == _enabled )
   {
      // make RELASED DISABLED Text Visible
      if ( _disabledText )
      {
         _disabledText->setVisible( true );
         _disabledText->setCaptureBackgroundColor();
      }
      // Hide all other text
      if ( _pressedText ) _pressedText->setVisible( false );
      if ( _enabledText ) _enabledText->setVisible( false );
      if ( _pressedDisabledText ) _pressedDisabledText->setVisible( false );
   }
   else if ( PRESSED == _pressed && DISABLED == _enabled )
   {
      // make PRESSED DISABLED text visible
      if ( _pressedDisabledText )
      {
         _pressedDisabledText->setVisible( true );
         _pressedDisabledText->setCaptureBackgroundColor();
      }
      // hide all other text
      if ( _pressedText ) _pressedText->setVisible( false );
      if ( _enabledText )_enabledText->setVisible( false );
      if ( _disabledText )_disabledText->setVisible( false );
   }
}
/**
 * Set the Specified Text for all button states
 *
 * @param string
 */
void CGUIButton::setText (const StringChar * string = NULL) // ptr to a text object associated with the button
{
   setText( string, RELEASED, ENABLED );
   setText( string, PRESSED, ENABLED );
   setText( string, RELEASED, DISABLED );
   setText( string, PRESSED, DISABLED );
}

/**
 * Activate the text for all the states
 */
void CGUIButton::setText()
{
   setText( RELEASED, ENABLED );
   setText( PRESSED, ENABLED );
   setText( RELEASED, DISABLED );
   setText( PRESSED, DISABLED );
}

/**
 * Set the text string for the specified state
 *
 * @param textItem
 * @param pressed
 * @param enabled
 */
void CGUIButton::setText(CGUITextItem *textItem, const bool pressed, const bool enabled )
{
   if ( textItem )
   {
      if ( !_alternateLogTextUsed )
      {
         strncpy(_buttonPressLogText, textItem->getId(), MAX_BUTTON_LOG_SIZE);
         _buttonPressLogText[MAX_BUTTON_LOG_SIZE] = 0;
      }

      // handle the RELASED / ENABLED case
      if ( RELEASED == pressed && ENABLED == enabled )
      {
         if ( _enabledText )
         {
            _enabledText->setText(textItem);
         }
         else
         {
            _enabledText = new CGUIText(_display, textItem);
            addObjectToFront(_enabledText);
         }
      }
      // handle the RELEASED / DISABLED case
      else if ( RELEASED == pressed && DISABLED == enabled )
      {
         if ( _disabledText )
         {
            _disabledText->setText(textItem);
         }
         else
         {
            _disabledText = new CGUIText(_display, textItem);
            // text is always added to the front and we control it's visibility based on the state
            addObjectToFront(_disabledText);
         }
      } // end disabled case
      // handle the PRESSED / ENABLED case
      else if ( PRESSED == pressed && ENABLED == enabled )
      {
         if ( _pressedText )
         {
            _pressedText->setText(textItem);
         }
         else
         {
            _pressedText = new CGUIText(_display, textItem);
            // text is always added to the front and we control it's visibility based on the state
            addObjectToFront(_pressedText);
         }
      } // end PRESSED / ENABLED case
      else if ( PRESSED == pressed && DISABLED == enabled )
      {
         if ( _pressedDisabledText )
         {
            _pressedDisabledText->setText( textItem );
         }
         else
         {
            _pressedDisabledText = new CGUIText( _display, textItem );
            // text is always added to the front and we control it's visibility based on the state
            addObjectToFront( _pressedDisabledText );
         }
      }
      // call helper to display the correct text for this state
      updateButtonText();
   }
}

/**
 * Set the text string for the specified state
 *
 * @param string
 * @param pressed
 * @param enabled
 */
void CGUIButton::setText(const StringChar * string, const bool pressed, const bool enabled)
{
   if ( string )
   {
      if ( RELEASED == pressed && ENABLED == enabled )
      {
         if ( _enabledText )
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
      else if ( PRESSED == pressed && ENABLED == enabled )
      {
         if ( _pressedText )
         {
            _pressedText->setText(string);
         }
         else
         {
            _pressedText = new CGUIText(_display);
            _pressedText->setText(string);
            addObjectToFront(_pressedText);
         }
      }
      else if ( RELEASED == pressed && DISABLED == enabled )
      {
         if ( _disabledText )
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
      else if ( PRESSED == pressed && DISABLED == enabled )
      {
         if ( _pressedDisabledText )
         {
            _pressedDisabledText->setText(string);
         }
         else
         {
            _pressedDisabledText = new CGUIText(_display);
            _pressedDisabledText->setText(string);
            addObjectToFront(_pressedDisabledText);
         }
      }
      // call helper to display the correct text for this state
      updateButtonText();
   }
}

/**
 * Displays the text for the specified state
 *
 * @param pressed
 * @param enabled
 */
void CGUIButton::setText( bool pressed, bool enabled )
{
   if ( _pressedText && PRESSED == pressed && ENABLED == enabled )
   {
      _pressedText->setText();
   }
   else if ( _enabledText && RELEASED == pressed && ENABLED == enabled )
   {
      _enabledText->setText();
   }
   else if ( _disabledText && RELEASED == pressed && DISABLED == enabled )
   {
      _disabledText->setText();
   }
   else if ( _pressedDisabledText && CGUIButton::PRESSED == pressed && DISABLED == enabled )
   {
      _pressedDisabledText->setText();
   }
}

/**
 * Set the Bitmap corresponding to the enabled State
 * Deprecated - use setBitmap( CGUIBitmapInfo*, bool, bool)
 * @param enabledBitmapId
 */
void CGUIButton::setEnabledBitmap(CGUIBitmapInfo *enabledBitmapId)
{
   setBitmap( enabledBitmapId, RELEASED, ENABLED );
}
//SET DISABLED BITMAP
/// loads a new disabled bitmap object for the button.
void CGUIButton::setDisabledBitmap(CGUIBitmapInfo *disabledBitmapId)
{
   setBitmap( disabledBitmapId, RELEASED, DISABLED );
}

// SET PRESSED BITMAP
/// loads a new pressed bitmap object for the button
void CGUIButton::setPressedBitmap(CGUIBitmapInfo *pressedBitmapId)
{
   setBitmap( pressedBitmapId, PRESSED, ENABLED );
}

/// Deprecated - use setText(CGUITextItem, const bool pressed, const bool enabled )
void CGUIButton::setEnabledText(CGUITextItem * textItem)
{
   setText( textItem, RELEASED, ENABLED );
};

/// Deprecated - use setText(CGUITextItem, const bool pressed, const bool enabled )
void CGUIButton::setEnabledText(const char * string = NULL)
{
   setText( string, RELEASED, ENABLED );
};

/// Deprecated - use setText(CGUITextItem, const bool pressed, const bool enabled )
void CGUIButton::setEnabledText(const StringChar * string = NULL)
{
   setText( string, RELEASED, ENABLED );
};

/// Deprecated - use setText(CGUITextItem, const bool pressed, const bool enabled )
void CGUIButton::setEnabledText()
{
   setText( RELEASED, ENABLED);
};

/// Deprecated - use setText(CGUITextItem, const bool pressed, const bool enabled )
void CGUIButton::setDisabledText(CGUITextItem * textItem = NULL)
{
   setText( textItem, RELEASED, DISABLED);
};

/// Deprecated - use setText(CGUITextItem, const bool pressed, const bool enabled )
void CGUIButton::setDisabledText(const char * string = NULL)
{
   setText( string, RELEASED, DISABLED );
};

/// Deprecated - use setText(CGUITextItem, const bool pressed, const bool enabled )
void CGUIButton::setDisabledText(const StringChar * string = NULL)
{
   setText( string, RELEASED, DISABLED );
};

/// Deprecated - use setText(CGUITextItem, const bool pressed, const bool enabled )
void CGUIButton::setDisabledText()
{
   setText( RELEASED, DISABLED );
};

/// Deprecated - use setText(CGUITextItem, const bool pressed, const bool enabled )
void CGUIButton::setPressedText(CGUITextItem * textItem = NULL)
{
   setText( textItem, PRESSED, ENABLED );
};

/// Deprecated - use setText(CGUITextItem, const bool pressed, const bool enabled )
void CGUIButton::setPressedText(const char * string = NULL)
{
   setText( string, PRESSED, ENABLED );
};

/// Deprecated - use setText(CGUITextItem, const bool pressed, const bool enabled )
void CGUIButton::setPressedText(const StringChar * string = NULL)
{
   setText( string, PRESSED, ENABLED );
};

/// Deprecated - use setText(const bool pressed, const bool enabled )
void CGUIButton::setPressedText()
{
   setText( PRESSED, ENABLED);
};

// SET ENABLED TEXTSTYLE - set/change the style of text associated with this button
/// deprecated - use setStylingRecord( StylingRecord * textStylingRecord, bool pressed, bool enabled ) instead
void CGUIButton::setEnabledStylingRecord (StylingRecord * enabledTextStylingRecord) // style record with appropriate features set
{
   setStylingRecord( enabledTextStylingRecord, RELEASED, ENABLED );
};

// SET DISABLED TEXTSTYLE - set/change the style of text associated with this button
/// deprecated - use setStylingRecord( StylingRecord * textStylingRecord, bool pressed, bool enabled ) instead
void CGUIButton::setDisabledStylingRecord (StylingRecord * disabledTextStylingRecord)
{
   setStylingRecord( disabledTextStylingRecord, RELEASED, DISABLED );
};

// SET PRESSED TEXTSTYLE - set/change the style of text associated with this button
/// deprecated - use setStylingRecord( StylingRecord * textStylingRecord, bool pressed, bool enabled ) instead
void CGUIButton::setPressedStylingRecord (StylingRecord * pressedTextStylingRecord)
{
   setStylingRecord( pressedTextStylingRecord, PRESSED, ENABLED );
};

/// Deprecated - use setTextColor( CGUIColor, bool pressed, bool enabled )
void CGUIButton::setEnabledTextColor(CGUIColor color)
{
   setTextColor( color, RELEASED, ENABLED );
};

/// Deprecated - use setTextColor( CGUIColor, bool pressed, bool enabled )
void CGUIButton::setDisabledTextColor(CGUIColor color)
{
   setTextColor( color, RELEASED, DISABLED );
};

/// Deprecated - use setTextColor( CGUIColor, bool pressed, bool enabled )
void CGUIButton::setPressedTextColor(CGUIColor color)
{
   setTextColor( color, PRESSED, ENABLED );
};



/**
  * Set/change the style of the text associated with
 * this button in ALL states.  This is a pass-thru to the
 * text object previously set with this button.
 *
 * @param textStylingRecord
 */
void CGUIButton::setStylingRecord (StylingRecord * textStylingRecord) // style record with appropriate features set
{
   if ( _enabledText )  _enabledText->setStylingRecord(textStylingRecord);
   if ( _disabledText ) _disabledText->setStylingRecord(textStylingRecord);
   if ( _pressedText )  _pressedText->setStylingRecord(textStylingRecord);
   if ( _pressedDisabledText ) _pressedDisabledText->setStylingRecord( textStylingRecord );
}

/**
 * Set the give StylingRecord for the specified state
 *
 * @param textStylingRecord
 * @param pressed
 * @param enabled
 */
void CGUIButton::setStylingRecord( StylingRecord * textStylingRecord, const bool pressed, const bool enabled )
{
   if ( _enabledText && ENABLED == enabled && RELEASED == pressed ) _enabledText->setStylingRecord(textStylingRecord);
   if ( _disabledText && DISABLED == enabled && RELEASED == pressed ) _disabledText->setStylingRecord(textStylingRecord);
   if ( _pressedText && ENABLED == enabled && PRESSED == pressed ) _pressedText->setStylingRecord(textStylingRecord);
   if ( _pressedDisabledText && DISABLED == enabled && PRESSED == pressed ) _pressedDisabledText->setStylingRecord( textStylingRecord );
}

/**
 * Set the give text color for the specified state
 *
 * @param color
 * @param pressed
 * @param enabled
 */
void CGUIButton::setTextColor( CGUIColor color, bool pressed, bool enabled )
{
   if ( _enabledText && ENABLED == enabled && RELEASED == pressed ) _enabledText->setColor(color);
   if ( _disabledText && DISABLED == enabled && RELEASED == pressed ) _disabledText->setColor(color);
   if ( _pressedText && ENABLED == enabled && PRESSED == pressed ) _pressedText->setColor(color);
   if ( _pressedDisabledText && DISABLED == enabled && PRESSED == pressed ) _pressedDisabledText->setColor( color );
}

/**
 * Set the specified text color for all button states
 *
 * @param color
 */
void CGUIButton::setTextColor(CGUIColor color)
{
   setTextColor( color, RELEASED, ENABLED );
   setTextColor( color, PRESSED, ENABLED );
   setTextColor( color, RELEASED, DISABLED );
   setTextColor( color, PRESSED, DISABLED );
}

/**
 * Transparent bitmap management
 * add transparent bitmap object to transparency icon list
 *
 * @param bitmapInfo
 * @param x
 * @param y
 * @param pressed
 * @param enabled
 *
 * @return int
 */
int CGUIButton::addIcon( CGUIBitmapInfo * bitmapInfo, const short x, const short y, const bool pressed, const bool enabled )
{
   int result = -1;

   ButtonIcon * buttonIcon = NULL;

   if ( bitmapInfo )
      buttonIcon = new ButtonIcon( _display, bitmapInfo, pressed, enabled, x, y );

   if ( buttonIcon )
   {
      result = buttonIcon->getIconId();

      // Must append to vectors at the end
      _iconList.push_back( buttonIcon );

      if ( enabled == _enabled && pressed == _pressed )
      {
         buttonIcon->setVisible( true );
         addObjectToFront( buttonIcon->getBitmap());
      }
      else
      {
         buttonIcon->setVisible( false );
         addObjectToBack( buttonIcon->getBitmap());
      }
   }

   return result;
}

/**
 * remove icon in list to front
 *
 * @param iconId
 *
 * @return bool
 */
bool CGUIButton::removeIcon(const int iconId )
{
   bool result = false;

   vector< ButtonIcon * >::iterator iconIter = _iconList.begin();

   ButtonIcon * buttonIcon = NULL;

   while ( !_iconList.empty() && iconIter != _iconList.end() )
   {
      buttonIcon = ( *iconIter );

      if ( buttonIcon && buttonIcon->getIconId() == iconId )
      {
         delete buttonIcon;

         iconIter = _iconList.erase( iconIter );

         result = true;
      }
      else
      {
         iconIter++;
      }
   }

   return result;
}

/**
 * update button state type association of icon in list
 *
 * @param iconId
 * @param pressed
 * @param enabled
 *
 * @return bool
 */
bool CGUIButton::setIconState( int iconId, const bool pressed, const bool enabled )
{
   bool result = false;

   vector< ButtonIcon *>::iterator iconIter = _iconList.begin();

   ButtonIcon * buttonIcon = NULL;

   while ( iconIter != _iconList.end( ) )
   {
      buttonIcon = ( *iconIter );

      if ( buttonIcon && buttonIcon->getIconId() == iconId )
      {
         buttonIcon->setPressed( pressed );
         buttonIcon->setEnabled( enabled );

         result = true;
      }
      iconIter++;
   }

   updateButtonIcon();

   return result;
}

/**
 * Set an icon to be associated with the button.
 *
 * @param iconId
 * @param x
 * @param y
 * @param visible
 */
void CGUIButton::setIcon (CGUIBitmapInfo * iconId,  // ptr to bitmap object for icon
                          const short x=-1,         // upper left X coordinate to start icon relative to button location (0 means coincident with x coordinate of upper left position of button)
                          const short y=-1,         // upper left Y coordinate to start icon relative to button location
                          bool visible=true         // visibility of icon upon creation, uses CGUIBitmap class to set visibility
                         )
{

   if ( _iconPointer )
   {
      deleteObject(_iconPointer);
      delete _iconPointer;
      _iconPointer = NULL;
   }

   if ( iconId && _enabledBitmap )
   {
      CGUIRegion bitmapSize;
      CGUIRegion buttonSize;

      bitmapSize.height = iconId->getHeight();
      buttonSize = _enabledBitmap->getRegion();

      if ( (x == -1 ) && (y == -1) )
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

/**
 * ENABLE ICON
 * Set a previously set icon as visible on the button *
 */
void CGUIButton::enableIcon ()
{
   if ( _iconPointer )
   {
      _iconPointer->setVisible(true);
      moveObjectToFront(_iconPointer);
   }
}


/**
 * DISABLE ICON
 * Set a previously set icon as invisible *
 */
void CGUIButton::disableIcon ()
{
   if ( _iconPointer )
   {
      _iconPointer->setVisible(false);
   }
}

/**
 * The following methods are called when state of the button is changed.  These can be overriden
 * and supllemented by derived types if different behavior is needed.
 * DO ON PRESS
 * actions performed by the button when it is pressed.  Actions are:
 *  log button press event to data log if provided
 *        verify button is enabled (disabled means the following actions are skipped)
 *        generate audio feedback associated with button press, if any
 *        perform callback associated with button press, if any
 *  send event message associated with button press (=ButtonPress), if any
 *        display pressed bitmap if provided (otherwise leave the current bitmap up)
 *  display text in "pressed" textstyle if provided
 *
 */
void CGUIButton::doOnPress()
{
   if ( ENABLED == _enabled )
   {
      _pressed = true;

      updateButtonBitmap();
      // call helper operation to display the correct icon for this state
      updateButtonIcon();
      // call helper to display the correct text for this state
      updateButtonText();

      if ( _enabledBitmap ) invalidateObjectRegion(_enabledBitmap);

      if ( _audioMessagePointer ) _audioMessagePointer->send();

      DataLogReserved( guiButtonMessage.name, log_level_cgui_button_press_info )
      << taggedItem( guiButtonMessage.buttonName, _buttonPressLogText )
      << endmsg;
   }
}

/**
 * actions performed when the button is released.  Actions are:
 *        perform callback associated with button release, if any
 *        send event message associated with button release, if any
 *        display enabled bitmap
 *        display text in "released" textstyle if provided.
 *
 */
void CGUIButton::doOnRelease()
{
   if ( ENABLED == _enabled )
   {
      _pressed = false;

      updateButtonBitmap();

      // call helper operation to display the correct icon for this state
      updateButtonIcon();

      // call helper to display the correct text for this state
      updateButtonText();

      if ( _enabledBitmap ) invalidateObjectRegion(_enabledBitmap);

      if ( _audioMessagePointer ) _audioMessagePointer->send();
   }

}

/**
 * actions performed when the button is enabled.
 * Actions are:
 *        if the button is invisible, reset to visible internally and with parent
 *  display enabled bitmap
 *  display text in "enabled" text style
 *
 */
void CGUIButton::doOnEnable()
{
   if ( ENABLED == _enabled )
   {
      _pressed = false;

      updateButtonBitmap();

      // call helper operation to display the correct icon for this state
      updateButtonIcon();

      // call helper to display the correct text for the current state
      updateButtonText();

      if ( _enabledBitmap ) invalidateObjectRegion(_enabledBitmap);
   }
}

