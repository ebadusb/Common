/*
 * Copyright (c) 2002 by Gambro BCT, Inc.  All rights reserved.
 *
 * TITLE:      monitorbase.cpp
 *
 */

#include "monitorbase.h"


MonitorBase :: MonitorBase() : 
_EnableMonitoring( 0 )
{
}

MonitorBase :: ~MonitorBase()
{
}

void MonitorBase :: Initialize( )
{
}

void MonitorBase :: enable()
{
   _EnableMonitoring = 1;
}

void MonitorBase :: disable()
{
   _EnableMonitoring = 0;
}

