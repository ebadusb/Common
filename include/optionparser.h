/* FILENAME: OptionParser.hpp
 * $Header: //bctquad3/home/BCT_Development/vxWorks/Common/include/rcs/optionparser.h 1.2 2002/09/19 20:10:32Z td07711 Exp ms10234 $
 * PURPOSE: used to parse options from command line or elsewhere
 * CHANGELOG:
 *   $Log: optionparser.h $
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



// forward declarations
class Usage;


// CLASSNAME: OptionParser
// RESPONSIBILITIES:
//   1. Used to parse argc, argv style command line
//   2. makes a local copy of the command line upon construction
//   3. provides a parse function whose signature defines keyword being
//      looked for, and the location and data type of where it's value
//      is to be stored.
//   4. parses keyword value pairs where value can be int, float, or char* 
//   5. parses keyword flags as a bool
//   6. provides a done function that checks for any unparsed items remaining
//   7. errors result in an error message and exit
//   8. performs range checking on int and float values
//   9. does not allow copying or assignment
// COLLABORATIONS:
//   1. Uses Logging macros defined in Logger.hpp
class OptionParser
{
    public:

        OptionParser(int argc, char** argv, Usage& usage); // unix and nt compatible
        OptionParser(const char* options, Usage& usage);  // vxworks compatible
        ~OptionParser();

        // keyword/value options
        void parse(char* keyword, char* comment,
                   int* pStorage, int def, int min, int max); // int value
        void parse(char* keyword, char* comment,
                   float* pStorage, float def, float min, float max);
        void parse(char* keyword, char* comment,
                   double* pStorage, double def, double min, double max);
        void parse(char* keyword, char* comment, char** pStorage, char* def); // string value
        void parse(char* keyword, char* comment, bool* pStorage); // parses keyword flag
        
        // positional options
        void parse(char* comment,
                   int* pStorage, int def, int min, int max); // int value
        void parse(char* comment,
                   float* pStorage, float def, float min, float max);
        void parse(char* comment,
                   double* pStorage, double def, double min, double max);
        void parse(char* comment, char** pStorage, char* def); // string value
        
        void done(); // checks for trash remaining on command line

    protected:

        int _argc;
        char** _argv;
        void remove_token(int i);
        Usage& _usage;
        char* _options; // storage for option args

    private:

        OptionParser(); // catch unauthorized use
        OptionParser(const OptionParser&); // catch unauthorized use 
        OptionParser& operator=(const OptionParser& rhs); // catch unauthorized use
};

#endif // OptionParser_HPP
