/*
 *  Copyright(c) 2006 by Gambro BCT, Inc. All rights reserved.
 *
 * $Header: K:/BCT_Development/vxWorks/Common/cgui/rcs/cgui_list_box.cpp 1.5 2007/02/08 19:28:04Z rm10919 Exp wms10235 $
 *
 * cgui_list_box.cpp
 *
 * $Log: cgui_list_box.cpp $
 * Revision 1.3  2006/08/10 22:18:56Z  rm10919
 * Make border around list box if using a background.
 * Revision 1.2  2006/07/26 23:30:15Z  rm10919
 * add virtual attach() to override CGUIWindow::attach().
 * Revision 1.1  2006/07/12 23:36:44Z  rm10919
 * Initial revision
 *
 *
 */

#include <vxworks.h>
#include "cgui_list_box.h"
#include "datalog_levels.h"

CGUIListBox::CGUIListBox(CGUIDisplay & display, CGUIWindow * parent, 
                         const CallbackBase & callback,
                         list<CGUITextItem*> buttonNames, 
                         CGUIButton::ButtonData * listButtonData, 
                         CGUIButton::ButtonData * upButtonData, 
                         CGUIButton::ButtonData * downButtonData, 
                         int numberOfButtons,
                         bool haveBackground, 
                         CGUIColor backgroundColor,
                         int buttonSpacing):
                         CGUIWindow(display), _parent(parent), _callback(callback),
                         _numberOfButtons(numberOfButtons), 
                         _listSize(buttonNames.size()), 
                         _buttonSpacing(buttonSpacing),
                         _backgroundBitmapId(NULL),
                         _backgroundBitmap(NULL),
                         _listBoxBackground(NULL),
                         _haveBackground(haveBackground),
                         _buttonNames(buttonNames)
{
   initializeData(display, parent, callback, buttonNames, listButtonData, upButtonData, downButtonData, numberOfButtons, _haveBackground);

   // add background colored rectangle
   if (haveBackground)
   {
      _listBoxBackground = new CGUIRectangle(_display, CGUIRegion(0, 0, this->width(), this->height()), backgroundColor);
      addObjectToBack(_listBoxBackground);
   }
}

CGUIListBox::CGUIListBox(CGUIDisplay & display, CGUIWindow * parent,
                         const CallbackBase & callback, 
                         list<CGUITextItem*> buttonNames, 
                         CGUIButton::ButtonData * listButtonData, 
                         CGUIButton::ButtonData * upButtonData, 
                         CGUIButton::ButtonData * downButtonData, 
                         int numberOfButtons,
                         CGUIBitmapInfo * backgroundBitmapId,
                         int buttonSpacing):
                         CGUIWindow(display), _parent(parent), _callback(callback),
                         _numberOfButtons(numberOfButtons), 
                         _listSize(buttonNames.size()), 
                         _buttonSpacing(buttonSpacing),
                         _backgroundBitmapId(backgroundBitmapId),
                         _listBoxBackground(NULL),
                         _buttonNames(buttonNames),
                         _haveBackground(true),
                         _indexTopButton(0)
{
   initializeData(display, parent, callback, buttonNames, listButtonData, upButtonData, downButtonData, numberOfButtons, _haveBackground);

   _backgroundBitmap = new CGUIBitmap(display, CGUIRegion(0, 0, 0, 0), *_backgroundBitmapId);
   addObjectToBack(_backgroundBitmap);
}

void CGUIListBox::initializeData(CGUIDisplay & display, CGUIWindow * parent, const CallbackBase & callback, list<CGUITextItem*> buttonNames, CGUIButton::ButtonData * listButtonData, CGUIButton::ButtonData * upButtonData, CGUIButton::ButtonData * downButtonData, int numberOfButtons, bool haveBackground = false)
{
   // Determine list box window & list button regions
   CGUIRegion listBoxRegion;
   CGUIRegion buttonRegion;

   if (_listSize < _numberOfButtons)
   {
      DataLog( log_level_cgui_error ) << "CGUIListBox: list size smaller that number button to display" << endmsg;
   }

   // initialize to beginning of list
   _indexTopButton = _buttonNames.begin();

   for (int i = 0; i <= MAX_NUMBER_LIST_BUTTONS; i++)
   {
      _listButton[i] = NULL;
   }

   if (upButtonData == NULL)
   {
      _haveArrowButtons = false;

      listBoxRegion.x = listButtonData->left;
      listBoxRegion.y = listButtonData->top;
      listBoxRegion.width = listButtonData->enabledBitmapId->getWidth() + 2 * _buttonSpacing;

      if (haveBackground)
      {
         listBoxRegion.height = listButtonData->enabledBitmapId->getHeight() * _numberOfButtons + _buttonSpacing * (_numberOfButtons-1)+ 2 * _buttonSpacing;

//         listButtonData->left = _buttonSpacing;
//         listButtonData->top = listButtonData->enabledBitmapId->getHeight() * i + _buttonSpacing * i + _buttonSpacing;

         for (int i = 0; i < numberOfButtons; i++)
         {
            buttonRegion = CGUIRegion( _buttonSpacing, (listButtonData->enabledBitmapId->getHeight() * i + _buttonSpacing * i + _buttonSpacing), listButtonData->enabledBitmapId->getWidth(), listButtonData->enabledBitmapId->getHeight());

            _listButton[i] = new CGUIButton(display, this, *listButtonData);
            _listButton[i]->setRegion(buttonRegion);
            _listButton[i]->disable();
         }
      } else
      {
         listBoxRegion.height = listButtonData->enabledBitmapId->getHeight() * _numberOfButtons;

         for (int i = 0; i < numberOfButtons; i++)
         {
            buttonRegion = CGUIRegion( 0, listButtonData->enabledBitmapId->getHeight() * i, listButtonData->enabledBitmapId->getWidth(), listButtonData->enabledBitmapId->getHeight());
            _listButton[i] = new CGUIButton(display, this, *listButtonData);
            _listButton[i]->setRegion(buttonRegion);
            _listButton[i]->disable();
         }
      }
   } else
   {
      _haveArrowButtons = true;

      int downArrowY = 0;

      listBoxRegion.x = upButtonData->left;
      listBoxRegion.y = upButtonData->top;
      listBoxRegion.width = upButtonData->enabledBitmapId->getWidth();

      if (haveBackground)
      {
         listBoxRegion.height = upButtonData->enabledBitmapId->getHeight() * 2 + listButtonData->enabledBitmapId->getHeight() * _numberOfButtons + _buttonSpacing * (_numberOfButtons+1);
         downArrowY = upButtonData->enabledBitmapId->getHeight() + listButtonData->enabledBitmapId->getHeight() * _numberOfButtons + _buttonSpacing * (_numberOfButtons+1);

         for (int i = 0; i < numberOfButtons; i++)
         {
            buttonRegion = CGUIRegion( 0, upButtonData->enabledBitmapId->getHeight() + listButtonData->enabledBitmapId->getHeight() * i + _buttonSpacing + _buttonSpacing * i, listButtonData->enabledBitmapId->getWidth(), listButtonData->enabledBitmapId->getHeight());
            _listButton[i] = new CGUIButton(display, this, *listButtonData);
            _listButton[i]->setRegion(buttonRegion);
            _listButton[i]->disable();
         }
      } else
      {
         listBoxRegion.height = upButtonData->enabledBitmapId->getHeight() * 2 + listButtonData->enabledBitmapId->getHeight() * _numberOfButtons;
         downArrowY = upButtonData->enabledBitmapId->getHeight() + listButtonData->enabledBitmapId->getHeight() * _numberOfButtons;

         for (int i = 0; i < numberOfButtons; i++)
         {
            buttonRegion = CGUIRegion( 0, upButtonData->enabledBitmapId->getHeight() + listButtonData->enabledBitmapId->getHeight() * i, listButtonData->enabledBitmapId->getWidth(), listButtonData->enabledBitmapId->getHeight());
            _listButton[i] = new CGUIButton(display, this, *listButtonData);
            _listButton[i]->setRegion(buttonRegion);
            _listButton[i]->disable();
         }
      }
      // make up  & down arrow button
      buttonRegion = CGUIRegion( 0, 0, upButtonData->enabledBitmapId->getWidth(), upButtonData->enabledBitmapId->getHeight());
      _upArrowButton = new CGUIButton(display, this, *upButtonData);
      _upArrowButton->setRegion(buttonRegion);
      _upArrowButton->setReleasedCallback(Callback<CGUIListBox>(this, &CGUIListBox::upButtonCallback));

      buttonRegion = CGUIRegion( 0, downArrowY, upButtonData->enabledBitmapId->getWidth(), upButtonData->enabledBitmapId->getHeight());
      _downArrowButton = new CGUIButton(display, this, *downButtonData);
      _downArrowButton->setRegion(buttonRegion);
      _downArrowButton->setReleasedCallback(Callback<CGUIListBox>(this, &CGUIListBox::downButtonCallback));
   }

   if (_listButton[0] != NULL) 
      _listButton[0]->setReleasedCallback(Callback<CGUIListBox>(this, &CGUIListBox::listButton0Callback));
   if (_listButton[1] != NULL) 
      _listButton[1]->setReleasedCallback(Callback<CGUIListBox>(this, &CGUIListBox::listButton1Callback));
   if (_listButton[2] != NULL) 
      _listButton[2]->setReleasedCallback(Callback<CGUIListBox>(this, &CGUIListBox::listButton2Callback));
   if (_listButton[3] != NULL) 
      _listButton[3]->setReleasedCallback(Callback<CGUIListBox>(this, &CGUIListBox::listButton3Callback));
   if (_listButton[4] != NULL) 
      _listButton[4]->setReleasedCallback(Callback<CGUIListBox>(this, &CGUIListBox::listButton4Callback));
   if (_listButton[5] != NULL && numberOfButtons == 5) 
      _listButton[5]->setReleasedCallback(Callback<CGUIListBox>(this, &CGUIListBox::listButton5Callback));

   // Set the region for the list box window
   setRegion(listBoxRegion);

   // Set the state of buttons.
   updateButtons();
}

CGUIListBox::~CGUIListBox()
{
   if (_backgroundBitmap)
   {
      delete _backgroundBitmap;
      _backgroundBitmap = NULL;
   }
   if (_listBoxBackground)
   {
      delete _listBoxBackground;
      _listBoxBackground = NULL;
   }
   for (int i = 0; i < _numberOfButtons; i++)
   {
      if (_listButton[i])
      {
         delete _listButton[i];
         _listButton[i] = NULL;
      }
   }
   if (_upArrowButton)
   {
      delete _upArrowButton;
      _upArrowButton = NULL;
   }
   if (_downArrowButton)
   {
      delete _downArrowButton;
      _downArrowButton = NULL;
   }
}

void CGUIListBox::attach(CGUIWindow * window, WIN_ATTRIB winAttrib)
{
   if (_parent)
   {
      assert(_parent);
      _parent->setDisabled(true);
   }

   CGUIWindow::attach(window, winAttrib);
}

CGUITextItem * CGUIListBox::returnTextItem(void)
{
   if (_returnTextItem != NULL)
   {
      assert(_parent);
      _parent->setDisabled(false);

      return _returnTextItem;
   }
   else return NULL;
} 

void CGUIListBox::upButtonCallback()
{
   if ((_indexTopButton != _buttonNames.end()) &&
       (_listSize > _numberOfButtons))
   {
      --_indexTopButton;
      updateButtons();
   }
}

void CGUIListBox::downButtonCallback()
{
   list<CGUITextItem*>::iterator  endOfList;
   endOfList = _indexTopButton;

   for (int i = 0; i < _numberOfButtons; i++)
   {
      ++endOfList;
   }

   if ((endOfList != _buttonNames.begin()) &&
       (_listSize > _numberOfButtons))
   {
      ++_indexTopButton;
      updateButtons();
   }
}

void CGUIListBox::listBoxButtonCallback(int listBoxButtonIndex)
{
   // don't know how to get this item yet!!!!
   _returnTextItem = NULL;  //listButton[listBoxButtonIndex]->getTextItem();
   _callback();
}

void CGUIListBox::listButton0Callback()
{
   _returnTextItem = (*_indexTopButton);
   _callback();
}

void CGUIListBox::listButton1Callback()
{
   list<CGUITextItem*>::iterator listIndex;
   listIndex = _indexTopButton;
   ++listIndex;

   _returnTextItem = (*listIndex);
   _callback();
}

void CGUIListBox::listButton2Callback()
{
   list<CGUITextItem*>::iterator listIndex;
   listIndex = _indexTopButton;
   ++listIndex;
   ++listIndex;

   _returnTextItem = (*listIndex);
   _callback();
}

void CGUIListBox::listButton3Callback()
{
   list<CGUITextItem*>::iterator listIndex;
   listIndex = _indexTopButton;
   ++listIndex;
   ++listIndex;
   ++listIndex;

   _returnTextItem = (*listIndex);
   _callback();
}

void CGUIListBox::listButton4Callback()
{
   list<CGUITextItem*>::iterator listIndex;
   listIndex = _indexTopButton;
   ++listIndex;
   ++listIndex;
   ++listIndex;
   ++listIndex;

   _returnTextItem = (*listIndex);
   _callback();
}

void CGUIListBox::listButton5Callback()
{
   list<CGUITextItem*>::iterator listIndex;
   listIndex = _indexTopButton;
   ++listIndex;
   ++listIndex;
   ++listIndex;
   ++listIndex;
   ++listIndex;

   _returnTextItem = (*listIndex);
   _callback();
}

void CGUIListBox::updateButtons()
{
   list<CGUITextItem*>::iterator listIndex;
   list<CGUITextItem*>::iterator first;

   first = _buttonNames.begin();
   first--;

   // set state of up arrow button

   if (_haveArrowButtons)
   {
      listIndex = _indexTopButton;

      --listIndex;

      if ((listIndex != /*_buttonNames.begin()) &&*/ first) &&
          (_listSize > (_numberOfButtons)))
      {
         _upArrowButton->enable();
      } else _upArrowButton->disable();

      // set state of down arrow button
      listIndex = _indexTopButton;

      for (int i = 0; i < _numberOfButtons; i++)
      {
         if (listIndex != _buttonNames.end())
         {
            ++listIndex;
         }
      }

      if ((listIndex != _buttonNames.end()) &&
          (_listSize > (_numberOfButtons)))
      {
         _downArrowButton->enable();
      } else _downArrowButton->disable();
   }
   //
   //  Set the state and the text for each button.
   //
   listIndex = _indexTopButton;

   bool done = false;

   for (int i=0; i < _numberOfButtons; i++)
   {
      if ( _listSize > 0 && listIndex != first) //_buttonNames.end() )
      {
         // Get the text for the button.
         CGUITextItem * textItem = (*listIndex);

         if (!done)
         {
            _listButton[i]->setText(textItem);
            _listButton[i]->enable();
         } else  // Do not have a valid text item id.
         {
            _listButton[i]->setText(" ");
            _listButton[i]->disable();
         }
         // Are we done updating buttons?  If not go to next button.
         if (!done)
         {
            ++listIndex;
         }

         if (listIndex == first) //_buttonNames.end())
         {
            done = true;
         }
      } else  // Do not have a valid text item id.
      {
         _listButton[i]->setText(" ");
         _listButton[i]->disable();
      }
   } // End of For number of buttons Loop.
}

