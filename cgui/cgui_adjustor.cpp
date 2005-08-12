/*
 * Copyright (c) 2005 by Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: //BCTquad3/home/BCT_Development/vxWorks/Common/cgui/rcs/cgui_adjustor.cpp 1.4 2006/11/01 15:35:19Z cf10242 Exp pn02526 $
 * $Log: cgui_adjustor.cpp $
 * Revision 1.2  2005/08/10 11:55:09  pn02526
 * Revision 1.1  2005/07/25 13:49:28  pn02526
 * Initial revision
 *
 */

#include <vxWorks.h>
#include "cgui_adjustor.h"

CGUIAdjustor::CGUIAdjustor  (CGUIDisplay        & display,                // reference to a cguidisplay object for display context
                             CGUIWindow         * parent,                 // pointer to a parent window
                             AdjustorData       & adjustorData,           // reference to adjustor data for bitmaps, text and behavior
                             const CallbackBase   increaseEventCallback,  // callback for an increase
                             const CallbackBase   decreaseEventCallback,  // callback for a decrease
                             Message<long>      * increaseEventObject,    // (NOT USED) ptr to int message object to output as long as increase button is pressed and held 
                                                                          // should not be NULL if increaseEventCallback is NULL
                             Message<long>      * decreaseEventObject,    // (NOT USED) ptr to int message object to output as long as decrease button is pressed and held 
                                                                          // should not be NULL if decreaseEventCallback is NULL
                             Message<long>      * increaseAudioMessage = NULL, // (NOT USED) ptr to audio message to repeat while increase button is held
                             Message<long>      * decreaseAudioMessage = NULL, // (NOT USED) ptr to audio message to repeat while decrease button is held
                             DataLog_Level      * adjustorLevel = NULL,   // datalog level object used to log button press events
                             bool                 enabled = true,         // buttons will be constructed as enabled unless otherwise specified here
                             bool                 visible = true          // adjustor will be constructed as visbile unless otherwise specified here
                             )
:CGUIWindow(display)
, _adjustorData(adjustorData)
, _enabled(enabled)
, _direction(NOCHANGE)
, _increaseCallback(increaseEventCallback)
, _decreaseCallback(decreaseEventCallback)
, _adjustorLevel(adjustorLevel)
{

    // Initialize.
    setRegion(CGUIRegion(_adjustorData.left, _adjustorData.top, 0, 0));
    // extent is yet to be determined.

   populateAdjustor(); // Populate the button and textbox data structs.
   layoutAdjustor();   // Layout the adjustor per the data structs (and set the region extent).

   WIN_ATTRIB winAttrib = WIN_ATTRIB_NO_INPUT|WIN_ATTRIB_VISIBLE;
   assert(parent);
   attach(parent, winAttrib);
}

CGUIAdjustor::~CGUIAdjustor(void)
{
    delete _titleTextBox;
    delete _valueTextBox;
    delete _increaseButton;
    delete _decreaseButton;
}

//
// Initializers
//
void CGUIAdjustor::initializeTextBoxes(void)
{
//    if( _titleTextBox ) _titleTextBox->setVisible(true);
//    if( _valueTextBox ) (_valueTextBox);
}

void CGUIAdjustor::initializeButtons(void)
{
    if( _increaseButton ) _increaseButton->enable();
    if( _decreaseButton ) _decreaseButton->enable();
}

void CGUIAdjustor::initializeAdjustor(void)
{
   initializeTextBoxes();
   initializeButtons();
}

//
// Removers
//
void CGUIAdjustor::removeTextBoxes(void)
{
    if (_titleTextBox) _titleTextBox->detach();
    if (_valueTextBox) _valueTextBox->detach();
}

void CGUIAdjustor::detachButtons(void)
{
    if (_increaseButton) _increaseButton->detach();
    if (_decreaseButton) _decreaseButton->detach();
}

//
// Remove adjustor from screen.
//
void CGUIAdjustor::removeAdjustor(void)
{
   removeTextBoxes();
   detachButtons();
}

//
//  Enable/disable the adjustor's buttons.
//
void CGUIAdjustor::enable(void)
{
    _enabled = true;
    if (_increaseButton) _increaseButton->enable();
    if (_decreaseButton) _decreaseButton->enable();
}

void CGUIAdjustor::disable(void)
{
    if (_enabled)
    {
        _enabled = false;
        if (_increaseButton) _increaseButton->disable();
        if (_decreaseButton) _decreaseButton->disable();
    }
}

//
// Button callbacks
//
void CGUIAdjustor::increasePressed(void)
{
//    DataLog(log_level_cgui_info) << "CGUIAdjustor: increase pressed & _enabled=" << _enabled << " & _direction=" << (_direction==NOCHANGE ? "NOCHANGE" : _direction==INCREASING ? "INCREASING" : _direction==DECREASING ? "DECREASING" : "UNKNOWN") << endmsg;
    if( _enabled && ( _adjustorData.behavior == Continuous  || (_adjustorData.behavior == Stepwise && _direction != INCREASING ) ) )
    {
        // It is critical that the callback preceed changing _direction,
        // since the callback can query _direction (via  the increaseHeld() method)
        // to determine that the increase button is not being held.
        _increaseCallback();
        _direction = INCREASING;
    }
}

void CGUIAdjustor::increaseReleased(void)
{
//    DataLog(log_level_cgui_info) << "CGUIAdjustor: increase released & _enabled=" << _enabled << " & _direction=" << (_direction==NOCHANGE ? "NOCHANGE" : _direction==INCREASING ? "INCREASING" : _direction==DECREASING ? "DECREASING" : "UNKNOWN") << endmsg;
    if( _enabled && _direction == INCREASING ) _direction = NOCHANGE;
}

void CGUIAdjustor::decreasePressed(void)
{
//    DataLog(log_level_cgui_info) << "CGUIAdjustor: decrease pressed & _enabled=" << _enabled << " & _direction=" << (_direction==NOCHANGE ? "NOCHANGE" : _direction==INCREASING ? "INCREASING" : _direction==DECREASING ? "DECREASING" : "UNKNOWN") << endmsg;
    if( _enabled && ( _adjustorData.behavior == Continuous  || (_adjustorData.behavior == Stepwise && _direction != DECREASING ) ) )
    {
        // It is critical that the callback preceed changing _direction,
        // since the callback can query _direction (via  the decreaseHeld() method)
        // to determine that the decrease button is not being held.
        _decreaseCallback();
        _direction = DECREASING;
    }
}

void CGUIAdjustor::decreaseReleased(void)
{
//    DataLog(log_level_cgui_info) << "CGUIAdjustor: decrease released & _enabled=" << _enabled << " & _direction=" << (_direction==NOCHANGE ? "NOCHANGE" : _direction==INCREASING ? "INCREASING" : _direction==DECREASING ? "DECREASING" : "UNKNOWN") << endmsg;
    if( _enabled && _direction == DECREASING ) _direction = NOCHANGE;
}

//
// Statemachine callbacks
//

//void CGUIAdjustor::displayValue( const float value )
//{
    // Update the displayed value.
//    if( _valueTextBox )
//    {
//        if( _valueFormattingRoutine )
//        {
//            _valueTextBox->setText( _valueFormattingRoutine( value ) );
//        }
//        else
//        {
//            char valueString[512];
//            sprintf( valueString, "%f", value );
//            _valueTextBox->setText( valueString );
//        }
//    }
//}
//
//void CGUIAdjustor::updateValue( const float value )
//{
//    if( _valueInitialized )
//    {
////        if( _awaiting1stChange )  // Awaiting first change after a button press?
////        {
////            displayValue(value);  // Just display the value.
////            _awaiting1stChange = false;
////        }
////        else
//            if( value != _value )
//        {
//            displayValue( value );
//            switch ( _direction )
//            {
//                case INCREASING:   // Increase still pressed?
//                {
                    // Send another _increaseMessage;
                    // _increaseMessage->send();
//                    if( _increaseCallback ) _increaseCallback();
                    //if( !_decreaseButton->isEnabled() ) _decreaseButton->enable();
//                    break;
//                }
//                case DECREASING: //Decrease still pressed?
//                {
                    // Send another _decreaseMessage;
                    // _decreaseMessage->send();
//                    if( _decreaseCallback ) _decreaseCallback();
                    //if( !_increaseButton->isEnabled() ) _increaseButton->enable();
//                    break;
//                }
//                case NOCHANGE:  // Neither pressed.
//                {
//                    break;
//                }
                // There is no default clause because _direction is an enum, and all possible
                // values should be covered in case statements, above.
//            }
//        }
////        else  // No value change from previous sample may indicate we've reached a limit
////        {
////            switch ( _direction )
////            {
////                case INCREASING:
////                {
////                    // disable _increaseButton;
////                    if( _increaseButton ) _increaseButton->disable();
////                    break;
////                }
////                case DECREASING:
////                {
////                    // disable _decreaseButton;
////                    if( _decreaseButton ) _decreaseButton->disable();
////                    break;
////                }
////                case NOCHANGE:
////                {
////                    break;
////                }
////                // There is no default clause because _direction is an enum, and all possible
////                // values should be covered in case statements, above.
////            }
////        }
//    }
//    else // _value has not received an initial value, yet, so just display it.
//    {
//        _valueInitialized = true;
//        displayValue( value );
//    }
    // Store current value.
//    _value = value;
//}

void CGUIAdjustor::updateValue( char * valueString )
{
    _valueTextBox->setText( valueString );
    if( _adjustorData.behavior == Continuous ) switch ( _direction )
    {
            case INCREASING:   // Increase still pressed?
            {
                // Send another _increaseCallback;
                // Because _direction = INCREASING, when it queries via the increaseHeld() method
                // the callback will see that the increase button is being held by the operator.
                _increaseCallback();
                break;
            }
            case DECREASING: //Decrease still pressed?
            {
                // Send another _decreaseCallback;
                // Because _direction = DECREASING, when it queries via the decreaseHeld() method
                // the callback will see that the decrease button is being held by the operator.
                _decreaseCallback();
                break;
            }
            case NOCHANGE:  // Neither pressed.
            {
                break;
            }
          // There is no default clause because _direction is an enum, and all possible
          // values should be covered in case statements, above.
    }
}

//
// Structure populators
//

// Populate ButtonData structs for increase and decrease buttons
void CGUIAdjustor::populateButtons()
{
    // This code counts on the button bitmap(s) containing the appropriate graphics
    // to imply "increase" and "decrease", without the necessity of text or a
    // separate icon.

    CGUIRegion region;
    getRegion( region );

    // INCREASE BUTTON
    _increaseData.left = _adjustorData.increaseHMargin;
    _increaseData.top = _adjustorData.increaseVMargin;
    _increaseData.type = CGUIButton::RaiseAfterRelease;

    _increaseData.vMargin = 0;       // SEE ABOVE; THERE IS NO TEXT ON THE INCREASE BUTTON
    _increaseData.hMargin = 0;
    _increaseData.enabledTextItem = NULL;
    _increaseData.enabledStylingRecord = NULL;
    _increaseData.disabledTextItem = NULL;
    _increaseData.disabledStylingRecord = NULL;
    _increaseData.pressedTextItem = NULL;
    _increaseData.pressedStylingRecord = NULL;

    _increaseData.enabledBitmapId = _adjustorData.increaseEnabledBitmapId;
    _increaseData.disabledBitmapId = _adjustorData.increaseDisabledBitmapId;
    _increaseData.pressedBitmapId = _adjustorData.increasePressedBitmapId;

    // DECREASE BUTTON
    _decreaseData.left = _adjustorData.decreaseHMargin;
    _decreaseData.top = _adjustorData.decreaseVMargin;
    _decreaseData.type = CGUIButton::RaiseAfterRelease;

    _decreaseData.vMargin = 0;       // SEE ABOVE; THERE IS NO TEXT ON THE DECREASE BUTTON
    _decreaseData.hMargin = 0;
    _decreaseData.enabledTextItem = NULL;
    _decreaseData.enabledStylingRecord = NULL;
    _decreaseData.disabledTextItem = NULL;
    _decreaseData.disabledStylingRecord = NULL;
    _decreaseData.pressedTextItem = NULL;
    _decreaseData.pressedStylingRecord = NULL;

    _decreaseData.enabledBitmapId = _adjustorData.decreaseEnabledBitmapId;
    _decreaseData.disabledBitmapId = _adjustorData.decreaseDisabledBitmapId;
    _decreaseData.pressedBitmapId = _adjustorData.decreasePressedBitmapId;
}

void CGUIAdjustor::populateAdjustor()
{
    populateTextBoxes();
    populateButtons();
}

// Populate TextBoxData structs for title and value text boxes.
//   Most of this is already done by copying the data in the constructor.
//   The only thing left is to set the absolute location of the text boxes.
void CGUIAdjustor::populateTextBoxes()
{
    CGUIRegion region;
    getRegion( region );

    //
    // TITLE TEXT BOX
    //
    _adjustorData.titleData.left = _adjustorData.titleHMargin; // position of top/left corner of title text box in screen pixels
    _adjustorData.titleData.top = _adjustorData.titleVMargin;
    
    //
    // VALUE TEXT BOX
    //
    _adjustorData.valueData.left = _adjustorData.valueHMargin; // position of top/left corner of value text box in screen pixels
    _adjustorData.valueData.top = _adjustorData.valueVMargin;
}

//
// Layout routines - create buttons & textboxes, set adjustor's extent, and render everything.
//

void CGUIAdjustor::layoutButtons()
{
    unsigned short width, height;
    CGUIRegion region;
    getRegion( region );

    // INCREASE BUTTON
   _increaseButton = new CGUIButton(_display, this, _increaseData, NULL, NULL, _adjustorLevel, _enabled );
   width = _adjustorData.increaseHMargin + _increaseButton->width();
   height = _adjustorData.increaseVMargin + _increaseButton->height();
   if( region.width <  width ) region.width = width;
   if( region.height < height ) region.height = height;
   // Set increase callbacks.
   _increaseButton->setPressedCallback(Callback<CGUIAdjustor>(this, &CGUIAdjustor::increasePressed));
   _increaseButton->setReleasedCallback(Callback<CGUIAdjustor>(this, &CGUIAdjustor::increaseReleased));

   // DECREASE BUTTON
   _decreaseButton = new CGUIButton(_display, this, _decreaseData, NULL, NULL, _adjustorLevel, _enabled );
   width = _adjustorData.decreaseHMargin + _decreaseButton->width();
   height = _adjustorData.decreaseVMargin + _decreaseButton->height();
   if( region.width <  width ) region.width = width;
   if( region.height < height ) region.height = height;
   // Set decrease callbacks.
   _decreaseButton->setPressedCallback(Callback<CGUIAdjustor>(this, &CGUIAdjustor::decreasePressed));
   _decreaseButton->setReleasedCallback(Callback<CGUIAdjustor>(this, &CGUIAdjustor::decreaseReleased));

   setRegion(region);
}

void CGUIAdjustor::layoutTextBoxes()
{
    unsigned short width, height;
    CGUIRegion region;
    getRegion( region );

    // initialize the title textbox
    _titleTextBox = new CGUITextBox(_display, this, _adjustorData.titleData );
    width = _adjustorData.titleHMargin + _titleTextBox->width();
    height = _adjustorData.titleVMargin + _titleTextBox->height();
    if( region.width < width ) region.width = width;
    if( region.height < height ) region.height = height;

    // initialize the value textbox
    _valueTextBox = new CGUITextBox(_display, this, _adjustorData.valueData );
    width = _adjustorData.valueHMargin + _valueTextBox->width();
    height = _adjustorData.valueVMargin + _valueTextBox->height();
    if( region.width < width ) region.width = width;
    if( region.height < height ) region.height = height;

    setRegion(region);
}

void CGUIAdjustor::layoutAdjustor()
{
    layoutTextBoxes();
    layoutButtons();
}
