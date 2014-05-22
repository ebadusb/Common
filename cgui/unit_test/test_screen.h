/*
 * $Header: 
 *
 * $Log: 
 *
 */

#ifndef SCREEN_TEST_INCLUDE
#define SCREEN_TEST_INCLUDE

#include "base_report_config.h"
#include "print_driver.h"
#include "gray_image.h"
#include "base/gui/base_screen.h"
#include "base/gui/base_string_data.h"
#include "base/gui/base_bitmaps.h"
#include "base/module_entry/control/base_config.h"
#include "base/data_layer/control/base_data_interface.h"
#include "cgui/cgui_bitmap.h"
#include "cgui/cgui_bitmap_info.h"
#include "cgui/cgui_button.h"
#include "cgui/cgui_frame.h"
#include "cgui/cgui_graphics.h"
#include "cgui/cgui_rectangle.h"
#include "cgui/cgui_text.h"
#include "cgui/cgui_window.h"
#include "cgui/cgui_window_object.h"

class TestScreen : public BaseScreen
{
public:
	TestScreen(CGUIDisplay & display,
						  const CallbackBase gobackCallbackPrint);

	virtual ~TestScreen();

   void initializeTestScreen();

   void disableButtons();
   void enableButtons(short pageNumber,short noOfPages);
   void createButtons();
	void hideButtons(void);
   void setModeTabState(bool state);

	unsigned short grayToRGB565(unsigned char pixelValue);

	virtual void disableAlarmActiveButton();
	
protected:

	virtual void addTimeDateObject();
	virtual void addProtocolTextObject();
	virtual void addAdornmentTextObject();
	virtual void setAlarmActiveButton();
	
	const CallbackBase _goBackCallback;
 
	CGUIButton::ButtonData _goBackButtonData;
	CGUIButton * _goBackButton;

	CGUIBitmap* _backgroundBmp;
	CGUIBitmapInfo* _cguiBitmapInfo;
	unsigned short _bmp[BaseReportConfig::ScreenHeight][BaseReportConfig::ScreenWidth];

	CGUIText    *_printNotesSummaryText;
};

#endif
