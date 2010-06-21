/*
 *	Copyright (c) 2005 by Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: //BCTquad3/home/BCT_Development/vxWorks/Common/cgui/rcs/cgui_adjustor.h 1.4 2009/06/24 18:41:44Z wms10235 Exp $
 * $Log: cgui_adjustor.h $
 * Revision 1.4  2009/06/24 18:41:44Z  wms10235
 * IT6958 - Interrmittent page fault occurs at protocol load or during procedure
 * Revision 1.3  2005/08/11 18:45:54Z  pn02526
 * Fix logic for sensing if a button is held by the operator.
 * Revision 1.2  2005/08/10 12:01:14  pn02526
 * Revision 1.1  2005/07/25 13:29:45  pn02526
 * Initial revision
 *
 */

#ifndef _CGUI_ADJUSTOR
#define _CGUI_ADJUSTOR

#include "datalog_levels.h"
#include "cgui_window.h"
#include "cgui_button.h"
#include "cgui_textbox.h"

class CGUIAdjustor : public CGUIWindow //: public BaseScreen
{
public:
    enum AdjustorBehavior
    {
       Continuous,         // Held button continues executing increase/decrease actions.
       Stepwise            // A button press performs a single increase/decrease action then waits for release.
    };

    enum Direction
    {
        NOCHANGE, INCREASING, DECREASING
    };

//    enum StepBehavior
//    {
//        Linear,            // Execute linear steps.
//        Logarithmic,       // Execute logarithmic steps.
//        ...                // Execute ? steps.
//    };

    struct AdjustorData
    {
       unsigned short left;               // position of top/left corner of adjustor in absolute screen pixels
       unsigned short top;

//       float increaseUpperLimit;
//       float decreaseLowerLimit;
//       float stepValue;

       // TITLE TEXTBOX
       unsigned short titleVMargin;       // position of top/left corner of title textbox in relative screen pixels
       unsigned short titleHMargin;       //
       CGUITextBox::TextBoxData titleData;

       // INCREASE BUTTON
       unsigned short increaseVMargin;       // position of top/left corner of increase button in relative screen pixels
       unsigned short increaseHMargin;
       CGUIBitmapInfo * increaseEnabledBitmapId;  // increase-enabled bitmap id
       CGUIBitmapInfo * increaseDisabledBitmapId; // increase-disabled bitmap id
       CGUIBitmapInfo * increasePressedBitmapId;  // increase-pressed bitmap id

       // VALUE TEXTBOX
       unsigned short valueVMargin;          // position of top/left corner of value textbox in relative screen pixels
       unsigned short valueHMargin;
       CGUITextBox::TextBoxData valueData;

       // DECREASE BUTTON
       unsigned short decreaseVMargin;       // position of top/left corner of decrease button in relative screen pixels
       unsigned short decreaseHMargin;
       CGUIBitmapInfo * decreaseEnabledBitmapId;  // decrease-enabled bitmap id
       CGUIBitmapInfo * decreaseDisabledBitmapId; // decrease-disabled bitmap id
       CGUIBitmapInfo * decreasePressedBitmapId;  // decrease-pressed bitmap id

       AdjustorBehavior  behavior;
    };

public:
    // CONSTRUCTOR
    CGUIAdjustor(CGUIDisplay   & display,                 // reference to a cguidisplay object for display context
                 CGUIWindow    * parent,                  // pointer to a parent window
                 AdjustorData  & adjustorData,            // reference to adjustor data for bitmaps, text and behavoir
                 const CallbackBase increaseEventCallback,   // callback for an increase
                 const CallbackBase decreaseEventCallback,   // callback for a decrease
                 Message<long>      * increaseEventObject,// (NOT USED) ptr to int message object to output as long as increase button is pressed and held
                                                          // should not be NULL if increaseEventCallback is NULL
                 Message<long>      * decreaseEventObject,// (NOT USED) ptr to int message object to output as long as decrease button is pressed and held
                                                          // should not be NULL if decreaseEventCallback is NULL
                 Message<long>      * increaseAudioMessage,    // (NOT USED) ptr to audio message to repeat while increase button is held
                 Message<long>      * decreaseAudioMessage,    // (NOT USED) ptr to audio message to repeat while decrease button is held
                 DataLog_Level * adjustorLevel,      // datalog level object used to log button press events
                 bool            enabled,          // buttons will be constructed as enabled unless otherwise specified here
                 bool            visible          // adjustor will be constructed as visbile unless otherwise specified here
                 );

   // DESTRUCTOR
   virtual ~CGUIAdjustor(void);

   virtual void initializeAdjustor(void);
   virtual void removeAdjustor(void);
   virtual void updateValue( char * );
   virtual void updateValue( const StringChar * );

   // ENABLE
   // set the state of the adjustor's buttons to enabled.
   virtual void enable(void);

   // DISABLE
   // set the state of the adjustor's buttons to disabled.
   virtual void disable();

   // IS ENABLED
   // returns true if the adjustor's buttons are enabled
   virtual inline bool isEnabled() const {return _enabled;};

   // INCREASE HELD
   // returns true if the adjustor's increase button is being held by the operator
   virtual inline bool increaseHeld() const {return _direction == INCREASING;};

   // DECREASE HELD
   // returns true if the adjustor's decrease button is being held by the operator
   virtual inline bool decreaseHeld() const {return _direction == DECREASING;};

   // IS HELD
   // returns true if the adjustor has a button being held by the operator
//   virtual inline bool isHeld() const {return _direction != NOCHANGE;};

//   virtual void updateValue(float);     // Updates displayed value and sends next increase/decrease message and/or callback.

protected:
    bool                     _enabled;              // current enabled/disabled state
   CallbackBase _increaseCallback;      // Callbacks to caller
   CallbackBase _decreaseCallback;      //

   virtual void increasePressed(void);  // For capturing button callbacks
   virtual void increaseReleased(void); // For capturing button callbacks
   virtual void decreasePressed(void);  // For capturing button callbacks
   virtual void decreaseReleased(void); // For capturing button callbacks

   virtual void layoutAdjustor(void);            // Gets the show on the road!

private:
    void initializeTextBoxes(void);
    void initializeButtons(void);
    void removeTextBoxes(void);
    void detachButtons(void);
//    void displayValue( const float value );
//    void updateValue( const float value );
    void populateButtons(void);
    void layoutButtons(void);
    void populateTextBoxes(void);
    void layoutTextBoxes(void);
    void populateAdjustor(void);

private:
   // Adjustor items
   AdjustorData _adjustorData;

   //unsigned short _increaseVMargin;          // position of top/left corner of title textbox in relative screen pixels
   //unsigned short _increaseHMargin;
   CGUIButton::ButtonData _increaseData;
   CGUIButton * _increaseButton;

   //unsigned short _decreaseVMargin;          // position of top/left corner of title textbox in relative screen pixels
   //unsigned short _decreaseHMargin;
   CGUIButton::ButtonData _decreaseData;
   CGUIButton * _decreaseButton;

   //unsigned short _titleVMargin;          // position of top/left corner of title textbox in relative screen pixels
   //unsigned short _titleHMargin;
   //CGUITextBox::TextBoxData _titleData;
   CGUITextBox * _titleTextBox;

   //unsigned short _valueVMargin;          // position of top/left corner of title textbox in relative screen pixels
   //unsigned short _valueHMargin;
  // char * (* _valueFormattingRoutine) ();
   //CGUITextBox::TextBoxData _valueData;
   CGUITextBox * _valueTextBox;

   //Message<long>      *_increaseMessage; // Message to be sent to increase the value.
   //Message<long>      *_decreaseMessage; // Message to be sent to decrease the value.

//   bool _valueIntitialized;              // Flag to indicate that _value has received its initial value via updateValue()
//   float _value;                         // Actual value.  Float used here, but could be int with code changes.

   //AdjustorBehavior  _behavior;

   Direction _direction;                 // Current direction of changing value, enum INCREASING, DECREASING, NOCHANGE.

   DataLog_Level * _adjustorLevel;      // Datalog level object used to log button press events

	CGUIAdjustor(void);
	CGUIAdjustor(const CGUIAdjustor & copy);
	CGUIAdjustor & operator=(const CGUIAdjustor & copy);
};

#endif /* ifndef _CGUI_ADJUSTOR */

