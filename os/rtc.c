/*
 * Copyright (C) 2002 Gambro BCT, Inc.  All rights reserved.
 *
 *	vxWorks utilities for accessing the real-time clock.  This file should be
 * included as part of the build for an vxWorks OS image and is not intended
 * to be compiled separately.
 *
 * $Header: //bctquad3/home/BCT_Development/vxWorks/Common/os/rcs/rtc.c 1.1 2002/05/30 13:00:52Z jl11312 Exp ms10234 $
 * $Log: rtc.c $
 *
 */

#include "config.h"	/* BSP configuration data */
#include <time.h>
#include "rtc.h"

/*
 *	RTC register addresses related to time/date
 */
enum
{
	RTC_Second = 0x00,	/* BCD seconds value */
	RTC_Minute = 0x02,	/* BCD minutes value */
	RTC_Hour = 0x04,		/* BCD hours value */
	RTC_Day = 0x07,		/* BCD day of month value */
	RTC_Month = 0x08,		/* BCD month value */
	RTC_Year = 0x09,		/* BCD year value */
	RTC_StatusA = 0x0a,	/* Status A register */
	RTC_StatusB = 0x0b,	/* Status B register */
	RTC_Century = 0x32,	/* BCD century value */
};

/*
 *	Local functions
 */
static int checkRTCUpdating(void);
static int lockRTC(void);
static int unlockRTC(void);
static int getRTCReg(int regAddr);
static void setRTCReg(int regAddr, int value);

/*
 *	Check if the RTC is currently updating the time/date registers.
 * Returns non-zero if an update is in progress, or 0 otherwise.
 */
static int checkRTCUpdating(void)
{
	int	result;

	/*
	 *	Interrupts must be disabled for the OutByte/InByte instruction pair,
	 * otherwise an RTC interrupt could change the index register between
	 * the two calls.
	 */
	__asm__ volatile ("cli");
	sysOutByte(RTC_INDEX, RTC_StatusA);
	result = sysInByte(RTC_DATA);
	__asm__ volatile ("sti");

	/*
	 *	Update in progress is bit 7
	 */
	return result&0x80;
}

/*
 *	Lock RTC so that time/date registers can be set
 */
static int lockRTC(void)
{
	int	statusValue;

	/*
	 *	Interrupts must be disabled for the OutByte/InByte instruction pair,
	 * otherwise an RTC interrupt could change the index register between
	 * the two calls.
	 */
	__asm__ volatile ("cli");
	sysOutByte(RTC_INDEX, RTC_StatusB);
	statusValue = sysInByte(RTC_DATA);

	/*
	 *	Setting bit 7 in the status B register blocks RTC update
	 * cycles, and allows setting the time/date registers
	 */
	sysOutByte(RTC_INDEX, RTC_StatusB);
	sysOutByte(RTC_DATA, statusValue | 0x80);
	__asm__ volatile ("sti");
}

/*
 *	Unlock RTC after setting time/date registers
 */
static int unlockRTC(void)
{
	int	statusValue;

	/*
	 *	Interrupts must be disabled for the OutByte/InByte instruction pair,
	 * otherwise an RTC interrupt could change the index register between
	 * the two calls.
	 */
	__asm__ volatile ("cli");
	sysOutByte(RTC_INDEX, RTC_StatusB);
	statusValue = sysInByte(RTC_DATA);

	/*
	 *	Clearing bit 7 in the status B register reenables RTC update cycles.
	 */
	sysOutByte(RTC_INDEX, RTC_StatusB);
	sysOutByte(RTC_DATA, statusValue & 0x7f);
	__asm__ volatile ("sti");
}

/*
 *	Get a single register value from the RTC.  Note that time/date
 * registers are stored in BCD.  Values are converted to decimal
 * before being returned by this function.
 */
static int getRTCReg(int regAddr)
{
	int	result;

	/*
	 *	Interrupts must be disabled for the OutByte/InByte instruction pair,
	 * otherwise an RTC interrupt could change the index register between
	 * the two calls.
	 */
	__asm__ volatile ("cli");
	sysOutByte(RTC_INDEX, regAddr);
	result = sysInByte(RTC_DATA);
	__asm__ volatile ("sti");

   result = ((result & 0xf0) >> 4) * 10 + (result & 0x0f);
   return result;
}

/*
 *	Set a single register value from the RTC.  Note that time/date
 * registers are stored in BCD.  The decimal values accepted by this
 * function are converted to BCD before writing the data to the RTC.
 */
static void setRTCReg(int regAddr, int value)
{
	int	bcdValue = (value%10) | ((value/10) << 4);

	/*
	 *	Interrupts must be disabled for the OutByte/InByte instruction pair,
	 * otherwise an RTC interrupt could change the index register between
	 * the two calls.
	 */
	__asm__ volatile ("cli");
	sysOutByte(RTC_INDEX, regAddr);
	sysOutByte(RTC_DATA, bcdValue);
	__asm__ volatile ("sti");
}

STATUS setCurrentTimeFromRTC(void)
{
	struct tm			rtcTime;
	time_t				currTime;
	struct timespec	clockTime;
	int	 				retry;
	int					rtcReadOK = 0;
	
	/*
	 *	Get time data from RTC.  We try twice in case a clock update occurred
	 * sometime during the read.  Since updates are once per second, we won't
	 * get 
	 */
	for ( retry=0; retry<2 && !rtcReadOK; retry++ )
	{
		if ( checkRTCUpdating() )
		{
			taskDelay(1);
		}

		memset((void *)&rtcTime, 0, sizeof(rtcTime));
		rtcTime.tm_sec  = getRTCReg(RTC_Second);
		rtcTime.tm_min  = getRTCReg(RTC_Minute);
		rtcTime.tm_hour = getRTCReg(RTC_Hour);
		rtcTime.tm_mday = getRTCReg(RTC_Day);
		rtcTime.tm_mon  = getRTCReg(RTC_Month)-1;
		rtcTime.tm_year = getRTCReg(RTC_Century)*100 + getRTCReg(RTC_Year) - 1900;

		if ( !checkRTCUpdating() )
		{
			rtcReadOK = 1;
		}
	}

	/*
	 *	Sanity check the year value - century value may be incorrect for
    * systems with an older BIOS
    */
   if ( rtcTime.tm_year < 0 )
	{
		/*
		 *	Time does not appear at all correct, set it to a valid default
		 */
		rtcTime.tm_sec  = 0;
		rtcTime.tm_min  = 0;
		rtcTime.tm_hour = 0;
		rtcTime.tm_mday = 1;
		rtcTime.tm_mon  = 0;
		rtcTime.tm_year = 2002-1900;
	}
	else if ( rtcTime.tm_year < 100 )
	{
		/*
		 *	Likely a BIOS problem - century is set to 19
		 */
		rtcTime.tm_year += 100;
	}

	currTime = mktime(&rtcTime);
   clockTime.tv_sec = currTime;
   clockTime.tv_nsec = 0;
   clock_settime(CLOCK_REALTIME, &clockTime);
   return OK;
}

STATUS setRTCFromCurrentTime(void)
{
	struct tm	rtcTime;
	time_t		currTime;
	
	/*
	 *	Get current time
	 */
	currTime = time(NULL);
	gmtime_r(&currTime, &rtcTime);

	/*
	 *	Write values to RTC hardware
	 */
	lockRTC();
	setRTCReg(RTC_Second, rtcTime.tm_sec);
	setRTCReg(RTC_Minute, rtcTime.tm_min);
	setRTCReg(RTC_Hour, rtcTime.tm_hour);
	setRTCReg(RTC_Day, rtcTime.tm_mday);
	setRTCReg(RTC_Month, rtcTime.tm_mon+1);
	setRTCReg(RTC_Year, rtcTime.tm_year%100);
	setRTCReg(RTC_Century, rtcTime.tm_year/100 + 19);
	unlockRTC();

   return OK;
}

