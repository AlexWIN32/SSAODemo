/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

#pragma once 
#include <vector>

namespace Utils
{

template<class TVar>
void RemoveFromVector(std::vector<TVar> &Data, const TVar &Value)
{
    typename std::vector<TVar>::iterator it;
    for(it = Data.begin(); it != Data.end(); ++it)
        if(*it == Value){
            Data.erase(it);
            return;
        }
}

template<class TVar, class Func>
void RemoveFromVector(std::vector<TVar> &Data, const TVar &Value, Func Function)
{
    typename std::vector<TVar>::iterator it;
    for(it = Data.begin(); it != Data.end(); ++it)
        if(*it == Value){
            Function(*it);
            Data.erase(it);
            return;
        }
}

}