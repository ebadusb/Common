#include <cxxtest/XUnitPrinter.h>
#include <iostream>
#include <fstream>
#include <string>
#include <time.h>
using namespace std;
#define MAX_FILENAME_LN 100
int main()
{
      char testResultsName[MAX_FILENAME_LN], logFileName[MAX_FILENAME_LN], xmlName[MAX_FILENAME_LN];
      
      // Need a timestamp for filenames
      time_t now = time(NULL);
      
      if(now != -1) // did we get the time?
      {
         struct tm * timeinfo = localtime(&now);
         strftime(testResultsName, MAX_FILENAME_LN, "TestResults_%H_%M_%S_%d%m%Y.log", timeinfo);
         strftime(logFileName, MAX_FILENAME_LN, "LogFile_%H_%M_%S_%d%m%Y.log", timeinfo);
         strftime(logFileName, MAX_FILENAME_LN, "LogFile_%H_%M_%S_%d%m%Y.log", timeinfo);
         strftime(xmlName, MAX_FILENAME_LN, "_%H_%M_%S_%d%m%Y.log", timeinfo);
         sprintf(xmlName, "%s%s", __FILE__, xmlName);
      }
      else // Didn't get the time, use generic non-unique names
      {
         sprintf(testResultsName, "TestResults.log");
         sprintf(logFileName, "LogFile.log");
      }
      
      // Redirect cout to a file
      ofstream testResults(testResultsName);
      cout.rdbuf(testResults.rdbuf());
      
      // Optia components may write to clog or cerr. Map them to a separate file.
      ofstream logFile(logFileName);
      clog.rdbuf(logFile.rdbuf());
      cerr.rdbuf(logFile.rdbuf());

      // Run the tests.
      std::ofstream ofstr("ButtonMonitorTests.xml");
      CxxTest::XUnitPrinter tmp(ofstr);
      CxxTest::RealWorldDescription::_worldName = "cxxtest";
      status = CxxTest::Main<CxxTest::XUnitPrinter>( tmp, argc, argv );
      return status;
}

// The CxxTest "world"
<CxxTest world>