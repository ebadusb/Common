/* Copyright 1998 Cobe BCT, Inc.
 *
 * FILENAME: Usage.hpp
 * PURPOSE: Provides interfaces for building application's usage message.
 * CHANGELOG:
 *   11/04/99 - dyes - initial version, functionality pulled from Logger
 */

#ifndef Usage_HPP // prevents multiple inclusion
#define Usage_HPP



// CLASSNAME: Usage
// RESPONSIBILITIES:
//   1. accepts an overall comment about the application
//   2. accepts incremental additions to the usage message
// COLLABORATIONS:
//   1. queried by Logger for usage string.
//   2. usage info added by OptionParser
class Usage
{
    public:

        Usage(char* programName, char* comment);
        ~Usage();

        void add_usage(char* usage);
        const char* get_usage();

    protected:

        char* _comment; 
        char* _usage_text; // built by add_usage
        char* _full_usage; // built by get_usage()
        char* _programName;


    private:

        Usage(); // catch unauthorized use
        Usage(const Usage&); // catch unauthorized use 
        Usage& operator=(const Usage& rhs); // catch unauthorized use
};

#endif // Usage_HPP
