/*
 *	Copyright (c) 2005 by Gambro BCT, Inc.  All rights reserved.
 *
 * $Header: K:/BCT_Development/vxWorks/Common/cgui/rcs/cgui_data_item.h 1.8 2010/04/27 21:26:08Z rm10919 Exp jl11312 $
 * $Log: cgui_data_item.h $
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
   ~CGUIDataItemDouble();

   StringChar * convertToString(void);

   void setValue(double value);
   void setPrecision(int precision);

protected:   
   double _value;
   int _precision;
};

class CGUIDataItemTextItem: public CGUIDataItem
{
public:
   CGUIDataItemTextItem();
   CGUIDataItemTextItem(CGUITextItem * value);
   ~CGUIDataItemTextItem();

   StringChar * convertToString(void);

   void setValue (CGUITextItem * value);
   void setValue (StringChar * value);
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
