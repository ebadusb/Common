/*
 *        Copyright (c) 2004 by Gambro BCT, Inc.  All rights reserved.
 *
 *  $Header$
 *  This file defines the base class for all button styles in the common GUI.  An object of this class types
 *  can be used to generate a standard button.
 */
#ifndef _CGUI_BUTTON_INCLUDE
#define _CGUI_BUTTON_INCLUDE

#include <vector>

#include "message.h"
#include "datalog_levels.h"
#include "cgui_window.h"
#include "cgui_text.h"
#include "cgui_bitmap.h"

#define MAX_BUTTON_LOG_SIZE  64

/**
    @class CGUIButton is the parent class for all button
          styles.Specialized buttons can derive from this class
          and pick up many of the standard behaviors of a
          button, but override some features for specialized
          behavior. The public methods of the class are grouped
          below into categories based on the things they affect.
          The groups are: messages/callbacks, bitmaps, audio,
          text, and icons. There are some basic rules in this
          class that are important to its usage:
          1) all buttons are bitmaps,
          2) a button has one of the following states: invisible, enabled, disabled, pressed, released,
          3) there is one text string associated with the button (although the styling can be different by state),
          4) there is one icon associated with the button,
          5) there is one message associated with the button press,
          6) there is one message associated with the button release,
          7) there is one callback routine associated with a button press,
          8) there is one callback routine associated with a button release
*/
class CGUIButton : public CGUIWindow {
public:

   static const bool PRESSED;
   static const bool RELEASED;
   static const bool ENABLED;
   static const bool DISABLED;

   enum ButtonBehavior
   {
      RaiseAfterRelease,         // button switches back to up bitmap on touch release, then executes callback
      RaiseAfterCallback,        // callback is executed on touch release, then utton switches back to up bitmap
      Manual                     // callback is executed on touch release, button stays down (setUp must be used to raise button again)
   };

   struct ButtonIcon
   {
      ButtonIcon( CGUIDisplay & display, CGUIBitmapInfo * iconBitmapInfo, const bool pressed, const bool enabled, const short x, const short y )
      : _iconId( 0 ),
         _pressed( pressed ),
         _enabled( enabled ),
         _iconBitmapInfo( iconBitmapInfo ),
         _width( x ),
         _height( y )
      {
         setIconId();
         _iconBitmap = new CGUIBitmap( display, CGUIRegion( x, y, 0, 0 ), *iconBitmapInfo);
      };

      ~ButtonIcon(void)
      {
         if ( _iconBitmap )
         {
            delete _iconBitmap;
            _iconBitmap = NULL;
         }
      }

      int getIconId( void )
      {
         return _iconId;
      };

      CGUIBitmap * getBitmap( void )
      {
         return _iconBitmap;
      };

      CGUIBitmapInfo * getBitmapInfo()
      {
         return _iconBitmapInfo;
      }

      void setVisible( bool newVisible )
      {
         _iconBitmap->setVisible( newVisible );
      };

      bool enabled(){ return _enabled; }
      bool pressed(){ return _pressed; }
      void setEnabled( bool enabled ){ _enabled = enabled; }
      void setPressed( bool pressed){ _pressed = pressed; }
      short width(){ return _width; }
      short height(){ return _height; }

   private:
      /// Copy Constructor
      ButtonIcon( const ButtonIcon&);

      /// Copy Assignment Operator
      ButtonIcon& operator=( const ButtonIcon&);

      bool _pressed;
      bool _enabled;
      short _width;
      short _height;
      CGUIBitmap * _iconBitmap;
      CGUIBitmapInfo* _iconBitmapInfo;

      int _iconId;

      static int _iconCounter;

      int setIconId( void )
      {
         if ( _iconId < 1 ) _iconId = _iconCounter++;return _iconId;
      };
   }; //  End ButtonIcon struct

   struct ButtonData
   {
      unsigned short left;          // position of top/left corner of button in screen pixels
      unsigned short top;

      unsigned short vMargin;    // vertical and horizontal margins from edge of button to
      unsigned short hMargin;    // where any button labels are allowed to start

      CGUIBitmapInfo * enabledBitmapId;      // enabled state bitmap id
      CGUITextItem * enabledTextItem;       // label text (if any) used in enabled state
      StylingRecord * enabledStylingRecord;  // label text styling information in enabled state
      ButtonIcon * enabledButtonIcon;        // background icon associated with the enabled state

      CGUIBitmapInfo * disabledBitmapId;     // disableded state bitmap id
      CGUITextItem * disabledTextItem;       // attributes for label text used in disabled state
      StylingRecord * disabledStylingRecord; // label text styling information in disableded state
      ButtonIcon * disabledButtonIcon;       // background icon associated with the disabled state

      CGUIBitmapInfo * pressedBitmapId;      // pressed state bitmap id
      CGUITextItem * pressedTextItem;        // label text used in pressed state
      StylingRecord * pressedStylingRecord;  // label text styling information in pressed state
      ButtonIcon * pressedButtonIcon;        // background icon associated with the pressed state

      CGUIBitmapInfo * pressedDisabledBitmapId;       // pressed state bitmap id
      CGUITextItem * pressedDisabledTextItem;         // label text used in pressed state
      StylingRecord * pressedDisabledStylingRecord;   // label text styling information in pressed state
      ButtonIcon * pressedDisabledButtonIcon;         // background icon associated with the pressed state


      char alternateButtonId[MAX_BUTTON_LOG_SIZE+1];  // enabled text item ID is used.  If none, then this field is used

      ButtonBehavior type;                                                        // button behavior

      ButtonData(void)
      : left( 0 ),
         top( 0 ),
         vMargin( 0 ),
         hMargin( 0 ),
         enabledBitmapId( NULL ),
         enabledTextItem( NULL ),
         enabledStylingRecord( NULL ),
         enabledButtonIcon( NULL ),
         disabledBitmapId( NULL ),
         disabledTextItem( NULL ),
         disabledStylingRecord( NULL ),
         disabledButtonIcon( NULL ),
         pressedBitmapId( NULL ),
         pressedTextItem( NULL ),
         pressedStylingRecord( NULL ),
         pressedButtonIcon(NULL ),
         pressedDisabledBitmapId( NULL ),
         pressedDisabledTextItem( NULL ),
         pressedDisabledStylingRecord( NULL ),
         pressedDisabledButtonIcon( NULL ),
         alternateButtonId(),
         type( RaiseAfterRelease )
      {

      };
   };

// PUBLIC METHODS //
public:
   /// CONSTRUCTOR
   CGUIButton( CGUIDisplay   & display,                 // reference to a cguidisplay object for display context
      CGUIWindow    * parent,                  // pointer to a parent window
      ButtonData    & buttonData,              // reference to button data for bitmaps, text and behavoir
      Message<long> * pressEventObject = NULL, // ptr to int message object to output when button is pressed and released
                                                                  // can be NULL to indicate no message is output
      Message<long> * audioMsg = NULL,         // ptr to audio message to send when button is pressed
      DataLog_Level * buttonLevel = NULL,      // datalog level object used to log button press events
      bool enabled = true,          // button will be constructed as disabled unless specified here
      bool visible = true,          // button will be constructed as visbile unless otherwise specified here
      bool  pressed = false);

   // DESTRUCTOR
   virtual ~CGUIButton ();

   // ENABLE
   /// set the state of the button to enabled.  If currently invisible, the button is made visible
   void enable(void);
   /// enable or disable
   void enable(const bool beEnabled);

   /// ENABLEWHENPRESSED
   /// set the state of the button to enabled with enable pressed bitmap on top.  If currently invisible, the button is made visible.
   void enableWhenPressed(void);
   /// set a disabled or pressed button to the enabled released state
   void enableReleased( void );

   // ENABLEIFRELEASED
   /// Only enable the button if it has been released
   void enableIfReleased(void);

  /// ENABLEPRESSED
   void enablePressed(void);

   // DISABLE
   /// set the state of the button to disabled.  If currently invisible, the button is made visible
   void disable();

   /// Set the button to the disabled pressed state
   void disablePressed();

   /// set the button to the disabled released (also known as disabled state )
   void disableReleased();

   // IS ENABLED
   // returns true if the button is enabled
   bool isEnabled() const
   {
      return _enabled;
   };

   // IS PRESSED
   // returns true if the button is pressed
   bool isPressed() const
   {
      return _pressed;
   };

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

   /// Set the bitmap for the given buttons state
   void setBitmap( CGUIBitmapInfo *bitmapInfo, const bool pressed, const bool enabled );
   /// Set the bitmap for all button states
   void setBitmap( CGUIBitmapInfo *bitmapInfo );

   //SET ENABLED BITMAP
   // loads a new enabled bitmap object for the button.
   /// Deprecated - use void setBitmap( CGUIBitmapInfo *bitmapInfo, const bool pressed, const bool enabled );
   void setEnabledBitmap(CGUIBitmapInfo *enabledBitmapId);

   //SET DISABLED BITMAP
   // loads a new disabled bitmap object for the button.
   /// Deprecated - use void setBitmap( CGUIBitmapInfo *bitmapInfo, const bool pressed, const bool enabled );
   void setDisabledBitmap(CGUIBitmapInfo *disabledBitmapId);

   // SET PRESSED BITMAP
   // loads a new pressed bitmap object for the button
   /// Deprecated - use void setBitmap( CGUIBitmapInfo *bitmapInfo, const bool pressed, const bool enabled );
   void setPressedBitmap(CGUIBitmapInfo *pressedBitmapId);

   // SET AUDIO
   // set the audio associated with a button press.
   void setAudio(Message<long> *audioObject);

   // TEXT METHODS //

   // SET TEXT
   /// set the text associated with the button.
   void setText(CGUITextItem * textItem); // ptr to a text object associated with the button
   void setText(const char * string); // ptr to a text object associated with the button
   void setText(const StringChar * string); // ptr to a text object associated with the button
   void setText();  // forces a redraw or update to button text.

   /// Set the text associated with a specified state
   void setText(CGUITextItem *, const bool pressed, const bool enabled );
   void setText(const char * string, const bool pressed, const bool enabled );
   void setText(const StringChar * string, const bool pressed, const bool enabled);
   /// Set the text for the specified state
   void setText( const bool pressed, const bool enabled );

   /// Deprecated - use setText(CGUITextItem, const bool pressed, const bool enabled )
   void setEnabledText(CGUITextItem * textItem);

   /// Deprecated - use setText(CGUITextItem, const bool pressed, const bool enabled )
   void setEnabledText(const char * string = NULL);

   /// Deprecated - use setText(CGUITextItem, const bool pressed, const bool enabled )
   void setEnabledText(const StringChar * string = NULL);

   /// Deprecated - use setText(CGUITextItem, const bool pressed, const bool enabled )
   void setEnabledText();

   /// Deprecated - use setText(CGUITextItem, const bool pressed, const bool enabled )
   void setDisabledText(CGUITextItem * textItem = NULL);

   /// Deprecated - use setText(CGUITextItem, const bool pressed, const bool enabled )
   void setDisabledText(const char * string = NULL);

   /// Deprecated - use setText(CGUITextItem, const bool pressed, const bool enabled )
   void setDisabledText(const StringChar * string = NULL);

   /// Deprecated - use setText(CGUITextItem, const bool pressed, const bool enabled )
   void setDisabledText();

   /// Deprecated - use setText(CGUITextItem, const bool pressed, const bool enabled )
   void setPressedText(CGUITextItem * textItem = NULL);

   /// Deprecated - use setText(CGUITextItem, const bool pressed, const bool enabled )
   void setPressedText(const char * string = NULL);

   /// Deprecated - use setText(CGUITextItem, const bool pressed, const bool enabled )
   void setPressedText(const StringChar * string = NULL);

   /// Deprecated - use setText(const bool pressed, const bool enabled )
   void setPressedText();

   /// SET TEXTSTYLE
   /// set/change the style of the text associated with this button in ALL states.  This is a pass-thru to the
   /// text object previously set with this button.
   void setStylingRecord(StylingRecord * textStylingRecord); // style record with appropriate features set

   /// set the text styling record for the specified state
   void setStylingRecord( StylingRecord * textStylingRecord, const bool pressed, const bool enabled );

   // SET ENABLED TEXTSTYLE - set/change the style of text associated with this button
   /// deprecated - use setStylingRecord( StylingRecord * textStylingRecord, bool pressed, bool enabled ) instead
   void setEnabledStylingRecord (StylingRecord * enabledTextStylingRecord); // style record with appropriate features set

   // SET DISABLED TEXTSTYLE - set/change the style of text associated with this button
   /// deprecated - use setStylingRecord( StylingRecord * textStylingRecord, bool pressed, bool enabled ) instead
   void setDisabledStylingRecord (StylingRecord * disabledTextStylingRecord);

   // SET PRESSED TEXTSTYLE - set/change the style of text associated with this button
   /// deprecated - use setStylingRecord( StylingRecord * textStylingRecord, bool pressed, bool enabled ) instead
   void setPressedStylingRecord (StylingRecord * pressedTextStylingRecord);

   /// Deprecated - use setTextColor( CGUIColor, bool pressed, bool enabled )
   void setEnabledTextColor(CGUIColor color);

   /// Deprecated - use setTextColor( CGUIColor, bool pressed, bool enabled )
   void setDisabledTextColor(CGUIColor color);

   /// Deprecated - use setTextColor( CGUIColor, bool pressed, bool enabled )
   void setPressedTextColor(CGUIColor color);

   /// Set the text color for all states
   void setTextColor(CGUIColor color);
   /// Set the Text Color for the specified state
   void setTextColor( CGUIColor, bool pressed, bool enabled );

   // ICON METHODS //

   /// SET ICON
   /// set an icon to be associated with the button.
   void setIcon(CGUIBitmapInfo *iconId,            // ptr to bitmap object for icon
                const short x=-1,             // upper left X coordinate to start icon relative to button location (0 means coincident with x coordinate of upper left position of button)
                const short y=-1,             // upper left Y coordinate to start icon relative to button location
                bool visible=true             // visibility of icon upon creation, uses CGUIBitmap class to set visibility
               );

   /// ENABLE ICON
   /// set a previously set icon as visible on the button
   void enableIcon();

   /// DISABLE ICON
   /// set a previously set icon as invisible
   void disableIcon();

   int addIcon( CGUIBitmapInfo * bitmapInfo, const short x, const short y, const bool pressed, const bool enabled );

   /// change the button state type of an icon in the list
   bool setIconState( int iconId, const bool pressed, const bool enabled );

   /// remove icon from icon list
   bool removeIcon(const int iconId );

// PUBLIC DATA //

// PROTECTED METHODS
protected:
   ///
   /// The following methods are called when state of the button is changed.  These can be overriden
   /// and supllemented by derived types if different behavior is needed.
   /// DO ON PRESS
   /// actions performed by the button when it is pressed.  Actions are:
   /// log button press event to data log if provided
   /// verify button is enabled (disabled means the following actions are skipped)
   /// generate audio feedback associated with button press, if any
   /// perform callback associated with button press, if any
   /// send event message associated with button press (=ButtonPress), if any
   /// display pressed bitmap if provided (otherwise leave the current bitmap up)
   /// display text in "pressed" textstyle if provided
   ///
   virtual void doOnPress();

   ///
   /// DO ON RELEASE
   /// actions performed when the button is released.  Actions are:
   /// perform callback associated with button release, if any
   /// send event message associated with button release, if any
   /// display enabled bitmap
   /// display text in "released" textstyle if provided
   ///
   virtual void doOnRelease();

   ///
   /// DO ON ENABLE
   /// actions performed when the button is enabled.
   /// Actions are:
   ///     if the button is invisible, reset to visible internally and with parent
   ///   display enabled bitmap
   ///   display text in "enabled" text style
   ///
   virtual void doOnEnable();

  /// Used to create button for derived classes.
   void setCGUIButton( CGUIDisplay & display, CGUIWindow * parent, ButtonData & buttonData,
                       Message<long> * pressEventObject = NULL, // ptr to int message object to output when button is pressed and released
                                                                                      // can be NULL to indicate no message is output
                       Message<long> * audioMessage = NULL,             // ptr to audio message to send when button is pressed
                       DataLog_Level * buttonLevel = NULL,              // datalog level object used to log button press events
                       const bool enabled = true,                          // button will be enabled unless specified here
                       const bool visible = true,                          // button will be visbile unless otherwise specified here
                       const bool pressed = false );

   /// common code for creating button ( constructor and setCGUIButton Method )
   void initializeButton( CGUIDisplay & display, CGUIWindow * parent, ButtonData & buttonData,
                          Message<long> * pressEventObject = NULL,
                          Message<long> * audioMessage = NULL,
                          DataLog_Level * buttonLevel = NULL,
                          const bool enabled = true,
                          const bool visible = true,
                          const bool                 pressed = false);

   ///Constructor for derived classes
   CGUIButton( CGUIDisplay & display );

   /// This operation can be used to access the CGUIText list after we make them private
   /// Currently some derived classes are accessing them directly
   CGUIText* getText( bool pressed, bool enabled );

// PROTECTED DATA
   Message<long> *_buttonMessagePointer; /// used to communicate a message to other tasks when a button is pressed and released

   /// Text displayed in the enabled/released state, Prefer using vector< StateTextStruct > _textList;
   CGUIText *_enabledText;          /// ptr to current text object
   CGUIBitmap *_enabledBitmap;      /// ptr to enabled bitmap object

   /// Text displayed in the disabled/released state, Prefer using vector< StateTextStruct > _textList;
   CGUIText *_disabledText;         /// ptr to disabled text object
   CGUIBitmap *_disabledBitmap;     /// ptr to disabled bitmap object

   /// Text displayed in the enabled/pressed  state, Prefer using vector< StateTextStruct > _textList;
   CGUIText *_pressedText;          /// ptr to pressed text object
   CGUIBitmap *_pressedBitmap;      /// ptr to pressed bitmap object

   /// Text displayed in the disabled/pressed state, Prefer using vector< StateTextStruct > _textList;
   CGUIText *_pressedDisabledText;
   CGUIBitmap *_pressedDisabledBitmap;

// PRIVATE METHODS
private:
   /// Helper operation used to update the displayed icon to correspond to the current button state
   void  updateButtonIcon();

   /// Helper operation used to display the text corresponding to the current state
   void updateButtonText();

   /// Helper operation used to display the bitmap corresponding to the current state
   void updateButtonBitmap();

   CGUIButton(void);
   /// Do No Use - Copy Constructor
   CGUIButton( const CGUIButton & copy );
   /// Do No Use = Copy Assignment operator
   CGUIButton operator = ( const CGUIButton &object );

// PRIVATE TYPES

   /// Type used to map the text to its corresponding state
   typedef struct StateTextStruct
   {
      CGUIText * textPtr;
      bool         textPressed;
      bool         textEnabled;
      StateTextStruct(){ textPtr = NULL, textPressed = CGUIButton::RELEASED; textEnabled = CGUIButton::ENABLED; };
      StateTextStruct(CGUIText* text, bool pressed, bool enabled){ textPtr = text, textPressed = pressed; textEnabled = enabled; }
   };

// PRIVATE DATA
   CGUIRegion _textRegion;            // area in which text can appear, uses v and h margin
   bool _haveTextRegion;        // flag to use textRegion member
   /// vector the the text associated with each state
   vector< StateTextStruct > _textList;

   Message<long> *_audioMessagePointer;  // ptr to audio message to send when button is pressed
   CallbackBase _CBOnPressed;          // callback object to use when button is pressed
   CallbackBase _CBOnReleased;         // callback object to use when button is released

   bool _pressEventMessageEnabled;   // flag is true if message is sent on press of button
   bool _releasedEventMessageEnabled;// flag is true if message is sent on release of button
   bool _pressedCBEnabled;     // flag is true if callback is made on button press
   bool _releasedCBEnabled;    // flag is true is callbacl is made on button release

   ButtonBehavior _behaviorType;         // how does button behave when pressed

   DataLog_Level *_btnDataLogLevel;      // level at which to log button press events

   char _buttonPressLogText[MAX_BUTTON_LOG_SIZE+1];  // button press logging text
   bool _alternateLogTextUsed;                  // true if alternate log text was specified

   /// vector of transparent icons
   vector< ButtonIcon * > _iconList;

   // Should be removed and instead just use the _iconList icons
   ///. This is an icon that applies to all states and is always place on top of any other icons
   CGUIBitmap *_iconPointer;          // ptr to the icon bitmap object

   /// True if the button is currently pressed
   bool _pressed;
   /// True if the button is currently enabled
   bool _enabled;
};

#endif /* #ifndef _CGUI_BUTTON_INCLUDE */
