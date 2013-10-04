/*
* Copyright (C) 2008 Gambro BCT, Inc.  All rights reserved.
*
*/

/*! \file MutexLocker.h
*/

 
#ifndef MutexLocker_Included
#define MutexLocker_Included

namespace Bct
{

  class Mutex;

  /// \brief This class can be used with a Mutex to lock it.

  ///
  /// When a mutex needs to be locked and unlocked within a function
  /// this class can be used as a utility to assure that the lock is
  /// properly released when the locker goes out of scope.  NOTE: This is 
  /// a convienence class and should only be used when the mutex can be taken out
  /// for the entire scope of the locker.
  ///
  class MutexLocker
  {
  public:
    ///Default Constructor
    explicit MutexLocker(Mutex& pMutex);
    virtual ~MutexLocker();
  
  private:
    Mutex& _pMutex;
    /// Private Copy Constructor to avoid accidental usage.
    MutexLocker(const MutexLocker&);
    /// Private assignment operator to avoid accidental usage.
    MutexLocker& operator = (const MutexLocker&);
  };
} // End namespace CES
#endif //MutexLocker_Included
