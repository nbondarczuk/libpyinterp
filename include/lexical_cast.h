#ifndef _LEXICAL_CAST_H_
#define _LEXICAL_CAST_H_

#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

//
// the most general version
//
template <typename Target, typename Source> Target lexical_cast(Source const arg)
{
    Target result;
    std::stringstream ss;

    if (!(ss << arg) || !(ss >> result) || !(ss >> std::ws).eof()) {
        throw std::runtime_error("lexical_cast<>() failed");
    }

    return result;
}

#endif /* _LEXICAL_CAST_H_ */
