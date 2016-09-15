/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

#pragma once
namespace Utils
{

template<class TVar, INT ArrSize>
class StaticArray
{
private:
    TVar data[ArrSize];
public:
    StaticArray(){}
    StaticArray(TVar Data[ArrSize])
    {
        memcpy(data, Data, sizeof(data));
    }
    TVar &operator [] (INT Index) {return data[Index];}
    const TVar &operator [] (INT Index) const {return data[Index];}
    operator TVar * const () const {return data;}
    operator TVar * () {return data;}
};

}