/*
* Copyright (C) 2009 Gambro BCT, Inc.  All rights reserved.
*
*/

/*! \file CriticalSection.h
*/

 
#ifndef CriticalSection_Included
#define CriticalSection_Included

namespace Bct
{

  /// \brief An OS independent CriticalSection object.

  ///
  /// This class can be used independently of the OS in order protect
  /// critical sections of code and prevent pre emption.
  ///
  class CriticalSection
  {
  public:
    ///Default Constructor
    CriticalSection();
    virtual ~CriticalSection();

    void lock();
    void unlock();
  private:
    /// Private Copy Constructor to avoid accidental usage.
    CriticalSection(const CriticalSection&);
    /// Private assignment operator to avoid accidental usage.
    CriticalSection& operator = (const CriticalSection&);

    void* _handle; ///< This will be defined in the OS specific implementation file.

  };

} // End namespace CES
#endif //CriticalSection_Included
