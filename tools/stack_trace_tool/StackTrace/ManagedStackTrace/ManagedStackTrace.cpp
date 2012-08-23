// This is the main DLL file.

#include "stdafx.h"
#include "ManagedStackTrace.h"
#include <vcclr.h>

using namespace System;

void MarshalString ( String ^ s, std::string& os ) {
   using namespace Runtime::InteropServices;
   const char* chars = 
      (const char*)(Marshal::StringToHGlobalAnsi(s)).ToPointer();
   os = chars;
   Marshal::FreeHGlobal(IntPtr((void*)chars));
}

void MarshalString ( String ^ s, std::wstring& os ) {
   using namespace Runtime::InteropServices;
   const wchar_t* chars = 
      (const wchar_t*)(Marshal::StringToHGlobalUni(s)).ToPointer();
   os = chars;
   Marshal::FreeHGlobal(IntPtr((void*)chars));
}

System::String^ TRACE::ManagedStackTrace::Main(System::String^ args)
{
	//argv is every whitespace delimited string in the command line including the 
	//program name. 
	std::string args_stdString;
	MarshalString(args, args_stdString);
	TRACE::StackTrace::Main( args_stdString );
	return "Hello... ";
}


