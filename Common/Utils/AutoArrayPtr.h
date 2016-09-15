/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

#pragma once 

namespace Utils
{

template<class TVar>
class AutoArrayPtr final
{
private:
    TVar *data;
public:
    AutoArrayPtr &operator= (const AutoArrayPtr &) = delete;
    AutoArrayPtr(const AutoArrayPtr &) = delete;
    explicit AutoArrayPtr(int Size){data = new TVar[Size];}
    ~AutoArrayPtr() { delete [] data;}
    operator TVar *(){return data;}
    TVar *Get(){return data;}
};

}