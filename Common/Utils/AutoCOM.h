/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

#pragma once
#include <D3DHeaders.h>

namespace Utils
{

template<class TVar>
class AutoCOM
{
private:
    TVar *data;
public:
    AutoCOM &operator= (const AutoCOM &) = delete;
    AutoCOM(const AutoCOM &) = delete;
    AutoCOM(TVar *Data){data = Data;}
    ~AutoCOM() { ReleaseCOM(data);}
    TVar *operator -> () {return data;}
    operator TVar *(){return data;}
    TVar *Get(){return data;}
};

}