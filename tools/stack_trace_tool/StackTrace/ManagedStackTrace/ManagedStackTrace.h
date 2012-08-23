// ManagedStackTrace.h

#pragma once
#include "StackTrace.h"
#pragma managed
#using <mscorlib.dll>
using namespace System;
namespace TRACE
{
public ref class ManagedStackTrace
{
private:
	//StackTrace*  _pStackTrace;
public:
	//ManagedStackTrace() { _pStackTrace = new StackTrace(); }
	//~ManagedStackTrace(){ delete _pStackTrace; }
	System::String^ Main(System::String^);
}; // MStackTrace

}; //namespace TRACE