/* FILENAME: OptionParser.cpp
 * $Header: //bctquad3/home/BCT_Development/vxWorks/Common/cmdline/rcs/optionparser.cpp 1.4 2002/09/20 21:35:48Z td07711 Exp ms10234 $
 * PURPOSE: option parsing class
 * CHANGELOG:
 *   $Log: optionparser.cpp $
 *   Revision 1.3  2002/09/19 21:06:02  td07711
 *   errmsg fix
 *   Revision 1.2  2002/09/19 20:13:13  td07711
 *   add public init() variants
 *   Revision 1.1  2002/09/18 23:33:07  td07711
 *   Initial revision
 *   Revision 6.2  2002/05/01 18:04:54  td07711
 *   vxworks port
 *   05/01/2002 - dyes - update vxworks constructor
 *   Revision 6.1  2002/04/30 22:17:05  td07711
 *   porting to vxworks
 *   04/30/2002 - dyes - added vxworks compatible constructor
 *   Revision 1.6  2000/08/10 20:53:56  td07711
 *     fix off by 1 argv parsing bug
 *   Revision 1.5  2000/05/18 21:43:10  MS10234
 *   Changed case on filenames to be all lower case
 *   Revision 1.4  2000/03/09 23:38:46  TD07711
 *   IT4580
 *   Revision 1.3  2000/01/13 21:22:46  TD07711
 *   01/10/2000 - dyes - add positional option parsing
 *   09/01/99 - dyes - move usage text to Logger
 *   05/25/99 - dyes - initial version
 */

#include <string.h>
#include <stdlib.h>

#include "optionparser.h"

#include "stdio.h"
#include "assert.h"

#define ASSERT(n) assert(n)



OptionParser::OptionParser(const char* programName, const char* comment)
: _usage(programName, comment), 
  _argc(1), _argv(0), _options(0)
{
}


OptionParser::~OptionParser()
{
    delete [] _argv;
    delete [] _options;
    _argv = 0;
    _options = 0;
}


void OptionParser::init(int argc, const char** argv)
{
    // PURPOSE: takes unix/nt style argc argv command line arguments

    ASSERT(argc);
    ASSERT(argv);

    // copy argc and argv
    _argv = new const char*[argc+1]; // BOGOSITY - +1 needed by vc++ 
    ASSERT(_argv);

    for(int i = 0; i < argc; i++)
        _argv[i] = argv[i];

    // null terminate argv since we have an unecessary but undefined ptr at the end
    _argv[argc] = 0;
}


void OptionParser::init(const char* options)
{
    // PURPOSE: take single string containing space delimited words and 
    // put them into argv[] and argc, accessible via getArgv() and getArgc()

    ASSERT(options);

    // 
    // count the args and allocate _argv 
    //
    bool looking_for_arg;
    const char* p;
    for(p = options, looking_for_arg = true; *p != 0 ; p++)
    {
        if(looking_for_arg)
        {
            if(*p != ' ')
            {
                _argc++;
                looking_for_arg = false;
                continue;
            }
        }
        else
        {
            if(*p == ' ')
            {
                looking_for_arg = true;
                continue;
            }
        }
    }
    _argv = new const char*[_argc+1];
    ASSERT(_argv);

    _argv[0] = _usage.getProgramName(); // dummy task/process name


    // make copy of options that strtok_r can modify
    // this also serves as storage for the args
    _options = new char[strlen(options)+1];
    ASSERT(_options);
    strcpy(_options, options);


    // use strtok to find and null terminate each arg,
    // and set pointer to each arg in _argv
    char * pLast = 0; // next place to start strtok scan 
    char* token = strtok_r(_options, " ", &pLast);
    int i;
    for(i = 1; token != NULL; i++)
    {
        ASSERT(i <= _argc);
        _argv[i] = token;
        token = strtok_r(NULL, " ", &pLast);
    }
    ASSERT(i == _argc);
    _argv[i] = 0; // NULL terminate
}


void OptionParser::done()
{
    // PURPOSE: error exit if any unparsed items remain

    if (_argc > 1)
    {
        for(int i = 1; i < _argc; i++)
        {  
            fprintf(stderr, "OptionParser: unparsed token %s\n", _argv[i]);
        }

        fprintf(stderr, "%s", _usage.get_usage() );
        exit(1);
    }
}


void OptionParser::parse(const char* keyword, const char* usage, bool* pStorage)
{
    // PURPOSE: parses boolean keyword flag

    ASSERT(keyword);
    ASSERT(pStorage);
    ASSERT(usage);

    char* buf = new char[strlen(keyword)+strlen(usage)+10];
    ASSERT(buf);
    sprintf(buf, "%s - %s", keyword, usage);
    _usage.add_usage(buf);
    delete [] buf;
    *pStorage = false; // flags always initialized false

    for(int i=1; i < _argc; i++)
    {
        if(strcmp(keyword, _argv[i]) == 0)
        {
            *pStorage = true;
            remove_token(i);
            return;
        }
    }
}


void OptionParser::parse(const char* keyword, const char* usage, int* pStorage, 
                         int def, int min, int max)
{
    // PURPOSE: parses keyword value, int data

    ASSERT(keyword);
    ASSERT(pStorage);
    ASSERT(usage);

    char* buf = new char[strlen(keyword)+strlen(usage)+44];
    ASSERT(buf);
    sprintf(buf, "%s <int> - %s (%d-%d), def=%d", keyword, usage, min, max, def);
    _usage.add_usage(buf);
    delete [] buf;
    *pStorage = def;

    for(int i=1; i < _argc; i++)
    {
        if(strcmp(keyword, _argv[i]) == 0)
        {
            if((i+1) >= _argc)
            {
                fprintf(stderr, "no value for keyword=%s", keyword);
                exit(1);
            }

            if(sscanf(_argv[i+1], "%i", pStorage) != 1)
            {
                fprintf(stderr, "scanf %%i failed, keyword=%s value=%s",
                          keyword, _argv[i+1]);
                exit(1);
            }

            if(*pStorage < min)
            {
                fprintf(stderr, "range check failed keyword=%s value=%i<%i",
                          keyword, *pStorage, min);
                exit(1);
            }

            if(*pStorage > max)
            {
                fprintf(stderr, "range check failed keyword=%s value=%i>%i",
                          keyword, *pStorage, max);
                exit(1);
            }

            remove_token(i+1);
            remove_token(i);
            return;
        }
    }
}


void OptionParser::parse(const char* usage, int* pStorage, 
                         int def, int min, int max)
{
    // PURPOSE: parses positional int data

    ASSERT(pStorage);
    ASSERT(usage);

    char* buf = new char[strlen(usage)+44];
    ASSERT(buf);
    sprintf(buf, "<int> - %s (%d-%d), def=%d", usage, min, max, def);
    _usage.add_usage(buf);
    delete [] buf;
    *pStorage = def;

    if(_argc <= 1)
    {
        return;
    }

    if(sscanf(_argv[1], "%i", pStorage) != 1)
    {
        fprintf(stderr, "scanf %%i failed, usage=%s value=%s",
                  usage, _argv[1]);
        exit(1);
    }

    if(*pStorage < min)
    {
        fprintf(stderr, "range check failed usage=%s value=%i<%i",
                  usage, *pStorage, min);
        exit(1);
    }

    if(*pStorage > max)
    {
        fprintf(stderr, "range check failed usage=%s value=%i>%i",
                  usage, *pStorage, max);
        exit(1);
    }

    remove_token(1);
}


void OptionParser::parse(const char* keyword, const char* usage, float* pStorage, 
                         float def, float min, float max)
{
    // PURPOSE: parses keyword value, float data

    ASSERT(keyword);
    ASSERT(pStorage);
    ASSERT(usage);

    // allow room floating point expansion, i.e. 1 may be displayed as 1.000000
    char* buf = new char[strlen(keyword)+strlen(usage)+60];
    ASSERT(buf);
    sprintf(buf, "%s <float> - %s (%f-%f), def=%f", keyword, usage, min, max, def);
    _usage.add_usage(buf);
    delete [] buf;
    *pStorage = def;

    for(int i=1; i < _argc; i++)
    {
        if(strcmp(keyword, _argv[i]) == 0)
        {
            if((i+1) >= _argc)
            {
                fprintf(stderr, "no value for keyword=%s", keyword);
                exit(1);
            }

            if(sscanf(_argv[i+1], "%f", pStorage) != 1)
            {
                fprintf(stderr, "scanf %%f failed, keyword=%s value=%s",
                          keyword, _argv[i+1]);
                exit(1);
            }

            if(*pStorage < min)
            {
                fprintf(stderr, "range check failed keyword=%s value=%f<%f",
                          keyword, *pStorage, min);
                exit(1);
            }

            if(*pStorage > max)
            {
                fprintf(stderr, "range check failed keyword=%s value=%f>%f",
                          keyword, *pStorage, max);
                exit(1);
            }

            remove_token(i+1);
            remove_token(i);
            return;
        }
    }
}


void OptionParser::parse(const char* usage, float* pStorage, 
                         float def, float min, float max)
{
    // PURPOSE: parses positional (no keyword) float data

    ASSERT(pStorage);
    ASSERT(usage);

    char* buf = new char[strlen(usage)+44];
    ASSERT(buf);
    sprintf(buf, "<float> - %s (%f-%f), def=%f", usage, min, max, def);
    _usage.add_usage(buf);
    delete [] buf;
    *pStorage = def;

    if(_argc <= 1)
    {
        return;
    }

    if(sscanf(_argv[1], "%f", pStorage) != 1)
    {
        fprintf(stderr, "scanf %%f failed, usage=%s value=%s",
                  usage, _argv[1]);
        exit(1);
    }

    if(*pStorage < min)
    {
        fprintf(stderr, "range check failed usage=%s value=%f<%f",
                  usage, *pStorage, min);
        exit(1);
    }

    if(*pStorage > max)
    {
        fprintf(stderr, "range check failed usage=%s value=%f>%f",
                  usage, *pStorage, max);
        exit(1);
    }

    remove_token(1);
}


void OptionParser::parse(const char* keyword, const char* usage, double* pStorage, 
                         double def, double min, double max)
{
    // PURPOSE: parses keyword value, double data

    ASSERT(keyword);
    ASSERT(pStorage);
    ASSERT(usage);

    char* buf = new char[strlen(keyword)+strlen(usage)+44];
    ASSERT(buf);
    sprintf(buf, "%s <double> - %s (%f-%f), def=%f", keyword, usage, min, max, def);
    _usage.add_usage(buf);
    delete [] buf;
    *pStorage = def;
    
    for(int i=1; i < _argc; i++)
    {
        if(strcmp(keyword, _argv[i]) == 0)
        {
            if((i+1) >= _argc)
            {
                fprintf(stderr, "no value for keyword=%s", keyword);
                exit(1);
            }

            if(sscanf(_argv[i+1], "%lf", pStorage) != 1)
            {
                fprintf(stderr, "scanf %%lf failed, keyword=%s value=%s",
                          keyword, _argv[i+1]);
                exit(1);
            }

            if(*pStorage < min)
            {
                fprintf(stderr, "range check failed keyword=%s value=%f<%f",
                          keyword, *pStorage, min);
                exit(1);
            }

            if(*pStorage > max)
            {
                fprintf(stderr, "range check failed keyword=%s value=%f>%f",
                          keyword, *pStorage, max);
                exit(1);
            }

            remove_token(i+1);
            remove_token(i);
            return;
        }
    }
}


void OptionParser::parse(const char* usage, double* pStorage, 
                         double def, double min, double max)
{
    // PURPOSE: parses positional (i.e. no keyword) double data

    ASSERT(pStorage);
    ASSERT(usage);

    char* buf = new char[strlen(usage)+44];
    ASSERT(buf);
    sprintf(buf, "<double> - %s (%f-%f), def=%f", usage, min, max, def);
    _usage.add_usage(buf);
    delete [] buf;
    *pStorage = def;
    
    if(_argc <= 1)
    {
        return;
    }

    if(sscanf(_argv[1], "%lf", pStorage) != 1)
    {
        fprintf(stderr, "scanf %%lf failed, usage=%s value=%s",
                  usage, _argv[1]);
        exit(1);
    }

    if(*pStorage < min)
    {
        fprintf(stderr, "range check failed usage=%s value=%f<%f",
                  usage, *pStorage, min);
        exit(1);
    }

    if(*pStorage > max)
    {
        fprintf(stderr, "range check failed usage=%s value=%f>%f",
                  usage, *pStorage, max);
        exit(1);
    }

    remove_token(1);
}


void OptionParser::parse(const char* keyword, const char* usage, 
                         const char** pStorage, const char* def)
{
    // PURPOSE: parses keyword value, string data

    ASSERT(keyword);
    ASSERT(pStorage);
    ASSERT(usage);

    char* buf;
    if(def) {
        buf = new char[strlen(keyword)+strlen(usage)+strlen(def)+20];
        ASSERT(buf);
        sprintf(buf, "%s <string> - %s, def=%s", keyword, usage, def);
    }
    else {
        buf = new char[strlen(keyword)+strlen(usage)+30];
        ASSERT(buf);
        sprintf(buf, "%s <string> - %s, def=NULL", keyword, usage);
    }
    _usage.add_usage(buf);
    delete [] buf;
    *pStorage = def;

    for(int i=1; i < _argc; i++)
    {
        if(strcmp(keyword, _argv[i]) == 0)
        {
            if((i+1) >= _argc)
            {
                fprintf(stderr, "no value for keyword=%s", keyword);
                exit(1);
            }

            *pStorage = _argv[i+1];

            remove_token(i+1);
            remove_token(i);
            return;
        }
    }
}


void OptionParser::parse(const char* usage, const char** pStorage, const char* def)
{
    // PURPOSE: parses positional (i.e. no keyword) string data

    ASSERT(pStorage);
    ASSERT(usage);

    char* buf;
    if(def) {
        buf = new char[strlen(usage)+strlen(def)+20];
        ASSERT(buf);
        sprintf(buf, "<string> - %s, def=%s", usage, def);
    }
    else {
        buf = new char[strlen(usage)+30];
        ASSERT(buf);
        sprintf(buf, "<string> - %s, def=NULL", usage);
    }
    _usage.add_usage(buf);
    delete [] buf;
    *pStorage = def;

    if(_argc <= 1)
    {
        return;
    }

    *pStorage = _argv[1];

    remove_token(1);
}


void OptionParser::remove_token(int i)
{
    // PURPOSE: removes parsed token from the command line

    ASSERT(i > 0);
    ASSERT(i <= _argc);
    
    for(; i<_argc; i++)
        _argv[i] = _argv[i+1];

    _argv[i] = 0;
    _argc--;
}


