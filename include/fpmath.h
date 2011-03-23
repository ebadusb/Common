/*
 *
 * TITLE:      fpmath.h
 *
 * ABSTRACT:   This namespace supports floating point math helper functions
 *
 */

#ifndef __FP_MATH_H__
#define __FP_MATH_H__

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
char fpCompare(const double lhs, const double rhs);

};

#endif
