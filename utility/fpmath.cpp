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
	double absLHS = fabs(lhs);
	double absRHS = fabs(rhs);
	double scaleValue = ( absLHS > absRHS ? absLHS : absRHS );
	double epsilon = 10 * FLT_EPSILON * ( scaleValue == 0 ? 1 : scaleValue );

	return fpCompare(lhs,rhs,epsilon); 
}

int FPMath::fpCompare(const double lhs, const double rhs, const double epsilon)
{
	int retVal;
	if ( lhs > rhs + epsilon )	retVal = 1;
	else if ( lhs < rhs - epsilon ) retVal = -1;
	else retVal	= 0;

	return retVal;
}

double FPMath::fpAvg(std::list<double> &listToAvg)
{
	double runningAvg = 0.0;

	if ( listToAvg.size() > 0 )
	{
		std::list<double>::iterator iter = listToAvg.begin();
		for ( ; iter != listToAvg.end(); ++iter )
		{
			runningAvg += *iter;
		}

		runningAvg = runningAvg / listToAvg.size();
	}

	return runningAvg;
}

double FPMath::fpStdDev(std::list<double> &list)
{
	double stddev = 0.0;

	if ( list.size() > 0 )
	{
		double avgOfList = fpAvg(list);
		double denominator = list.size();
		double numerator = 0.0;

		std::list<double>::iterator iter = list.begin();
		for ( ; iter != list.end(); ++iter )
		{
			numerator = numerator + pow(*iter - avgOfList, 2);
		}

		// Sample population (not 100%) therefore n-1 for the denominator
		stddev = sqrt(numerator / (denominator - 1));
	}

	return stddev;
}


bool FPMath::compareEqual(double lhs, double rhs, double epsilon)
{
	return( fpCompare(lhs,rhs,epsilon) != 0) ? false : true;
}

bool FPMath::compareEqual(double lhs, double rhs)
{
	return( fpCompare(lhs,rhs) != 0) ? false : true;
}

bool FPMath::compareGreater(double lhs, double rhs, double epsilon)
{
	return( fpCompare(lhs,rhs,epsilon) > 0) ? true : false;
}

bool FPMath::compareGreater(double lhs, double rhs)
{
	return( fpCompare(lhs,rhs) > 0) ? true : false;
}

bool FPMath::compareLess(double lhs, double rhs, double epsilon)
{
	return( fpCompare(lhs,rhs,epsilon) < 0) ? true : false;
}

bool FPMath::compareLess(double lhs, double rhs)
{
	return( fpCompare(lhs,rhs) < 0) ? true : false;
}

bool FPMath::compareGreaterOrEqual(double lhs, double rhs, double epsilon)
{
	return( fpCompare(lhs,rhs,epsilon) >= 0) ? true : false;
}
bool FPMath::compareGreaterOrEqual(double lhs, double rhs)
{
	return( fpCompare(lhs,rhs) >= 0) ? true : false;
}

bool FPMath::compareLessOrEqual(double lhs,double rhs, double epsilon)
{
	return( fpCompare(lhs,rhs,epsilon) <= 0) ? true : false;
}
bool FPMath::compareLessOrEqual(double lhs, double rhs)
{
	return( fpCompare(lhs,rhs) <= 0) ? true : false;
}

bool FPMath::compareInRange(double value, double min, double max)
{
	return( fpCompare(value, min) >= 0 && fpCompare(value, max) <= 0 );
}

bool FPMath::compareInRange(double value, double min, double max, double epsilon)
{
	return( fpCompare(value, min, epsilon) >= 0 && fpCompare(value, max, epsilon) <= 0 );
}

int FPMath::getNumberOfIntegralDigits(double value)
{
	if ( FPMath::compareLess(value, 0.0) )
		value = -value;
	return static_cast<int>(log(value)/log(10.0)) + 1;
}

bool FPMath::isNan(double value)
{
	return value != value;
}

bool FPMath::isInf(double value)
{
	if ( (value == value) && ((value - value) != 0.0) )
		return true;
	else
		return false;

}

double FPMath::fpFloor(double value)
{
	if ( compareGreater(value, 0.0) )
		return static_cast<double>(static_cast<int>(value + 0.5));
	else
	{
		double temp = ceil(value);
		if ( FPMath::compareEqual(temp - value, 0.0) )
			return value;
		else
			return static_cast<double>(static_cast<int>(value - 1.0));
	}
}

double FPMath::fpCeiling(double value)
{
	if ( compareGreater(value, 0.0) )
	{
		double temp = floor(value);
		if ( FPMath::compareEqual(temp - value, 0.0) )
			return value;
		else
			return static_cast<double>(static_cast<int>(value + 1.0));
	}
	else // Less than 0
		return static_cast<double>(static_cast<int>(value - 0.5));
}

int FPMath::fpRound(double value)
{
	return( value > 0 ) ? (int)( value + 0.5 ) : (int)( value - 0.5 );
}

