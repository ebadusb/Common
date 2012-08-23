/*****************************************************************************\
**
** AppParamUtil.h
**
** Utility classes for application parameters.
**
\*****************************************************************************/

#ifndef APPPARAMUTIL_H
#define APPPARAMUTIL_H

/* Includes ******************************************************************/

#include <algorithm>
#include <iostream>
#include <list>
#include <set>
#include <map>

namespace DECODER {
namespace UTIL {

/* Types *********************************************************************/

typedef std::multimap<std::string, std::string> Arguments;

/* Classes *******************************************************************/

class AppParamUtil {
public:
	static void Parse(Arguments &params, char* arg[], int num)
	{
		for (int i = 0; i < num; ++i) {
			if (((i != num - 1) && *arg[i] == '-') && (*arg[i + 1] != '-')) {
				params.insert(std::pair<std::string, std::string>(arg[i - 1], arg[++i]));
			}
			else if (*arg[i] == '-') {
				params.insert(std::pair<std::string, std::string>(arg[i], ""));
			}
			else {
				params.insert(std::pair<std::string, std::string>("", arg[i]));
			}
		}
	}

	static bool Exist(Arguments &params, const char *arg)
	{
		return params.find(arg) != params.end();
	}

	static int Count(Arguments &params, const char *arg)
	{
		return params.count(arg);
	}

	static const char* Get(Arguments &params, const char *arg, int index = 0)
	{
		if (index < params.count(arg)) {
			std::multimap<std::string, std::string>::const_iterator iter = params.lower_bound(arg);

			for (int i = 0; i < index; ++i) {
				++iter;
			}

			return iter->second.c_str();
		}

		return NULL;
	}

	static Arguments::iterator Begin(Arguments &params, const char *arg)
	{
		return params.lower_bound(arg);
	}

	static Arguments::iterator End(Arguments &params, const char *arg)
	{
		return params.upper_bound(arg);
	}

	static void Extract(Arguments &params, const char *arg, std::vector<std::string> &vector)
	{
		Arguments::iterator iter = params.lower_bound(arg);
		Arguments::iterator end = params.upper_bound(arg);

		for (; iter != end; ++iter) {
			vector.push_back(iter->second);
		}
	}
};

} // namespace UTIL
} // namespace DECODER

#endif // APPPARAMUTIL_H

