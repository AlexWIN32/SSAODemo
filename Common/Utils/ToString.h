/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

#pragma once
#include <string>
#include <sstream>
namespace Utils
{

template<class T>
static std::string to_string(const T &Var)
{
    std::ostringstream sstrm;
    sstrm << Var;
    return sstrm.str();
}

template<class T>
static std::wstring to_wstring(const T &Var)
{
    std::wstringstream sstrm;
    sstrm << Var;
    return sstrm.str();
}

}