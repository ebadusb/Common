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
class Usage
{
    public:

        Usage();
        Usage(const char* programName, const char* comment);
        ~Usage();

        void add_usage(const char* usage);

        const char* get_usage() { return _usage_text; };

        const char* getProgramName() { return _programName; };


    protected:


    private:

        const char* _programName;
        const char* _comment; 
        char* _usage_text; // built by add_usage

        Usage(const Usage&); // catch unauthorized use 
        Usage& operator=(const Usage& rhs); // catch unauthorized use
};

#endif // Usage_HPP
