/*
 * $Header$
 *
 *	English/Metric units conversion functions
 *
 * $Log$
 *
 */

#ifndef _UNITS_CONVERSION_INCLUDE
#define _UNITS_CONVERSION_INCLUDE

class UnitsConversion
{
public:

   // weight conversions
   static float lbToKg(float lb){return lb * 0.4536;}
   static float kgToLb(float kg){return kg * 2.20456;}

   // height conversons
   static float inToCm(float in){return in * 2.54;}
   static float cmToIn(float cm){return cm * 0.3937;}

};

#endif /* ifndef _UNITS_CONVERSION_INCLUDE */

