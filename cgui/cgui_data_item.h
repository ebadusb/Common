/*
 *	Copyright (c) 2005 by Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: K:/BCT_Development/vxWorks/Common/cgui/rcs/cgui_data_item.h 1.8 2010/04/27 21:26:08Z rm10919 Exp jl11312 $
 * $Log: cgui_data_item.h $
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
   CGUIDataItem(void);
   CGUIDataItem(bool valueChanged);
   ~CGUIDataItem();

   virtual StringChar * convertToString(void) = 0;

protected:
   
   StringChar * _string;

   bool _valueChanged;
};


class CGUIDataItemInteger: public CGUIDataItem
{
public:
   CGUIDataItemInteger();
   CGUIDataItemInteger(int value);
   ~CGUIDataItemInteger();

   StringChar * convertToString(void);

   void setValue(int value);

protected:
   int _value;
};

class CGUIDataItemDouble: public CGUIDataItem
{
public:
   CGUIDataItemDouble();
   CGUIDataItemDouble(double value, int precision );
   CGUIDataItemDouble(double value, int precision, CGUITextItem * separator );
   ~CGUIDataItemDouble();

   StringChar * convertToString(void);

   void setValue(double value);
   void setPrecision(int precision);
   void setDecimalSeparator( CGUITextItem * separator ) { _separator = separator; _valueChanged=true; };

protected:   
   double _value;
   int _precision;
   CGUITextItem * _separator;

   size_t stringCharLen( const StringChar * s );
   size_t stringCharAllocation(const size_t len);
   size_t stringCharAllocation(const StringChar * s);
   StringChar * stringChar( const char * string );
   void stringCharCpy( StringChar ** ps1, const StringChar * s2 );
   void stringCharCat( StringChar ** ps1, const StringChar * s2 );
};

class CGUIDataItemTextItem: public CGUIDataItem
{
public:
   CGUIDataItemTextItem();
   CGUIDataItemTextItem(CGUITextItem * value);
   ~CGUIDataItemTextItem();

   StringChar * convertToString(void);

   void setValue (CGUITextItem * value);
   void setValue (const StringChar * value);
   void setValue (char * value);

protected:
   CGUITextItem * _value;
};

class CGUIDataItemText: public CGUIDataItem
{
public:
   CGUIDataItemText();
   CGUIDataItemText(char * value);
   ~CGUIDataItemText();

   StringChar * convertToString(void);

   void setValue(char * value);

protected:
   char * _value;
};

#endif /* ifndef _DATA_ITEM_INCLUDE */
