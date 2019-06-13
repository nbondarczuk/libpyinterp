#ifndef _STRUTL_H_
#define _STRUTL_H_

#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include <stdarg.h>
#include <fnmatch.h>
#include <stdio.h>

namespace strutl
{
typedef std::string Token;
typedef std::vector<std::string> Tokens;
typedef std::vector<std::string>::iterator TokensIterator;
typedef std::vector<std::string>::const_iterator ConstTokensIterator;

template <typename T> std::vector<T> split(const T& str, const T& delimiters)
{
    std::vector<T> rv;
    typename T::size_type start = 0;
    typename T::size_type pos = str.find(delimiters, start);
    while (pos != T::npos) {
        if (pos != start) {
            rv.push_back(str.substr(start, pos - start));
        }

        start = pos + 1;
        pos = str.find_first_of(delimiters, start);
    }

    if (start < str.length()) {
        rv.push_back(str.substr(start, str.length() - start));
    }

    return rv;
}

template <typename T> bool match(const T& value, const T& pattern)
{
    return 0 == fnmatch(pattern.c_str(), value.c_str(), 0);
}

inline std::string& ltrim(std::string& str, const std::string& chars = "\t\n\v\f\r ")
{
    str.erase(0, str.find_first_not_of(chars));
    return str;
}

inline std::string& rtrim(std::string& str, const std::string& chars = "\t\n\v\f\r ")
{
    str.erase(str.find_last_not_of(chars) + 1);
    return str;
}

inline std::string& trim(std::string& str, const std::string& chars = "\t\n\v\f\r ")
{
    return ltrim(rtrim(str, chars), chars);
}

#define MAX_FORMAT_SIZE 256

inline std::string format(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    char buf[MAX_FORMAT_SIZE];
    int n = snprintf(buf, MAX_FORMAT_SIZE, fmt, args);
    if (n > MAX_FORMAT_SIZE) {
        throw std::runtime_error("Size of format string too big");
    }
    va_end(args);

    return std::string(buf);
}

}

#endif /* _STRUTL_H_ */
