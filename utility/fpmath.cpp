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

   return fpCompare(lhs,rhs,epsilon); 
}

int FPMath::fpCompare(const double lhs, const double rhs, const double epsilon)
{
   int retVal;
   if ( lhs > rhs + epsilon ) retVal = 1;
   else if ( lhs < rhs - epsilon ) retVal = -1;
   else retVal = 0;

   return retVal;
}

double FPMath::fpAvg(std::list<double> &listToAvg)
{
   double runningAvg = 0.0;

   if (listToAvg.size() > 0)
   {
      std::list<double>::iterator iter = listToAvg.begin();
      for (; iter != listToAvg.end(); ++iter)
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

   if (list.size() > 0)
   {
      double avgOfList = fpAvg(list);
      double denominator = list.size();
      double numerator = 0.0;

      std::list<double>::iterator iter = list.begin();
      for (; iter != list.end(); ++iter)
      {
         numerator = numerator + pow(*iter - avgOfList, 2);
      }

      // Sample population (not 100%) therefore n-1 for the denominator
      stddev = sqrt(numerator / (denominator - 1));
   }

   return stddev;
}

