/* FILENAME: OptionParser.cpp
 * $Header: //bctquad3/home/BCT_Development/vxWorks/Common/cmdline/rcs/optionparser.cpp 1.4 2002/09/20 21:35:48Z td07711 Exp ms10234 $
 * PURPOSE: option parsing class
 * CHANGELOG:
 *   $Log: optionparser.cpp $
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
#include "usage.h"

#include "stdio.h"
#include "assert.h"

#define LOG_FATAL printf
#define ASSERT assert


// FUNCTION: OptionParser
// PURPOSE: constructs a OptionParser instance.
// 1. makes local copies of argc and argv
OptionParser::OptionParser(int argc, char** argv, Usage& usage)
: _argc(argc), _usage(usage), _options(0)
{
    ASSERT(argc);
    ASSERT(argv);

    // copy argc and argv
    _argv = new char*[argc+1]; // BOGOSITY - +1 needed by vc++ 
    ASSERT(_argv);
    for(int i=0; i<argc; i++)
        _argv[i] = argv[i];


//    LOG_DEBUG("OptionParser constructed, argc=%d", argc);
}


// FUNCTION: OptionParser
// PURPOSE: constructs a OptionParser instance.
// 1. creates argc and argv from options string
// 2. allocates storage for argv strings
OptionParser::OptionParser(const char* options, Usage& usage)
: _usage(usage), _options(0), _argc(1)
{
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
    _argv = new char*[_argc+1];
    ASSERT(_argv);
//    LOG_DEBUG("argc=%d", _argc);

    _argv[0] = "dummy"; // dummy task/process name



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


OptionParser::~OptionParser()
{
    delete [] _argv;
    delete [] _options;
}


// FUNCTION: done
// PURPOSE: error exit if any unparsed items remain
void OptionParser::done()
{
    if (_argc > 1)
    {
        for(int i=1; i < _argc; i++)
            fprintf(stderr, "unparsed token %s", _argv[i]);
        printf("%s", _usage.get_usage() );
        exit(1);
    }
}


// FUNCTION: parse
// PURPOSE: parses keyword flag
void OptionParser::parse(char* keyword, char* usage, bool* pStorage)
{
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
//            LOG_DEBUG("keyword=%s flag=true", keyword);
            *pStorage = true;
            remove_token(i);
            return;
        }
    }
//    LOG_DEBUG("keyword=%s not found", keyword);
}



// FUNCTION: parse
// PURPOSE: parses keyword value, int data
void OptionParser::parse(char* keyword, char* usage, 
                         int* pStorage, int def, int min, int max)
{
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
                LOG_FATAL("no value for keyword=%s", keyword);

            if(sscanf(_argv[i+1], "%i", pStorage) != 1)
                LOG_FATAL("scanf %%i failed, keyword=%s value=%s",
                          keyword, _argv[i+1]);

            if(*pStorage < min)
                LOG_FATAL("range check failed keyword=%s value=%i<%i",
                          keyword, *pStorage, min);

            if(*pStorage > max)
                LOG_FATAL("range check failed keyword=%s value=%i>%i",
                          keyword, *pStorage, max);

//            LOG_DEBUG("keyword=%s value=%i", keyword, *pStorage);
            remove_token(i+1);
            remove_token(i);
            return;
        }
    }
//    LOG_DEBUG("keyword=%s not found", keyword);
}



// FUNCTION: parse
// PURPOSE: parses positional int data
void OptionParser::parse(char* usage, 
                         int* pStorage, int def, int min, int max)
{
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
//        LOG_DEBUG("no value for usage=%s", usage);
        return;
    }

    if(sscanf(_argv[1], "%i", pStorage) != 1)
        LOG_FATAL("scanf %%i failed, usage=%s value=%s",
                  usage, _argv[1]);

    if(*pStorage < min)
        LOG_FATAL("range check failed usage=%s value=%i<%i",
                  usage, *pStorage, min);

    if(*pStorage > max)
        LOG_FATAL("range check failed usage=%s value=%i>%i",
                  usage, *pStorage, max);

//    LOG_DEBUG("usage=%s value=%i", usage, *pStorage);
    remove_token(1);
}



// FUNCTION: parse
// PURPOSE: parses keyword value, float data
void OptionParser::parse(char* keyword, char* usage,
                         float* pStorage, float def, float min, float max)
{
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
                LOG_FATAL("no value for keyword=%s", keyword);

            if(sscanf(_argv[i+1], "%f", pStorage) != 1)
                LOG_FATAL("scanf %%f failed, keyword=%s value=%s",
                          keyword, _argv[i+1]);

            if(*pStorage < min)
                LOG_FATAL("range check failed keyword=%s value=%f<%f",
                          keyword, *pStorage, min);

            if(*pStorage > max)
                LOG_FATAL("range check failed keyword=%s value=%f>%f",
                          keyword, *pStorage, max);

//            LOG_DEBUG("keyword=%s value=%f", keyword, *pStorage);
            remove_token(i+1);
            remove_token(i);
            return;
        }
    }
//    LOG_DEBUG("keyword=%s not found", keyword);
}


// FUNCTION: parse
// PURPOSE: parses positional float data
void OptionParser::parse(char* usage,
                         float* pStorage, float def, float min, float max)
{
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
//        LOG_DEBUG("no value for usage=%s", usage);
        return;
    }

    if(sscanf(_argv[1], "%f", pStorage) != 1)
        LOG_FATAL("scanf %%f failed, usage=%s value=%s",
                  usage, _argv[1]);

    if(*pStorage < min)
        LOG_FATAL("range check failed usage=%s value=%f<%f",
                  usage, *pStorage, min);

    if(*pStorage > max)
        LOG_FATAL("range check failed usage=%s value=%f>%f",
                  usage, *pStorage, max);

//    LOG_DEBUG("usage=%s value=%f", usage, *pStorage);
    remove_token(1);
}


// FUNCTION: parse
// PURPOSE: parses keyword value, double data
void OptionParser::parse(char* keyword, char* usage,
                         double* pStorage, double def, double min, double max)
{
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
                LOG_FATAL("no value for keyword=%s", keyword);

            if(sscanf(_argv[i+1], "%lf", pStorage) != 1)
                LOG_FATAL("scanf %%lf failed, keyword=%s value=%s",
                          keyword, _argv[i+1]);

            if(*pStorage < min)
                LOG_FATAL("range check failed keyword=%s value=%f<%f",
                          keyword, *pStorage, min);

            if(*pStorage > max)
                LOG_FATAL("range check failed keyword=%s value=%f>%f",
                          keyword, *pStorage, max);

//            LOG_DEBUG("keyword=%s value=%lf", keyword, *pStorage);
            remove_token(i+1);
            remove_token(i);
            return;
        }
    }
//    LOG_DEBUG("OptionParser::parse: keyword=%s not found", keyword);
}


// FUNCTION: parse
// PURPOSE: parses positional double data
void OptionParser::parse(char* usage,
                         double* pStorage, double def, double min, double max)
{
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
//        LOG_DEBUG("no value for usage=%s", usage);
        return;
    }

    if(sscanf(_argv[1], "%lf", pStorage) != 1)
        LOG_FATAL("scanf %%lf failed, usage=%s value=%s",
                  usage, _argv[1]);

    if(*pStorage < min)
        LOG_FATAL("range check failed usage=%s value=%f<%f",
                  usage, *pStorage, min);

    if(*pStorage > max)
        LOG_FATAL("range check failed usage=%s value=%f>%f",
                  usage, *pStorage, max);

//    LOG_DEBUG("usage=%s value=%lf", usage, *pStorage);
    remove_token(1);
}


// FUNCTION: parse
// PURPOSE: parses keyword value, string data
void OptionParser::parse(char* keyword, char* usage, char** pStorage, char* def)
{
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
                LOG_FATAL("no value for keyword=%s", keyword);

            *pStorage = _argv[i+1];

//            LOG_DEBUG("keyword=%s value=%s", keyword, *pStorage);
            remove_token(i+1);
            remove_token(i);
            return;
        }
    }
//    LOG_DEBUG("keyword=%s not found", keyword);
}


// FUNCTION: parse
// PURPOSE: parses positional string data
void OptionParser::parse(char* usage, char** pStorage, char* def)
{
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
//        LOG_DEBUG("no value for usage=%s", usage);
        return;
    }

    *pStorage = _argv[1];

//    LOG_DEBUG("usage=%s value=%s", usage, *pStorage);
    remove_token(1);
}


// FUNCTION: remove_token
// PURPOSE: removes parsed token from the command line
void OptionParser::remove_token(int i)
{
    ASSERT(i > 0);
    ASSERT(i <= _argc);
    
    for(; i<_argc; i++)
        _argv[i] = _argv[i+1];

    _argv[i] = 0;
    _argc--;
}


