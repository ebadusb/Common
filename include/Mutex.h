/*
* Copyright (C) 2008 Gambro BCT, Inc.  All rights reserved.
*
*/

/*! \file Mutex.h
*/

 
#ifndef Mutex_Included
#define Mutex_Included

#ifdef WIN32
enum Options
{
  SEM_Q_FIFO,
  SEM_Q_PRIORITY,
  SEM_DELETE_SAFE,
  SEM_INVERSION_SAFE,
  SEM_EVENTSEND_ERR_NOTIFY
};
#else if defined VXWORKS
#include "semLib.h"
#endif

namespace Bct
{

  /// \brief An OS independent mutex.

  ///
  /// This class can be used independently of the OS for mutual exclusion
  class Mutex
  {
  public:
    ///Default Constructor
    Mutex(int options = SEM_Q_PRIORITY | SEM_INVERSION_SAFE);
    virtual ~Mutex();
  
    /// Locks the mutex.  
    /// \return ture If the mutex was successfully granted, false if it wasn't or if the
    ///              mutex is not valid (not able to create).
    bool lock(int dMilliSecondTimeout = -1);
    /// Unlocs the mutex.
    void unlock();
  private:
#ifdef WIN32
    void* _handle;
#elif defined VXWORKS
    SEM_ID _handle;
#endif

    /// Private Copy Constructor to avoid accidental usage.
    Mutex(const Mutex&);
    /// Private assignment operator to avoid accidental usage.
    Mutex& operator = (const Mutex&);
  };

} // End namespace Bct
#endif //Mutex_Included
