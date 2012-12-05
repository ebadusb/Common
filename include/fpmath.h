/*
 *
 * TITLE:      fpmath.h
 *
 * ABSTRACT:   This class supports floating point math helper functions
 *
 */

#ifndef __FP_MATH_H__
#define __FP_MATH_H__

#include <list>
#include <math.h>

namespace FPMath
{

	//
	// COMPARE TWO FLOATING POINT VALUES --
	// The first parameter will be compared to the second parameter with a scaled 
	//    epsilon value (10 * FLT_EPSILON * first parmeter). 

	//    Note: If using the form that allows a passed in epsion value, thought 
	//          should be given to choosing an epsilon value.  Here is 
	//          a link to use when determining the epsilon value:
	//
	//			http://www.cygnus-software.com/papers/comparingfloats/comparingfloats.htm
	//
	// The function will return:
	//   -1 - the first parameter is less than the second
	//    0 - the parameter are equal to each other, within the epsilon value
	//    1 - the first parameter is greater than the second
	//
	int fpCompare(const double lhs, const double rhs);
	int fpCompare(const double lhs, const double rhs, const double epsilon);

	// 
	// Returns the average of a linked list of doubles
	//
	double fpAvg(std::list<double> &listToAvg);

	//
	// Returns the standard deviation of a linked list of doubles
	// 
	double fpStdDev(std::list<double> &list);

	//
	// Returns the value rounded to the given precision. Use 0 to negative precision for integers.
	//
	template<class T>
	T fpPreciseRound(T value, int precision)
	{
		T rounded_value;
		rounded_value = static_cast<T>(floor(value * pow(10.0, precision) + .5) / pow(10.0, precision));
		return rounded_value;
	}

	/// \return true if the value in lhs is within epsilon of rhs.
	bool compareEqual(double lhs, double rhs, double epsilon);
	/// This version of the comparison implicitly supplies an appropriate epsilon value
	/// as a comparison value.
	bool compareEqual(double lhs, double rhs);

	/// (lhs > rhs)
	bool compareGreater(double lhs, double rhs, double epsilon);
	bool compareGreater(double lhs, double rhs);

	/// (lhs < rhs)
	bool compareLess(double lhs, double rhs, double epsilon);
	bool compareLess(double lhs, double rhs);

	/// (lhs >= rhs)
	bool compareGreaterOrEqual(double lhs, double rhs, double epsilon);
	bool compareGreaterOrEqual(double lhs, double rhs);

	bool compareLessOrEqual(double lhs, double rhs, double epsilon);
	bool compareLessOrEqual(double lhs, double rhs);

	bool compareInRange(double value, double min, double max, double epsilon);
	bool compareInRange(double value, double min, double max);

	bool isNan(double value);
	bool isInf(double value);

	double fpFloor(double value);
	double fpCeiling(double value);
	int fpRound(double value);

	int getNumberOfIntegralDigits(double value);
};

#endif

