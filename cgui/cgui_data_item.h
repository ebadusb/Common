/*
 *	Copyright (c) 2005 by Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: K:/BCT_Development/vxWorks/Common/cgui/rcs/cgui_data_item.h 1.8 2010/04/27 21:26:08Z rm10919 Exp jl11312 $
 * $Log: cgui_data_item.h $
 * Revision 1.7  2010/04/27 20:58:20Z  rm10919
 * Make decimal separator for dataItemDouble class a static. IT 8256
 * Revision 1.6  2007/11/15 21:02:37Z  rm10919
 * Create new data item class for clock type numeric strings.
 * Revision 1.5  2007/06/04 22:04:20Z  wms10235
 * IT83 - Updates for the common GUI project to use the unicode string class
 * Revision 1.4  2007/04/05 17:37:17Z  pn02526
 * Make StringChar * arg a const StringChar *
 * Revision 1.3  2007/03/28 12:03:12  pn02526
 * Add ability to specify a decimal separator.
 * Revision 1.2  2007/03/01 12:09:55  rm10919
 * Add DataItemText to file reader and setText(char *) to DITextItem.
 * Revision 1.1  2005/04/27 13:40:44Z  rm10919
 * Initial revision
 *
 */

#ifndef _DATA_ITEM_INCLUDE
#define _DATA_ITEM_INCLUDE

#include "cgui_graphics.h"
#include "cgui_text_item.h"

class CGUIDataItem
{
public:
	CGUIDataItem( void );
	CGUIDataItem( bool valueChanged );
	virtual ~CGUIDataItem();

	virtual const StringChar * convertToString( void ) = 0;

protected:
	UnicodeString _string;
	bool _valueChanged;

private:
	CGUIDataItem( const CGUIDataItem& object );
	CGUIDataItem& operator=( const CGUIDataItem& object );
};

class CGUIDataItemInteger : public CGUIDataItem
{
public:
	CGUIDataItemInteger( void );
	CGUIDataItemInteger( int value );
	virtual ~CGUIDataItemInteger();

	virtual const StringChar * convertToString( void );

	void setValue( int value );

protected:
	int _value;

private:
	CGUIDataItemInteger( const CGUIDataItemInteger& object );
	CGUIDataItemInteger& operator=( const CGUIDataItemInteger& object );
};

class CGUIDataItemDouble : public CGUIDataItem
{
public:
	CGUIDataItemDouble( void );
	CGUIDataItemDouble( double value, int precision );
	CGUIDataItemDouble( double value, int precision, CGUITextItem * separator );
	virtual ~CGUIDataItemDouble();

	virtual const StringChar * convertToString( void );

	void setValue( double value );
	void setPrecision( int precision );
	
	void setDecimalSeparator( CGUITextItem * separator ) { _separator = separator; _valueChanged = true; };
	static void setDefaultSeparator( CGUITextItem * defaultSeparator ) { _defaultSeparator = defaultSeparator; };

protected:
	double	_value;
	int		_precision;
	CGUITextItem * _separator;
	static CGUITextItem * _defaultSeparator;

private:
	CGUIDataItemDouble( const CGUIDataItemDouble& object );
	CGUIDataItemDouble& operator=( const CGUIDataItemDouble& object );
};

class CGUIDataItemTextItem : public CGUIDataItem
{
public:
	CGUIDataItemTextItem( CGUITextItem * value );
	virtual ~CGUIDataItemTextItem();

	virtual const StringChar * convertToString( void );

	void setValue( CGUITextItem * value );
	void setValue( const StringChar * value );
	void setValue( char * value);

protected:
	CGUITextItem * _value;

private:
	CGUIDataItemTextItem( void );
	CGUIDataItemTextItem( const CGUIDataItemTextItem& object );
	CGUIDataItemTextItem& operator=( const CGUIDataItemTextItem& object );
};

class CGUIDataItemText : public CGUIDataItem
{
public:
	CGUIDataItemText( void );
	CGUIDataItemText( const char * value );
	virtual ~CGUIDataItemText();

	virtual const StringChar * convertToString( void );

	void setValue( const char * value );

private:
	CGUIDataItemText( const CGUIDataItemText& object );
	CGUIDataItemText& operator=( const CGUIDataItemText& object );
};

// Used to insert a leading 0 for clock time minutes and seconds.
class CGUIDataItemClock : public CGUIDataItem
{
public:
	CGUIDataItemClock( void );
	CGUIDataItemClock( int value );
	virtual ~CGUIDataItemClock();

	virtual const StringChar * convertToString( void );

	void setValue( int value );

protected:
	int _value;

private:
	CGUIDataItemClock( const CGUIDataItemClock& object );
	CGUIDataItemClock& operator=( const CGUIDataItemClock& object );
};

#endif /* ifndef _DATA_ITEM_INCLUDE */

