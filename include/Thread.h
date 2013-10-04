/*
* Copyright (C) 2008 Gambro BCT, Inc.  All rights reserved.
*
*/

/*! \file Thread.h
*/

 
#ifndef Thread_Included
#define Thread_Included

#include <string>
#include "Mutex.h"

namespace Bct
{
  /// \brief An OS idependent light weight thread class.

  ///
  /// This light weight threading class is meant provide a means for threading,
  /// without the user/derived type having to know about the underlying operating system.
  /// This is not meant to be complete, only serve the purposes of the CES software.
  /// Deriving from this class and overriding the run() will create an 
  /// operating system indepentent class for multithreading.
  /// 
  /// Calling start on the thread will create spawn a new thread running the function 
  /// run().
  ///
  /// Derived classes who override preRun, postRun or onePass MUST call _exitLock.lock()
  /// in there desructor to make sure virutal methods are not called after the derived class
  /// destructor is called.  
  /// 
  /// If new virtual methods are added to this thread class they must not 
  /// be called from this class if the _exitReady flag is set to true.  
  class Thread
  {
  public:
    /// Used to set the priority of the thread class.
    class Priority
    {
    public:
      static Priority lower(){return Priority(threadPriorityLower);}
      static Priority low(){return Priority(threadPriorityLow);}
      static Priority mediumLow(){return Priority(threadPriorityMediumLow);}
      static Priority medium(){return Priority(threadPriorityMedium);}
      static Priority mediumHigh(){return Priority(threadPriorityMediumHigh);}
      static Priority high(){return Priority(threadPriorityHigh);}

      int getPriority() const {return _priorityValue;}
      bool isLowerPriority(){return getPriority() == threadPriorityLower;}
      bool isLowPriority(){return getPriority() == threadPriorityLow;}
      bool isMediumLowPriority(){return getPriority() == threadPriorityMediumLow;}
      bool isMediumPriority(){return getPriority() == threadPriorityMedium;}
      bool isMediumHighPriority(){return getPriority() == threadPriorityMediumHigh;}
      bool isHighPriority(){return getPriority() == threadPriorityHigh;}

    protected:
      explicit Priority(int priority): _priorityValue(priority){}
    private:
      Priority();
      static const int threadPriorityLower = 120 ;
      static const int threadPriorityLow = 100 ;
      static const int threadPriorityMediumLow = 75 ;
      static const int threadPriorityMedium = 50 ;
      static const int threadPriorityMediumHigh = 35 ;
      static const int threadPriorityHigh = 20 ;
      int _priorityValue;
    };  

    Thread(std::string sName, 
           unsigned int waitMilliseconds = 1000, 
           Priority threadPriority = Priority::medium());
    virtual ~Thread();
  
    /// \return true Iff the thread is currently set to be running.
    /// \return false If the thread is set to not run.
    bool isRunning() const;
    void setIntervalMilliSec(unsigned int milliSec);

    /// Creates the new thread context and will ultimatly call the run() function.
    int start();
    /// Sets the threads context _bRunning to false, and should will stop the thread from running.
    void stop();
    static void threadWait(int nMilliSec);

    void setPriorityHigh();
    void setPriorityMedium();
    void setPriorityLow();
    void setPriority(const Priority& priority);
  protected:
    /// Derived classes should call this method in their destructors.
    void setExiting();

    /// This callback provides a mechanism to start the thread
    static int entryPoint(void* val);
    /// Deriving from this class requires the derived type to overide
    /// the function onePass().  This should be treated as the duty function of
    /// the thread and will be called once every _waitMilliSeconds.
    virtual void onePass() = 0; 
    /// This method is called once after start() is called on the thread.  Derived types
    /// should overide this method and provide any setup needed. This method will be called within the context
    /// of the spawned thread.
    virtual void preRun(){};
    /// This method is called once after stop() is called on the thread and the thread has completed
    /// executing run().  Derived types should overide this method and provide any tear down needed.  This method will be called within the context
    /// of the spawned thread.
    virtual void postRun(){};
    /// This function needs to be defined in derived classes that wish to implement
    /// threading behavior.
    void setRunning(bool bRunning);
    
    mutable Mutex _exitLock;
    /// Set to false on creation and true in destrutor.  
    /// Prevents calls to virtual methods from this class if this class 
    /// is in the process of being destroyed  
    bool _exitReady; 
    volatile bool _canExit;
  private:
    /// This structure is used for callbacks. This assurs that
    /// if derived types are created and need to be cast to the base (this class)
    /// there is not an error (undefined behavior in C++ for cast X* to void* then back to anything
    /// other than an X*.
    typedef struct threadContainer
    {
      Thread* pThread;
    } ThreadContainer;

    int run();
    /// Private Copy Constructor to avoid accidental usage.
    Thread(const Thread&);
    /// Private assignment operator to avoid accidental usage.
    Thread& operator = (const Thread&);

    bool _bRunning;

#ifdef WIN32
    void* _handle;
#elif defined VXWORKS
    int _handle;
#endif

    mutable Mutex _stopMux;
    std::string _sName;
    unsigned int _waitMilliSeconds;  ///< Period of time to wait in between calls to onePass()
    Priority _priority ; // Thread Priority
  };
} // End namespace CES
#endif //Thread_Included
