/*
 *
 * TITLE:      fpmath.cpp
 *
 * ABSTRACT:   This file supports floating point math helper functions
 *
 */

#include <float.h>
#include <math.h>
#include "fpmath.h"


int FPMath::fpCompare(const double lhs, const double rhs)
{
	double epsilon = 10 * FLT_EPSILON * ( lhs == 0 ? 1 : fabs(lhs) );

	int retVal;
	if ( lhs > rhs + epsilon ) retVal = 1;
	else if ( lhs < rhs - epsilon ) retVal = -1;
	else retVal = 0;

	return retVal;
}

