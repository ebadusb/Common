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
// The function will return:
//   -1 - the first parameter is less than the second
//    0 - the parameter are equal to each other, within the epsilon value
//    1 - the first parameter is greater than the second
//
int fpCompare(const double lhs, const double rhs);

//
// Same as above, but lets the user of the function dictate what epsilon is
//    Note: epsilon will be set = (10 * FLT_EPSILON * first parameter)
//    if epsilon is < (10 * FLT_EPSILON * first parameter)
//
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

};

#endif

