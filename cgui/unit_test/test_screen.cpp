/*
 * $Header: 
 *
 * $Log:  
 *
 */

#include <vxWorks.h>
#include "test_screen.h"

UnicodeString testUstr;
bool useDefault=true;
int BoxHeight=100;
int BoxWidth=100;
int BoxAttrib=0; 
int BoxX=100;
int BoxY=100;

void setTestText(char* str)
{
	testUstr = str;
	useDefault = false;
}

TestScreen::TestScreen( CGUIDisplay & display,
												const CallbackBase goBackCallback):
	BaseScreen(display),
	_goBackCallback(goBackCallback)
{
	_backgroundBmp = NULL;
	_cguiBitmapInfo = NULL;

	unsigned long height = BaseReportConfig::ScreenHeight;
	unsigned long width = BaseReportConfig::ScreenWidth;

	for( int i= 0 ; i < height ; i++ )
	{
		for( int j=0 ; j < width ; j++ )
		{
			if( ( i == BoxY-1 && ( j >= BoxX-1 && j <= (BoxX+BoxWidth) ) ) ||
				 ( i == (BoxY+BoxHeight) && ( j >= BoxX-1 && j <= (BoxX+BoxWidth) ) ) ||
				 ( j == BoxX-1 && ( i >= BoxY-1 && i <= (BoxY+BoxHeight) ) ) || 
				 ( j == (BoxX+BoxWidth) && ( i >= BoxY-1 && i <= (BoxY+BoxHeight) ) ) )
			{
				_bmp[i][j] = grayToRGB565(0x00);
			}
			else
			{
				_bmp[i][j] = grayToRGB565(0xFF);
			}
		}
	} 

	_cguiBitmapInfo = new CGUIBitmapInfo(_bmp[0],
													 (unsigned long) height*width,
													 (unsigned short) width,
													 (unsigned short) height);

	if( _backgroundBmp == NULL )
		_backgroundBmp = new CGUIBitmap (_display, CGUIRegion(0,0,0,0), *_cguiBitmapInfo );
}

TestScreen::~TestScreen()
{
	if ( _goBackButton )
	{
		delete _goBackButton;
		_goBackButton = NULL;
	}

	if( _printNotesSummaryText )
	{
		delete _printNotesSummaryText;
		_printNotesSummaryText =  NULL;
	}

	if( _cguiBitmapInfo != NULL )
	{
		delete _cguiBitmapInfo;
		_cguiBitmapInfo=NULL;
	}

	if ( _backgroundBmp != NULL )
	{
		delete _backgroundBmp;
		_backgroundBmp = NULL;
	}
}

unsigned short TestScreen::grayToRGB565(unsigned char pixelValue)
{
	unsigned short pixel = ( unsigned short) pixelValue;

	unsigned short red = pixel >> 3 ;   // first 5 MSB's 
	unsigned short green = pixel >> 2;  // mid 6 bits
	unsigned short blue = pixel >> 3;   // 5 LSB's

	red = red << 11;
	green = green << 5;

	unsigned short rgb565Pixel = 0;
	rgb565Pixel = rgb565Pixel | red | green | blue;
	return rgb565Pixel;
}

void TestScreen::setModeTabState(bool state)
{
	BaseScreen::_configModeButton->setWindowVisibility(state);
	BaseScreen::_dataModeButton->setWindowVisibility(state);
	BaseScreen::_runModeButton->setWindowVisibility(state);
	BaseScreen::_endRunModeButton->setWindowVisibility(state);
	BaseScreen::setTempTabsDisabled(!state);
	refreshModeButtons();
}

void TestScreen::disableAlarmActiveButton()
{
	if( _activeAlarmButton != NULL && _activeAlarmButton->isVisible() )
	{
		_activeAlarmButton->invisible();
	}
}

// Notice start
// The four functions below belong to BaseScreen class from which this class has been derived
// The functions below add some static objects that all derived screens use except this one.
// They are called during BaseScreen::initialize(). Hence they have been overloaded as blanks
// as of now. This was done to cause minimum code change in BaseScreen
void TestScreen::addTimeDateObject()
{
}

void TestScreen::setAlarmActiveButton()
{
}

void TestScreen::addProtocolTextObject()
{
}

void TestScreen::addAdornmentTextObject()
{
}
//Notice end

void TestScreen::initializeTestScreen()
{
	initializeScreen(NULL);

	if(_screenshotButton)
	{
		disableSnapButton();				 // Screen Shot stuff
		_screenshotButton->detach();
	}

	BaseScreen::setTempTabsDisabled(true);
	BaseScreen::_configModeButton->setWindowVisibility(false);
	BaseScreen::_dataModeButton->setWindowVisibility(false);
	BaseScreen::_runModeButton->setWindowVisibility(false);
	BaseScreen::_endRunModeButton->setWindowVisibility(false);

	_goBackButtonData.left = 270;
	_goBackButtonData.top = 40;
	_goBackButtonData.vMargin = 3;
	_goBackButtonData.hMargin = 3;
	_goBackButtonData.enabledTextItem = NULL;
	_goBackButtonData.enabledStylingRecord = NULL;

	_goBackButtonData.disabledBitmapId = BaseBitmaps::gobackDisabledButton;
	_goBackButtonData.disabledTextItem = NULL;
	_goBackButtonData.disabledStylingRecord = NULL;

	_goBackButtonData.pressedTextItem = NULL;
	_goBackButtonData.pressedStylingRecord = NULL;

	strcpy(_goBackButtonData.alternateButtonId, "GoBackReport");
	_goBackButtonData.type = CGUIButton::RaiseAfterRelease;

   bool ifScreenDefault = false;

	switch ( BaseScreen::getScreenState() )
	{
	case GuiCommonDef::ScreenDefault:
		DataLog(log_level_gui_info) << " ERROR : GUI Commondef state is ScreenDefault  " << endmsg;
		ifScreenDefault = true;

	case GuiCommonDef::Alarm:
		if( ifScreenDefault == false )
			DataLog(log_level_gui_info) << " ERROR : GUI Commondef state is Alarm  " << endmsg;

	case GuiCommonDef::PreRun:
		_goBackButtonData.enabledBitmapId = BaseBitmaps::dismissEnabledPrerunButton;
		_goBackButtonData.pressedBitmapId = BaseBitmaps::dismissPressedPrerunButton;
		break;

	case GuiCommonDef::PostRun:

		_goBackButtonData.enabledBitmapId = BaseBitmaps::dismissEnabledPostrunButton;
		_goBackButtonData.pressedBitmapId = BaseBitmaps::dismissPressedPostrunButton;
		break;

	case GuiCommonDef::Run:
		_FATAL_ERROR(__FILE__, __LINE__, "Attempted to create a report screen during a run ");
		break;

	}

	CGUITextItem notesCopy(BaseStringData::printNotesSummary);

	StylingRecord sr = notesCopy.getStylingRecord();

	sr.region.x = BoxX;
	sr.region.y = BoxY;
	sr.region.width = BoxWidth;
	sr.region.height = BoxHeight;
	sr.attributes = BoxAttrib;

	notesCopy.setStylingRecord(sr);

	if( useDefault )
	{
		UnicodeString ustr = BaseStringData::testJapString.getTextObj();
		testUstr = ustr;
	}
		
	notesCopy.setText(testUstr.getString());

	_printNotesSummaryText = new CGUIText(_display,&notesCopy);

	addObjectToFront(_backgroundBmp);
	addObjectToFront(_printNotesSummaryText);
}

void TestScreen::disableButtons()
{
	if ( _goBackButton )
	{
		_goBackButton->disable();
	}
}

void TestScreen::hideButtons(void)
{
	if ( _goBackButton )
	{
		_goBackButton->invisible();
	}
}

void TestScreen::createButtons()
{
	_goBackButton = new CGUIButton(_display, this, _goBackButtonData);
	_goBackButton->setReleasedCallback(_goBackCallback);
	_goBackButton->disable();
}

void TestScreen::enableButtons(short pageNumber,short noOfPages)
{
	_goBackButton->enable();
}


