/* FILENAME: OptionParser.hpp
 * $Header: //bctquad3/home/BCT_Development/vxWorks/Common/include/rcs/optionparser.h 1.2 2002/09/19 20:10:32Z td07711 Exp ms10234 $
 * PURPOSE: used to parse options from command line or elsewhere
 * CHANGELOG:
 *   $Log: optionparser.h $
 *   Revision 1.1  2002/09/18 23:31:10  td07711
 *   Initial revision
 *   Revision 6.2  2002/05/01 18:04:44  td07711
 *   vxworks port
 *   Revision 6.1  2002/04/30 22:17:22  td07711
 *   porting to vxworks
 *   04/30/2002 - dyes - add vxworks compatible constructor and _options storage
 *   Revision 1.3  2000/05/18 21:43:11  MS10234
 *   Changed case on filenames to be all lower case
 *   Revision 1.2  2000/03/06 19:38:37  TD07711
 *   IT4580
 *   Revision 1.2  2000/01/13 21:22:50  TD07711
 *   01/11/2000 - dyes - add positional option parsing
 *   09/01/99 - dyes - move usage text to Logger
 *   05/25/99 - dyes - initial version
 */

#ifndef OptionParser_HPP // prevents multiple inclusion
#define OptionParser_HPP

#include "usage.h"


// CLASSNAME: OptionParser
// RESPONSIBILITIES:
//   1. Used to parse argc, argv style command line,
//      also can split a single string of space delimited tokens into argc argv
//   2. makes a local copy of the command line upon construction
//   3. provides a parse function whose signature defines keyword being
//      looked for, and the location and data type of where it's value
//      is to be stored.
//   4. parses keyword value pairs where value can be int, float, or char* 
//   5. parses keyword flags as a bool
//   6. provides a done function that checks for any unparsed items remaining
//   7. errors result in an error message and exit
//   8. performs range checking on int and float values
//   9. assigns default values to option variables
//   10. builds a usage error message, displayed if usage error detected
// COLLABORATIONS:
//   1. contains a Usage class to hold usage info
class OptionParser
{
    public:

        OptionParser(const char* programName, const char* comment);
        ~OptionParser();

        void init(int argc, const char** argv); // unix and nt 
        void init(const char* options); // vxworks

        // 
        // getArgv() and getArgc() return argc argv containing unparsed data,
        // parse() functions remove parsed data from argc argv
        //
        const char** getArgv() { return _argv; };
        int getArgc() { return _argc; };

        void done(); // usage error and exit if unparsed items remaining
   
        //
        // parse functions for keyword and keyword value pairs
        //

        // int
        void parse(const char* keyword, const char* comment, int* pStorage, 
                   int def, int min, int max); // int value
        
        // float
        void parse(const char* keyword, const char* comment, float* pStorage, 
                   float def, float min, float max);

        // double float
        void parse(const char* keyword, const char* comment, double* pStorage, 
                   double def, double min, double max);
        
        // char* string
        void parse(const char* keyword, const char* comment, const char** pStorage, 
                   const char* def);

        // bool keyword flag
        void parse(const char* keyword, const char* comment, bool* pStorage); 
        
        //
        // parse functions for positional (i.e. no keyword) data
        //
        
        // int
        void parse(const char* comment, int* pStorage, 
                   int def, int min, int max);

        // float
        void parse(const char* comment, float* pStorage, 
                   float def, float min, float max);

        // double
        void parse(const char* comment, double* pStorage, 
                   double def, double min, double max);

        // char* string
        void parse(const char* comment, const char** pStorage, const char* def);
        

    protected:


    private:

        int _argc;
        const char** _argv;
        void remove_token(int i);
        Usage _usage;
        char* _options; // storage for option args

        OptionParser(); // catch unauthorized use
        OptionParser(const OptionParser&); // catch unauthorized use 
        OptionParser& operator=(const OptionParser& rhs); // catch unauthorized use
};

#endif // OptionParser_HPP
