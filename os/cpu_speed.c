/*
 * Copyright (C) 2003 Gambro BCT, Inc.  All rights reserved.
 *
 * $Header$
 * $Log$
 */

/*
 * This variable is declared static to prevent optimizer from
 * removing computations in the cpuSpeedTest function (the optimizer
 * could detect that the value for an automatic was never used).
 */
static volatile unsigned long	testValue = 0;
unsigned long cpuSpeedTestResult = 0;

void cpuSpeedTest(void)
{
	int	ticksPerSec = sysClkRateGet();
	int	testDuration = ticksPerSec/10;
	int	testIter;
	unsigned long	startTick;

	if ( testDuration < 2 ) testDuration = 2;
	startTick = tickGet();
	while ( startTick == tickGet() )
		;

	startTick = tickGet();
	cpuSpeedTestResult = 0;
	while ( tickGet() < startTick+testDuration )
	{
		cpuSpeedTestResult += 1;
		for ( testIter=0; testIter<100; testIter+=1 )
		{
			/*
			 * An arbitrary expression
			 */
			testValue = ((testValue + 12345) << 3) ^ 0x1234;
		}
	}
}


