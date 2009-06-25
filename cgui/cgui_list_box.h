/*
 *  Copyright(c) 2006 by Gambro BCT, Inc. All rights reserved.
 *
 * $Header: K:/BCT_Development/vxWorks/Common/cgui/rcs/cgui_list_box.h 1.4 2007/01/17 14:31:23Z rm10919 Exp wms10235 $
 *
 * list_box.h
 *
 * $Log: cgui_list_box.h $
 * Revision 1.4  2007/01/17 14:31:23Z  rm10919
 * Update to use a bitmap background.
 * Revision 1.3  2006/08/10 22:18:57Z  rm10919
 * Make border around list box if using a background.
 * Revision 1.2  2006/07/26 23:30:47Z  rm10919
 * Add virtual attach to override CGUIWindow::attach().
 * Revision 1.1  2006/07/12 23:36:44Z  rm10919
 * Initial revision
 *
 *
 */

#ifndef _CGUI_LIST_BOX_
#define _CGUI_LIST_BOX_

#include "cgui_bitmap.h"
#include "cgui_button.h"
#include "cgui_graphics.h"
#include "cgui_rectangle.h"
#include "cgui_text.h"
#include "cgui_text_item.h"
#include "cgui_time_date.h"
#include "cgui_window.h"

// If the MAX_NUMBER_LIST_BUTTONS changes
//   then the number of callback methods for
//    list buttons will need to be modified!!!
//  This means there are six buttons (0,1,2,3,4,5)
#define  MAX_NUMBER_LIST_BUTTONS  6

class CGUIListBox : public CGUIWindow
{
public:
   CGUIListBox(CGUIDisplay & display, CGUIWindow * parent, const CallbackBase & callback, list<CGUITextItem*> buttonNames, CGUIButton::ButtonData * listButtonData, CGUIButton::ButtonData * upButtonData, CGUIButton::ButtonData * downButtonData, int numberOfButtons, bool haveBackground = false, CGUIColor backgroundColor = 0, int buttonSpacing = 3);
   CGUIListBox(CGUIDisplay & display, CGUIWindow * parent, const CallbackBase & callback, list<CGUITextItem*> buttonNames, CGUIButton::ButtonData * listButtonData, CGUIButton::ButtonData * upButtonData, CGUIButton::ButtonData * downButtonData, int numberOfButtons, CGUIBitmapInfo * backgroundBitmapId, int buttonSpacing = 3);

   ~CGUIListBox();

   // Button Callbacks
   void upButtonCallback();
   void downButtonCallback();
   void listBoxButtonCallback(int listBoxButtonIndex);
   CallbackBase _callback;

   // returns text item selected
   CGUITextItem * returnTextItem(void); //{return _returnTextItem;} // returns text item selected

   // override cguiwindow attach
   virtual void attach(CGUIWindow * window, WIN_ATTRIB winAttrib = WIN_ATTRIB_VISIBLE);
protected:
   CGUIWindow * _parent;

private:
   void initializeData(CGUIDisplay & display, CGUIWindow * parent, const CallbackBase & callback, list<CGUITextItem*> buttonNames, CGUIButton::ButtonData * listButtonData, CGUIButton::ButtonData * upButtonData, CGUIButton::ButtonData * downButtonData, int numberOfButtons, bool haveBackground = false);

   CGUITextItem * _returnTextItem;

   void listButton0Callback(void);
   void listButton1Callback(void);
   void listButton2Callback(void);
   void listButton3Callback(void);
   void listButton4Callback(void);
   void listButton5Callback(void);

   bool _haveBackground;
   bool _haveArrowButtons;

   int _numberOfButtons;
   int _listSize;
   int _buttonSpacing;

   // Will always point to listButton[0] place in list.
   list<CGUITextItem*>::iterator _indexTopButton;
   list<CGUITextItem*> _buttonNames;

   CGUIBitmapInfo * _backgroundBitmapId;
   CGUIBitmap * _backgroundBitmap;

   CGUIRectangle *_listBoxBackground;

   CGUIButton * _listButton[MAX_NUMBER_LIST_BUTTONS];

   // List postion of parent window based on CGUIRegion passed in
   //  for _upArrowButtonData.  If upArrowButtonData is null then
   //  placement of list window is based on CGUIRegion in listButtonData.
   CGUIButton * _upArrowButton;
   CGUIButton * _downArrowButton;

   void updateButtons();

private:
   CGUIListBox(void);
   CGUIListBox(const CGUIListBox & copy);
   CGUIListBox operator=(const CGUIListBox &obj);
}; // END of CGUIListBox Class Declaration

#endif  //_CGUI_LIST_BOX_

