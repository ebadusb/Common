/*
 * Copyright (c) 2002 by Gambro BCT, Inc.  All rights reserved.
 *
 * TITLE:      monitorbase.h
 *
 * ABSTRACT:  This is the base class for simple monitoring objects
 *            which can be automatically used in the procedure task.
 *            Derived classes are automatically added and used in
 *            proc by specifying the [monitor] tag in the states file,
 *            followed by the class name.
 *            All derived classes must use the DECLARE_OBJ( classname)
 *            in their header (.h) file and the DEFINE_OBJ( classname )
 *            in their source (.cpp) file to make them automatic.
 *            If the object must persist through several states, then its
 *            tag should be moved to the lowest level state containing
 *            all the states for which it must persist.  If there are
 *            substates that are to be exempt from monitoring, then it
 *            is the derived class's responsibility to sense them and
 *            suspend monitoring during them.
 *
 */

#ifndef _MONITOR_BASE_H_
#define _MONITOR_BASE_H_

#include <vxWorks.h>

class MonitorBase
{
public:

   // Constructor
   MonitorBase();

   // Destructor
   virtual ~MonitorBase();

   virtual void Initialize( );

   virtual void Monitor( )=0;

   // Function to enable monitoring ...
   virtual void enable();
   // Function to disable monitoring ...
   virtual void disable();

protected:
   //
   // Set/Get the enable monitoring flag
   void enableMonitoring( const int flag ) { _EnableMonitoring = flag; };
   const int enableMonitoring() const { return _EnableMonitoring; };

protected:

   int _EnableMonitoring;
};

#endif
